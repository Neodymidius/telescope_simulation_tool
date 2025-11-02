#!/usr/bin/env python3
"""
Ray History → GIF Animation
(x-axis scaling + exits at z=0 + sensor projection at z = Zs + optional accumulation
 + sensor histogram strip with configurable bins)

New in this version:
- Adds a sensor "row" visualization: a 1D histogram with (default) 2000 bins across x at z=z_sensor.
- Renders the main ray chart and the histogram as two separate figures, then stacks them vertically per frame.
- Histogram respects --accumulate (i.e., uses the same set of rays shown in that frame).
- Heat-style colormap is used for the sensor strip as requested.

CLI highlights:
  --hist_bins 2000        number of sensor bins ("pixels")
  --hist_height_px 40     pixel height of the sensor strip (visual thickness)
  --accumulate            build up rays over frames
  --show_planes           dashed z=0 and z=z_sensor lines on main chart

Coordinate assumptions:
- Coordinates are (x, y, z). Rays are effectively 2D in x–z.
- Exit is at z = 0 → (x_exit, 0, 0).
- Sensor is at z = Zs → (x_sensor, 0, Zs).
"""
import argparse
import csv
import math
import sys
from typing import List, Tuple, Optional

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg
import imageio.v2 as imageio


# Types
Pos = Tuple[float, float, float]
Dir = Tuple[float, float, float]
Seg = Tuple[Pos, Dir, int]


def parse_row_to_segments(row: str) -> List[Seg]:
    parts = row.strip().replace(',', ' ').split()
    if len(parts) < 1 + 9 + 7:
        return []
    try:
        nums = [float(p) for p in parts]
    except ValueError:
        return []
    start_idx = 1 + 9
    segs: List[Seg] = []
    i = start_idx
    while i + 6 < len(nums):
        px, py, pz, dx, dy, dz, wallf = nums[i:i+7]
        wall = int(round(wallf))
        if wall == 0:
            break
        segs.append(((px, py, pz), (dx, dy, dz), wall))
        i += 7
    return segs


def load_rays(path: str) -> List[List[Seg]]:
    rays: List[List[Seg]] = []
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            if not line.strip():
                continue
            segs = parse_row_to_segments(line)
            if segs:
                rays.append(segs)
    return rays


def gather_polylines_xz(rays: List[List[Seg]], x_scale: float = 2.0):
    polylines = []
    for segs in rays:
        xs, zs = [], []
        for pos, _dir, _wall in segs:
            x, _y, z = pos
            xs.append(x * x_scale)
            zs.append(z)
        polylines.append((np.array(xs), np.array(zs)))
    return polylines


def intersect_plane_z(pos: Pos, dire: Dir, z_plane: float) -> Optional[Tuple[float, float, float]]:
    (x, y, z) = pos
    (dx, dy, dz) = dire
    if abs(dz) < 1e-15:
        return None
    t = (z_plane - z) / dz
    if t < 0:
        return None
    return (x + t * dx, y + t * dy, z + t * dz)


def compute_exit_point_for_ray(segs: List[Seg]) -> Optional[Tuple[float, float, float]]:
    if not segs:
        return None
    pos, dire, _ = segs[-1]
    return intersect_plane_z(pos, dire, 0.0)


def compute_sensor_point_for_ray(segs: List[Seg], z_sensor: float) -> Optional[Tuple[float, float, float]]:
    if not segs:
        return None
    pos, dire, _ = segs[-1]
    return intersect_plane_z(pos, dire, z_sensor)


def compute_all_intersections(rays: List[List[Seg]], z_sensor: float):
    exits = []
    sensors = []
    for segs in rays:
        exits.append(compute_exit_point_for_ray(segs))
        sensors.append(compute_sensor_point_for_ray(segs, z_sensor))
    return exits, sensors


def compute_axis_limits(polylines, exits_scaled=None, sensors_scaled=None, pad_ratio=0.05):
    xs_list = [p[0] for p in polylines] if polylines else []
    zs_list = [p[1] for p in polylines] if polylines else []
    if exits_scaled:
        ex = np.array([e[0] for e in exits_scaled if e is not None])
        ez = np.zeros_like(ex)
        if ex.size:
            xs_list.append(ex)
            zs_list.append(ez)
    if sensors_scaled:
        sx = np.array([s[0] for s in sensors_scaled if s is not None])
        sz = np.array([s[2] for s in sensors_scaled if s is not None])
        if sx.size:
            xs_list.append(sx)
            zs_list.append(sz)
    if xs_list:
        all_x = np.concatenate(xs_list)
        all_z = np.concatenate(zs_list)
    else:
        all_x = np.array([0.0])
        all_z = np.array([0.0])
    xmin, xmax = float(np.min(all_x)), float(np.max(all_x))
    zmin, zmax = float(np.min(all_z)), float(np.max(all_z))
    dx = xmax - xmin
    dz = zmax - zmin
    if dx == 0: dx = 1.0
    if dz == 0: dz = 1.0
    xmin -= dx * pad_ratio
    xmax += dx * pad_ratio
    zmin -= dz * pad_ratio
    zmax += dz * pad_ratio
    return xmin, xmax, zmin, zmax


def fig_to_rgb_array(fig):
    canvas = FigureCanvasAgg(fig)
    canvas.draw()
    width, height = fig.canvas.get_width_height()
    buf = np.frombuffer(canvas.tostring_rgb(), dtype=np.uint8)
    img = buf.reshape((height, width, 3))
    plt.close(fig)
    return img


from matplotlib.backends.backend_agg import FigureCanvasAgg  # placed here for fig_to_rgb_array


def render_frames(polylines, exits_scaled, sensors_scaled, out_gif: str, dpi=150, frame_duration=0.1,
                  figsize=(6, 6), accumulate=False, show_planes=False, z_sensor=-300.0,
                  hist_bins=2000, hist_height_px=40):
    """
    Rendering pipeline per frame:
      1) Main chart (rays + exit/sensor marks) → figure A
      2) Sensor histogram strip (1D heat) → figure B
      3) Convert A and B to RGB arrays and vstack them → one frame
    """
    if not polylines:
        raise ValueError("No rays to plot. Check your input file.")

    xmin, xmax, zmin, zmax = compute_axis_limits(polylines, exits_scaled, sensors_scaled)

    # Precompute sensor x positions (scaled) list for quick selection per frame
    sensor_x_scaled = [pt[0] if pt is not None else None for pt in sensors_scaled]

    images = []
    N = len(polylines)
    for idx in range(1, N + 1):
        lo, hi = (idx, idx) if not accumulate else (1, idx)

        # === MAIN CHART (figure A) ===
        figA = plt.figure(figsize=figsize, dpi=dpi)
        axA = figA.add_subplot(111)

        for j in range(lo-1, hi):
            xs, zs = polylines[j]
            if len(xs) >= 2:
                axA.plot(xs, zs, marker='o', linewidth=1.5, markersize=3)
            else:
                axA.plot(xs, zs, marker='o', linewidth=0, markersize=3)

            ex = exits_scaled[j]
            if ex is not None:
                axA.plot([ex[0]], [0.0], marker='x', markersize=8, linewidth=0)
                axA.plot([xs[-1], ex[0]], [zs[-1], 0.0], linestyle='--', linewidth=1.0)

            sp = sensors_scaled[j]
            if sp is not None:
                axA.plot([sp[0]], [z_sensor], marker='s', markersize=6, linewidth=0)
                axA.plot([xs[-1], sp[0]], [zs[-1], z_sensor], linestyle='--', linewidth=1.0)

        if show_planes:
            axA.axhline(0.0, linestyle='--', linewidth=0.8)
            axA.axhline(z_sensor, linestyle='--', linewidth=0.8)

        axA.set_xlabel('x (scaled)')
        axA.set_ylabel('z')
        title_suffix = "accumulated" if accumulate else f'ray {idx}'
        axA.set_title(f'Rays in x–z plane ({title_suffix})')
        axA.set_aspect('equal', adjustable='box')
        axA.set_xlim(xmin, xmax)
        axA.set_ylim(zmin, zmax)
        axA.grid(True, linewidth=0.3, alpha=0.4)

        imgA = fig_to_rgb_array(figA)

        # === HISTOGRAM STRIP (figure B) ===
        # Collect current sensors for this frame
        curr_xs = [sensor_x_scaled[j] for j in range(lo-1, hi) if sensor_x_scaled[j] is not None]
        counts = None
        bin_edges = None
        if curr_xs:
            counts, bin_edges = np.histogram(curr_xs, bins=hist_bins, range=(xmin, xmax))
        else:
            counts = np.zeros(hist_bins, dtype=np.int64)
            bin_edges = np.linspace(xmin, xmax, hist_bins + 1)

        # Create a small figure with the same width (in pixels) and small height
        # Keep physical width consistent by using same figsize width and dpi; height adjusted by hist_height_px
        # Given width_px = figsize[0]*dpi, set height inches = hist_height_px/dpi
        width_in = figsize[0]
        height_in = hist_height_px / dpi
        figB = plt.figure(figsize=(width_in, height_in), dpi=dpi)
        axB = figB.add_subplot(111)

        # Build a 2D image: shape (1, bins) then imshow with extent matching (xmin,xmax)
        strip = counts[np.newaxis, :]
        axB.imshow(strip, aspect='auto', extent=(xmin, xmax, 0, 1), cmap='hot', origin='lower')
        axB.set_yticks([])
        axB.set_xlim(xmin, xmax)
        axB.set_xlabel('sensor x (scaled)')

        imgB = fig_to_rgb_array(figB)

        # === STACK A (top) and B (bottom) ===
        # Ensure same width via cropping/padding if off by a pixel due to rounding
        hA, wA, _ = imgA.shape
        hB, wB, _ = imgB.shape
        if wA != wB:
            # simple fix: resize the histogram width by cropping or padding to match wA
            if wB > wA:
                imgB = imgB[:, :wA, :]
            else:
                pad = wA - wB
                pad_left = pad // 2
                pad_right = pad - pad_left
                imgB = np.pad(imgB, ((0,0),(pad_left,pad_right),(0,0)), mode='edge')
        frame = np.vstack([imgA, imgB])
        images.append(frame)

    imageio.mimsave(out_gif, images, duration=frame_duration)


def save_points_csv(points_raw, x_scale, out_csv_path, header=('ray_index', 'x_raw', 'x_scaled', 'z_plane')):
    with open(out_csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(list(header))
        for i, pt in enumerate(points_raw, 1):
            if pt is None:
                writer.writerow([i, '', '', ''])
            else:
                x_raw, _y, z_plane = pt
                writer.writerow([i, f"{x_raw:.9g}", f"{(x_raw * x_scale):.9g}", f"{z_plane:.9g}"])


def main():
    parser = argparse.ArgumentParser(description="Animate ray histories with exit/sensor projections and sensor histogram.")
    parser.add_argument("--input", "-i", required=True, help="Path to input CSV/space-separated file.")
    parser.add_argument("--output", "-o", default="rays.gif", help="Output GIF path (default: rays.gif).")
    parser.add_argument("--limit", "-n", type=int, default=None, help="Limit to first N rays.")
    parser.add_argument("--duration_ms", type=int, default=100, help="Frame duration per frame (ms), default 100.")
    parser.add_argument("--dpi", type=int, default=150, help="Figure DPI for frames.")
    parser.add_argument("--x_scale", type=float, default=2.0, help="Scale factor applied to x for plotting (default 2.0).")
    parser.add_argument("--z_sensor", type=float, default=-300.0, help="Sensor plane z (default -300).")
    parser.add_argument("--exits_csv", type=str, default="ray_exits.csv", help="Path to write exit points CSV.")
    parser.add_argument("--sensor_csv", type=str, default="ray_sensor_hits.csv", help="Path to write sensor hits CSV.")
    parser.add_argument("--accumulate", action="store_true", help="Accumulate rays across frames for the GIF.")
    parser.add_argument("--show_planes", action="store_true", help="Draw dashed lines at z=0 and z=z_sensor.")
    parser.add_argument("--hist_bins", type=int, default=2000, help="Number of bins (pixels) for sensor histogram (default 2000).")
    parser.add_argument("--hist_height_px", type=int, default=40, help="Pixel height of the sensor histogram strip (default 40).")
    args = parser.parse_args()

    rays = load_rays(args.input)
    if args.limit is not None:
        rays = rays[: args.limit]
    if not rays:
        print("No valid rays parsed from input. Exiting.", file=sys.stderr)
        sys.exit(1)

    exits_raw, sensors_raw = compute_all_intersections(rays, z_sensor=args.z_sensor)
    polylines = gather_polylines_xz(rays, x_scale=args.x_scale)

    exits_scaled = []
    sensors_scaled = []
    for ex, sp in zip(exits_raw, sensors_raw):
        if ex is None:
            exits_scaled.append(None)
        else:
            exits_scaled.append((ex[0] * args.x_scale, 0.0, 0.0))
        if sp is None:
            sensors_scaled.append(None)
        else:
            sensors_scaled.append((sp[0] * args.x_scale, 0.0, args.z_sensor))

    render_frames(
        polylines, exits_scaled, sensors_scaled, args.output,
        dpi=args.dpi, frame_duration=args.duration_ms / 1000.0,
        accumulate=args.accumulate, show_planes=args.show_planes, z_sensor=args.z_sensor,
        hist_bins=args.hist_bins, hist_height_px=args.hist_height_px
    )

    if args.exits_csv:
        save_points_csv(exits_raw, args.x_scale, args.exits_csv, header=('ray_index', 'x_exit_raw', 'x_exit_scaled', 'z_plane'))
    if args.sensor_csv:
        save_points_csv(sensors_raw, args.x_scale, args.sensor_csv, header=('ray_index', 'x_sensor_raw', 'x_sensor_scaled', 'z_plane'))

    print(f"Wrote {args.output} with {len(polylines)} frame(s).")
    if args.exits_csv:
        print(f"Wrote exit points to {args.exits_csv}.")
    if args.sensor_csv:
        print(f"Wrote sensor hits to {args.sensor_csv}.")


if __name__ == "__main__":
    main()

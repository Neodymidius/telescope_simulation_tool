#!/usr/bin/env python3
"""
Outer Ray → Sensor GIF (v2)
===========================

Update per spec:
- Sensor plane Z is adjustable (default 300) via --z_sensor.
- Take sensor X and Y from columns 2 and 3 (1-based) of each row (i.e., after idx). Ignore the file's third value.
- Optionally compute the sensor X from trajectory if desired.
- Draw a single straight segment from the outer start (last pos before wall==0) to the sensor hit at z = --z_sensor.
- Show an accumulating 1D heat strip (default 2000 bins) of sensor X (scaled).

Row format reminder (space- or comma-separated):
  idx  sensor_x  sensor_y  sensor_z_file  meta4  meta5  meta6  meta7  meta8  meta9  then repeating:
  (px py pz) (dx dy dz) (wall) ... until a block with wall==0

We use:
  sensor_x := column 2
  sensor_y := column 3  (not plotted; x–z plane only)
  z_sensor := CLI --z_sensor (default 300)
  start    := last (pos,dir) before wall==0

CLI:
  --xy_source [file|compute]   choose whether to use (sensor_x from file) or compute x from intersection
  --z_sensor 300               sensor plane height (z)
  --x_scale 2.0                visual x scaling
  --accumulate                 build-up view
  --hist_bins 2000             heat strip "pixels"
  --hist_height_px 40          heat strip thickness
  --show_plane                 dashed line at z=z_sensor
  --sensor_csv                 save chosen sensor hits; also records both file_x and computed_x for diagnostics

No seaborn; matplotlib only. Two figures are stacked per frame (main plot + heat strip).
"""
import argparse
import csv
import sys
from typing import List, Tuple, Optional

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg
import imageio.v2 as imageio

Pos = Tuple[float, float, float]
Dir = Tuple[float, float, float]
Seg = Tuple[Pos, Dir, int]


def parse_row(row: str):
    """
    Returns:
      sensor_xy: (sx, sy) from columns 2 and 3 (1-based)
      segments: list of (pos, dir, wall) until wall==0 (excluded)
    """
    parts = row.strip().replace(',', ' ').split()
    if len(parts) < 1 + 9 + 7:
        return None, []
    try:
        nums = [float(p) for p in parts]
    except ValueError:
        return None, []

    # After idx, columns 2 and 3 are sensor x and y
    sensor_xy = (nums[1], nums[2])

    # Skip idx + 9 metadata
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
    return sensor_xy, segs


def load_rows(path: str):
    sensor_xys = []
    rays = []
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            if not line.strip():
                continue
            sensor_xy, segs = parse_row(line)
            if sensor_xy is None or not segs:
                continue
            sensor_xys.append(sensor_xy)
            rays.append(segs)
    return sensor_xys, rays


def intersect_plane_z(pos: Pos, dire: Dir, z_plane: float) -> Optional[Pos]:
    (x, y, z) = pos
    (dx, dy, dz) = dire
    if abs(dz) < 1e-15:
        return None
    t = (z_plane - z) / dz
    if t < 0:
        return None
    return (x + t * dx, y + t * dy, z + t * dz)


def fig_to_rgb_array(fig):
    canvas = FigureCanvasAgg(fig)
    canvas.draw()
    width, height = fig.canvas.get_width_height()
    buf = np.frombuffer(canvas.tostring_rgb(), dtype=np.uint8)
    img = buf.reshape((height, width, 3))
    plt.close(fig)
    return img


def render_gif(sensor_xys: List[Tuple[float, float]], rays: List[List[Seg]], out_gif: str,
               x_scale=2.0, duration_s=0.1, dpi=150, figsize=(6, 6),
               accumulate=False, hist_bins=2000, hist_height_px=40, show_plane=False,
               z_sensor: float = 300.0, xy_source: str = 'file', sensor_csv: Optional[str] = None):

    # Prepare chosen hits and line segments
    starts: List[Pos] = []
    hits_raw: List[Optional[Pos]] = []  # chosen hits at z=z_sensor (x,y,z_sensor)
    diag_rows = []  # for CSV: file_x, calc_x, used

    for (sx_file, sy_file), segs in zip(sensor_xys, rays):
        start_pos, start_dir, _ = segs[-1]

        # Compute intersection x if possible
        calc_hit = intersect_plane_z(start_pos, start_dir, z_sensor)

        # Decide which x to use
        if xy_source == 'file':
            # Use file sx, with y from file if available; fall back to calc if sx_file is NaN
            use_calc = (calc_hit is not None) and (not np.isfinite(sx_file))
            if use_calc:
                chosen = calc_hit
                used = 'calc'
            else:
                chosen = (sx_file, sy_file, z_sensor)
                used = 'file'
        else:  # xy_source == 'compute'
            if calc_hit is not None:
                chosen = calc_hit
                used = 'calc'
            else:
                # fallback to file x if compute fails
                chosen = (sx_file, sy_file, z_sensor)
                used = 'file_fallback'

        starts.append(start_pos)
        hits_raw.append(chosen)
        diag_rows.append((sx_file, calc_hit[0] if calc_hit is not None else None, used))

    # Build polylines (scaled x) from start to chosen hit
    polylines = []
    for start, hit in zip(starts, hits_raw):
        xs = [start[0] * x_scale]
        zs = [start[2]]
        if hit is not None:
            xs.append(hit[0] * x_scale)
            zs.append(hit[2])
        polylines.append((np.array(xs), np.array(zs)))

    # Sensor x (scaled) for histogram from chosen hits
    hits_x_scaled = [(p[0] * x_scale) if p is not None else None for p in hits_raw]

    # Axis limits
    all_x = np.concatenate([p[0] for p in polylines]) if polylines else np.array([0.0])
    all_z = np.concatenate([p[1] for p in polylines]) if polylines else np.array([0.0])
    hx = np.array([x for x in hits_x_scaled if x is not None])
    if hx.size:
        all_x = np.concatenate([all_x, hx])
    xmin, xmax = float(np.min(all_x)), float(np.max(all_x))
    zmin, zmax = float(np.min(all_z)), float(np.max(all_z))
    dx = xmax - xmin or 1.0
    dz = zmax - zmin or 1.0
    pad = 0.05
    xmin -= dx * pad
    xmax += dx * pad
    zmin -= dz * pad
    zmax += dz * pad

    images = []
    N = len(polylines)
    for idx in range(1, N + 1):
        lo, hi = (idx, idx) if not accumulate else (1, idx)

        # Main chart
        figA = plt.figure(figsize=figsize, dpi=dpi)
        axA = figA.add_subplot(111)
        for j in range(lo-1, hi):
            xs, zs = polylines[j]
            if len(xs) >= 2:
                axA.plot(xs, zs, marker='o', linewidth=1.5, markersize=3)
            else:
                axA.plot(xs, zs, marker='o', linewidth=0, markersize=3)

            # Sensor hit marker
            hxj = hits_x_scaled[j]
            if hxj is not None:
                axA.plot([hxj], [z_sensor], marker='s', markersize=6, linewidth=0)

        if show_plane:
            axA.axhline(z_sensor, linestyle='--', linewidth=0.8)

        axA.set_xlabel('x (scaled)')
        axA.set_ylabel('z')
        axA.set_title(f'Outer rays to sensor ({"accumulated" if accumulate else f"ray {idx}"})')
        axA.set_aspect('equal', adjustable='box')
        axA.set_xlim(xmin, xmax)
        axA.set_ylim(zmin, zmax)
        axA.grid(True, linewidth=0.3, alpha=0.4)

        imgA = fig_to_rgb_array(figA)

        # Histogram strip
        curr_hits = [hits_x_scaled[j] for j in range(lo-1, hi) if hits_x_scaled[j] is not None]
        if curr_hits:
            counts, _edges = np.histogram(curr_hits, bins=hist_bins, range=(xmin, xmax))
        else:
            counts = np.zeros(hist_bins, dtype=np.int64)

        width_in = figsize[0]
        height_in = hist_height_px / dpi
        figB = plt.figure(figsize=(width_in, height_in), dpi=dpi)
        axB = figB.add_subplot(111)
        strip = counts[np.newaxis, :]
        axB.imshow(strip, aspect='auto', extent=(xmin, xmax, 0, 1), cmap='hot', origin='lower')
        axB.set_yticks([])
        axB.set_xlim(xmin, xmax)
        axB.set_xlabel('sensor x (scaled)')
        imgB = fig_to_rgb_array(figB)

        # Stack
        hA, wA, _ = imgA.shape
        hB, wB, _ = imgB.shape
        if wA != wB:
            if wB > wA:
                imgB = imgB[:, :wA, :]
            else:
                padw = wA - wB
                left = padw // 2
                right = padw - left
                imgB = np.pad(imgB, ((0,0),(left,right),(0,0)), mode='edge')
        frame = np.vstack([imgA, imgB])
        images.append(frame)

    imageio.mimsave(out_gif, images, duration=duration_s)

    # Optional CSV of hits (file vs computed vs used)
    if sensor_csv:
        with open(sensor_csv, 'w', newline='', encoding='utf-8') as f:
            w = csv.writer(f)
            w.writerow(['ray_index', 'file_x', 'calc_x', 'used', 'chosen_x', 'chosen_x_scaled', 'z_sensor'])
            for i, (diag, chosen) in enumerate(zip(diag_rows, hits_raw), 1):
                file_x, calc_x, used = diag
                if chosen is None:
                    w.writerow([i, _fmt(file_x), _fmt(calc_x), used, '', '', _fmt(z_sensor)])
                else:
                    cx = chosen[0]
                    w.writerow([i, _fmt(file_x), _fmt(calc_x), used, _fmt(cx), _fmt(cx * x_scale), _fmt(z_sensor)])


def _fmt(v):
    if v is None:
        return ''
    try:
        return f'{float(v):.9g}'
    except Exception:
        return str(v)


def main():
    ap = argparse.ArgumentParser(description="Animate outer rays to adjustable sensor plane with heat-strip.")
    ap.add_argument('--input', '-i', required=True, help='Path to input file.')
    ap.add_argument('--output', '-o', default='outer_rays.gif', help='Output GIF path.')
    ap.add_argument('--x_scale', type=float, default=2.0, help='Scale factor applied to x for plotting (default 2.0).')
    ap.add_argument('--duration_ms', type=int, default=100, help='Per-frame duration in ms (default 100).')
    ap.add_argument('--dpi', type=int, default=150, help='Figure DPI.')
    ap.add_argument('--accumulate', action="store_true", help='Accumulate rays frame-by-frame.')
    ap.add_argument('--hist_bins', type=int, default=2000, help='Number of sensor bins (default 2000).')
    ap.add_argument('--hist_height_px', type=int, default=40, help='Pixel height of the sensor histogram (default 40).')
    ap.add_argument('--show_plane', action="store_true", help='Draw dashed line at sensor z.')
    ap.add_argument('--z_sensor', type=float, default=300.0, help='Sensor plane z (default 300).')
    ap.add_argument('--xy_source', choices=['file', 'compute'], default='file',
                    help="Use sensor x from file (columns 2,3) or compute intersection x from trajectory.")
    ap.add_argument('--sensor_csv', type=str, default='outer_sensor_hits.csv', help='CSV to write sensor hits.')
    args = ap.parse_args()

    sensor_xys, rays = load_rows(args.input)
    if not rays:
        print("No valid rows parsed.", file=sys.stderr)
        sys.exit(1)

    render_gif(
        sensor_xys, rays, args.output,
        x_scale=args.x_scale, duration_s=args.duration_ms/1000.0, dpi=args.dpi,
        accumulate=args.accumulate, hist_bins=args.hist_bins, hist_height_px=args.hist_height_px,
        show_plane=args.show_plane, z_sensor=args.z_sensor, xy_source=args.xy_source,
        sensor_csv=args.sensor_csv
    )

    print(f"Wrote {args.output} with {len(rays)} frame(s).")
    if args.sensor_csv:
        print(f"Wrote sensor hits to {args.sensor_csv}.")


if __name__ == '__main__':
    main()

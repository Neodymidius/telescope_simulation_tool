#!/usr/bin/env python3
import sys
import re
import argparse
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")  # headless rendering
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
import imageio.v2 as imageio  # mimsave

#Usage: python make_psf_gif_idx.py --dir "C:\Users\neore\Documents\finaltalk\" --out psf_animation.gif --duration 0.2
# ------------------------
# Data loading & utilities
# ------------------------

def sort_list(list_of_lines):
    return sorted(list_of_lines, key=lambda x: x[0])

def get_list_of_lines(path):
    hits = []
    with open(path, "r") as f:
        for line in f:
            parts = line.split()
            if len(parts) != 24:
                continue
            hit = [int(parts[0]), float(parts[1]), float(parts[2])]
            hits.append(hit)
    return hits

def get_x_y_lists(hits):
    x = [hit[1] for hit in hits]
    y = [hit[2] for hit in hits]
    return x, y

def compute_sensor(xs, ys, resolution, size_pixel):
    cols = np.floor(resolution[0] / 2 + xs / size_pixel).astype(int)
    rows = np.floor(resolution[1] / 2 - ys / size_pixel).astype(int)
    rows = np.clip(rows, 0, resolution[1] - 1)
    cols = np.clip(cols, 0, resolution[0] - 1)

    sensor = np.full((resolution[1], resolution[0]), 1e-5, dtype=float)
    np.add.at(sensor, (rows, cols), 1)
    return sensor


# regex for filenames like "123_something.txt"
IDX_RE = re.compile(r"^(\d+)_", re.IGNORECASE)

def parse_index_from_filename(path: Path) -> int:
    m = IDX_RE.match(path.name)
    if not m:
        raise ValueError(f"Cannot parse index from filename: {path}")
    return int(m.group(1))


# ------------------------
# Plotting / rendering
# ------------------------

def plot_sensor(ax, sensor, extent, title, cmap='hot', vmin=1e-2, vmax=None):
    im = ax.imshow(
        sensor,
        origin='upper',
        extent=extent,
        cmap=cmap,
        norm=colors.LogNorm(vmin=vmin, vmax=vmax if vmax is not None else sensor.max())
    )
    ax.set(
        xlim=[-4, 14.4], ylim=[-5, 5],
        xlabel='x [mm]', ylabel='y [mm]',
        title=title
    )
    return im

def render_frame(sensor, extent, frame_idx, cmap, vmin, vmax,
                 figsize=(8, 6), dpi=100, show_colorbar=True):
    fig, ax = plt.subplots(1, 1, figsize=figsize, dpi=dpi)
    im = plot_sensor(ax, sensor, title="On Axis Point Source",
                     extent=extent, cmap=cmap, vmin=vmin, vmax=vmax)

    # Annotation with index
    ax.text(
        0.02, 0.98, f"Frame = {frame_idx}",
        transform=ax.transAxes, ha="left", va="top",
        fontsize=12, bbox=dict(facecolor="white", edgecolor="none", alpha=0.5)
    )

    if show_colorbar:
        fig.colorbar(im, ax=ax, label='Counts (log scale)')

    fig.tight_layout()

    canvas = FigureCanvas(fig)
    canvas.draw()
    buf = np.frombuffer(canvas.tostring_argb(), dtype=np.uint8)
    w, h = fig.canvas.get_width_height()
    buf = buf.reshape((h, w, 4))
    buf = buf[:, :, [1, 2, 3, 0]]  # ARGB → RGBA

    rgb = buf[:, :, :3].copy()
    plt.close(fig)
    return rgb


# ------------------------
# Main pipeline
# ------------------------

def main():
    parser = argparse.ArgumentParser(description="Generate PSF GIF from idx_*.txt files.")
    parser.add_argument(
        "--dir",
        required=True,
        help="Directory containing idx_*.txt files."
    )
    parser.add_argument(
        "--out",
        default="psf_animation.gif",
        help="Output GIF filename (default: psf_animation.gif)."
    )
    parser.add_argument(
        "--duration",
        type=float,
        default=0.2,
        help="Frame duration in seconds (default: 0.2 → 200 ms)."
    )
    args = parser.parse_args()

    base_dir = Path(args.dir)
    files = sorted(
        base_dir.glob("*.txt"),
        key=lambda p: parse_index_from_filename(p)
    )
    if not files:
        print(f"No matching txt files found in {base_dir}")
        sys.exit(1)

    # Geometry / sensor parameters
    resolution = (3840, 3840)
    size_pixel = 0.0075
    physical_size = resolution[0] * size_pixel
    extent = [-physical_size / 2, physical_size / 2,
              -physical_size / 2, physical_size / 2]

    cmap = "hot"
    vmin = 1e-2

    # vmax from first file (index 0)
    first_path = files[0]
    hits = get_list_of_lines(first_path)
    sorted_hits = sort_list(hits)
    xs, ys = get_x_y_lists(sorted_hits)
    xs, ys = np.array(xs), np.array(ys)
    first_sensor = compute_sensor(xs, ys, resolution, size_pixel)
    vmax = float(first_sensor.max())
    print(f"[INFO] Using LogNorm vmin={vmin}, vmax={vmax} (from first file: {first_path.name})")

    frames = []
    for idx, path in enumerate(files):
        file_idx = parse_index_from_filename(path)

        hits = get_list_of_lines(path)
        sorted_hits = sort_list(hits)
        xs, ys = get_x_y_lists(sorted_hits)
        xs, ys = np.array(xs), np.array(ys)
        sensor = compute_sensor(xs, ys, resolution, size_pixel)

        frame = render_frame(sensor, extent, file_idx,
                             cmap=cmap, vmin=vmin, vmax=vmax,
                             figsize=(8, 6), dpi=100, show_colorbar=True)
        frames.append(frame)

        print(f"[{idx+1:04d}/{len(files):04d}] added frame for {path.name} (index={file_idx})")

    out_path = Path(args.out)
    imageio.mimsave(out_path, frames, duration=args.duration, loop=0)
    print(f"[DONE] Saved GIF with {len(frames)} frames → {out_path.resolve()}")


if __name__ == "__main__":
    main()

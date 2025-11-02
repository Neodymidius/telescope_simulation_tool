#!/usr/bin/env python3

"""
Score simulated point-source files against a reference PSF radial profile.

Usage:
  python score_point_sources.py \
      --psf erosita_psf_v3.1.fits \
      --pattern "point_off_focus_*.txt" \
      --top 10 \
      --out results.csv

Notes:
- Uses the same binning/normalization you used:
    * bins = logspace(0, 2, 50)  # radii in mm, 1..100
    * per-area normalization (1 / (r2^2 - r1^2) / pi)
    * then normalized by total counts
- Score = mean(|(sim/ref) - 1|) over bins where ref > 0.
  (You can switch to RMSE or chi-square-like if preferred.)
"""

import argparse
import glob
import math
import os
from typing import Tuple, Optional

import numpy as np
from astropy.io import fits
from astropy.wcs import WCS

def compute_reference_profile(psf_fits: str, histbins: np.ndarray) -> np.ndarray:
    with fits.open(psf_fits) as hdul:
        hdr = hdul[0].header
        img = hdul[0].data.astype(float)
    w = WCS(hdr)
    X_wcs = w.pixel_to_world(np.arange(0, img.shape[0]), np.zeros(img.shape[0]))[0].to("mm").value
    Y_wcs = w.pixel_to_world(np.zeros(img.shape[1]), np.arange(0, img.shape[1]))[1].to("mm").value
    X_idx, Y_idx = np.meshgrid(np.arange(len(X_wcs)), np.arange(len(Y_wcs)))
    dists = np.sqrt(X_wcs[X_idx] ** 2 + Y_wcs[Y_idx] ** 2)
    hist, _ = np.histogram(dists, bins=histbins, weights=img)
    hist_norm = 1.0 / (histbins[1:] ** 2 - histbins[:-1] ** 2) / np.pi
    total = hist.sum()
    return (hist / total) * hist_norm if total > 0 else np.zeros_like(hist_norm)

def load_hits_xy(path: str, radius_cut: float = None) -> Tuple[np.ndarray, np.ndarray]:
    xs, ys = [], []
    with open(path, "r") as f:
        for line in f:
            parts = line.split()
            if len(parts) < 3:
                continue
            try:
                x = float(parts[1]); y = float(parts[2])
            except ValueError:
                continue
            if radius_cut is not None and math.hypot(x, y) > radius_cut:
                continue
            xs.append(x); ys.append(y)
    return np.array(xs, dtype=float), np.array(ys, dtype=float)

def compute_sim_profile(xs: np.ndarray, ys: np.ndarray, histbins: np.ndarray, area_norm: np.ndarray) -> np.ndarray:
    if xs.size == 0:
        return np.zeros_like(area_norm)
    dists = np.sqrt(xs ** 2 + ys ** 2)
    hist, _ = np.histogram(dists, bins=histbins)
    total = hist.sum()
    return (hist / total) * area_norm if total > 0 else np.zeros_like(area_norm)

def parse_bin_slice(s: Optional[str], length: int) -> slice:
    """Parse strings like ':20', '10:', '5:25' into a slice within [0, length]."""
    if not s:
        return slice(None)
    if ":" not in s:
        raise ValueError("Use 'start:end' form, e.g. ':20', '10:', or '5:25'.")
    start_str, end_str = s.split(":", 1)
    start = int(start_str) if start_str.strip() != "" else None
    end = int(end_str) if end_str.strip() != "" else None
    # Clamp to valid range
    if start is not None:
        start = max(0, min(start, length))
    if end is not None:
        end = max(0, min(end, length))
    return slice(start, end)

def score_profile(sim: np.ndarray, ref: np.ndarray, slc: slice = slice(None), eps: float = 1e-15) -> float:
    sim = sim[slc]
    ref = ref[slc]
    mask = ref > eps
    if not np.any(mask):
        return float("inf")
    ratio = np.zeros_like(sim)
    ratio[mask] = sim[mask] / ref[mask]
    return float(np.mean(np.abs(ratio[mask] - 1.0)))

def main():
    ap = argparse.ArgumentParser(description="Score simulated point-sources vs a reference PSF radial profile.")
    ap.add_argument("--psf", required=True, help="Path to reference PSF FITS, e.g., erosita_psf_v3.1.fits")
    ap.add_argument("--pattern", required=True, help="Glob for simulation files, e.g., 'point_off_focus_*.txt'")
    ap.add_argument("--top", type=int, default=10, help="How many top files to print")
    ap.add_argument("--out", default="results.csv", help="CSV to write all scores")
    ap.add_argument("--rmin", type=float, default=1.0, help="Min radius bin edge in mm")
    ap.add_argument("--rmax", type=float, default=100.0, help="Max radius bin edge in mm")
    ap.add_argument("--nbins", type=int, default=50, help="Number of radial edges (log-spaced). Produces nbins-1 bins.")
    ap.add_argument("--radius-cut", type=float, default=None,
                    help="Optional cut on |r| for sim hits (mm). Default: no cut.")
    ap.add_argument("--score-bins", type=str, default=None,
                    help="Bin slice to score, e.g. ':20' (first 20), '10:30', '25:'. Indices refer to the nbins-1 bins.")
    args = ap.parse_args()

    histbins = np.logspace(np.log10(args.rmin), np.log10(args.rmax), args.nbins)
    area_norm = 1.0 / (histbins[1:] ** 2 - histbins[:-1] ** 2) / np.pi

    ref_density = compute_reference_profile(args.psf, histbins)
    slc = parse_bin_slice(args.score_bins, len(ref_density))

    files = sorted(glob.glob(args.pattern))
    if not files:
        print(f"No files matched pattern: {args.pattern}")
        return

    rows = []
    for fp in files:
        xs, ys = load_hits_xy(fp, radius_cut=args.radius_cut)
        sim_density = compute_sim_profile(xs, ys, histbins, area_norm)
        sc = score_profile(sim_density, ref_density, slc=slc)
        rows.append((fp, sc, xs.size))

    rows.sort(key=lambda r: r[1])

    top_n = min(args.top, len(rows))
    print(f"\nTop {top_n} files (lower score = closer to reference):")
    for i in range(top_n):
        fp, sc, n = rows[i]
        print(f"{i+1:2d}. {os.path.basename(fp)}  score={sc:.6f}  hits={n}")

    import csv
    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["file", "score_mean_abs_dev_from_1", "num_hits", "score_bins"])
        for fp, sc, n in rows:
            w.writerow([fp, f"{sc:.12g}", n, args.score_bins or "all"])

    print(f"\nWrote {len(rows)} rows to {args.out}")

if __name__ == "__main__":
    main()

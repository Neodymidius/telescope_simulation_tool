#!/usr/bin/env python3
import sys
import csv
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as colors

# -------------------- Sensor config --------------------
RES_X = 8840
RES_Y = 8840
PIX_MM = 0.0075  # mm per pixel

# -------------------- Fixed view window (in mm) --------------------
X_LIM = (14.0, 20.0)
Y_LIM = (-4.0, 4.0)

# -------------------- CSV readers --------------------
def _to_float(s):
    try:
        return float(s)
    except Exception:
        return None

def _to_int(s):
    try:
        return int(float(s))
    except Exception:
        return None

def read_embree_hits(path):
    xs, ys = [], []
    with open(path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            row = {k.lower(): v for k, v in row.items()}
            if "hit_sensor" in row:
                h = _to_int(row["hit_sensor"])
                if h != 1:
                    continue
            x = _to_float(row.get("hit_x_mm"))
            y = _to_float(row.get("hit_y_mm"))
            if x is None or y is None:
                continue
            xs.append(x); ys.append(y)
    return np.asarray(xs, dtype=float), np.asarray(ys, dtype=float)

def read_opengl_hits(path):
    xs, ys = [], []
    with open(path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            if row is None:
                continue
            row = {k.lower(): v for k, v in row.items()}
            # Optional gating flags if present
            flag_keys = ("hit_sensor", "sensor_hit", "plane_hit")
            flagged = True
            for k in flag_keys:
                if k in row and row[k] not in ("", None):
                    val = row[k].strip().lower()
                    flagged = val in ("1", "true", "yes")
                    break
            if not flagged:
                continue
            x = _to_float(row.get("sensor_x_mm"))
            y = _to_float(row.get("sensor_y_mm"))
            if x is None or y is None:
                continue
            xs.append(x); ys.append(y)
    return np.asarray(xs, dtype=float), np.asarray(ys, dtype=float)

# -------------------- Binning --------------------
def bin_hits(xs_mm, ys_mm, res_x, res_y, pix_mm, add_floor=True):
    if xs_mm.size == 0:
        base = np.zeros((res_y, res_x), dtype=float)
        if add_floor:
            base += 1e-1
        return base
    cols = np.floor(res_x / 2 + xs_mm / pix_mm).astype(int)
    rows = np.floor(res_y / 2 - ys_mm / pix_mm).astype(int)  # matches origin='upper'
    rows = np.clip(rows, 0, res_y - 1)
    cols = np.clip(cols, 0, res_x - 1)

    img = np.zeros((res_y, res_x), dtype=float)
    if add_floor:
        img[:] = 1e-1
    np.add.at(img, (rows, cols), 1.0)
    return img

# -------------------- Plot helpers --------------------
def extent_from_sensor(res_x, res_y, pix_mm):
    w = res_x * pix_mm
    h = res_y * pix_mm
    return [-w/2, w/2, -h/2, h/2]

def apply_limits(ax):
    ax.set_xlim(X_LIM)
    ax.set_ylim(Y_LIM)
    ax.set_xlabel("x [mm]")
    ax.set_ylabel("y [mm]")

def plot_log_heat(ax, img, extent, title, cmap="hot"):
    vmax = max(img.max(), 1.0)
    im = ax.imshow(
        img, origin="upper", extent=extent, cmap=cmap,
        norm=colors.LogNorm(vmin=1e-1, vmax=vmax)
    )
    ax.set_title(title)
    apply_limits(ax)
    return im

def plot_linear(ax, img, extent, title, cmap="gray"):
    im = ax.imshow(img, origin="upper", extent=extent, cmap=cmap, vmin=0.0, vmax=1.0)
    ax.set_title(title)
    apply_limits(ax)
    return im

# -------------------- Main --------------------
def main():
    if len(sys.argv) < 3:
        print("Usage: python compare_psf.py <embree_retrace.csv> <opengl_bake.csv>")
        sys.exit(1)

    embree_csv = sys.argv[1]
    opengl_csv = sys.argv[2]

    ex, ey = read_embree_hits(embree_csv)
    ox, oy = read_opengl_hits(opengl_csv)

    A = bin_hits(ox, oy, RES_X, RES_Y, PIX_MM, add_floor=True)    # OpenGL counts
    B = bin_hits(ex, ey, RES_X, RES_Y, PIX_MM, add_floor=True)    # Embree counts

    # Presence maps (binary) for union/diff
    A_bin = (bin_hits(ox, oy, RES_X, RES_Y, PIX_MM, add_floor=False) > 0.0).astype(float)
    B_bin = (bin_hits(ex, ey, RES_X, RES_Y, PIX_MM, add_floor=False) > 0.0).astype(float)
    union_bin = np.clip(A_bin + B_bin, 0, 1)
    diff_open_minus_embree = np.clip(A_bin - B_bin, 0, 1)

    extent = extent_from_sensor(RES_X, RES_Y, PIX_MM)

    fig, axs = plt.subplots(2, 2, figsize=(14, 12))
    im0 = plot_log_heat(axs[0,0], A, extent, "OpenGL (counts, log)", cmap="hot")
    fig.colorbar(im0, ax=axs[0,0], label="Counts")

    im1 = plot_log_heat(axs[0,1], B, extent, "Embree (counts, log)", cmap="hot")
    fig.colorbar(im1, ax=axs[0,1], label="Counts")

    im2 = plot_linear(axs[1,0], union_bin, extent, "Merged presence (OpenGL ∪ Embree)", cmap="gray")
    fig.colorbar(im2, ax=axs[1,0], label="Presence (0/1)")

    im3 = plot_linear(axs[1,1], diff_open_minus_embree, extent, "Difference presence (OpenGL − Embree)", cmap="gray")
    fig.colorbar(im3, ax=axs[1,1], label="Presence (0/1)")

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()

import sys
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as colors


def sort_list(list_of_lines):
    return sorted(list_of_lines, key=lambda x: x[0])


def get_list_of_lines(path):
    hits = []
    with open(path) as f:
        for line in f:
            parts = line.split()
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

    sensor = np.full((resolution[1], resolution[0]), 1e-1, dtype=float)
    np.add.at(sensor, (rows, cols), 1)
    return sensor


def plot_diff(ax, diff, extent, title):
    vmax = np.nanmax(np.abs(diff))
    norm = colors.TwoSlopeNorm(vmin=-vmax, vcenter=0.0, vmax=vmax)
    im = ax.imshow(
        diff,
        origin='upper',
        extent=extent,
        cmap='RdBu_r',
        norm=norm
    )
    ax.set(xlim=[-80, 80], ylim=[-80, 80],
           xlabel='x [mm]', ylabel='y [mm]',
           title=title)
    return im


def main():
    file_paths = sys.argv[1:]
    if len(file_paths) != 2:
        print("Usage: python script.py first.txt second.txt")
        sys.exit(1)

    path1, path2 = file_paths

    resolution = (8840, 8840)
    size_pixel = 0.44
    physical_size = resolution[0] * size_pixel
    extent = [-physical_size / 2, physical_size / 2,
              -physical_size / 2, physical_size / 2]

    # Load and compute sensors
    hits1 = sort_list(get_list_of_lines(path1))
    xs1, ys1 = (np.array(v) for v in get_x_y_lists(hits1))
    sensor1 = compute_sensor(xs1, ys1, resolution, size_pixel)

    hits2 = sort_list(get_list_of_lines(path2))
    xs2, ys2 = (np.array(v) for v in get_x_y_lists(hits2))
    sensor2 = compute_sensor(xs2, ys2, resolution, size_pixel)

    # Compute difference (second − first)
    diff = sensor2 - sensor1

    fig, ax = plt.subplots(figsize=(8, 7))
    im = plot_diff(ax, diff, extent, title="Difference (2 − 1)")
    fig.colorbar(im, ax=ax, label='Δ Counts (linear)')

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    main()

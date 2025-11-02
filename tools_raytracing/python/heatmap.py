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


def plot_sensor(ax, sensor, extent, title, cmap='hot'):
    im = ax.imshow(sensor, origin='upper',
                   extent=extent,
                   cmap=cmap,
                   norm=colors.LogNorm(vmin=1e-2, vmax=sensor.max()))
    ax.set(xlim=[-18, 18], ylim=[-18, 18],
           xlabel='x [mm]', ylabel='y [mm]',
           title=title)
    return im


def main():
    file_paths = sys.argv[1:]
    if not file_paths:
        print("Usage: python script.py file1.txt file2.txt ...")
        sys.exit(1)

    resolution = (4840, 4840)
    size_pixel = 0.0075
    physical_size = resolution[0] * size_pixel
    extent = [-physical_size / 2, physical_size / 2,
              -physical_size / 2, physical_size / 2]

    num_files = len(file_paths)
    fig, axes = plt.subplots(1, num_files, figsize=(8 * num_files, 6), squeeze=False)

    for idx, path in enumerate(file_paths):
        hits = get_list_of_lines(path)
        sorted_hits = sort_list(hits)
        xs, ys = get_x_y_lists(sorted_hits)
        xs = np.array(xs)
        ys = np.array(ys)
        sensor = compute_sensor(xs, ys, resolution, size_pixel)

        ax = axes[0, idx]
        title = f"On Axis Point Source"
        im = plot_sensor(ax, sensor, extent, title, cmap='hot')
        fig.colorbar(im, ax=ax, label='Counts (log scale)')

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    main()

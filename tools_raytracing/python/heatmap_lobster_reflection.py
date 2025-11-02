import sys
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as colors


def sort_list(list_of_lines):
    return sorted(list_of_lines, key=lambda x: x[0])


def get_list_of_lines(path):
    reflections = {}
    with open(path) as f:
        for line in f:
            parts = line.split()
            # Original key logic preserved
            key = (len(parts) - 10) // 7 - 2
            if key not in reflections:
                reflections[key] = []
            hit = [int(parts[0]), float(parts[1]), float(parts[2])]
            reflections[key].append(hit)
    return reflections


def get_x_y_lists(hits):
    return_dic = {}
    for key, value in hits.items():
        x = [hit[1] for hit in hits[key]]
        y = [hit[2] for hit in hits[key]]
        return_dic[key] = [x, y]
    return return_dic


def compute_sensor(xs, ys, resolution, size_pixel):
    xs = np.asarray(xs)
    ys = np.asarray(ys)

    cols = np.floor(resolution[0] / 2 + xs / size_pixel).astype(int)
    rows = np.floor(resolution[1] / 2 - ys / size_pixel).astype(int)
    rows = np.clip(rows, 0, resolution[1] - 1)
    cols = np.clip(cols, 0, resolution[0] - 1)

    sensor = np.full((resolution[1], resolution[0]), 1e-1, dtype=float)
    np.add.at(sensor, (rows, cols), 1)
    return sensor


def plot_sensor(ax, sensor, extent, title, cmap='hot'):
    im = ax.imshow(
        sensor,
        origin='upper',
        extent=extent,
        cmap=cmap,
        norm=colors.LogNorm(vmin=1e-1, vmax=max(sensor.max(), 1e-1))
    )
    ax.set(
        xlim=[-100, 100],
        ylim=[-100, 100],
        xlabel='x [mm]',
        ylabel='y [mm]',
        title=title
    )
    return im


def chunked(iterable, n):
    """Yield successive n-sized chunks from a sorted list."""
    for i in range(0, len(iterable), n):
        yield iterable[i:i + n]


def main():
    file_paths = sys.argv[1:]
    if not file_paths:
        print("Usage: python script.py file1.txt file2.txt ...")
        sys.exit(1)

    # Sensor and plotting parameters
    resolution = (512, 512)
    size_pixel = 0.44
    physical_size = resolution[0] * size_pixel
    extent = [-physical_size / 2, physical_size / 2,
              -physical_size / 2, physical_size / 2]

    for path in file_paths:
        # Parse and organize hits
        hits = get_list_of_lines(path)
        reflection_dic = get_x_y_lists(hits)

        # Ensure numpy arrays for compute_sensor
        for key in list(reflection_dic.keys()):
            reflection_dic[key][0] = np.array(reflection_dic[key][0])
            reflection_dic[key][1] = np.array(reflection_dic[key][1])

        # Compute sensors for each reflection
        sensors = {}
        for key, (xs, ys) in reflection_dic.items():
            sensors[key] = compute_sensor(xs, ys, resolution, size_pixel)

        # Sort keys for consistent ordering
        keys_sorted = sorted(sensors.keys())

        # Create 3x3 multi-plot grids, chunking if more than 9
        page_num = 1
        for key_chunk in chunked(keys_sorted, 9):
            fig, axes = plt.subplots(3, 3, figsize=(13, 13))
            axes = axes.flatten()

            for idx, key in enumerate(key_chunk):
                ax = axes[idx]
                title = f"Reflection {key}"
                im = plot_sensor(ax, sensors[key], extent, title, cmap='hot')
                fig.colorbar(im, ax=ax, label='Counts (log scale)')

            # Hide any unused subplots (e.g., only 7 or 8 reflections)
            for j in range(len(key_chunk), 9):
                axes[j].set_visible(False)

            fig.suptitle(f"Composition of PSF per reflection", y=0.995)
            plt.tight_layout()
            plt.show()
            page_num += 1

        # Optional: scatter plot of hit points for the LAST processed file (as in original code)
        # Keep colors list and guard against index overflow
        colors_list = ['red', 'blue', 'green', 'yellow', 'cyan', 'fuchsia',
                       'orange', 'pink', 'grey', 'purple', 'brown', 'lightgreen']

        plt.figure(figsize=(8, 8))
        for i, key in enumerate(sorted(reflection_dic.keys())):
            value = reflection_dic[key]
            color = colors_list[i % len(colors_list)]
            plt.plot(value[0], value[1], '.', color=color,
                     label=f"Reflections: {key}", markersize=1)

        plt.xlabel('x [mm]')
        plt.ylabel('y [mm]')
        plt.legend(loc="upper right", fontsize='small', markerscale=5)
        plt.tight_layout()
        plt.show()


if __name__ == '__main__':
    main()

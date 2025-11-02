import numpy as np
import matplotlib.pyplot as plt
import sys


class Coordinate:
    def __init__(self, x: float, y: float, shell: int):
        self.x = x
        self.y = y
        self.shell = shell


def enter_photons_to_sensor(filename: str):
    with open(filename, 'r') as photons:
        coordinates_per_shell = [[] for _ in range(54)]
        for line in photons:
            if line.strip() == '':
                continue
            split_line = line.strip().split()
            if len(split_line) != 24:
                continue
            coordinates_per_shell[int(split_line[9]) % 54] \
                .append(Coordinate(float(split_line[1]), float(split_line[2]),
                                   int(split_line[9]) % 54))

    create_plot(coordinates_per_shell)


def create_plot(coordinates):
    # Take only the "first plot from each row"
    # Original layout was 9 rows × 6 columns → we take col=0 of each row → 9 plots
    selected_coords = [coordinates[row * 6] for row in range(9)]

    rows, cols = 3, 3
    fig, axs = plt.subplots(rows, cols, figsize=(cols * 5, rows * 5), constrained_layout=True)
    axs = axs.ravel()

    xlim = (12, 14)
    ylim = (-1.5, 1.5)

    for i, arr in enumerate(selected_coords):
        ax = axs[i]
        if arr:
            xs = [c.x for c in arr]
            ys = [c.y for c in arr]
            ax.scatter(xs, ys, s=1)
        ax.set_title(f"Shell {i*6}")  # index corresponds to first column in original
        ax.grid(True)
        ax.set_xlim(*xlim)
        ax.set_ylim(*ylim)
        ax.set_aspect('equal', adjustable='box')

    # Shared axis labels
    fig.supxlabel("X position (mm)")
    fig.supylabel("Y position (mm)")

    plt.show()


def main():
    dir_path = sys.argv[1]
    enter_photons_to_sensor(dir_path)


if __name__ == "__main__":
    main()

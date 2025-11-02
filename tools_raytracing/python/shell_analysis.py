import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import sys


class Coordinate:
    def __init__(self, x: float, y: float, shell: int):
        self.x = x
        self.y = y
        self.shell = shell


def enter_photons_to_sensor(filename: str):
    with open(filename, 'r') as photons:
        coordinates_per_shell = []
        for i in range(54):
            coordinates_per_shell.append([])
        for line in photons:
            if line.strip() == '':
                continue
            split_line = line.strip().split()
            if len(split_line) != 24:
                continue
            coordinates_per_shell[int(split_line[9])%54] \
                .append(Coordinate(float(split_line[1]), float(split_line[2]), int(split_line[9])%54))

    #for coordinates in coordinates_per_shell:
        #if len(coordinates) != 0:
    create_plot(coordinates_per_shell)


def create_plot(coordinates):
    shell = -1
    rows, cols = 9, 6
    fig, axs = plt.subplots(rows, cols, figsize=(cols * 4, rows * 4), constrained_layout=True)
    axs = axs.ravel()
   # plt.xlabel('X position (mm)')
   # plt.ylabel('Y position (mm)')

    xlim = (12, 14)
    ylim = (-1.5, 1.5)
    for i, arr in enumerate(coordinates):
        ax = axs[i]
        if arr:
            xs = [c.x for c in arr]
            ys = [c.y for c in arr]
            ax.scatter(xs, ys, s=1)  # tiny points; use marker='.' if you prefer
        else:
            # No hits for this shell; keep empty axes but still label it
            pass
        ax.set_title(f"Shell {i}")
        ax.grid(True)
        ax.set_xlim(*xlim)
        ax.set_ylim(*ylim)
        ax.set_aspect('equal', adjustable='box')
    plt.show()


def main():
    dir_path = sys.argv[1]
    enter_photons_to_sensor(dir_path )#+ "/concatenated.txt")


if __name__ == "__main__":
    main()

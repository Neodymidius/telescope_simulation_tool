import sys
import ast
import numpy as np
import matplotlib.pyplot as plt

def first_draft():
    list_of_photons = []
    for line in sys.stdin:
        # Strip newline characters from the end of the line
        line = line.rstrip('\n')
        # Check for a specific stop condition
        if line == "Seed = 1":
            continue
        if line == "STOP":
            print("Stop condition met. Exiting loop.")
            break  # Exit the loop
        try:
            # Safely evaluate the line as a Python literal
            data = ast.literal_eval(line)
            list_of_photons.append(data)
        except ValueError as e:
            print(f"Error processing line: {e}")

    xs = []
    ys = []
    shells = []
    for photon in list_of_photons:
        xs.append(photon[-1][0])
        ys.append(photon[-1][1])
        shells.append(photon[0][2])

    # Color:
    cmap = plt.cm.get_cmap('tab20', 54)  # tab20 is a colormap with distinct colors
    colors = [cmap(i) for i in range(54)]

    for x, y, shell in zip(xs, ys, shells):
        plt.plot(x, y, 'o', markersize=1)
    plt.xlabel('distance in X [mm]')  # Optional: Label for the x-axis
    plt.ylabel('distance in Y [mm]')  # Optional: Label for the y-axis
    plt.title('Photon Sensor Hits')  # Optional: Title for the plot
    plt.grid(True)  # Optional: Adds a grid
    plt.axis([-100, 100, -100, 100]) # [xmin, xmax, ymin, ymax]
    plt.gca().set_aspect('equal')
    plt.show()


def plot_points(filename):
    with open(filename, 'r') as photons:
        x_array = []
        y_array = []
        for line in photons:
            coordinates = line.strip().split()
            x_array.append(float(coordinates[0]))
            y_array.append(float(coordinates[1]))
    i = 0
    for x, y in zip(x_array, y_array):
        if i == 10000:
            break
        i = i+1
        plt.plot(x, y, 'bo', markersize=1)
    print("finished")
    plt.xlabel('distance in X [mm]')  # Optional: Label for the x-axis
    plt.ylabel('distance in Y [mm]')  # Optional: Label for the y-axis
    plt.title('Photon Sensor Hits')  # Optional: Title for the plot
    plt.grid(True)  # Optional: Adds a grid
    plt.axis([-100, 100, -100, 100]) # [xmin, xmax, ymin, ymax]
    plt.gca().set_aspect('equal')
    plt.show()



if __name__ == "__main__":
    plot_points("../results/point_off_focus241224_1-0.0140000.000000.txt")

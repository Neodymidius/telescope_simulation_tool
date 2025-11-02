import math
import sys
import ast
from turtle import color

import numpy as np
import matplotlib.pyplot as plt

color_flag = 0
counter = 0


def plot_parabola(coefficients, start, stop):
    a, b = coefficients
    x = np.linspace(start, stop, 400)
    y = a**-1*x**2 / 2 - a/2
    plt.plot(x, y, label='Parabola', color='b')


def plot_hyperboloid(coefficients, start, stop):
    a, b, c = coefficients
    x = np.linspace(start, stop, 400)
    y_positive = c + (a * np.sqrt(b**2 + x**2))/b
    #y_negative = -np.sqrt(b**2 * ((x - c)**2 / a**2 - 1))  # Negative branch

    plt.plot(x, y_positive, label='Hyperbola Positive Branch', color='b')
    #plt.plot(x, y_negative, label='Hyperbola Negative Branch', color='g')


def plot_ray(position, direction, parabola1, parabola2, length=220000):
    # Start point
    x_start, y_start = position
    """if abs(x_start) > 80 and direction[0] == 0:
        y = parabola1[0]**-2*x_start**2 + parabola1[1]

        length = math.sqrt((y_start - y)**2)
    elif direction[0] == 0:
        y = parabola2[0]**-2*x_start**2 + parabola2[1]
        length = math.sqrt((y_start - y)**2)
        print(x_start)
"""
    # End point calculated based on the direction and a fixed length
    x_end = x_start + direction[0] * length
    y_end = y_start + direction[1] * length
    global color_flag
    global counter
    if color_flag == 0:
        counter += 1
        plt.plot([x_start, x_end], [y_start, y_end], color='g', markersize=0.01, marker='o')
        color_flag = 1
    elif color_flag == 1:
        plt.plot([x_start, x_end], [y_start, y_end], color='r', markersize=0.01, marker='o')
        color_flag = 2
    else:
        plt.plot([x_start, x_end], [y_start, y_end], color='c', markersize=0.01, marker='o')
        color_flag = 0


if __name__ == "__main__":
    first_line = 0
    parabola_1 = []
    parabola_2 = []
    for line in sys.stdin:
        line = line.rstrip('\n')
        if line == "STOP":
            print("Stop condition met. Exiting loop.")
            break
        try:
            data = ast.literal_eval(line)
            if first_line < 2:
                # Plot the parabola using the first line
                if first_line == 0:
                    parabola_1 = data
                    plot_parabola(data, -200, 200)
                else:
                    parabola_2 = data
                    plot_hyperboloid(data, -200, 200)
                first_line += 1
            else:
                # Plot rays using the other lines
                position, direction = data
                plot_ray(position, direction, parabola_1, parabola_2)
        except ValueError as e:
            print(f"Error processing line: {e}")

    plt.xlabel(' diameter [mm]')
    plt.ylabel('height [mm]')
    plt.title('Raytracing 2D representation')
    plt.grid(False)
    plt.axis([-200, 200, -10, 20000])
    plt.legend()
    plt.show()
    print(counter)

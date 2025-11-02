import pandas as pd
import datashader as ds
import datashader.transfer_functions as tf
from colorcet import fire
from PIL import Image
import sys

# File path to the large text file
dir_path = sys.argv[1] 
file_path = dir_path + "concatenated.txt"

# Load the data
def load_data(file_path):
    chunks = []
    with open(file_path, "r") as file:
        for line in file:
            if line.strip() == '':
                continue
            x, y = map(float, line.split()[0:2])
            chunks.append((x, y))
    return pd.DataFrame(chunks, columns=["x", "y"])

print("Loading data...")
data = load_data(file_path)
print("Data loaded.")

# Get coordinate range to maintain aspect ratio
x_range = (data["x"].min(), data["x"].max())
y_range = (data["y"].min(), data["y"].max())

# Define canvas size proportional to the coordinate range
x_span = x_range[1] - x_range[0]
y_span = y_range[1] - y_range[0]
aspect_ratio = x_span / y_span
canvas_width = 10000  # High resolution
canvas_height = int(canvas_width / aspect_ratio)

# Create Datashader canvas with proportional dimensions
canvas = ds.Canvas(plot_width=canvas_width, plot_height=canvas_height,
                   x_range=x_range, y_range=y_range)

# Aggregate and render the data
print("Rendering...")
agg = canvas.points(data, "x", "y")
img = tf.shade(agg, cmap=fire)

# Save the image
output_path = dir_path + "/concatenated.png"
img.to_pil().save(output_path)
print(f"Image saved as {output_path}.")

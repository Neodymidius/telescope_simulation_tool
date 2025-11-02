import sys
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as colors
import matplotlib.animation as animation


def sort_list(list_of_lines):
    """
    Now the 0th element of each line is the simulation time (an integer).
    Sort the hits in ascending order of simulation time.
    """
    return sorted(list_of_lines, key=lambda x: x[0])


def get_list_of_lines(path):
    """
    Read the file and extract the data.
    In the new file format, the 0th column is an integer simulation time,
    the original x and y coordinates are now in columns 1 and 2, and the
    previously used column (index 5) is now at index 6.
    """
    with open(path) as f:
        hits = []
        for line in f:
            parts = line.split()
            # Extract simulation time as int, x and y as floats, and extra value (if needed) from index 6.
            hit = [int(parts[0]), float(parts[1]), float(parts[2]), float(parts[6])]
            hits.append(hit)
    return hits


def get_x_y_lists(hits):
    """
    Since the simulation time is now at index 0,
    the x and y coordinates are at indices 1 and 2.
    """
    x = [hit[1] for hit in hits]
    y = [hit[2] for hit in hits]
    return x, y


def update(frame, sensor, rows, cols, sim_times, im, time_step):
    """
    Update the sensor array by adding photon hits that fall within the current simulation time window.

    Parameters:
      frame     - current frame number
      sensor    - the sensor array being updated
      rows, cols - precomputed sensor pixel indices for each hit
      sim_times - sorted array of simulation times for each hit
      im        - the AxesImage object to update
      time_step - the simulation time interval to process per frame
    """
    t_min = frame * time_step
    t_max = (frame + 1) * time_step

    # Find the index range for hits within the current simulation time window.
    start_idx = np.searchsorted(sim_times, t_min, side='left')
    end_idx = np.searchsorted(sim_times, t_max, side='right')

    # Stop updating if we've processed all hits.
    if start_idx >= len(sim_times):
        return im,

    # Increment the sensor pixel counts for all hits in this time window.
    np.add.at(sensor, (rows[start_idx:end_idx], cols[start_idx:end_idx]), 1)
    im.set_data(sensor)
    return im,


def main():
    # Get hit data from file.
    dir_path = sys.argv[1]
    hits = get_list_of_lines(dir_path)

    # Sort by simulation time (the 0th element).
    sorted_hits = sort_list(hits)

    # Extract x and y coordinates from the sorted hits.
    xs, ys = get_x_y_lists(sorted_hits)

    # Also extract simulation times (for timeline updates).
    sim_times = np.array([hit[0] for hit in sorted_hits])

    # Sensor settings.
    resolution = (3840, 3840)  # (width, height) in pixels
    size_pixel = 0.0075  # pixel size in mm

    # Precompute sensor pixel indices from the x,y coordinates.
    xs = np.array(xs)
    ys = np.array(ys)
    cols = np.floor(resolution[0] / 2 + xs / size_pixel).astype(int)
    rows = np.floor(resolution[1] / 2 - ys / size_pixel).astype(int)
    rows = np.clip(rows, 0, resolution[1] - 1)
    cols = np.clip(cols, 0, resolution[0] - 1)

    # Create an initially nearly-empty sensor.
    # A small baseline is added to avoid issues with the logarithmic scale.
    sensor = np.full((resolution[1], resolution[0]), 1e-5, dtype=float)

    # Define the physical extent of the sensor (in mm).
    physical_size = resolution[0] * size_pixel
    extent = [-physical_size / 2, physical_size / 2, -physical_size / 2, physical_size / 2]

    # Set up the figure and axis.
    fig, ax = plt.subplots(figsize=(8, 6))
    im = ax.imshow(sensor, origin='lower',
                   extent=extent,
                   cmap='hot',
                   norm=colors.LogNorm(vmin=1e-5, vmax=10000))
    cbar = plt.colorbar(im, ax=ax, label='Counts (log scale)')
    ax.set(xlim=[-15, 15], ylim=[-15, 15],
           xlabel='x [mm]', ylabel='y [mm]',
           title="Sensor Simulation: Accumulated Photon Hits")

    # Define simulation time increment per frame.
    # For instance, with a time_step of 100000 simulation units, we get about 1000 frames for 100e6 steps.
    time_step = 10000
    total_frames = int(np.ceil((sim_times[-1] + 1) / time_step))

    print("Total simulation time range: 0 to", sim_times[-1])
    print("Animating over", total_frames, "frames with a time step of", time_step, "simulation units per frame.")

    # Create the animation.
    ani = animation.FuncAnimation(
        fig=fig,
        func=update,
        frames=total_frames,
        fargs=(sensor, rows, cols, sim_times, im, time_step),
        interval=30,
        blit=True
    )

    # Optionally, save the animation.
    print("Saving animation to './line.gif'")
    ani.save(filename="./line.mp4", writer="ffmpeg")
    plt.show()


if __name__ == '__main__':
    main()

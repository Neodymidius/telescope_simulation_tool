import sys
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as colors

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
            # Extract simulation time as int, x and y as floats, and extra value from index 6.
           # if len(parts) != 27:
           #     continue
            hit = [int(parts[0]), float(parts[1]), float(parts[2])]#, float(parts[6])]
            hits.append(hit)
    return hits

def get_x_y_lists(hits: list):
    """
    Since the simulation time is now at index 0,
    the x and y coordinates are at indices 1 and 2.
    """
    small_hits = []
    for hit in hits:
        if (hit[1]**2 + hit[2]**2)**0.5 < 15:
            small_hits.append(hit)
    x = [hit[1] for hit in small_hits]
    y = [hit[2] for hit in small_hits]
    return x, y


def plot_error():
    import numpy as np
    from astropy.io import fits
    from astropy.wcs import WCS

    ################ INPUT
    hdulist = fits.open("erosita_psf_v3.1.fits")
    # hdulist = fits.open("/home/sixte/instruments.git/instruments/srg/tm1_2dpsf_190219v05.fits")

    histbins = np.logspace(0, 2, 50)

    w = WCS(hdulist[0].header)
    img = hdulist[0].data

    ################ WORK

    # get WCS positions for each pixel, in arcsec
    X_wcs = w.pixel_to_world(np.arange(0, img.shape[0]), np.zeros(img.shape[0]))[0].to("mm").value
    Y_wcs = w.pixel_to_world(np.zeros(img.shape[1]), np.arange(0, img.shape[1]))[1].to("mm").value

    # get X and Y indices
    X, Y = np.meshgrid(np.arange(0, len(X_wcs)), np.arange(0, len(Y_wcs)))

    # determine distance (in this case from (0,0)) for every pixel
    dists = np.sqrt(X_wcs[X] ** 2 + Y_wcs[Y] ** 2)

    # make a histogram of counts per radial bin
    (hist, bins) = np.histogram(dists, bins=histbins, weights=img)

    # calculate the area normalization for each radial bin
    hist_norm = 1 / (histbins[1:] ** 2 - histbins[:-1] ** 2) / np.pi;

    hist_normed = hist / np.sum(hist)

    return hist_normed * hist_norm


def main():
    # Get hit data from file.
    dir_path = sys.argv[1]
    hits = get_list_of_lines(dir_path)

    # Sort by simulation time (the 0th element).
    sorted_hits = sort_list(hits)

    # Extract x and y coordinates from the sorted hits.
    xs, ys = get_x_y_lists(sorted_hits)

    # Sensor settings.
    resolution = (384, 384)  # (width, height) in pixels
    size_pixel = 0.075      # pixel size in mm

    # Precompute sensor pixel indices from the x,y coordinates.
    xs = np.array(xs)
    ys = np.array(ys)
    cols = np.floor(resolution[0] / 2 + xs / size_pixel).astype(int)
    rows = np.floor(resolution[1] / 2 - ys / size_pixel).astype(int)
    rows = np.clip(rows, 0, resolution[1] - 1)
    cols = np.clip(cols, 0, resolution[0] - 1)

    # Create an initially nearly-empty sensor.
    # A small baseline is added to avoid issues with the logarithmic scale.
    sensor = np.full((resolution[1], resolution[0]), 0, dtype=float)

    # Increment sensor counts for all hits.
    np.add.at(sensor, (rows, cols), 1)




    histbins = np.logspace(0, 2, 50)
    # get X and Y indices
    #X, Y = np.meshgrid(np.arange(0, len(sensor[0])), np.arange(0, len(sensor[1])))

    # determine distance (in this case from (0,0)) for every pixel
    dists = np.sqrt(xs ** 2 + ys ** 2)

    # Set up the figure and axis.
    (hist, bins) = np.histogram(dists, bins=histbins)

    # calculate the area normalization for each radial bin
    hist_norm = 1 / (histbins[1:] ** 2 - histbins[:-1] ** 2) / np.pi;

    ## plotting
    import matplotlib.pyplot as plt
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("radius [mm]")
    plt.ylabel("counts/mm²")
    hist_normed = hist/np.sum(hist)
    plt.stairs(hist_normed * hist_norm, bins)
    plt.show()

    a = hist_normed * hist_norm
    b = plot_error()
    res = [[]]
    for i in range(len(a)):
        if b[i] != 0:
            res[0].append(a[i] / b[i])
        else:
            res[0].append(1)


    plt.xlabel("radius [mm]")
    plt.ylabel("ratio")
    plt.plot(res[0][:29])
    plt.show()
    import csv

    with open('out.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(res)


if __name__ == '__main__':
    main()

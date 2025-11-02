import numpy as np
from astropy.io import fits
from astropy.wcs import WCS

################ INPUT
hdulist = fits.open("erosita_psf_v3.1.fits")
#hdulist = fits.open("/home/sixte/instruments.git/instruments/srg/tm1_2dpsf_190219v05.fits")

histbins = np.logspace(0,2,50)

w = WCS(hdulist[0].header)
img = hdulist[0].data

################ WORK

# get WCS positions for each pixel, in arcsec
X_wcs = w.pixel_to_world(np.arange(0,img.shape[0]), np.zeros(img.shape[0]))[0].to("mm").value
Y_wcs = w.pixel_to_world(np.zeros(img.shape[1]), np.arange(0,img.shape[1]))[1].to("mm").value

# get X and Y indices
X,Y = np.meshgrid(np.arange(0,len(X_wcs)), np.arange(0,len(Y_wcs)))

# determine distance (in this case from (0,0)) for every pixel
dists = np.sqrt(X_wcs[X]**2 + Y_wcs[Y]**2)

# make a histogram of counts per radial bin
(hist,bins) = np.histogram(dists, bins=histbins, weights=img)

# calculate the area normalization for each radial bin
hist_norm = 1 / (histbins[1:]**2 - histbins[:-1]**2) / np.pi;

## plotting
import matplotlib.pyplot as plt
plt.xscale("log")
plt.yscale("log")
plt.xlabel("radius [mm]")
plt.ylabel("counts/mm²")
hist_normed = hist / np.sum(hist)
plt.stairs(hist_normed*hist_norm, bins)
plt.show()

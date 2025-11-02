import numpy as np
from astropy.io import fits
from astropy.wcs import WCS

################ INPUT
given_hdulist = fits.open("/home/neo/Desktop/master_plots/arf_MirrorOnAxis.fits")
my_hdulist = fits.open("/home/neo/Desktop/master_plots/arfcorr_on_axis_result.fits")
new_hdulist = fits.open("/home/neo/fau/astronomy/master/FINALTALK/arfcorr_on_axis_result.fits")


given_data = given_hdulist[1].data
my_data = my_hdulist[1].data
new_data = new_hdulist[1].data
################ WORK
idx = np.nanargmin(np.abs(given_data['ENERG_LO'] - 5))
closest_val = given_data['SPECRESP'][idx]
given_data_normed = given_data['SPECRESP']/closest_val


idx = np.nanargmin(np.abs(my_data['ENERG_LO'] - 5))
closest_val = my_data['SPECRESP'][idx]
my_data_normed = my_data['SPECRESP']/closest_val

idx = np.nanargmin(np.abs(new_data['ENERG_LO'] - 5))
closest_val = new_data['SPECRESP'][idx]
new_hdulist_normed = new_data['SPECRESP']/closest_val

## plotting

import matplotlib.pyplot as plt

rows, cols = 2, 1
fig, axs = plt.subplots(rows, cols, figsize=(cols * 12, rows * 4), constrained_layout=True)
axs = axs.ravel()
axs[0].set_xlabel("keV")
axs[0].set_ylabel("cm²/5keV")
axs[0].set_title("MPE ARF")
axs[0].plot(given_data['ENERG_LO'][:950],given_data_normed[:950], color="b", label="MPE ARF")
#axs[0].plot(my_data['ENERG_LO'],my_data_normed, color="g", label="Old ARF")
axs[0].plot(new_data['ENERG_LO'][:950],new_hdulist_normed[:950], color="r", label="Ray traced ARF")
axs[0].legend(loc="upper right")

#axs[1].set_xlabel("keV")
#axs[1].set_ylabel("cm²/5keV")
#axs[1].set_title("Raytracing ARF Data")
#axs[1].plot(my_data['ENERG_LO'],my_data_normed)

#axs[2].set_xlabel("keV")
#axs[2].set_ylabel("cm²/5keV")
#axs[2].set_title("Raytracing New ARF Data")
#axs[2].plot(my_data['ENERG_LO'],new_hdulist_normed)

axs[1].set_title("MPE/RT ARF")
axs[1].set_xlabel("keV")
#axs[1].plot(my_data['ENERG_LO'],given_data_normed/my_data_normed, label="Old ARF")
axs[1].plot(my_data['ENERG_LO'][:950],given_data_normed[:950]/new_hdulist_normed[:950], label="New ARF", color="r")
axs[1].legend(loc="upper right")




plt.show()
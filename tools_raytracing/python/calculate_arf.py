from astropy.io import fits
import matplotlib.pyplot as plt


arf_on_axis = fits.open("/userdata/data/reinmann/sixte/results/juli/sixte_tools/sixte_tools./arfcorr_saved.fits")

arf_constant = fits.open("/userdata/data/reinmann/sixte/software/LOCAL_INSTALL/sixte/share/sixte/instruments/srg/erosita/tm1_arf_rt.fits")

arf_sixte = fits.open("/userdata/data/reinmann/sixte/software/LOCAL_INSTALL/sixte/share/sixte/instruments/srg/erosita/tm1_arf_filter_000101v02.fits")

factor = arf_on_axis[1].data['SPECRESP'] / arf_constant[1].data['SPECRESP']

arf_sixte[1].data['SPECRESP'] = arf_sixte[1].data['SPECRESP'] / factor

arf_sixte.writeto("/userdata/data/reinmann/sixte/software/LOCAL_INSTALL/sixte/share/sixte/instruments/srg/erosita/tm1_arf_rt_adjusted.fits")

from astropy.io import fits
import matplotlib.pyplot as plt


arf_off_axis = fits.open("//userdata/data/reinmann/sixte/results/juli/sixte_tools/sixte_tools./arfcorr_off_axis.fits")
arf_on_axis = fits.open("/userdata/data/reinmann/sixte/results/juli/sixte_tools/sixte_tools./arfcorr_saved.fits")

#plt.plot(arf_off_axis[1].data['ENERG_LO'], arf_off_axis[1].data['SPECRESP'], label='off axis arf')
#plt.plot(arf_on_axis[1].data['ENERG_LO'], arf_on_axis[1].data['SPECRESP'], label='on axis arf')
arf_ratio = arf_off_axis[1].data['SPECRESP'][:] / arf_on_axis[1].data['SPECRESP'][:]
plt.plot(arf_off_axis[1].data['ENERG_LO'], arf_ratio, label='arf ratio')

vig_hdu = fits.open("/userdata/data/reinmann/sixte/software/LOCAL_INSTALL/sixte/share/sixte/instruments/srg/erosita/erosita_vignetting_tm2_v4.0.fits")

plt.plot(vig_hdu[1].data['ENERG_LO'][0,:], vig_hdu[1].data['VIGNET'][0,0,44,:], label='vignetting')
plt.legend()
plt.show()

from astropy.io import fits
import matplotlib.pyplot as plt

arf_list = []
for i in range(1, 101):
    arf_list.append(fits.open(f"/userdata/data/reinmann/sixte/results/juli/sixte_tools/sixte_tools./arf/arfcorr_on_axis_{i}.fits"))

for arf in arf_list[1:]:
    arf_list[0][1].data['SPECRESP'] += arf[1].data['SPECRESP']

arf_list[0][1].data['SPECRESP'] /= 100

arf_list[0].writeto("/userdata/data/reinmann/sixte/results/juli/sixte_tools/sixte_tools./arf/arfcorr_on_axis_result.fits")


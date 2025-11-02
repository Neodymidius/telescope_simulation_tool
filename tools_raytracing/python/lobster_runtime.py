import numpy as np
import matplotlib.pyplot as plt

# Photon counts
photons = np.array([1000, 10000, 100000, 1000000, 10000000])

# Runtime measurements in ms
times_3 = np.array([
    [0.393834, 0.418094, 0.420854, 0.412294],
    [2.73785, 2.8113, 2.81359, 2.75264],
    [24.0277, 24.0288, 23.789, 23.5089],
    [237.228, 233.297, 232.349, 231.779],
    [2367.13, 2344.77, 2321.11, 2325.58]
])

times_30 = np.array([
            [1.29908, 1.32914, 1.33043, 1.3015],
            [10.3631, 10.3708, 10.3988, 10.2979],
            [95.9171, 95.6743, 96.9262, 95.8875],
            [960.474, 963.434, 977.683, 960.769 ],
            [9512.67, 9575.83, 9682.77, 9521.14],
        ])

times_300 = np.array([
    [],
    [],
    [],
    [],
    []
])

# Compute mean and std for each photon count
def stats(times):
    return np.mean(times, axis=1), np.std(times, axis=1)

mean_3, std_3 = stats(times_3)
mean_30, std_30 = stats(times_30)
#mean_300, std_300 = stats(times_300)

# Plot
plt.figure(figsize=(10,6))
plt.errorbar(photons, mean_3, yerr=std_3, marker='o', label='Lobster-MPO')
plt.errorbar(photons, mean_30, yerr=std_30, marker='s', label='Wolter 30 Shells')
#plt.errorbar(photons, mean_300, yerr=std_300, marker='^', label='Mirror = 300')

plt.xscale('log')
plt.yscale('log')
plt.xlabel('Number of Photons')
plt.ylabel('Runtime (ms)')
plt.title('Runtime vs. Photon Count (with Std. Dev.)')
#plt.grid(True, which="both", linestyle="--")
plt.legend()
plt.tight_layout()
plt.show()

import numpy as np
import matplotlib.pyplot as plt

# ---- INPUT: put your data here ----
# photons must be shared across setups
photons = np.array([1000, 10000, 100000, 1000000, 10000000])

# Each array is shape (len(photons), 4) with the 4 measurements per point
data = {
 #  "old": {
 #       3: np.array([
 #           [6.35897, 6.42017, 6.42110, 6.38397],
 #           [47.6787, 48.0392, 47.0603, 47.3711],
#            [454.606, 458.822, 455.466, 458.392],
  #          [4515.14, 4531.59, 4574.22, 4519.33],
  #          [45382.5, 45220.7, 45315.9, 45450.4],
  #      ]),
  #      30: np.array([
  #          [49.1879, 41.4734, 40.9877, 42.7727],
  #          [387.837, 391.516, 388.425, 394.407],
  #          [3821.66, 3818.32, 3836.12, 3835.72],
  #          [38413.4, 38429.1, 38363.3, 38272.6],
  #          [383434, 384529, 384531, 387039],
  #      ]),
  #      300: np.array([
  #          [363.198, 356.317, 355.984, 355.770],
  #          [3604.80, 3570.75, 3579.57, 3569.76],
  #          [35725.7, 35784.1, 36039.6, 35679.6],
  #          [356571, 357196, 356147, 357460],
  #          [3567473, 3583510, 360086, 3562814],
  #      ]),
  #  },
    "new": {
        # <<< Fill your NEW measurements here, same structure as "old"
        3: np.array([
            [0.678898, 0.607935, 0.608376, 0.592805],
            [4.11038, 4.10458, 3.95799, 3.97661],
            [34.8516, 34.0725, 33.427, 33.481],
            [322.087, 321.478, 321.179, 321.718],
            [3238.05, 3208.84, 3222.38, 3218.33],
        ]),
        30: np.array([
            [1.29908, 1.32914, 1.33043, 1.3015],
            [10.3631, 10.3708, 10.3988, 10.2979],
            [95.9171, 95.6743, 96.9262, 95.8875],
            [960.474, 963.434, 977.683, 960.769 ],
            [9512.67, 9575.83, 9682.77, 9521.14],
        ]),
        300: np.array([
            [4.49889, 4.41328, 4.51369, 4.40489],
            [40.3856, 39.7469, 40.1609, 39.7165],
            [381.228, 375.332, 379.096, 375.854],
            [3809.38, 3738.45, 3794.39, 3746.23],
            [38022.4, 37403.3, 37848.2, 37467.0],
        ]),
    }
}
# -----------------------------------

def compute_stats(arr):
    """Return mean and std along axis=1 (per photon count)."""
    return np.mean(arr, axis=1), np.std(arr, axis=1)

def plot_runtime_overlay(photons, data):
    """
    One figure: overlay OLD vs NEW for each mirror count with error bars.
    Line style encodes setup (old/new); marker encodes mirror count.
    """
    markers = ['o', 's', '^', 'D', 'v', 'P', '*']
    mirror_values = sorted({m for setup in data.values() for m in setup.keys()})
    setup_styles = {"old": "-", "new": "-"}  # line styles only (no explicit colors)

    plt.figure(figsize=(8, 6))
    for si, (setup_name, mirrors) in enumerate(data.items()):
        for mi, m in enumerate(mirror_values):
            if m not in mirrors:
                continue
            means, stds = compute_stats(mirrors[m])
            label = f"#Wolter I mirrors={m}"
            plt.errorbar(
                photons, means, yerr=stds,
                marker=markers[mi % len(markers)],
                linestyle=setup_styles.get(setup_name, "-."),
                label=label
            )

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Photons', fontsize=12)
    plt.ylabel('Runtime (ms)', fontsize=12)
    plt.title('Runtime vs. Photons (mean ± std)', fontsize=14)
    #plt.grid(True, which='both', linestyle='--')
    plt.legend()
    plt.tight_layout()
    plt.show()

def plot_speedup(photons, data):
    """
    Second figure: ratio old/new means for mirror counts present in both setups.
    Values > 1 = new is faster; < 1 = new is slower.
    """
    if not {"old", "new"}.issubset(data.keys()):
        return  # Need both setups

    markers = ['o', 's', '^', 'D', 'v', 'P', '*']
    common_mirrors = sorted(set(data["old"].keys()).intersection(set(data["new"].keys())))
    if not common_mirrors:
        return

    plt.figure(figsize=(10, 6))
    for mi, m in enumerate(common_mirrors):
        old_mean, _ = compute_stats(data["old"][m])
        new_mean, _ = compute_stats(data["new"][m])
        # Protect against division by zero
        ratio = np.divide(old_mean, new_mean, out=np.full_like(old_mean, np.nan), where=new_mean!=0)
        plt.plot(
            photons, ratio,
            marker=markers[mi % len(markers)],
            linestyle='-',
            label=f"mirror={m}"
        )

    plt.axhline(1.0, linestyle='--')  # baseline
    plt.xscale('log')
    plt.xlabel('Number of Photons')
    plt.ylabel('Speedup (old / new)')
    plt.title('Speedup of NEW vs OLD (values > 1 mean NEW is faster)')
    # plt.grid(True, which='both', linestyle='--')
    plt.legend(title='Mirror count')
    plt.tight_layout()
    plt.show()

# ---- Run the plots ----
plot_runtime_overlay(photons, data)
plot_speedup(photons, data)

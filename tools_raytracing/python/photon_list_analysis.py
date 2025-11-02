import matplotlib.pyplot as plt

# Data
methods = ["PSF imaging", "Ray tracing imaging"]
photons_per_cpu_sec = [4693.885869565217, 3292.4946004319654]

# Create the bar chart
plt.figure(figsize=(6, 5))
bars = plt.bar(methods, photons_per_cpu_sec, color=["steelblue", "orange"])

# Y-axis label
plt.ylabel("Photons per CPU-sec", fontsize=12)
plt.title("Photon Throughput per Imaging Method", fontsize=14)

# Add horizontal line at y=200
plt.axhline(y=200, color='red', linestyle='--', linewidth=2, label="Count rate of medium bright\n (100 mCrab) source")

# Add value labels on top of bars
for bar in bars:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2, yval + 5, f"{yval:.1f}", ha='center', va='bottom', fontsize=10)

plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

import numpy as np
import matplotlib.pyplot as plt

# Read data from sizes.txt
data = np.loadtxt('sizes.txt')

# Separate pipeline and module data
pipeline_data = data[::2]  # Every alternate row starting from index 0
module_data = data[1::2]   # Every alternate row starting from index 1

labels = ["Proto",  "Proto + zlib",  "Proto + brotli", 
          "JSON",   "JSON + zlib",   "JSON + brotli", 
          "String", "String + zlib", "String + brotli"]

colors = ["red", "orangered", "darkred",    # Variants of red
          "blue", "royalblue", "darkblue",  # Variants of blue
          "green", "limegreen", "darkgreen"] # Variants of green

# Create a figure with two subplots side by side
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

for i in range(1, pipeline_data.shape[1]):  # Iterate over size values
    ax1.plot(pipeline_data[:, 0], pipeline_data[:, i], label=labels[i-1], color=colors[i-1])

for i in range(1, module_data.shape[1]):  # Iterate over size values
    ax2.plot(module_data[:, 0], module_data[:, i], label=labels[i-1], color=colors[i-1])

ax1.set_xlabel('Configuration cardinality')
ax1.set_ylabel('Size in bytes')
ax1.set_title('Pipeline definition configuration sizes')
ax1.set_ylim(0, 750)
ax1.legend()

ax2.set_xlabel('Configuration cardinality')
ax2.set_ylabel('Size in bytes')
ax2.set_title('Module parameter configuration sizes')
ax2.set_ylim(0, 750)
ax2.legend()

# Adjust layout to prevent overlap of labels
plt.tight_layout()

# Show the combined figure
plt.show()
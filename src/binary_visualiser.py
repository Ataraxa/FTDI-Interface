import numpy as np
import matplotlib.pyplot as plt

# 1. Read binary file as uint16 (assumes little-endian)
filename = 'data/recordings9.bin'
data = np.fromfile(filename, dtype='<i2')  # '<u2' = little-endian uint16

# 2. Convert to decimal (already done by numpy during load)
decimal_data = data.astype(np.int32)  # Optional: Handle any scaling here

# 3. Plot
plt.figure(figsize=(10, 4))
plt.plot(decimal_data, 'b-', linewidth=0.5)
plt.title("ADC Samples (4kHz)")
plt.xlabel("Sample Index")
plt.ylabel("Value")
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.show()
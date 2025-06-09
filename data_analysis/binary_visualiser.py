import numpy as np
import matplotlib.pyplot as plt

# 1. Read binary file as uint16 (assumes little-endian)
filename = 'data/recordings118.bin'  # Replace with your file path
with open(filename, 'rb') as f:
    raw_data = f.read()  # Read all bytes

data = np.frombuffer(raw_data, dtype='<i2')  # '<i2' = little-endian int16

# 2. Convert to decimal (already done by numpy during load)
decimal_data = data.astype(np.int32)  # Optional: Handle any scaling here
# f = lambda x: x/(2**15)*5
# data = f(decimal_data) 
# 3. Plot
plt.figure(figsize=(12, 6))
plt.plot(data, 
         '-',  # Solid line
         marker='o',  # Circle markers
         markersize=4,  # Adjust size as needed
         linewidth=1,  # Line thickness
         alpha=0.7,  # Slight transparency
         label='int16 Data')

plt.xlabel('Sample Index')
plt.ylabel('Value')
plt.title('int16 Data with Points Superimposed')
plt.grid(True, linestyle='--', alpha=0.5)  # Optional: dashed grid
plt.legend()
plt.tight_layout()  # Prevent label cutoff
plt.show()
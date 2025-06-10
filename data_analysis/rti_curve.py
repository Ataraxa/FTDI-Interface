import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import welch

import time_series_utils as tsu

# Generate a sample time series (replace with your data)
fs = 4000  # Sampling frequency (Hz)
t = np.arange(0, 1, 1/fs)  # Time vector
# x = np.sin(2 * np.pi * 50 * t) + 0.5 * np.random.randn(len(t))  # 50 Hz sine + noise
x = tsu.get_timeseries("data/recordings118.bin")

# Compute PSD using Welch's method
frequencies, psd = welch(x, fs, nperseg=1024, scaling='density')

# Plot PSD
plt.figure(figsize=(10, 5))
plt.semilogy(frequencies, psd, color='blue', lw=2)  # Log Y-axis (common for PSD)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Power Spectral Density (V²/Hz)')
plt.title('Power Spectral Density (Welch’s Method)')
plt.grid(True, linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()
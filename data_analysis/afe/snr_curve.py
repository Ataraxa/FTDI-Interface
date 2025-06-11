import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import welch

import time_series_utils as tsu
from linkage import freq_data

def main():
    # Generate a sample time series (replace with your data)
    fs = 4000  # Sampling frequency (Hz)
    t = np.arange(0, 2, 1/fs)  # Time vector
    
    snr_axis = []
    freq_axis = []
    for freq, fileNb in freq_data.items():
        recorded_signal = tsu.get_timeseries(f"data/recordings{fileNb}.bin")
        clean_signal = np.sin(2 * np.pi * freq * t) * 2
        noise = recorded_signal - clean_signal

        snr = 20*np.log10(tsu.get_amplitude(clean_signal)/tsu.get_amplitude(noise))
        snr_axis.append(snr)
        freq_axis.append(freq)


    plt.figure(figsize=(12, 6))
    plt.plot(freq_axis, snr_axis, 
            '-',  # Solid line
            marker='o',  # Circle markers
            markersize=4,  # Adjust size as needed
            linewidth=1,  # Line thickness
            alpha=0.7,  # Slight transparency
            )
    plt.xscale('log')  # Set x-axis to logarithmic scale (base 10)
    plt.xlabel('Frequency (Hz)')  # Update x-label to indicate log scale
    plt.ylabel("SNR (dB)")  # Adjust y-label as needed
    plt.title('SNR plot')
    plt.grid(True, linestyle='--', alpha=0.5, which='both')  # Show grid for both major and minor ticks
    plt.tight_layout()  # Prevent label cutoff
    plt.show()


if __name__ == "__main__":
    main()
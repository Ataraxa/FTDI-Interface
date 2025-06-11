import numpy as np 
import time_series_utils as tsu
import matplotlib.pyplot as plt

from linkage import freq_data

def main():
    freq_axis = []
    gain_axis = []

    for key,value in freq_data.items():
        filename = f"data/recordings{value}.bin"
        print(f"Frequency: {key} | Filename: {filename}", end=" | ")
        data = tsu.get_timeseries(filename)
        amplitude = tsu.get_amplitude(data)
        print(amplitude)
        gain_db = 20*np.log10(amplitude/0.002)

        freq_axis.append(key)
        gain_axis.append(gain_db)

    plt.figure(figsize=(12, 6))
    plt.plot(freq_axis, gain_axis, 
            '-',  # Solid line
            marker='o',  # Circle markers
            markersize=4,  # Adjust size as needed
            linewidth=1,  # Line thickness
            alpha=0.7,  # Slight transparency
            )

    plt.xscale('log')  # Set x-axis to logarithmic scale (base 10)
    plt.xlabel('Frequency (log scale)')  # Update x-label to indicate log scale
    plt.ylabel('Gain (dB or linear)')  # Adjust y-label as needed
    plt.title('Frequency Response (Log Scale)')
    plt.grid(True, linestyle='--', alpha=0.5, which='both')  # Show grid for both major and minor ticks
    plt.tight_layout()  # Prevent label cutoff
    plt.show()

if __name__ == "__main__":
    main()

        
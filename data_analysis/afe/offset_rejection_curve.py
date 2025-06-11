import numpy as np
import time_series_utils as tsu
import matplotlib.pyplot as plt
from linkage import offset_data

def main():
    output_offset = []
    input_offset = []
    for key, value in offset_data.items():
        filename = f"data/recordings{value}.bin"
        data = tsu.get_timeseries(filename)

        output_offset.append(np.mean(data)*1000)
        input_offset.append(key)

    plt.figure(figsize=(12, 6))
    plt.plot(input_offset, output_offset, 
            '-',  # Solid line
            marker='o',  # Circle markers
            markersize=4,  # Adjust size as needed
            linewidth=1,  # Line thickness
            alpha=0.7,  # Slight transparency
            )

    plt.xlabel('Input offset (mV)')  # Update x-label to indicate log scale
    plt.ylabel('Output offset (mV)')  # Adjust y-label as needed
    plt.title('Frequency Response (Log Scale)')
    plt.grid(True, linestyle='--', alpha=0.5, which='both')  # Show grid for both major and minor ticks
    plt.tight_layout()  # Prevent label cutoff
    plt.show()

if __name__ == "__main__":
    main()



        
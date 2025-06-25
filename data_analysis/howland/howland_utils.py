import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter1d

def detect_pulses_first_deriv(index="01", channel=1, threshold=0.001):
    # Load data and extract last two columns
    csv_file = f"data/ALL00{index}/F00{index}CH{channel}.csv"
    df = pd.read_csv(csv_file, header=None)

    xscale = float(df.iloc[11,1])
    time = df.iloc[:, -3].values.astype(float) # Second last column as time
    time = pd.to_numeric(df.iloc[:, -3], errors='coerce').values  # Safer conversion
    print(time.dtype)
    time  =time*1_000_000
    amplitude = df.iloc[:, -2].values  # Last column as amplitude

    
    print(xscale)

    # Calculate first derivative
    first_deriv = np.gradient(amplitude)
    smoothed_first_deriv = gaussian_filter1d(first_deriv, sigma=10)
    # plt.figure(figsize=(10, 6))
    # plt.plot(smoothed_first_deriv)
    # plt.show()

    # Some statistics about the data
    min_close = 0.9*np.min(gaussian_filter1d(amplitude, sigma=2))
    max_close = 0.9*np.max(gaussian_filter1d(amplitude, sigma=4))
    print(min_close)
    print(max_close)

    # Detect start and end point of pulses
    edges_time = []
    edges_ampl = []

    slew_time = []
    slew_ampl = []
    prev_dx = 0
    look_for_extrm = 0
    for i, dx in enumerate(np.abs(smoothed_first_deriv)):
        if prev_dx < threshold and dx > threshold:
            edges_time.append(time[i])
            edges_ampl.append(dx)

            # Hardcoded: bad practice!
            if len(edges_ampl) == 1:
                look_for_extrm = -1
            elif  len(edges_ampl) == 3:
                look_for_extrm = 1

        prev_dx = dx
        if (look_for_extrm == -1):
            if amplitude[i-1] > min_close and amplitude[i] < min_close:
                print("90% of minimum detected")
                slew_time.append(time[i])
                slew_ampl.append(amplitude[i])
                look_for_extrm = 0
        elif (look_for_extrm == 1):
            if amplitude[i-1] < max_close and amplitude[i] > max_close:
                print("90% of maximum detected")
                slew_time.append(time[i])
                slew_ampl.append(amplitude[i])
                look_for_extrm = 0

    edges_time = edges_time[:-1]
    edges_ampl = edges_ampl[:-1]

    # Calculate slew rate
    for i, _ in enumerate(slew_ampl):
        dv = slew_ampl[i] - edges_ampl[i*2];
        dt = slew_time[i] - edges_time[i*2];
        slew_rate = np.abs(dv/dt) # in V/µs !! 
        print(f"Slew rate calculated in {slew_rate} V/µs")
    
    # Plot results
    plt.figure(figsize=(10, 6))
    plt.plot(time, amplitude, label='Signal')
    plt.xlabel("Time (µs)")
    plt.ylabel("Amplitude (V)")
    plt.scatter(edges_time, edges_ampl, 
                c='r', s=50, label='Pulse Edges')
    plt.scatter(slew_time, slew_ampl, c='b', s=50, zorder=100)
    plt.legend()
    plt.show()
    
    # return filtered_crossings
    return 0

# Usage:
pulse_edges = detect_pulses_first_deriv(index="20",channel=1, threshold=0.01)

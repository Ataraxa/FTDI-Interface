import numpy as np
import matplotlib.pyplot as plt
import struct

def get_full_recordings(filename):
    with open(filename, 'rb') as f:
        raw_data = f.read()  # Read all bytes
    data = np.frombuffer(raw_data, dtype='<i2')  # '<i2' = little-endian int16
    decimal_data = data.astype(np.int32)  # Optional: Handle any scaling here
    return decimal_data

def read_fftw_log(filename, N):
    data = {}
    with open(filename, 'rb') as f:
        while True:
            # Read segment start (uint64)
            chunk = f.read(8)
            if len(chunk) < 8:
                break
            
            segment_start = struct.unpack('<Q', chunk)[0]
            if segment_start > 10_000:
                print(segment_start)

            # Read N real values
            real_chunk = f.read(N * 8)
            if len(real_chunk) < N * 8:
                break
            time_series = np.frombuffer(real_chunk, dtype=np.float64)
            
            # Read N complex values
            complex_chunk = f.read(N * 16)
            if len(complex_chunk) < N * 16:
                break
            fft_complex = np.frombuffer(complex_chunk, dtype=np.float64).reshape(-1, 2)
            fft_result = fft_complex[:, 0] + 1j * fft_complex[:, 1]

            # Read the corresponding stimualtion configuration
            config_chunk = f.read(6 * 2)
            config = np.frombuffer(config_chunk, dtype=np.int16)

            state_chunk = f.read(1)
            state = np.frombuffer(state_chunk, dtype=np.bool)
            
            data[segment_start] = {
                'time_series': time_series,
                'fft': fft_result,
                'config': config,
                'state': state
            }
    return data

def plot_fft_results(data, N, sample_rate=1.0, segment_index=0):
    """Plot time series and FFT results for one segment"""
    segment = data[segment_index]
    time_series = segment['time_series']
    fft_result = segment['fft']
    
    # Create time axis (assuming sample_rate is in Hz)
    time_axis = np.arange(len(time_series)) / sample_rate
    
    # Calculate FFT magnitude and frequency axis
    fft_magnitude = np.abs(fft_result)[:N//2]  # Take first half (real FFT symmetry)
    freq_axis = np.linspace(0, sample_rate/2, N//2)
    
    # Create figure with two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    # Plot time series
    ax1.plot(time_axis, time_series)
    ax1.set_title(f'Time Domain Signal (Segment starting at sample {segment["segment_start"]})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.grid(True)
    
    # Plot FFT magnitude
    ax2.plot(freq_axis, fft_magnitude)
    ax2.set_title('Frequency Domain (FFT Magnitude)')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('Magnitude')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.show()

if __name__=="__main__":
    # Usage example:
    N = 1024  # Same N as used in your C++ code
    sample_rate = 4_000
    fft_data = read_fftw_log('./output/fft_log.bin', N)
    print(fft_data)

    # Access the first segment's data
    # first_segment = fft_data[0]
    # print(f"Segment starts at sample: {first_segment['segment_start']}")
    # print(f"Input signal shape: {first_segment['input_signal'].shape}")
    # print(f"FFT output shape: {first_segment['fft_output'].shape}")

    # plot_fft_results(fft_data, N, sample_rate, segment_index=1000)
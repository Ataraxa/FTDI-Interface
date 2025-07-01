import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

class SimulationConfig():
    def __init__(self, dc, freq_ampl, fs, total_time):
        self.dc_offset = dc
        self.freq_ampl = freq_ampl
        self.sampling_rate = fs
        self.total_time = total_time

def simulate_LFP(cfg):
    t = np.linspace(0, cfg.total_time, int(cfg.sampling_rate * cfg.total_time))
    x = np.zeros((cfg.total_time*cfg.sampling_rate))
    # (len(t), cfg.dc_offset)  # Start with DC offset
    
    for freq, ampl in cfg.freq_ampl.items():
        x += ampl * np.sin(2 * np.pi * freq * t)
    return x, t

def add_noise(lfp, noise_std=1.0):
    wgn = noise_std * np.random.normal(0, 1, len(lfp))
    return lfp + wgn

def analyze_segment(signal, start_idx, downsample_factor, segment_length, fs):
    """
    Extract and downsample a segment of the signal for FFT analysis
    
    Args:
        signal: Input signal
        start_idx: Starting index (n)
        downsample_factor: Downsampling factor (m)
        segment_length: Length of segment after downsampling (N)
        fs: Original sampling rate
    """
    # Calculate how many samples we need from original signal
    original_segment_length = segment_length * downsample_factor
    end_idx = start_idx + original_segment_length
    
    if end_idx > len(signal):
        raise ValueError(f"Segment extends beyond signal length. Need {end_idx}, have {len(signal)}")
    
    # Extract segment
    segment = signal[start_idx:end_idx]
    
    # Downsample
    downsampled = segment[::downsample_factor]
    
    # Ensure we have exactly N samples
    downsampled = downsampled[:segment_length]
    
    # New sampling rate after downsampling
    fs_new = fs / downsample_factor
    
    return downsampled, fs_new

def compute_fft(signal, fs):
    """Compute FFT and return frequencies and magnitudes"""
    N = len(signal)
    fft_vals = fft(signal)
    freqs = fftfreq(N, 1/fs)
    
    # Take only positive frequencies
    pos_mask = freqs >= 0
    freqs_pos = freqs[pos_mask]
    fft_mag = np.abs(fft_vals[pos_mask])
    
    return freqs_pos, fft_mag

def plot_fft_comparison(clean_signal, noisy_signal, t, cfg, start_idx, downsample_factor, segment_length):
    """Plot FFT comparison between clean and noisy signals"""
    
    # Analyze noisy signal segment
    noisy_segment, fs_new = analyze_segment(noisy_signal, start_idx, downsample_factor, segment_length, cfg.sampling_rate)
    freqs_noisy, fft_noisy = compute_fft(noisy_segment, fs_new)
    
    # Analyze equivalent clean signal segment
    clean_segment, _ = analyze_segment(clean_signal, start_idx, 1, segment_length, cfg.sampling_rate)
    freqs_clean, fft_clean = compute_fft(clean_segment, fs_new)
    
    # Create time vector for segments
    t_segment = np.arange(segment_length) / fs_new
    
    # Create figure with subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
    
    # Plot time domain signals
    ax1.plot(t_segment, clean_segment, 'b-', label='Clean', alpha=0.8)
    ax1.set_title(f'Clean Signal Segment (N={segment_length}, m={downsample_factor})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    ax2.plot(t_segment, noisy_segment, 'r-', label='Noisy', alpha=0.8)
    ax2.set_title(f'Noisy Signal Segment (N={segment_length}, m={downsample_factor})')
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Amplitude')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # Plot FFT magnitude
    ax3.semilogy(freqs_clean, fft_clean, 'b-', label='Clean FFT', alpha=0.8)
    ax3.set_title('FFT - Clean Signal')
    ax3.set_xlabel('Frequency (Hz)')
    ax3.set_ylabel('Magnitude')
    ax3.grid(True, alpha=0.3)
    ax3.legend()
    ax3.set_xlim(0, fs_new/2)  # Nyquist frequency
    
    ax4.semilogy(freqs_noisy, fft_noisy, 'r-', label='Noisy FFT', alpha=0.8)
    ax4.set_title('FFT - Noisy Signal')
    ax4.set_xlabel('Frequency (Hz)')
    ax4.set_ylabel('Magnitude')
    ax4.grid(True, alpha=0.3)
    ax4.legend()
    ax4.set_xlim(0, fs_new/2)  # Nyquist frequency
    
    plt.tight_layout()
    
    # Print analysis info
    print(f"Analysis Parameters:")
    print(f"  Start index (n): {start_idx}")
    print(f"  Downsample factor (m): {downsample_factor}")
    print(f"  Segment length (N): {segment_length}")
    print(f"  Original sampling rate: {cfg.sampling_rate} Hz")
    print(f"  New sampling rate: {fs_new} Hz")
    print(f"  Frequency resolution: {fs_new/segment_length:.2f} Hz")
    print(f"  Nyquist frequency: {fs_new/2} Hz")
    
    return fig

if __name__ == "__main__":
    # Set random seed for reproducibility
    np.random.seed(42)
    
    # Configuration
    dc_offset = 1  # DC offset in arbitrary units
    freq_ampl = {
        0.5: 0.1,   # 0.5 Hz component
        20: 2,      # 20 Hz component  
        25: 0.2,
        30: 1.4,   
        40: 2,
        90: 2.4,
        100: 0.9,
        150: 1.8
    }
    fs = 100_000  # 100 kHz sampling rate
    total_time = 10  # 10 seconds
    
    # Create configuration
    sim_config = SimulationConfig(dc_offset, freq_ampl, fs, total_time)
    
    # Generate signals
    clean_lfp, t = simulate_LFP(sim_config)
    noisy_lfp = add_noise(clean_lfp, noise_std=2)
    
    # Analysis parameters
    n = 50000        # Start index
    m = 100          # Downsample factor (100 kHz -> 1 kHz)
    N = 1024         # Segment length after downsampling
    
    # Plot comparison
    fig = plot_fft_comparison(clean_lfp, noisy_lfp, t, sim_config, n, m, N)
    plt.show()
    
    # Optional: Save signals as binary files for your C++ GUI
    # Convert to int16 for binary storage
    clean_int16 = (clean_lfp * 1000).astype(np.int16)  # Scale and convert
    noisy_int16 = (noisy_lfp * 1000).astype(np.int16)
    
    clean_int16.tofile('clean_data.bin')
    noisy_int16.tofile('noisy_data.bin')
    print(f"\nSaved binary files:")
    print(f"  clean_data.bin: {len(clean_int16)} samples")
    print(f"  noisy_data.bin: {len(noisy_int16)} samples")
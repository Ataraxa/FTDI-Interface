import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import bisect

# === IMPORT TES FONCTIONS ===
from fft_readings import get_full_recordings, read_fftw_log  # adapte le nom
# Supposons que ces fonctions existent déjà dans ton fichier

# === PARAMÈTRES ===
N = 1024
sample_rate = 4000
fft_data = read_fftw_log('./output/fft_log.bin', N)
full_signal = get_full_recordings('./output/output_data.bin')
segment_starts = sorted(fft_data.keys())
TOLERANCE_SAMPLES = 10  # or whatever fits your data resolution

class FFTViewer(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("FFT Viewer")

        # === Figures matplotlib ===
        self.fig = Figure()
        self.ax_main = self.fig.add_subplot(2, 1, 1)
        self.ax_zoom = self.fig.add_subplot(2, 2, 3)
        self.ax_fft = self.fig.add_subplot(2, 2, 4)

        self.canvas = FigureCanvasTkAgg(self.fig, master=self)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        self.update()  # Update geometry info
        self.minsize(self.winfo_width(), self.winfo_height())
        self.geometry(f"{self.winfo_width()}x{self.winfo_height()}")

        self.segment_lines = []

        self.plot_main_signal()

        self.canvas.mpl_connect("button_press_event", self.on_click)

    def plot_main_signal(self):
        time = np.arange(len(full_signal)) / sample_rate
        self.ax_main.clear()
        self.ax_main.plot(time, full_signal, 
                        color=(0.4, 0.5, 0.8),
                        linewidth=0.7,
                        alpha=0.4)
        self.ax_main.set_title("Full Signal")
        self.ax_main.set_ylabel("Amplitude")
        self.ax_main.set_xlabel("Time (s)")

        # Draw segment indicators
        for key in fft_data.keys():
            # print(key)
            self.ax_main.scatter(time[key], full_signal[key],
                                color=(1., 0., 0.),
                                marker='+')
        self.canvas.draw()

    def find_nearest_segment(self, clicked_sample):
        # bisect.bisect_left returns insertion point
        pos = bisect.bisect_left(segment_starts, clicked_sample)
        if pos == 0:
            return segment_starts[0]
        if pos == len(segment_starts):
            return segment_starts[-1]
        
        before = segment_starts[pos - 1]
        after = segment_starts[pos]
        
        if abs(clicked_sample - before) < abs(after - clicked_sample):
            return before
        else:
            return after    

    def on_click(self, event):
        if event.inaxes != self.ax_main:
            return
        clicked_time = event.xdata
        clicked_sample = int(clicked_time * sample_rate)
        nearest = self.find_nearest_segment(clicked_sample)
        if abs(clicked_sample - nearest) <= TOLERANCE_SAMPLES:
            self.plot_zoom_and_fft(fft_data[nearest])
            print(fft_data[nearest]['config'], fft_data[nearest]['state'])
        else:
            print("Click too far from any segment, ignoring.")


    def plot_zoom_and_fft(self, segment):
        self.ax_zoom.clear()
        self.ax_fft.clear()

        # Time series
        t = np.arange(N) / sample_rate
        self.ax_zoom.plot(t, segment['time_series'], color='green')
        self.ax_zoom.set_title("Zoomed Time Signal")
        self.ax_zoom.set_xlabel("Time (s)")

        # FFT
        freqs = np.linspace(0, sample_rate/2, N//2)
        fft_mag = np.abs(segment['fft'])[:N//2]
        self.ax_fft.plot(freqs, fft_mag, color='red')
        self.ax_fft.set_title("FFT Magnitude")
        self.ax_fft.set_xlabel("Frequency (Hz)")

        self.canvas.draw()

if __name__ == "__main__":
    app = FFTViewer()
    app.mainloop()

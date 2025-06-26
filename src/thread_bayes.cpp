#define _USE_MATH_DEFINES

#include <fstream>
#include <iostream>
#include <cmath>
#include <math.h>

#include "../include/thread_bayes.h"
#include "../include/fftw3.h"

void thread_bayes(ThreadSafeBuffer& memoryBuffer, SharedConfig& config)
{   
    // ------------------------------------------------------------------
    //    Variables related to getting and storing data 
    // ------------------------------------------------------------------
    std::vector<uint16_t> fileBuffer;
    constexpr size_t min_batch_size = 100;
    constexpr size_t flush_threshold_bytes = 1 * 1024 * 1024; // 1 MB
    constexpr size_t flush_threshold_elements = flush_threshold_bytes / sizeof(uint16_t);

    // ------------------------------------------------------------------
    //    Variables related to Fourier transform
    // ------------------------------------------------------------------
    int N = 1024; // Size of FFT array

    //Allocate memory to the FFTW objects
    fftw_complex *in, *out;
    fftw_plan p;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_MEASURE);

    // Initialise to dummy data
    for (int i = 0; i < N; ++i) 
    {
        double value = std::sin(2 * M_PI * i / N); // or any other signal
        in[i][0] = value; // real part
        in[i][1] = 0.0;   // imaginary part
    }


    std::ofstream outfile("output/output_data.bin", std::ios::binary | std::ios::out);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file.\n";
        return;
    }

    // ------------------------------------------------------------------
    //    MAIN LOOP
    // ------------------------------------------------------------------
    while (!memoryBuffer.shouldStop() || !memoryBuffer.getData(1).empty()) {
        auto data = memoryBuffer.getData(min_batch_size);

        // ------------------------------------------------------------------
        //    Data storage
        // ------------------------------------------------------------------
        fileBuffer.insert(fileBuffer.end(), data.begin(), data.end());

        // Flush to disk if the buffer exceeds the threshold
        if (fileBuffer.size() >= flush_threshold_elements) {
            outfile.write(reinterpret_cast<char*>(fileBuffer.data()), fileBuffer.size() * sizeof(uint16_t));
            fileBuffer.clear();
        }

        // ------------------------------------------------------------------
        //    Data processing
        // ------------------------------------------------------------------
        // fftw_execute(p);
    }

    // Final flush for remaining data
    if (!fileBuffer.empty()) {
        outfile.write(reinterpret_cast<char*>(fileBuffer.data()), fileBuffer.size() * sizeof(uint16_t));
    }
    outfile.close();
    std::cout << "Consumer finished writing data.\n";

    // Destroy the FFTW objects
    fftw_free(in); fftw_free(out);
    fftw_destroy_plan(p);
}
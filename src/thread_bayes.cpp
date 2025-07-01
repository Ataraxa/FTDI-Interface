#define _USE_MATH_DEFINES

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <math.h>
#include <cstdio>
#include <vector>
#include <span>

#include "../include/thread_bayes.h"
#include "../include/fftw3.h"
#include "../include/SPI_utils.h"
#include "../include/sharedConfig.h"

void thread_bayes(ThreadSafeBuffer& memoryBuffer, SharedConfigDBS& sharedStimConfig)
{      
    printf("Signal processing thread started!\n");
    // ------------------------------------------------------------------
    //    Variables related to getting and storing data 
    // ------------------------------------------------------------------
    std::vector<int16_t> fileBuffer;
    constexpr size_t min_batch_size = 1;
    constexpr size_t flush_threshold_bytes = 1 * 1024 * 1024; // 1 MB
    constexpr size_t flush_threshold_elements = flush_threshold_bytes / sizeof(uint16_t);
    size_t head = 0; // pointer of circular buffer

    // ------------------------------------------------------------------
    //    Variables related to Fourier transform
    // ------------------------------------------------------------------
    int N = 1024; // Size of FFT array
    int threshold = 900;
    //Allocate memory to the FFTW objects
    fftw_complex *in, *out;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p;
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_MEASURE);

    // Initialise to dummy data
    for (int i = 0; i < N; i++) 
    {
        double value = std::sin(2 * M_PI * 20 * i / N); // or any other signal
        in[i][0] = value; // real part
        in[i][1] = 0.0;   // imaginary part
    }

    // ------------------------------------------------------------------
    //    File logging
    // ------------------------------------------------------------------
    // Open file for logging the full recorded time series
    std::ofstream outfile("output/output_data.bin", std::ios::binary | std::ios::out);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open output file.\n";
        return;
    }

    // Open file for logging FFT results
    std::ofstream fftlog("output/fft_log.bin", std::ios::binary | std::ios::out);
    if (!fftlog.is_open()) {
        std::cerr << "Failed to open FFT log file.\n";
        return;
    }
    uint64_t sample_index = 0; // Total samples written to raw file so far
    uint64_t fftw_segment_start = 0;

    // ------------------------------------------------------------------
    //    MAIN LOOP
    // ------------------------------------------------------------------
    while (!memoryBuffer.shouldStop() || !memoryBuffer.isEmpty()) {
        auto data = memoryBuffer.getData();

        // ------------------------------------------------------------------
        //    Data processing
        // ------------------------------------------------------------------
        // Fill the FFTW input vector with converted variables
        std::cout << "Receved data: " << data.size() << std::endl;
        if(data.size() >= N)
        {
            for(size_t i=0; i < N; i++)
            {
                in[i][0] = word2volt(data[i]);
                in[i][1] = 0.0;
            }
        } else {
            for(size_t i=0; i < N-data.size(); i++)
            {
                in[i][0] = in[i+data.size()][0];
                in[i][1] = 0.0;
            }
            for(size_t i=0; i < data.size(); i++)
            {
                in[i+N-data.size()][0] = word2volt(data[i]);
                in[i+N-data.size()][1] = 0.0;
            }
        }

        // Store the data
        fileBuffer.insert(fileBuffer.end(), data.begin(), data.end());

        // Update sample index
        sample_index += data.size();


        // Perform FFT
        fftw_execute(p);

        // Calculate frequency of with maximum power
        int max_index = 0;
        double max_frequency = 0.0;
        double max_magnitude = 0.0;

        for (int i = 0; i < 512; i++) {
            double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
            if (magnitude > max_magnitude) {
                max_magnitude = magnitude;
                max_index = i;
            }
        }
        max_frequency = max_index * 4000/N;
        
        // Adaptive control 
        if(max_magnitude > threshold) 
        {
            sharedStimConfig.toggleState(false);
        } else {
            sharedStimConfig.toggleState(false);
        }

        printf("Peak at index %d (%.1f Hz) with magnitude %f\n", max_index, max_frequency, max_magnitude);

        // Log FFT input and output
        if(sample_index > N)
        {
            fftw_segment_start = sample_index-N;
            fftlog.write(reinterpret_cast<char*>(&fftw_segment_start), sizeof(uint64_t));
            for (int i = 0; i < N; ++i)
                fftlog.write(reinterpret_cast<char*>(&in[i][0]), sizeof(double)); // only real part
            fftlog.write(reinterpret_cast<char*>(out), sizeof(fftw_complex) * N);
            fftlog.write(reinterpret_cast<char*>(&sharedStimConfig.data), sizeof(int16_t) * 6);
            fftlog.write(reinterpret_cast<char*>(&sharedStimConfig.state), sizeof(bool));
        } 

        // Flush recorded data to file when necessary
        if (fileBuffer.size() >= flush_threshold_elements) {
            outfile.write(reinterpret_cast<char*>(fileBuffer.data()), fileBuffer.size() * sizeof(uint16_t));
            fileBuffer.erase(fileBuffer.begin(), fileBuffer.end() - N);
        }
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
#pragma once
#include <vector>
#include "rpi_ws281x/ws2811.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <deque>
#include <complex>
#include "RGBColorPalette.h"
#include "parameters.h"

class WaveformVisualizer {
public:
    WaveformVisualizer() : history_buffer_(LED_COUNT / 2, 0) {}

    ~WaveformVisualizer() {}

    void CalculateWaveform(const std::vector<std::complex<double>> &fft_data);
    void CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette);

private:
    float vu_final_smoothed = 0.0f;

    // Buffer for the outward ripple effect
    std::vector<uint8_t> history_buffer_;
};


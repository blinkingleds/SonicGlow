#pragma once
#include <vector>
#include "rpi_ws281x/ws2811.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <deque> // Include deque for the history buffer
#include "RGBColorPalette.h"
#include "parameters.h" // Make sure LED_COUNT is available

class WaveformVisualizer {
public:
    WaveformVisualizer() : history_buffer_(LED_COUNT / 2, 0) {}

    ~WaveformVisualizer() {}

    void CalculateWaveform(const std::array<float, 512UL> &audio_samples);
    void CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette);

private:
    static constexpr float max_amplitude_smoothing_factor = 0.02f;

    float vu_final_smoothed = 0.0f;
    const float final_VU_smoothing_factor = 0.9f;

    // Buffer for the outward ripple effect
    std::vector<uint8_t> history_buffer_;
};


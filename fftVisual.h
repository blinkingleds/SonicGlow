#pragma once

#include <vector>
#include <complex>
#include <cstdint>
#include "rpi_ws281x/ws2811.h"
#include "RGBColorPalette.h"
#include "parameters.h"


class FFTVisualizer {
public:
    FFTVisualizer(size_t fft_size, int sample_rate);
    void update(const std::vector<std::complex<double>>& fft_data);
    void CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette);
    // Provide LED levels (0..255), leftmost is 0 Hz
    const std::vector<uint8_t>& ledLevels() const { return led_levels_; }

private:
    const int num_leds_ = LED_COUNT;
    const size_t fft_size_;
    const int sample_rate_;

    std::vector<double> smoothed_magnitudes_;
    std::vector<double> peak_magnitudes_;
    std::vector<uint8_t> led_levels_;


    static constexpr double SMOOTHING_FACTOR = 0.5;  // Lowered for faster reaction
    static constexpr double PEAK_DECAY_RATE = 0.96;   // Lowered for faster peak decay
};
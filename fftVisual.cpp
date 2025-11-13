#include "fftVisual.h"
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include <algorithm>
#include <numeric> 
#include "parameters.h"

FFTVisualizer:: FFTVisualizer (size_t fft_size, int sample_rate)
        : fft_size_(fft_size),
          sample_rate_(sample_rate),
          smoothed_magnitudes_(num_leds_, 0.0),
          peak_magnitudes_(num_leds_, 0.0),
          led_levels_(num_leds_, 0)
        {

        }

void FFTVisualizer::update(const std::vector<std::complex<double>>& fft_data) {
        const int useful_bins = fft_size_ / 2; // 0 Hz to Nyquist
        const int bins_per_led = useful_bins / num_leds_;

        if (bins_per_led < 1) {
            std::cout << "Error: Not enough frequency bins to map to every LED." << std::endl;
            return;
        }

        std::vector<double> current_magnitudes(num_leds_);

        // Average magnitudes for each LED from 0 Hz upward (leftmost LED is 0 Hz)
        for (int i = 0; i < num_leds_; ++i) {
            double avg_magnitude = 0.0;
            int start_bin = i * bins_per_led;
            int end_bin = start_bin + bins_per_led;
            if (end_bin > useful_bins) end_bin = useful_bins;

            for (int k = start_bin; k < end_bin; ++k) {
                avg_magnitude += std::abs(fft_data[k]);
            }
            current_magnitudes[i] = (end_bin > start_bin) ? (avg_magnitude / (end_bin - start_bin)) : 0.0;
        }

        // Gain then smooth and compress to LED levels 0..255
        const double input_gain = 1.0; // tune overall sensitivity
        const double compress_k = 1.0; // higher -> brighter for same magnitude
        const double gravity = 0.67; // Value < 1.0. Closer to 1.0 is slower falloff.

        for (int i = 0; i < num_leds_; ++i) {
            double scaled_mag = current_magnitudes[i] * input_gain;

            // optional peak hold maintenance
            if (scaled_mag > peak_magnitudes_[i]) {
                peak_magnitudes_[i] = scaled_mag;
            } else {
                peak_magnitudes_[i] *= PEAK_DECAY_RATE;
            }

            // Apply gravity. If new magnitude is lower, fall slowly. Otherwise, jump up.
            if (scaled_mag < smoothed_magnitudes_[i]) {
                smoothed_magnitudes_[i] *= gravity;
            } else {
                smoothed_magnitudes_[i] = scaled_mag;
            }

            // soft-knee compression to map to 0..255 without AGC
            double norm = 1.0 - std::exp(-compress_k * std::max(0.0, smoothed_magnitudes_[i]));
            int level = static_cast<int>(std::round(norm * 255.0));
            if (level < 0) level = 0;
            if (level > 255) level = 255;
            led_levels_[i] = static_cast<uint8_t>(level);
        }
}

void FFTVisualizer::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette) {
    // Draw each LED from left to right (0 Hz to Nyquist)
    for (int i = 0; i < num_leds_; ++i) {
        // The brightness is the calculated level for this LED's frequency bin
        uint8_t linear_brightness = led_levels_[i];
        
        // The color is determined by the LED's physical position on the strip
        float pos_normalized = static_cast<float>(i) / (float)num_leds_;
        
        ws2811.channel[0].leds[i] = getGradientColor(pos_normalized, palette, linear_brightness);
    }
}

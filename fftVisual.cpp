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

        std::vector<double> current_magnitudes(num_leds_);


        int start_led = 0;
        // Calculate the brightness levels for each LED based based on the FFT data and the defined Frequency bands.
        for (int band = 0; band < FREQUENCY_BANDS.size(); ++band) {
            int start_freq;
            int end_freq = FREQUENCY_BANDS[band][0];
            int band_width = FREQUENCY_BANDS[band][1];

            if (band == 0)
            {
                start_freq = 0;
            }
            else
            {
                start_freq = FREQUENCY_BANDS[band - 1][0];  
            }

            int band_start_bin = static_cast<int>((static_cast<double>(start_freq) / sample_rate_) * fft_size_);
            int band_end_bin = static_cast<int>((static_cast<double>(end_freq) / sample_rate_) * fft_size_);
            int end_led = start_led + (band_width/100.0f * num_leds_) - 1;
             

            for (size_t i = 0; i < end_led - start_led; ++i) {
                double pos = double(i) * (band_end_bin-band_start_bin - 1) / (end_led - start_led - 1);

                size_t left  = floor(pos);
                size_t right = ceil(pos);

                double t = pos - left;

                if (left == right) {
                    current_magnitudes[start_led + i] = std::abs(fft_data[band_start_bin+left]);
                } else {
                    current_magnitudes[start_led + i] = std::abs(fft_data[band_start_bin+left]) * (1.0 - t) + std::abs(fft_data[band_start_bin+right]) * t;
                }
            }
            start_led = end_led + 1;
        }

        for (int i = 0; i < num_leds_; ++i) {
            // Apply a simple gain to the raw magnitude.
            double scaled_mag = current_magnitudes[i] * FFT_VISUAL_OVERALL_GAIN;

            // apply addtition gain to the leftmost leds 
            if ( i > (num_leds_/1.5) ) {
                double freq_ratio = static_cast<double>(i) / static_cast<double>(num_leds_);
                double additional_gain = 1.0f + (FFT_VISUAL_ADDITION_GAIN_HIGH_FREQ - 1.0f) * freq_ratio;
                scaled_mag *= additional_gain;
            }

            // 3. Apply gravity. If the new magnitude is lower, fall slowly. Otherwise, jump up instantly.
            if (scaled_mag < smoothed_magnitudes_[i]) {
                smoothed_magnitudes_[i] *= FFT_VISUAL_GRAVITY;
            } else {
                smoothed_magnitudes_[i] = scaled_mag;
            }

            // Clamp the final value to the [0.0, 1.0] range before converting to LED level.
            double final_level = std::max(0.0, std::min(1.0, smoothed_magnitudes_[i]));

            led_levels_[i] = static_cast<uint8_t>(final_level * 255.0);
        }
    }


void FFTVisualizer::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette) {
    // Draw each LED from left to right (0 Hz to Nyquist)
    for (int i = 0; i < num_leds_; ++i) {
        // The brightness is the calculated level for this LED's frequency bin
        uint8_t linear_brightness = led_levels_[i];

        if (led_levels_[i] < 12) {
            linear_brightness = 0; // Noise gate
        }

        // The color is determined by the LED's physical position on the strip
        float pos_normalized = static_cast<float>(i) / (float)num_leds_;
        
        ws2811.channel[0].leds[i] = getGradientColor(pos_normalized, palette, linear_brightness);
    }
}

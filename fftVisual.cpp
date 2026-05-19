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


constexpr std::array<float, 256> NOISE_FLOOR = {
    0.081,   0.33,    0.57,    0.29,    0.056,   0.078,   0.087,   0.036,   0.036,   0.046,
    0.035,   0.026,   0.017,   0.016,   0.015,   0.01,    0.0092,  0.013,   0.011,   0.01,
    0.0079,  0.008,   0.0054,  0.0045,  0.0046,  0.0057,  0.0042,  0.0046,  0.004,   0.0056,
    0.0039,  0.0038,  0.0044,  0.0048,  0.0032,  0.0028,  0.0032,  0.0035,  0.0038,  0.0026,
    0.003,   0.0027,  0.0029,  0.0029,  0.0037,  0.0031,  0.0024,  0.0026,  0.0029,  0.0022,
    0.0018,  0.0023,  0.0023,  0.0017,  0.0013,  0.0015,  0.0015,  0.0017,  0.0014,  0.0018,
    0.0016,  0.0016,  0.0015,  0.0016,  0.0015,  0.0015,  0.0016,  0.0018,  0.0018,  0.0016,
    0.0017,  0.0017,  0.0017,  0.0016,  0.0016,  0.0017,  0.0014,  0.0013,  0.0012,  0.0016,
    0.0019,  0.0014,  0.0011,  0.0012,  0.0012,  0.0015,  0.0018,  0.0015,  0.0012,  0.0013,
    0.0014,  0.0013,  0.0011,  0.0011,  0.0014,  0.0015,  0.0015,  0.0015,  0.0013,  0.0012,
    0.0012,  0.0012,  0.0013,  0.0012,  0.0012,  0.0014,  0.0012,  0.0011,  0.0011,  0.0013,
    0.0014,  0.0012,  0.001,   0.001,   0.0011,  0.0013,  0.0012,  0.0011,  0.001,   0.0012,
    0.0012,  0.0013,  0.00091, 0.0013,  0.0013,  0.0014,  0.0013,  0.0012,  0.0013,  0.0014,
    0.0015,  0.0013,  0.0012,  0.0012,  0.0012,  0.0011,  0.0012,  0.0012,  0.0012,  0.0013,
    0.0012,  0.001,   0.00093, 0.0011,  0.0011,  0.0011,  0.0011,  0.0014,  0.0011,  0.0011,
    0.0012,  0.0012,  0.0013,  0.0013,  0.0011,  0.001,   0.0012,  0.00091, 0.00085, 0.00096,
    0.00087, 0.00088, 0.00093, 0.00095, 0.00092, 0.00088, 0.0011,  0.00098, 0.00097, 0.00092,
    0.0011,  0.0011,  0.0012,  0.0012,  0.0011,  0.0011,  0.00095, 0.001,   0.0012,  0.0012,
    0.001,   0.001,   0.0011,  0.0011,  0.001,   0.00086, 0.001,   0.00072, 0.00097, 0.00096,
    0.0011,  0.0011,  0.00093, 0.0011,  0.0012,  0.0012,  0.0011,  0.001,   0.0009,  0.00074,
    0.00077, 0.00095, 0.0013,  0.0013,  0.0011,  0.0012,  0.0011,  0.0011,  0.0012,  0.00094,
    0.001,   0.0011,  0.0011,  0.0011,  0.00092, 0.0011,  0.0013,  0.0013,  0.0011,  0.001,
    0.00092, 0.0011,  0.0011,  0.00092, 0.00087, 0.00094, 0.00081, 0.00081, 0.00098, 0.0009,
    0.0009,  0.00083, 0.00085, 0.0012,  0.0011,  0.00086, 0.001,   0.00099, 0.00091, 0.00082,
    0.00082, 0.00088, 0.00082, 0.00084, 0.00079, 0.00075, 0.00065, 0.00076, 0.00088, 0.0008,
    0.00067, 0.00058, 0.00058, 0.00068, 0.00078, 0.00082
};


FFTVisualizer:: FFTVisualizer (size_t fft_size, int sample_rate)
        : fft_size_(fft_size),
          sample_rate_(sample_rate),
          smoothed_magnitudes_(num_leds_, 0.0),
          led_levels_(num_leds_, 0),
          raw_led_magnitudes(LED_COUNT, 0.0f),
          fft_magnitudes(fft_size_ / 2, 0.0f)
        {

        }

void FFTVisualizer::update(const std::vector<std::complex<double>>& fft_data) {

        const int useful_bins = fft_size_ / 2; // 0 Hz to Nyquist
        const int bins_per_led = useful_bins / num_leds_;

        //convert fft_data from complex to magnitude
        for (int i = 0; i < useful_bins; ++i) {
            fft_magnitudes[i] = std::abs(fft_data[i]);
        }

        //subtract noise floor
        for (int i = 0; i < useful_bins; ++i) {
            fft_magnitudes[i] = std::max(0.0f, fft_magnitudes[i] - NOISE_FLOOR[i]);
        }



        // Calculate the brightness levels for each LED based based on the FFT data and the defined Frequency bands.
        int start_led = 0;
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
            int end_led = start_led + (band_width/100.0f * num_leds_);
            
            // Clamp to ensure we don't exceed LED count
            if (end_led > num_leds_) end_led = num_leds_;
            
            int num_leds_in_band = end_led - start_led;
            
            for (int i = 0; i < num_leds_in_band; ++i) {
                double pos = double(i) * (band_end_bin - band_start_bin - 1) / std::max(1, num_leds_in_band - 1);

                size_t left  = floor(pos);
                size_t right = ceil(pos);

                double t = pos - left;

                if (left == right) {
                    raw_led_magnitudes[start_led + i] = fft_magnitudes[band_start_bin + left];
                } else {
                    raw_led_magnitudes[start_led + i] = fft_magnitudes[band_start_bin + left] * (1.0 - t) + fft_magnitudes[band_start_bin + right] * t;
                }
            }
            start_led = end_led;
        }

        // Apply per-band gain to raw LED magnitudes
        start_led = 0;
        for (int band = 0; band < FREQUENCY_BANDS.size(); ++band) {
            int band_width = FREQUENCY_BANDS[band][1];
            float band_gain = FREQUENCY_BANDS[band][2] / 100.0f;
            int band_led_count = (band_width / 100.0f * num_leds_);
            int end_led = start_led + band_led_count;
            
            if (end_led > num_leds_) end_led = num_leds_;
            
            for (int i = start_led; i < end_led; ++i) {
                raw_led_magnitudes[i] *= band_gain;
            }
            
            start_led = end_led;
        }

        // Apply gravity. If the new magnitude is lower, fall slowly. Otherwise, jump up instantly.
        // Then scale to 0..255 range for LED brightness levels.
        for (int i = 0; i < num_leds_; ++i) {
            float scaled_mag = raw_led_magnitudes[i];

            
            if (scaled_mag < smoothed_magnitudes_[i]) {
                smoothed_magnitudes_[i] *= FFT_VISUAL_GRAVITY;
            } else {
                smoothed_magnitudes_[i] = smoothed_magnitudes_[i] * 0.3f + scaled_mag * 0.7f;
            }

            
            float final_level = std::max(0.0f, std::min(1.0f, (smoothed_magnitudes_[i])));

            led_levels_[i] = static_cast<uint8_t>(final_level * 255.0);
        }
        
    }


void FFTVisualizer::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette) {
    // Draw each LED from left to right (0 Hz to Nyquist)
    for (int i = 0; i < num_leds_; ++i) {
        
        uint8_t linear_brightness = led_levels_[i];

        if (led_levels_[i] < 10) {
            linear_brightness = 0; // Noise gate
        }

        // Apply a power curve to the brightness for better perceptual response. This compresses the lower end and expands the upper end of the brightness range.
        float normalized = linear_brightness / 255.0f;
        float perceptual = std::pow(normalized, 2.3f); // Square the value to compress lower end
        uint8_t corrected_brightness = static_cast<uint8_t>(perceptual * 255.0f);

        // The color is determined by the LED's physical position on the strip
        float pos_normalized = static_cast<float>(i) / (float)num_leds_;
        ws2811.channel[0].leds[i] = getGradientColor(pos_normalized, palette, corrected_brightness);
    }
}

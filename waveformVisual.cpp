#include "waveformVisual.h"
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

void WaveformVisualizer::CalculateWaveform(const std::vector<std::complex<double>> &fft_data)
{
    // Sum of FFT magnitudes (skip bin 0 to remove DC offset)
    // Uses only the positive-frequency half (0..N/2)
    float energy = 0.0f;
    const int bins = static_cast<int>(fft_data.size()) / 2;
    for (int i = 1; i < bins; ++i)
        energy += static_cast<float>(std::abs(fft_data[i]));
    float rms_level = energy / bins;

    // Smooth the level to prevent flickering
    vu_final_smoothed = (WAVEFORM_VISUAL_SMOOTHING_FACTOR * rms_level) + ((1.0f - WAVEFORM_VISUAL_SMOOTHING_FACTOR) * vu_final_smoothed);

    // Shift all values in the history buffer one step outwards
    if (history_buffer_.size() > 1) {
        for (size_t i = history_buffer_.size() - 1; i > 0; --i) {
            history_buffer_[i] = history_buffer_[i - 1];
        }
    }

    // Calculate the new brightness for the center and insert it
    float new_brightness = vu_final_smoothed * 255.0f * (WAVEFORM_VISUAL_BRIGHTNESS_GAIN * 10);
    if (new_brightness > 255.0f) new_brightness = 255.0f;
    if (new_brightness < 10.0f) new_brightness = 0.0f; // Noise gate

    if (!history_buffer_.empty()) {
        history_buffer_[0] = static_cast<uint8_t>(new_brightness);
    }


    //std::cout << "New brightness: " << new_brightness << std::endl;
}

void WaveformVisualizer::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette)
{
    int center = LED_COUNT / 2;

    // Render the ripple outwards from the center using the history buffer
    for (size_t i = 0; i < history_buffer_.size(); ++i) {
        uint8_t brightness = history_buffer_[i];

        // Scale brightness based on distance from the center
        float ratio = static_cast<float>(i) / static_cast<float>(history_buffer_.size());
        float distance_scale = std::pow(1.0f - ratio, 0.8f); // Quadratic falloff
        brightness = static_cast<uint8_t>(static_cast<float>(brightness) * distance_scale);


        // Apply a power curve to the brightness for better perceptual response. This compresses the lower end and expands the upper end of the brightness range.
        float normalized = static_cast<float>(brightness) / 255.0f;
        float perceptual = std::pow(normalized, 1.3f); // Square the value to compress lower end
        uint8_t corrected_brightness = static_cast<uint8_t>(perceptual * 255.0f);


        // Calculate positions for the left and right side of the ripple
        int left_pos = center - 1 - i;
        int right_pos = center + i;

        float norm_brightness = static_cast<float>(brightness) / 255.0f;

        // Render the left side
        if (left_pos >= 0) {
            float pos_normalized = static_cast<float>(left_pos) / (float)LED_COUNT;
            ws2811.channel[0].leds[left_pos] = getGradientColor(norm_brightness, palette, corrected_brightness);
        }

        // Render the right side
        if (right_pos < LED_COUNT) {
            float pos_normalized = static_cast<float>(right_pos) / (float)LED_COUNT;
            ws2811.channel[0].leds[right_pos] = getGradientColor(norm_brightness, palette, corrected_brightness);
        }
    }
}
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

void WaveformVisualizer::CalculateWaveform(const std::array<float, 512UL> &audio_samples)
{
    // Calculate RMS of the audio buffer
    float sum_sq = 0.0f;
    for (const auto &sample : audio_samples) {
        sum_sq += sample * sample;
    }
    float rms_level = std::sqrt(sum_sq / audio_samples.size());

    // Smooth the level to prevent flickering
    vu_final_smoothed = (WAVEFORM_VISUAL_SMOOTHING_FACTOR * rms_level) + ((1.0f - WAVEFORM_VISUAL_SMOOTHING_FACTOR) * vu_final_smoothed);

    // Shift all values in the history buffer one step outwards
    if (history_buffer_.size() > 1) {
        for (size_t i = history_buffer_.size() - 1; i > 0; --i) {
            history_buffer_[i] = history_buffer_[i - 1];
        }
    }

    // Calculate the new brightness for the center and insert it
    float new_brightness = vu_final_smoothed * 255.0f * (WAVEFORM_VISUAL_BRIGHTNESS_GAIN*10);
    if (new_brightness > 255.0f) new_brightness = 255.0f;
    if (new_brightness < 10.0f) new_brightness = 0.0f; // Noise gate

    if (!history_buffer_.empty()) {
        history_buffer_[0] = static_cast<uint8_t>(new_brightness);
    }
}



void WaveformVisualizer::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette)
{
    int center = LED_COUNT / 2;

    // Render the ripple outwards from the center using the history buffer
    for (size_t i = 0; i < history_buffer_.size(); ++i) {
        uint8_t brightness = history_buffer_[i];

        // Scale brightness based on distance from the center
        float ratio = static_cast<float>(i) / static_cast<float>(history_buffer_.size());
        float distance_scale = std::pow(1.0f - ratio, 1.2f); // Quadratic falloff
        brightness = static_cast<uint8_t>(static_cast<float>(brightness) * distance_scale);

        // Calculate positions for the left and right side of the ripple
        int left_pos = center - 1 - i;
        int right_pos = center + i;

        // Render the left side
        if (left_pos >= 0) {
            float pos_normalized = static_cast<float>(left_pos) / (float)LED_COUNT;
            ws2811.channel[0].leds[left_pos] = getGradientColor(pos_normalized, palette, brightness);
        }

        // Render the right side
        if (right_pos < LED_COUNT) {
            float pos_normalized = static_cast<float>(right_pos) / (float)LED_COUNT;
            ws2811.channel[0].leds[right_pos] = getGradientColor(pos_normalized, palette, brightness);
        }
    }
}
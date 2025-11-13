#include "vuVisual.h"
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


PerlinNoise::PerlinNoise() {
    // Generate a permutation vector
    p.resize(256);
    std::iota(p.begin(), p.end(), 0); // Fill with 0, 1, 2, ... 255
    std::default_random_engine engine(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end()); // Duplicate to avoid overflow
}

double PerlinNoise::fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
double PerlinNoise::lerp(double t, double a, double b) { return a + t * (b - a); }
double PerlinNoise::grad(int hash, double x, double y) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y) {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    x -= floor(x);
    y -= floor(y);
    double u = fade(x);
    double v = fade(y);
    int aa = p[p[X] + Y];
    int ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y];
    int bb = p[p[X + 1] + Y + 1];

    double res = lerp(v, lerp(u, grad(p[aa], x, y), grad(p[ba], x - 1, y)),
                      lerp(u, grad(p[ab], x, y - 1), grad(p[bb], x - 1, y - 1)));
    return (res + 1.0) / 2.0; // Return in range [0, 1]
}



VULevelMeter::VULevelMeter() {
    
}

void VULevelMeter::CalculateVolumeLevel(const std::array<float, 512UL> &audio_samples)
{
    float max_amplitude_now = 0.0f;
    float sum_sq = 0.0f;
    for (const auto &sample : audio_samples) {
        sum_sq += sample * sample;
    }
    max_amplitude_now = std::sqrt(sum_sq / audio_samples.size())  * 10.0f; // RMS scaled
    raw_max_current_amplitude = max_amplitude_now;
    max_current_amplitude = max_amplitude_now;



    
    amplitude_noise_floor = (amplitude_noise_floor * (1.0f - max_amplitude_smoothing_factor)) + (max_amplitude_smoothing_factor * max_current_amplitude);
    amplitude_noise_floor = amplitude_noise_floor *  0.90f;

    max_current_amplitude = fmaxf(max_current_amplitude - amplitude_noise_floor, 0.0f);
    float max_current_amplitude_after_noise = max_current_amplitude;
    

    
    // normaalization based on recent max value
    if (max_amplitude_now > max_recent_amplitude_ceiling) {
        max_recent_amplitude_ceiling = (0.90f * max_recent_amplitude_ceiling) + (0.1f * max_amplitude_now);
    } else {
        max_recent_amplitude_ceiling = (0.90f * max_recent_amplitude_ceiling) - (0.1f * max_amplitude_now);
    }


    const float compression_factor = 0.99f; // Tunable: 0.0 to 1.0. Lower is more compression.
     max_current_amplitude = std::pow(max_current_amplitude, compression_factor);
    

    vu_final_smoothed = (final_VU_smoothing_factor * max_current_amplitude) + ((1.0f - final_VU_smoothing_factor) * vu_final_smoothed);

    // Add the new value to the front of the history and maintain size
    level_history.push_front(vu_final_smoothed);
    if (level_history.size() > history_size) {
        level_history.pop_back();
    }
}



void VULevelMeter::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette)
{
    // Draw dynamic Perlin noise background
    noise_time_ += 0.03f; // Controls the speed of the noise evolution

    const float noise_spatial_scale = 0.1f; // Controls the "zoom" of the noise pattern

    for (int i = 0; i < LED_COUNT; i++) {
        double noise_val = noise_generator_.noise(i * noise_spatial_scale, noise_time_);
        // Map the noise value [0, 1] to a low brightness range, e.g., [2, 15]
        uint8_t bg_brightness = static_cast<uint8_t>(2 + noise_val * 13);
        ws2811.channel[0].leds[i] = getGradientColor(static_cast<float>(i) / (float)LED_COUNT, palette, bg_brightness);
    }

    // Iterate through the history to draw the trail over the background
    for (size_t i = 0; i < level_history.size(); ++i) {
        float historical_level = level_history[i];
        int level_pos = static_cast<int>(historical_level * LED_COUNT / 2);

        // Clamp the position to prevent writing out of bounds
        if (level_pos >= LED_COUNT / 2) {
            level_pos = (LED_COUNT / 2) - 1;
        }
        if (level_pos < 0) {
            level_pos = 0;
        }

        // Calculate brightness that fades with age. Newest (i=0) is brightest.
        unsigned char trail_brightness = static_cast<unsigned char>(100 * (1.0f - ((static_cast<float>(i)* 1.2) / history_size)));
        
        // Ensure a minimum brightness for the tail, but full brightness for the head
        if (i == 0) {
            trail_brightness = 100;
        } else if (trail_brightness < 10) {
            trail_brightness = 10;
        }

        //  Calculate colors based on the LED's position in the gradient 
        int center = LED_COUNT / 2;
        int right_led_index = center + level_pos;
        int left_led_index = center - 1 - level_pos;

        // Get the color for the right side LED based on its position
        float right_pos_normalized = static_cast<float>(right_led_index) / (float)LED_COUNT;
        unsigned int right_trail_color = getGradientColor(right_pos_normalized, palette, trail_brightness);

        // Get the color for the left side LED based on its position
        float left_pos_normalized = static_cast<float>(left_led_index) / (float)LED_COUNT;
        unsigned int left_trail_color = getGradientColor(left_pos_normalized, palette, trail_brightness);

        // Set the LEDs for both sides of the meter with their respective colors
        ws2811.channel[0].leds[right_led_index] = right_trail_color;
        ws2811.channel[0].leds[left_led_index] = left_trail_color;
    }
}
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
    float sum_sq = 0.0f;
    for (const auto &sample : audio_samples) {
        sum_sq += sample * sample;
    }
    current_rms = std::sqrt(sum_sq / audio_samples.size());
    current_rms = current_rms * VU_VISUAL_BRIGHTNESS_GAIN * 10;

    
    //noise_floor = (noise_floor * (1.0f - VU_VISUAL_NOISE_FLOOR_SMOOTHING_FACTOR)) + (VU_VISUAL_NOISE_FLOOR_SMOOTHING_FACTOR * current_rms);

    //float rms_noise_suppressed = fmaxf(current_rms - noise_floor, 0.0f);


    vu_final_smoothed = (VU_VISUAL_FINAL_SMOOTHING_FACTOR * current_rms) + ((1.0f - VU_VISUAL_FINAL_SMOOTHING_FACTOR) * vu_final_smoothed);

    


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

        // Exponential decay for trail brightness
        const float decay_rate = 5.0f;  //Adjust for faster/slower decay
        unsigned char trail_brightness = static_cast<unsigned char>(255.0f * std::exp(-decay_rate * i / history_size));

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
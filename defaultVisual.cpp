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
#include "defaultVisual.h"

DefaultVisual::DefaultVisual() {
    
}


void DefaultVisual::CalculateLevel(const std::array<float, 512UL> &audio_samples)
{
}


void DefaultVisual::CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette)
{
    // Draw dynamic Perlin noise background
    noise_time_ += 0.007f; // Controls the speed of the noise evolution

    for (int i = 0; i < LED_COUNT; i++) {
        double noise_val = noise_generator_.noise(i * DEFAULT_VISUAL_NOISE_SCALE, noise_time_);
        // Map the noise value [0, 1] to a low brightness range
        uint8_t bg_brightness = static_cast<uint8_t>(noise_val * DEFAULT_VISUAL_BRIGHTNESS*255.0f);
        ws2811.channel[0].leds[i] = getGradientColor(static_cast<float>(i) / (float)LED_COUNT, palette, bg_brightness);
    }
}
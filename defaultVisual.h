#pragma once
#include <vector>
#include "rpi_ws281x/ws2811.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <deque> 
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include "RGBColorPalette.h"
#include "parameters.h"



class DefaultVisual {
public:
    DefaultVisual();

    ~DefaultVisual() {}

    void CalculateLevel(const std::array<float, 512UL> &audio_samples);
    void CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette);

private:
    static constexpr float max_amplitude_smoothing_factor = 0.02f;

    std::deque<float> level_history;

private:
    // For the dynamic background effect
    PerlinNoise noise_generator_;
    float noise_time_ = 0.0f;
};


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

// Perline noise for background effect
class PerlinNoise {
public:
    PerlinNoise();
    double noise(double x, double y);
private:
    std::vector<int> p;
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y);
};


class VULevelMeter {
public:
    VULevelMeter();

    ~VULevelMeter() {}

    void CalculateVolumeLevel(const std::array<float, 512UL> &audio_samples);
    void CalculateVisual(ws2811_t &ws2811, const std::vector<GradientPoint> &palette);

private:

    float current_rms = 0.0f;
    float noise_floor = 0.0f;
    float vu_final_smoothed = 0.0f;

    // New: History buffer for the trail effect
    std::deque<float> level_history;
    const size_t history_size = 10;

private:
    // For the dynamic background effect
    PerlinNoise noise_generator_;
    float noise_time_ = 0.0f;
};


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
    void getraw_max_current_amplitude(float &out_value) { out_value = raw_max_current_amplitude; }
    void getvu_final_smoothed(float &out_value) { out_value = vu_final_smoothed; }  
    float getFinalSmoothedValue() const { return vu_final_smoothed; }

private:
    static constexpr float max_amplitude_smoothing_factor = 0.02f;

    float raw_max_current_amplitude = 0.0f;
    float max_current_amplitude = 0.0f;
    float amplitude_noise_floor = 0.0f;
    float vu_final_smoothed = 0.0f;
    float max_recent_amplitude_ceiling = 0.0f;
    const float final_VU_smoothing_factor = 0.5f;

    // New: History buffer for the trail effect
    std::deque<float> level_history;
    const size_t history_size = 10;

private:
    // For the dynamic background effect
    PerlinNoise noise_generator_;
    float noise_time_ = 0.0f;
};


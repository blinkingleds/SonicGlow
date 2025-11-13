#include "preProcessing.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>


PreProcessor::PreProcessor()
    : running_avg_rms_(0.0f), // Initialize AGC state with a scaled-down value
      smoothed_gain_(1.0f)
{}

PreProcessor::~PreProcessor() = default;



std::array<float, FRAMES_PER_BUFFER> PreProcessor::process(const std::array<short, CHUNK_SIZE>& captured_chunk) {
    // Average channels to mono and scale to float
    auto mono_buffer = averageChannelsAndScale(captured_chunk);

    // Remove any DC offset
    removeDCOffset(mono_buffer);

    
    // Automatic Gain Control (AGC) 
    // Calculate RMS of the current buffer
    float sum_sq = std::accumulate(mono_buffer.begin(), mono_buffer.end(), 0.0f, 
                                   [](float acc, float val) { return acc + val * val; });
    float current_rms = std::sqrt(sum_sq / mono_buffer.size());
    running_avg_rms_ = (1.0f - SMOOTHING_FACTOR) * running_avg_rms_ + SMOOTHING_FACTOR * current_rms;
    

    float target_gain = (running_avg_rms_ > 0.003f) ? (TARGET_RMS / running_avg_rms_) : 1.0f;
    if (target_gain > 10.0f) target_gain = 10.0f; // Prevent extreme gain

    smoothed_gain_ = (1.0f - SMOOTHING_FACTOR) * smoothed_gain_ + SMOOTHING_FACTOR * target_gain;


    //std::cout << "RMS: " << running_avg_rms_ << " Gain: " << smoothed_gain_ << std::endl; 

    std::array<float, FRAMES_PER_BUFFER> normalized_buffer;
    for (size_t i = 0; i < mono_buffer.size(); ++i) {
        float amplified_sample = mono_buffer[i] * smoothed_gain_;
        // Limiter now clamps to the correct float range.
        normalized_buffer[i] = std::max(-1.0f, std::min(1.0f, amplified_sample));
    }
    return mono_buffer;
    
}

float PreProcessor::getCurrentGain() const {
    return smoothed_gain_;
}

float PreProcessor::getCurrentRMS() const {
    return running_avg_rms_;
}

// De-interleave, average, and SCALE to the [-1.0, 1.0] float range
std::array<float, FRAMES_PER_BUFFER> PreProcessor::averageChannelsAndScale(const std::array<short, CHUNK_SIZE>& captured_chunk) {
    std::array<float, FRAMES_PER_BUFFER> mono_buffer;
    constexpr float MAX_INT16_VALUE = 32767.0f;

    for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
        int sum = 0;
        for (int ch = 0; ch < CHANNELS; ++ch) {
            sum += captured_chunk[i * CHANNELS + ch];
        }
        // Average the channels and then scale the result to the correct float range.
        mono_buffer[i] = (static_cast<float>(sum) / CHANNELS) / MAX_INT16_VALUE;
    }
    return mono_buffer;
}

// Remove DC offset from the mono signal
void PreProcessor::removeDCOffset(std::array<float, FRAMES_PER_BUFFER>& buffer) {
    float average = std::accumulate(buffer.begin(), buffer.end(), 0.0f) / buffer.size();
    for (float& sample : buffer) {
        sample -= average;
    }
}



void applyHannWindow(std::vector<float>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        double multiplier = 0.5 * (1.0 - cos(2.0 * PI * i / (data.size() - 1)));
        data[i] *= static_cast<float>(multiplier);
    }
}

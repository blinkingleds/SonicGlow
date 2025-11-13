#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <array>
#include <vector>
#include "parameters.h"
#include "fft.h"

class PreProcessor {
public:
    PreProcessor();
    ~PreProcessor();
    std::array<float, FRAMES_PER_BUFFER> process(const std::array<short, CHUNK_SIZE>& captured_chunk);
    float getCurrentGain() const;
    float getCurrentRMS() const;

private:
    std::array<float, FRAMES_PER_BUFFER> averageChannelsAndScale(const std::array<short, CHUNK_SIZE>& captured_chunk);
    void removeDCOffset(std::array<float, FRAMES_PER_BUFFER>& buffer);

    float running_avg_rms_;
    float smoothed_gain_;


    static constexpr float TARGET_RMS = 0.05f; 
    static constexpr float SMOOTHING_FACTOR = 0.02f;
};


void applyHannWindow(std::vector<float>& data);

#endif // PREPROCESSING_H
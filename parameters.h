#pragma once

#include "rpi_ws281x/ws2811.h"
#include <array>
#include "RGBColorPalette.h"

constexpr  int   TARGET_FREQ            =     WS2811_TARGET_FREQ;   // Target frequency for the WS2811 driver (800kHz for WS2812B).
constexpr  int   GPIO_PIN               =     18;                   // GPIO pin connected to the LED strip's data line.
constexpr  int   intDMA                 =     10;                   // DMA channel to use for generating the signal.
constexpr  int   STRIP_TYPE             =     WS2811_STRIP_GRB;		// WS2812/SK6812RGB integrated chip+leds
constexpr  int   LED_COUNT              =     150;                  // Total number of LEDs on the strip.
constexpr  int   SAMPLE_RATE            =     16000;                // Audio sample rate in Hz.

constexpr int FRAMES_PER_BUFFER         =     512;                  // Number of audio frames per buffer. Also determines FFT size.
constexpr int CHANNELS                  =     4;                    // Number of audio channels.
constexpr int CHUNK_SIZE                =     FRAMES_PER_BUFFER * CHANNELS; // Total samples per audio chunk.
constexpr int VISUAL_SWITCH_INTERVAL    =     2; // in minutes

constexpr int FFT_SIZE    =     512; // FFT size


constexpr int ML_MUSIC_CLASSIFIER_AUDIO_DURATION = 3; // Duration of audio (in seconds) sent to the ML music classifier for each inference.
constexpr const char* ML_MUSIC_CLASSIFIER_PRJ_PATH = "/home/admin/Projects/AudioClassifier/Realtime-Audio-Classifier";
constexpr const char* ML_MUSIC_CLASSIFIER_VENV_PATH = "./venv/bin/python3";  // Path to the ML music classifier virtual environment's Python interpreter.
constexpr const char* ML_MUSIC_CLASSIFIER_SCRIPT_PATH = "/home/admin/Projects/AudioClassifier/Realtime-Audio-Classifier/inference2.py";  // Path to the ML music classifier script.

constexpr std::array<std::array<int, 3>, 5> FREQUENCY_BANDS = {
    {{500, 20, 10},    // 0-500Hz: 20% LEDs, 0.1 gain 
     {1000, 20, 20},   // 500-1000Hz: 20% LEDs, 0.2 gain
     {2200, 30, 20},   // 1000-2200hz: 30% LEDs, 0.2 gain
     {4000, 15, 50},   // 2200-4000Hz: 15% LEDs, 0.5 gain
     {8000, 15, 200}}   // 4000-8000Hz: 15% LEDs, 2.0 gain
};


constexpr float DEFAULT_VISUAL_THRESHOLD = 0.0060f; // RMS threshold for switching to default visual. RMS below this uses default visual.
constexpr float NON_DEFAULT_VISUAL_THRESHOLD = 0.0100f; // RMS threshold for switching back from default visual. RMS above this enables other visuals.

constexpr float DEFAULT_VISUAL_NOISE_SCALE = 0.1f; // Controls the "zoom" of the noise pattern
constexpr float DEFAULT_VISUAL_BRIGHTNESS = 0.10f; 

constexpr float WAVEFORM_VISUAL_BRIGHTNESS_GAIN = 0.11f;  // gain multipled by 10 in waveform visual
constexpr float WAVEFORM_VISUAL_SMOOTHING_FACTOR = 0.9f;  // Temporal averaging of the RMS value. Higher = less averaging from previous frame


constexpr float VU_VISUAL_BRIGHTNESS_GAIN = 0.3f;  // gain multipled by 10 in VU visual
constexpr float VU_VISUAL_NOISE_FLOOR_SMOOTHING_FACTOR = 0.02f;  // Temporal averaging of the RMS value. Higher = less averaging from previous frame
constexpr float VU_VISUAL_FINAL_SMOOTHING_FACTOR = 0.23f;  // Temporal averaging of the RMS value. Higher = less averaging from previous frame

constexpr float PREPROCESSING_AGC_TARGET_RMS = 0.05f; //0.05f
constexpr float PREPROCESSING_RMS_SMOOTHING_FACTOR = 0.002f;
constexpr float PREPROCESSING_GAIN_SMOOTHING_FACTOR = 0.02f;
constexpr float PREPROCESSING_GAIN_THRESHOLD = 5.0f;

constexpr float FFT_VISUAL_GRAVITY = 0.87f;


// Container to hold all the defined color palettes
constexpr std::array palettes = {
    &kingYnaPalette,
    &lavaLampPalette,
    &iceAndFirePalette,
    &intensityFirePalette1,
    &intensityFirePalette,
    &flickrPalette,
    &miakaPalette,
    &neonPalette,
    &middle,
};





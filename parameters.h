#pragma once

#include "rpi_ws281x/ws2811.h"

constexpr  int   TARGET_FREQ            =     WS2811_TARGET_FREQ;   // Target frequency for the WS2811 driver (800kHz for WS2812B).
constexpr  int   GPIO_PIN               =     18;                   // GPIO pin connected to the LED strip's data line.
constexpr  int   intDMA                 =     10;                   // DMA channel to use for generating the signal.
constexpr  int   STRIP_TYPE             =     WS2811_STRIP_GRB;		// WS2812/SK6812RGB integrated chip+leds
constexpr  int   LED_COUNT              =     150;                  // Total number of LEDs on the strip.
constexpr  int   SAMPLE_RATE            =     16000;                // Audio sample rate in Hz.

constexpr int FRAMES_PER_BUFFER         =     512;                  // Number of audio frames per buffer. Also determines FFT size.
constexpr int CHANNELS                  =     4;                    // Number of audio channels.
constexpr int CHUNK_SIZE                =     FRAMES_PER_BUFFER * CHANNELS; // Total samples per audio chunk.
constexpr int VISUAL_SWITCH_INTERVAL    =     1; // in minutes
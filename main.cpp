#include <iostream>
#include <vector>
#include <array>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>
#include <csignal>
#include <random>
#include "fft.h"
#include "vuVisual.h"
#include "portaudio.h"
#include "preProcessing.h"
#include "fftVisual.h"
#include "RGBColorPalette.h"
#include "rpi_ws281x/ws2811.h"
#include "parameters.h"
#include "waveformVisual.h"
#include "portaudiosetup.h"
#include <stdlib.h>     
#include <time.h>    
#include "defaultVisual.h"


static void calculaterms(const std::array<float, 512UL> &audio_samples, float &out_rms);

// Global flag to control the main loop, allowing for graceful shutdown.
// This is atomic to ensure safe access from both the main thread and the signal handler.
std::atomic<bool> running = true;

// Signal handler function to catch Ctrl+C (SIGINT)
void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
    running = false;
}

// Enum to define the available visualization modes.
enum class VisualizationMode {
    DEFAULT,
    FFT,
    Waveform,
    VU
};

// Configuration for the WS281x LED strip.
// This struct is from the rpi_ws281x library.
ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = intDMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = STRIP_TYPE,
            .brightness = 255,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};



int main()
{
    // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);

    AudioData audioData;
    ws2811_return_t ret;
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        std::cerr << "ws2811_init failed: " << ws2811_get_return_t_str(ret) << std::endl;
        return -1;
    }

    PaError err;

    err = Pa_Initialize();
    if( err != paNoError )
    {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }
 
    // Find the desired input usb microphone device by name and get its index
    int inputDeviceNum = -1;
    const int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) { /* ... error handling ... */ return -1; }
    for(int i=0; i < numDevices; i++ ) {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo( i );
        if (deviceInfo && strstr(deviceInfo->name, "USB Camera-B4.09.24.1: Audio") != NULL) {
            inputDeviceNum = i;
            std::cout << "Audio Recording Device Found: " << deviceInfo->name << std::endl;
            break;
        }
    }
    if (inputDeviceNum == -1) {
        std::cerr << "Error: Audio Recording Device Not Found." << std::endl;
        Pa_Terminate();
        return -1;
    }

    PaStreamParameters inputParameters;
    PaStream *stream;
    
    inputParameters.device = inputDeviceNum;
    inputParameters.channelCount = CHANNELS;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream( &stream, &inputParameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, audioCallback, &audioData );
    if( err != paNoError ) 
    { 
        std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err) << std::endl;
        return -1; 
    }

    err = Pa_StartStream( stream );
    if( err != paNoError ) 
    { 
        std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    // Container to hold all the defined color palettes
    constexpr std::array palettes = {
        &wiretapPalette, &rastafariPalette, &sinCityRedPalette, 
        &citrusPeelPalette, &weddingDayBluesPalette, &relaxingRedPalette, 
        &ohhappinessPalette, &argonPalette, &orangeCoralPalette, 
        &terminalPalette, &melloPalette, &sunkistPalette, 
        &pacificDreamPalette, &loveCouplePalette, &firewatchPalette, 
        &grapefruitSunsetPalette, &vinePalette, &flickrPalette, 
        &atlasPalette, &martiniPalette, &virginPalette, 
        &miakaPalette, &monteCarloPalette, &cherryRedPalette, 
        &summerVibesPalette, &oliveGardenFeastPalette, &earthyForestHuesPalette, 
        &softSandPalette, &oceanSunsetPalette, &sunsetPalette, 
        &oceanPalette, &forestPalette, &firePalette, 
        &icePalette, &rainbowPalette, &purpleHazePalette, 
        &desertPalette, &neonPalette, &pastelPalette, 
        &complexGradientPalette, &kingYnaPalette, &solidRedPalette, 
        &solidGreenPalette, &solidBluePalette, &solidWhitePalette, 
        &solidYellowPalette, &solidCyanPalette
    };
    int current_palette_index = 0;
    srand(static_cast<unsigned int>(time(nullptr)));

    // Instantiate the main audio pre processing and visualization classes.
    PreProcessor preProcessor;
    FFTVisualizer fft_visualizer(FRAMES_PER_BUFFER, SAMPLE_RATE);
    WaveformVisualizer waveform_visualizer;
    VULevelMeter level_meter; 
    DefaultVisual default_visual;


    VisualizationMode current_mode = VisualizationMode::FFT;
    auto last_switch_time = std::chrono::steady_clock::now();
    const auto switch_interval = std::chrono::minutes(VISUAL_SWITCH_INTERVAL);

    double max  = 0;
    float average_rms = 0;

    while (running) 
    {
        if (audioData.bufferReady.exchange(false, std::memory_order_acquire)) // check if the buffer is ready
        {
            // Fetch the latest audio buffer from the PortAudio setup.
            auto start = std::chrono::steady_clock::now();

            int readBufferIdx = 1 - audioData.writeBuffer.load(std::memory_order_relaxed);
            const auto& captured_chunk = audioData.buffer[readBufferIdx];
            
            auto final_buffer = preProcessor.process(captured_chunk);
            
            std::vector<float> windowed_buffer(final_buffer.begin(), final_buffer.end());
            applyHannWindow(windowed_buffer);

            std::vector<cd> fft_input(windowed_buffer.size());
            for (size_t i = 0; i < windowed_buffer.size(); i++) {
                fft_input[i] = cd(windowed_buffer[i], 0.0);
            }
            fft(fft_input);

            // Calculate RMS for mode switching and normalization
            float rms_current = 0;
            calculaterms(final_buffer, rms_current);
            average_rms = (PREPROCESSING_RMS_SMOOTHING_FACTOR * rms_current) + ((1.0f - PREPROCESSING_RMS_SMOOTHING_FACTOR) * average_rms);

            // --- Normalize the FFT output by dividing by the current RMS ---
            const float epsilon = 1e-6; // To prevent division by zero
            if (rms_current > epsilon) {
                for (auto& val : fft_input) {
                    val /= rms_current;
                }
            }

            // Handle visualization mode switching based on the timer.
            auto now = std::chrono::steady_clock::now();
            if (average_rms < DEFAULT_VISUAL_THRESHOLD) 
            {
                current_mode = VisualizationMode::DEFAULT;
            } 
            else if ((average_rms > NON_DEFAULT_VISUAL_THRESHOLD) || (current_mode != VisualizationMode::DEFAULT))
            {
                if ((now - last_switch_time > switch_interval)  || (current_mode == VisualizationMode::DEFAULT)) 
                {
                    // Cycle to the next mode
                    if (current_mode == VisualizationMode::FFT) {
                        current_mode = VisualizationMode::Waveform;
                    } else if (current_mode == VisualizationMode::Waveform) {
                        current_mode = VisualizationMode::VU;
                    } else { // VU
                        current_mode = VisualizationMode::FFT;
                    }
                    last_switch_time = now; // Reset the timer

                    current_palette_index = (rand() % palettes.size());
                }
            }

            const auto& current_palette_vector = *palettes[current_palette_index];


            // Update and render the visualization based on the current mode
            switch (current_mode) {
                case VisualizationMode::DEFAULT:
                    default_visual.CalculateVisual(ledstring, wiretapPalette);
                    break;
                case VisualizationMode::FFT:
                    fft_visualizer.update(fft_input);
                    fft_visualizer.CalculateVisual(ledstring, current_palette_vector);
                    break;
                case VisualizationMode::Waveform:
                    waveform_visualizer.CalculateWaveform(final_buffer);
                    waveform_visualizer.CalculateVisual(ledstring, current_palette_vector);
                    break;
                case VisualizationMode::VU:
                    level_meter.CalculateVolumeLevel(final_buffer);
                    level_meter.CalculateVisual(ledstring, current_palette_vector);
                    break;
            }
            ws2811_render(&ledstring);


            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> processing_time = end - start;
            if (processing_time.count() > max) {
                max = processing_time.count();
                std::cout << "New max processing time: " << max << " ms" << std::endl;
            }

        }
    }

    std::cout << "Exiting main loop. Cleaning up..." << std::endl;

    for (int x = 0; x < LED_COUNT; x++)
    {
        ledstring.channel[0].leds[(x)] = 0;
    }

	ws2811_render(&ledstring);
    ws2811_fini(&ledstring);


    err = Pa_StopStream( stream );
    if( err != paNoError ) { std::cerr << "ERROR: Pa_StopStream returned 0x" << err << std::endl; }

    err = Pa_CloseStream( stream );
    if( err != paNoError ) { std::cerr << "ERROR: Pa_CloseStream returned 0x" << err << std::endl; }

    Pa_Terminate();
    
    return 0;
}


static void calculaterms(const std::array<float, 512UL> &audio_samples, float &out_rms)
{
    float sum_sq = 0.0f;
    for (const auto &sample : audio_samples) {
        sum_sq += sample * sample;
    }
    out_rms = std::sqrt(sum_sq / audio_samples.size());
}

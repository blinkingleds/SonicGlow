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
#include <filesystem>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <sched.h>
#include <pthread.h>
#include "webServer.h"

static void calculaterms(const std::array<float, 512UL> &audio_samples, float &out_rms);
static void cleanup(PaStream* stream,
                    bool leds_init,
                    bool pa_initialized,
                    int& server_fd,
                    int& client_fd,
                    std::thread& sender,
                    std::thread& reader);

// Global flag to control the main loop, allowing for graceful shutdown.
// This is atomic to ensure safe access from both the main thread and the signal handler.
std::atomic<bool> running = true;

// Controls whether LED visuals are rendered. When false the LEDs are blanked
// but audio capture, ML classification, and the web server keep running.
std::atomic<bool> visualsEnabled{true};

// Signal handler function to catch Ctrl+C (SIGINT)
void signal_handler(int signum) {
    running = false;
}

// Enum to define the available visualization modes.
enum class VisualizationMode {
    OFF,
    DEFAULT,
    FFT,
    Waveform, 
    VU,          // disabled for now


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



std::array<short, (2 * (SAMPLE_RATE * CHANNELS * ML_MUSIC_CLASSIFIER_AUDIO_DURATION)) + CHUNK_SIZE> ml_audio_buffer;
int buffer_write_head = 0;
std::mutex socket_mutex;
std::condition_variable socket_cv;
int pending_buffer_half = -1; // 0 for first half, 1 for second half, -1 for none
std::atomic<int> socket_received_data = 0;



ssize_t send_all(int sock, const void* buf, size_t len) {
    const uint8_t* data = static_cast<const uint8_t*>(buf);
    size_t total = 0;
    while (total < len) {
        ssize_t sent = ::send(sock, data + total, len - total, 0);
        if (sent > 0) {
            total += sent;
        } else if (sent == -1 && errno == EINTR) {
            continue;
        } else if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        } else {
            return -1;
        }
    }
    return total;
}

void socket_sender_thread(int client_fd) {
    pthread_setname_np(pthread_self(), "SocketSender");
    while (running) {
        int half_to_send = -1;
        
        {
            std::unique_lock<std::mutex> lock(socket_mutex);
            socket_cv.wait(lock, []{ return pending_buffer_half != -1 || !running; });
            
            if (!running && pending_buffer_half == -1) break;
            
            half_to_send = pending_buffer_half;
            pending_buffer_half = -1; // Reset flag
        }

        if (half_to_send != -1) {
            // Determine pointer to start of data
            short* data_ptr = &ml_audio_buffer[half_to_send * SAMPLE_RATE * CHANNELS * ML_MUSIC_CLASSIFIER_AUDIO_DURATION];
            size_t data_size_bytes = SAMPLE_RATE * CHANNELS * ML_MUSIC_CLASSIFIER_AUDIO_DURATION * sizeof(short);

            // Send raw audio data
            ssize_t result = send_all(client_fd, data_ptr, data_size_bytes);
            if (result < 0) {
                std::cerr << "Socket send failed: " << strerror(errno) << std::endl;
                running = false;
                socket_cv.notify_all();
                return;
            }
        }
    }
}


void socket_reader_thread(int client_fd) {
    pthread_setname_np(pthread_self(), "SocketReader");
    while (running) {
        // Wait 3 seconds before attempting to read
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        if (!running) break;
        int received_data = 0;
        ssize_t received = ::recv(client_fd, &received_data, sizeof(received_data), 0);
        if (received == 0) {
            std::cerr << "ML client disconnected." << std::endl;
            running = false;
            socket_cv.notify_all();
            return;
        } else if (received < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Socket recv failed: " << strerror(errno) << std::endl;
            running = false;
            socket_cv.notify_all();
            return;
        }

        //copy received data to global variable
        if (received > 0) 
        {
            socket_received_data.store(received_data, std::memory_order_relaxed);
        }
    }
}



int main()
{
    pthread_setname_np(pthread_self(), "MainVisual");
    pid_t pid = fork();
    if (pid == 0)
    {
        // Pin child process to specific CPU core(s)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(3, &cpuset); // Pin to CPU core 3

        
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
            std::cerr << "Failed to set CPU affinity for ML classifier." << std::endl;
        }
        
        //set working directory to the project path
        std::filesystem::current_path(ML_MUSIC_CLASSIFIER_PRJ_PATH);

        // Exec the venv python and pass the script path as argv[1]
        execl(ML_MUSIC_CLASSIFIER_VENV_PATH, ML_MUSIC_CLASSIFIER_VENV_PATH, ML_MUSIC_CLASSIFIER_SCRIPT_PATH, (char*)NULL);
        std::cerr << "Failed to start ML music classifier process." << std::endl;
        exit(1);
    }
    else
    {
        // Pin main process to cores 0, 1, 2 (leaving core 3 for ML classifier)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset);
        CPU_SET(1, &cpuset);
        CPU_SET(2, &cpuset);
        
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
            std::cerr << "Failed to set CPU affinity for main process." << std::endl;
        }
    }
    


    int server_fd = -1;
    int client_fd = -1;
    struct sockaddr_un addr;
    std::thread sender;
    std::thread reader;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed." << std::endl;
        cleanup(nullptr, false, false, server_fd, client_fd, sender, reader);
        return -1;
    }

    memset(&addr,0, sizeof(addr));
    const char* name = "Socket";
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    std::memcpy(&addr.sun_path[1], name, std::strlen(name));
    socklen_t addr_len = sizeof(addr.sun_family) + 1 + std::strlen(name);
    if (bind(server_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        std::cerr << "Socket bind failed." << std::endl;
        cleanup(nullptr, false, false, server_fd, client_fd, sender, reader);
        return -1;
    }

    if (listen(server_fd, 1) == -1) {
        std::cerr << "Socket listen failed." << std::endl;
        cleanup(nullptr, false, false, server_fd, client_fd, sender, reader);
        return -1;
    }
    std::cout << "Waiting for ML classifier socket connection..." << std::endl;

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        std::cerr << "Socket connection failed." << std::endl;
        cleanup(nullptr, false, false, server_fd, client_fd, sender, reader);
        return -1;
    }

    std::cout << "Socket connection established." << std::endl;

    sender = std::thread(socket_sender_thread, client_fd);
    reader = std::thread(socket_reader_thread, client_fd);
    
    // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);

    AudioData audioData;
    bool leds_init = false;
    bool pa_initialized = false;
    PaStream* stream = nullptr;

    ws2811_return_t ret = ws2811_init(&ledstring);
    if (ret != WS2811_SUCCESS) {
        std::cerr << "ws2811_init failed: " << ws2811_get_return_t_str(ret) << std::endl;
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1;
    }
    leds_init = true;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1;
    }
    pa_initialized = true;
 
    // Find the desired input usb microphone device by name and get its index
    int inputDeviceNum = -1;
    const int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "PortAudio device enumeration failed." << std::endl;
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1;
    }
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
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1;
    }

    PaStreamParameters inputParameters;
    
    inputParameters.device = inputDeviceNum;
    inputParameters.channelCount = CHANNELS;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream( &stream, &inputParameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, audioCallback, &audioData );
    if( err != paNoError ) 
    { 
        std::cerr << "PortAudio open stream failed: " << Pa_GetErrorText(err) << std::endl;
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1; 
    }

    err = Pa_StartStream( stream );
    if( err != paNoError ) 
    { 
        std::cerr << "PortAudio start stream failed: " << Pa_GetErrorText(err) << std::endl;
        cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);
        return -1;
    }


    int current_palette_index = 0;
    srand(static_cast<unsigned int>(time(nullptr)));

    // Instantiate the main audio pre processing and visualization classes.
    PreProcessor preProcessor;
    FFTVisualizer fft_visualizer(FRAMES_PER_BUFFER, SAMPLE_RATE);
    WaveformVisualizer waveform_visualizer;
    VULevelMeter level_meter; 
    DefaultVisual default_visual;


    VisualizationMode current_mode = VisualizationMode::Waveform;
    auto last_switch_time = std::chrono::steady_clock::now();
    const auto switch_interval = std::chrono::minutes(VISUAL_SWITCH_INTERVAL);

    double max  = 0;
    float average_rms = 0;

    WebServer webServer(visualsEnabled, 8080);
    webServer.start();

    auto defaultVisualstarttime = std::chrono::steady_clock::now();
    while (running) 
    {
        auto start = std::chrono::steady_clock::now();
        if (audioData.bufferReady.exchange(false, std::memory_order_acquire)) // check if the buffer is ready
        {
            // Fetch the latest audio buffer from the PortAudio setup.
            //auto start = std::chrono::steady_clock::now();

            int readBufferIdx = 1 - audioData.writeBuffer.load(std::memory_order_relaxed);
            const auto& captured_chunk = audioData.buffer[readBufferIdx];
            
            memcpy(&ml_audio_buffer[buffer_write_head], &captured_chunk, sizeof(short) * CHUNK_SIZE);
            const size_t half_samples = SAMPLE_RATE * CHANNELS * ML_MUSIC_CLASSIFIER_AUDIO_DURATION;

            buffer_write_head = (buffer_write_head + CHUNK_SIZE) % (2 * half_samples);


            if (buffer_write_head == half_samples) {
                {
                    std::lock_guard<std::mutex> lock(socket_mutex);
                    pending_buffer_half = 0;
                }
                socket_cv.notify_one();
            } else if (buffer_write_head == 0) {
                {
                    std::lock_guard<std::mutex> lock(socket_mutex);
                    pending_buffer_half = 1;
                }
                socket_cv.notify_one();
            }

            // If visuals are paused via the web interface, blank the LEDs and skip DSP.
            if (!visualsEnabled.load(std::memory_order_relaxed)) {
                for (int x = 0; x < LED_COUNT; x++) {
                    ledstring.channel[0].leds[x] = 0;
                }
                ws2811_render(&ledstring);
                continue;
            }

            auto final_buffer = preProcessor.process(captured_chunk);
            
            std::vector<float> windowed_buffer(final_buffer.begin(), final_buffer.end());
            applyHannWindow(windowed_buffer);

            std::vector<cd> fft_input(windowed_buffer.size());
            for (size_t i = 0; i < windowed_buffer.size(); i++) {
                fft_input[i] = cd(windowed_buffer[i], 0.0);
            }
            fft(fft_input);

            int Audio_Classification = socket_received_data.load(std::memory_order_relaxed);
            
            // Handle visualization mode switching based on the timer.
            auto now = std::chrono::steady_clock::now();
            if ( ((Audio_Classification== 0) || (Audio_Classification== 3)) && (current_mode != VisualizationMode::DEFAULT && current_mode != VisualizationMode::OFF) )
            {
                current_mode = VisualizationMode::DEFAULT;
                defaultVisualstarttime = now;
            } 
            else if ( (Audio_Classification== 1) || (Audio_Classification== 2) ) // Check if the audio classification indicates music. If so, allow mode switching. If not, force default visual.
            {
                if ((now - last_switch_time > switch_interval)  || (current_mode == VisualizationMode::DEFAULT || current_mode == VisualizationMode::OFF)) 
                {
                    // Cycle to the next mode
                    if (current_mode == VisualizationMode::FFT) {
                        current_mode = VisualizationMode::Waveform;
                    } else if (current_mode == VisualizationMode::Waveform) {
                        current_mode = VisualizationMode::FFT;
                    } else { 
                        current_mode = VisualizationMode::FFT;
                    }
                    last_switch_time = now; // Reset the timer

                    current_palette_index = (rand() % palettes.size());
                }
            }
            else
            {
                if ((now - defaultVisualstarttime) > std::chrono::seconds(200))
                {
                    current_mode = VisualizationMode::OFF;
                }

            }

            const auto& current_palette_vector = *palettes[current_palette_index];

            // Update and render the visualization based on the current mode
            switch (current_mode) {
                case VisualizationMode::OFF:
                    for (int x = 0; x < LED_COUNT; x++)
                    {
                        ledstring.channel[0].leds[(x)] = 0;
                    }
                    break;
                case VisualizationMode::DEFAULT:
                    default_visual.CalculateVisual(ledstring, current_palette_vector);
                    break;
                case VisualizationMode::FFT:
                    fft_visualizer.update(fft_input);
                    fft_visualizer.CalculateVisual(ledstring, current_palette_vector);
                    break;
                case VisualizationMode::Waveform:
                    waveform_visualizer.CalculateWaveform(fft_input);
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
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }



    }

    webServer.stop();

    std::cout << "Exiting main loop. Cleaning up..." << std::endl;

    for (int x = 0; x < LED_COUNT; x++)
    {
        ledstring.channel[0].leds[(x)] = 0;
    }

    ws2811_render(&ledstring);

    cleanup(stream, leds_init, pa_initialized, server_fd, client_fd, sender, reader);

    return 0;
}


static void cleanup(PaStream* stream,
                    bool leds_init,
                    bool pa_initialized,
                    int& server_fd,
                    int& client_fd,
                    std::thread& sender,
                    std::thread& reader)
{
    running = false;
    socket_cv.notify_all();

    if (sender.joinable()) sender.join();
    if (reader.joinable()) reader.join();

    if (client_fd != -1) { close(client_fd); client_fd = -1; }
    if (server_fd != -1) { close(server_fd); server_fd = -1; }

    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    if (pa_initialized) {
        Pa_Terminate();
    }
    if (leds_init) {
        ws2811_fini(&ledstring);
    }
}

static void calculaterms(const std::array<float, 512UL> &audio_samples, float &out_rms)
{
    float sum_sq = 0.0f;
    for (const auto &sample : audio_samples) {
        sum_sq += sample * sample;
    }
    out_rms = std::sqrt(sum_sq / audio_samples.size());
}

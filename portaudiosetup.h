#include "portaudio.h"
#include  <atomic>
#include <array>
#include "parameters.h"

struct AudioData
{
    std::array<std::array<short, CHUNK_SIZE>, 2> buffer;
    std::atomic<int> writeBuffer{0};
    std::atomic<bool> bufferReady{false};
    std::atomic<long long> totalFramesProcessed{0};
};



int audioCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData );
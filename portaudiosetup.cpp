#include "portaudiosetup.h"
#include <cstring>


int audioCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    auto *data = static_cast<AudioData*>(userData);
    const auto *in = static_cast<const short*>(inputBuffer);
    
    if (in == nullptr) {
        return paContinue;
    }

    int currentWriteBuffer = data->writeBuffer.load(std::memory_order_relaxed);
    memcpy(data->buffer[currentWriteBuffer].data(), in, CHUNK_SIZE * sizeof(short));
    
    data->totalFramesProcessed += framesPerBuffer;
    data->bufferReady.store(true, std::memory_order_release);
    data->writeBuffer.store(1 - currentWriteBuffer, std::memory_order_relaxed);

    return paContinue;
}
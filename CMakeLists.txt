cmake_minimum_required(VERSION 3.10.0)
project(SonicGlow VERSION 0.1.0 LANGUAGES C CXX)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")


add_executable(SonicGlow main.cpp preProcessing.cpp fft.cpp vuVisual.cpp fftVisual.cpp  portaudiosetup.cpp waveformVisual.cpp RGBColorPalette.cpp rpi_ws281x/mailbox.c rpi_ws281x/ws2811.c rpi_ws281x/pwm.c rpi_ws281x/pcm.c rpi_ws281x/dma.c rpi_ws281x/rpihw.c )



# Try pkg-config first to get include/link flags for portaudio
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PORTAUDIO QUIET portaudio-2.0)
endif()

if(PORTAUDIO_LIBRARIES)
    target_include_directories(SonicGlow PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
    target_link_libraries(SonicGlow PRIVATE ${PORTAUDIO_LIBRARIES})
else()
    # Fallback: try find_library for -lportaudio
    find_library(PORTAUDIO_LIB NAMES portaudio)
    if(PORTAUDIO_LIB)
        target_link_libraries(SonicGlow PRIVATE ${PORTAUDIO_LIB})
    else()
        message(FATAL_ERROR "PortAudio not found. Install the development package (e.g. portaudio19-dev) or set PKG_CONFIG_PATH.")
    endif()
endif()

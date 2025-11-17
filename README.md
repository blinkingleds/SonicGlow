# SonicGlow: Real-time LED Music Visualizer

A real-time audio visualizer for Raspberry Pi and WS281x LED strips.  It captures audio from an input device, performs signal processing, and drives vibrant, dynamic animations on an LED strip that dance to the music.

## Features

- Multiple Visualization Modes: Includes an FFT spectrum analyzer, a symmetric VU meter, a scrolling waveform, and an ambient idle pattern.
- FFT Spectrum : A classic frequency spectrum that displays the magnitude of frequences ranging from 0 to 8Khz. 

https://github.com/user-attachments/assets/f24d2d11-de67-4808-b763-e62971474b46


- VU Meter : Symmetric volume meter based on the RMS value of the raw audio with the decaying trail showing historical values.

https://github.com/user-attachments/assets/5f0a426b-6e67-4afa-8b28-7f2aa38218c6

- Scrolling Waveform : A scrolling oscilloscope style display of the raw audio waveform


https://github.com/user-attachments/assets/5a8dd7df-c440-4f30-9236-9f8bae7f1a24

- Ambient Mode: An ambient, gently shifting nosise pattern for when no music is playing


https://github.com/user-attachments/assets/1e804b41-af13-4527-8b58-6b446441fbee


  

## Hardware Requirements

-   Raspberry Pi 4
-   WS2811 / WS2812 / WS2812B LED Strip
-   USB Microphone ( This project uses PlayStation Eye )

## Hardware Connection



1.  **Data:** Connect the LED strip's **Data Input** pin to the Raspberry Pi's **GPIO 18** (physical pin 12). This can be changed via the `LED_PIN` setting in `parameters.h`.
2.  **Ground:** Connect the LED strip's **Ground** wire to a **Ground** pin on the Raspberry Pi.
3.  **Common Ground:** Crucially, the ground from your external 5V power supply must also be connected to the same ground on the Raspberry Pi.
4.  **Power:** Connect the LED strip's **5V** wire to the **positive (+)** terminal of your external 5V power supply.

## Software Dependencies

-   PortAudio
-   CMake
-   g++

## Building the Project

1.  **Clone the repository:**
    ```bash
    git clone <your-repository-url>
    cd SonicGlow
    ```

2.  **Install dependencies:**
    ```bash
    sudo apt-get update
    sudo apt-get install cmake libportaudio-dev
    ```

3.  **Build:**
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```

## Running SonicGlow

The application requires `sudo` to control the LEDs. From the `build` directory:
```bash
sudo ./SonicGlow
```

## Configuration

All major parameters can be tuned in the `parameters.h` file. This is the central place to configure your setup and tweak the visualizer's behavior.

## Code Structure

-   `main.cpp`: Main application loop, audio capture, and mode switching.
-   `preProcessing.cpp`: microphone audio preprocessing (Averaging from multiple channels, AUtomatic Gain Control, DC offset removal).
-   `fft.cpp`: : FFT calculation
-   `fftVisual.cpp`: Implements the FFT spectrum analyzer visualization.
-   `vuVisual.cpp`: Implements the VU meter visualization.
-   `waveformVisual.cpp`:Implements the scrolling waveform visualization.
-   `defaultVisual.cpp`:  Implements the idle/ambient visualization.
-   `parameters.h`: parameter configuration file.
-   `rpi_ws281x/`: Submodule for LED driver.

## Acknowledgments

-   This project uses the [rpi_ws281x](https://github.com/jgarff/rpi_ws281x) library by Jeremy Garff for LED control.
-   It was inspired by the [Emotiscope](https://github.com/Lixie-Labs/Emotiscope) project.

## License

This project is licensed under the MIT License. The included `rpi_ws281x` library is distributed under its own license.

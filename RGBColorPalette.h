#pragma once

#include <vector>


struct RGBColor {
    unsigned char R;
    unsigned char G;
    unsigned char B;
};

struct GradientPoint {
    float position; // Position in the gradient (0.0 to 1.0)
    RGBColor color; // Color at this position
};

const std::vector<GradientPoint> flickrPalette = {{
    {0.0f, {255, 0, 132}},   // #ff0084
    {1.0f, {51, 0, 27}}      // #33001b
}};

const std::vector<GradientPoint> miakaPalette = {{
    {0.0f, {252, 53, 76}},   // #fc354c
    {1.0f, {10, 191, 188}}    // #0abfbc
}};

const std::vector<GradientPoint> firePalette = {{
    {0.0f, {255, 0, 0}},     // #ff0000
    {0.5f, {255, 140, 0}},   // #ff8c00
    {1.0f, {241, 217, 0}}    // #f1d900
}};

const std::vector<GradientPoint> neonPalette = {{
    {0.0f, {57, 255, 20}},   // #39ff14
    {0.5f, {0, 255, 255}},   // #00ffff
    {1.0f, {255, 20, 147}}   // #ff1493
}};

const std::vector<GradientPoint> kingYnaPalette = {{
    {0.0f, {26, 42, 108}},     // #1a2a6c
    {0.5f, {178, 31, 31}},     // #b21f1f
    {1.0f, {253, 187, 45}}    // #fdbb2d
}};

const std::vector<GradientPoint> lavaLampPalette = {{
    {0.0f,  {255, 50,  0}},    // #ff3200
    {0.4f,  {255, 160, 0}},    // #ffa000
    {0.7f,  {255, 0,   120}},  // #ff0078
    {1.0f,  {200, 0,   255}}   // #c800ff
}};


const std::vector<GradientPoint> middle = {{
    {0.0f,  {255, 50,  0}},    // #ff3200
    {0.4f,  {255, 160, 0}},    // #ffa000
    {0.0f,  {255, 50,  0}},    // #ff3200
}};

const std::vector<GradientPoint> iceAndFirePalette = {{
    {0.0f,  {255, 60,  0}},   // #ff3c00
    {0.5f,  {15,  15,  60}},  // #0f0f3c
    {1.0f,  {0,   180, 255}}  // #00b4ff
}};


const std::vector<GradientPoint> intensityFirePalette1 = {{
    {0.0f,  {60,  60,  60}},   // Dark grey  (silence)
    {0.08f, {230, 200, 80}},   // Pale gold
    {0.15f, {255, 50,  0}},    // Yellow      (soft hit)
    {0.3f,  {255, 50,  0}},    // Orange      (medium)
    {0.5f,  {255, 50,  0}},    // Red-orange  (transition into red)
    {1.0f,  {255, 0,   0}}     // Pure red    (peak)
}};

const std::vector<GradientPoint> intensityFirePalette = {{
    {0.0f,  {60,  10,  0}},    // Near-black warm (silence)
    {0.2f,  {255, 210, 0}},    // Yellow      (soft hit)
    {0.3f, {255, 130, 0}},    // Orange      (medium)
    {0.7f,  {255, 50,  0}},    // Red-orange
    {1.0f,  {255, 0,   0}}     // Red         (peak)
}};

// Function to get the color at a specific position along the entire gradient
unsigned int getGradientColor(float position, const std::vector<GradientPoint>& palette, unsigned char brightness); 


unsigned int rgbToInt(const RGBColor& color);






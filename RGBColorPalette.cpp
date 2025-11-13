#include "RGBColorPalette.h"
#include <cstddef>

// Function to interpolate between two colors
RGBColor interpolateColor(RGBColor color1, RGBColor color2, float fraction) {
    RGBColor newColor;
    // Linear interpolation for each channel
    newColor.R = (unsigned char)((color2.R - color1.R) * fraction + color1.R);
    newColor.G = (unsigned char)((color2.G - color1.G) * fraction + color1.G);
    newColor.B = (unsigned char)((color2.B - color1.B) * fraction + color1.B);
    return newColor;
}

// Function to get the color at a specific position along the entire gradient
unsigned int getGradientColor(float position, const std::vector<GradientPoint>& palette, unsigned char brightness) {
    
    RGBColor calculatedColor;

    // Ensure the position is within the valid range [0.0, 1.0]
    if (position <= 0.0f) {
        calculatedColor = palette.front().color;
    } else if (position >= 1.0f) {
        calculatedColor = palette.back().color;
    } else {
        // Find the two gradient points the position falls between
        bool found = false;
        for (std::size_t i = 0; i < palette.size() - 1; ++i) {
            if (position >= palette[i].position && position <= palette[i+1].position) {
                // Calculate the fraction within this specific segment
                float segmentLength = palette[i+1].position - palette[i].position;
                // Avoid division by zero
                if (segmentLength <= 0) {
                    calculatedColor = palette[i].color;
                } else {
                    float segmentFraction = (position - palette[i].position) / segmentLength;
                    calculatedColor = interpolateColor(palette[i].color, palette[i+1].color, segmentFraction);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            calculatedColor = palette.back().color; // Fallback
        }
    }

    // Apply brightness scaling
    float brightnessScale = static_cast<float>(brightness) / 100.0f;
    if (brightnessScale < 0.0f) brightnessScale = 0.0f;
    if (brightnessScale > 1.0f) brightnessScale = 1.0f;

    calculatedColor.R = static_cast<unsigned char>(calculatedColor.R * brightnessScale);
    calculatedColor.G = static_cast<unsigned char>(calculatedColor.G * brightnessScale);
    calculatedColor.B = static_cast<unsigned char>(calculatedColor.B * brightnessScale);

    return (static_cast<unsigned int>(0) << 24) |
        (static_cast<unsigned int>(calculatedColor.R) << 16) |
        (static_cast<unsigned int>(calculatedColor.G) << 8)  |
        (static_cast<unsigned int>(calculatedColor.B));
}

unsigned int rgbToInt(const RGBColor& color) {
    return (static_cast<unsigned int>(0) << 24) |
           (static_cast<unsigned int>(color.R) << 16) |
           (static_cast<unsigned int>(color.G) << 8)  |
           (static_cast<unsigned int>(color.B));
}
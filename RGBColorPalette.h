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

const std::vector<GradientPoint> wiretapPalette = {{
    {0.0f, {138, 35, 135}},   // #8a2387
    {0.5f, {233, 64, 87}},    // #e94057
    {1.0f, {242, 113, 33}}    // #f27121
}};

const std::vector<GradientPoint> rastafariPalette = {{
    {0.0f, {30, 150, 0}},     // #1e9600
    {0.5f, {255, 242, 0}},    // #fff200
    {1.0f, {255, 0, 0}}       // #ff0000
}};

const std::vector<GradientPoint> sinCityRedPalette = {{
    {0.0f, {237, 33, 58}},    // #ed213a
    {1.0f, {147, 41, 30}}     // #93291e
}};

const std::vector<GradientPoint> citrusPeelPalette = {{
    {0.0f, {253, 200, 48}},   // #fdc830
    {1.0f, {243, 115, 53}}    // #f37335
}};

const std::vector<GradientPoint> weddingDayBluesPalette = {{
    {0.0f, {64, 224, 208}},   // #40e0d0
    {0.5f, {255, 140, 0}},    // #ff8c00
    {1.0f, {255, 0, 128}}     // #ff0080
}};

const std::vector<GradientPoint> relaxingRedPalette = {{
    {0.0f, {255, 251, 213}},  // #fffbd5
    {1.0f, {178, 10, 44}}     // #b20a2c
}};

const std::vector<GradientPoint> ohhappinessPalette = {{
    {0.0f, {0, 176, 155}},    // #00b09b
    {1.0f, {150, 201, 61}}    // #96c93d
}};

const std::vector<GradientPoint> argonPalette = {{
    {0.0f, {3, 0, 30}},        // #03001e
    {0.33f, {115, 3, 192}},   // #7303c0
    {0.66f, {236, 56, 188}},  // #ec38bc
    {1.0f, {253, 239, 249}}   // #fdeff9
}};

const std::vector<GradientPoint> orangeCoralPalette = {{
    {0.0f, {255, 153, 102}},  // #ff9966
    {1.0f, {255, 94, 98}}     // #ff5e62
}};

const std::vector<GradientPoint> terminalPalette = {{
    {0.0f, {0, 0, 0}},        // #000000
    {1.0f, {15, 155, 15}}     // #0f9b0f
}};

const std::vector<GradientPoint> melloPalette = {{
    {0.0f, {192, 57, 43}},    // #c0392b
    {1.0f, {142, 68, 173}}   // #8e44ad
}};

const std::vector<GradientPoint> sunkistPalette = {{
    {0.0f, {242, 153, 74}},   // #f2994a
    {1.0f, {242, 201, 76}}    // #f2c94c
}};

const std::vector<GradientPoint> pacificDreamPalette = {{
    {0.0f, {52, 232, 158}},   // #34e89e
    {1.0f, {15, 52, 67}}      // #0f3443
}};

const std::vector<GradientPoint> loveCouplePalette = {{
    {0.0f, {58, 97, 134}},   // #3a6186
    {1.0f, {137, 37, 62}}    // #89253e
}};

const std::vector<GradientPoint> firewatchPalette = {{
    {0.0f, {203, 45, 62}},    // #cb2d3e
    {1.0f, {239, 71, 58}}     // #ef473a
}};

const std::vector<GradientPoint> grapefruitSunsetPalette = {{
    {0.0f, {233, 100, 67}},   // #e96443
    {1.0f, {144, 78, 149}}    // #904e95
}};

const std::vector<GradientPoint> vinePalette = {{
    {0.0f, {0, 191, 143}},   // #00bf8f
    {1.0f, {0, 21, 16}}      // #001510
}};
const std::vector<GradientPoint> flickrPalette = {{
    {0.0f, {255, 0, 132}},   // #ff0084
    {1.0f, {51, 0, 27}}      // #33001b
}};
const std::vector<GradientPoint> atlasPalette = {{
    {0.0f, {254, 172, 94}},   // #feac5e
    {0.5f, {199, 121, 208}},  // #c779d0
    {1.0f, {75, 192, 200}}    // #4bc0c8
}};

const std::vector<GradientPoint> martiniPalette = {{
    {0.0f, {253, 252, 71}},   // #fdfc47
    {1.0f, {36, 254, 65}}    // #24fe41
}};
const std::vector<GradientPoint> virginPalette = {{
    {0.0f, {201, 255, 191}},  // #c9ffbf
    {1.0f, {255, 175, 189}}   // #ffafbd
}};

const std::vector<GradientPoint> miakaPalette = {{
    {0.0f, {252, 53, 76}},   // #fc354c
    {1.0f, {10, 191, 188}}    // #0abfbc
}};

const std::vector<GradientPoint> monteCarloPalette = {{
    {0.0f, {194, 149, 192}},  // #c295c0
    {0.5f, {219, 212, 180}},  // #dbd4b4
    {1.0f, {122, 161, 210}}   // #7aa1d2
}};

const std::vector<GradientPoint> cherryRedPalette = {{
    {0.0f, {230, 87, 88}},   // #E65758
    {1.0f, {119, 29, 50}}    // #771D32
}};

const std::vector<GradientPoint> summerVibesPalette = {{
    {0.0f, {142, 202, 230}},  // #8ECAE6
    {0.25f, {33, 158, 188}},   // #219EBC
    {0.5f, {2, 48, 71}},      // #023047
    {0.75f, {255, 183, 3}},    // #FFB703
    {1.0f, {251, 133, 0}}     // #FB8500
}};

const std::vector<GradientPoint> oliveGardenFeastPalette = {{
    {0.0f, {96, 108, 56}},    // #606C38
    {0.25f, {40, 54, 24}},     // #283618
    {0.5f, {254, 250, 224}},  // #FEFAE0
    {0.75f, {221, 161, 94}},   // #DDA15E
    {1.0f, {188, 108, 37}}    // #BC6C25
}};

const std::vector<GradientPoint> earthyForestHuesPalette = {{
    {0.0f, {218, 215, 205}},  // #DAD7CD
    {0.25f, {163, 177, 138}}, // #A3B18A
    {0.5f, {88, 129, 87}},    // #588157
    {0.75f, {58, 90, 64}},     // #3A5A40
    {1.0f, {52, 78, 65}}      // #344E41
}};

const std::vector<GradientPoint> softSandPalette = {{
    {0.0f, {237, 237, 229}},  // #EDEDE9
    {0.25f, {214, 204, 194}}, // #D6CCC2
    {0.5f, {245, 235, 224}},  // #F5EBEO
    {0.75f, {227, 213, 202}}, // #E3D5CA
    {1.0f, {213, 189, 175}}   // #D5BDAF
}};


const std::vector<GradientPoint> oceanSunsetPalette = {{
    {0.0f, {53, 80, 112}},    // #355070
    {0.25f, {109, 89, 122}},   // #6D597A
    {0.5f, {181, 101, 118}},  // #B56576
    {0.75f, {229, 107, 111}},  // #E56B6F
    {1.0f, {234, 172, 139}}   // #EAAC8B
}};

// Example color palettes
const std::vector<GradientPoint> sunsetPalette = {{
    {0.0f, {255, 94, 0}},    // Deep orange
    {0.5f, {255, 165, 0}},   // Orange
    {1.0f, {255, 223, 186}}  // Light peach
}};

const std::vector<GradientPoint> oceanPalette = {{
    {0.0f, {0, 105, 148}},   // Deep blue
    {0.5f, {0, 168, 232}},   // Medium blue
    {1.0f, {173, 216, 230}}  // Light blue
}};

const std::vector<GradientPoint> forestPalette = {{
    {0.0f, {34, 139, 34}},   // Forest green
    {0.5f, {107, 142, 35}},  // Olive drab
    {1.0f, {144, 238, 144}}  // Light green
}};

const std::vector<GradientPoint> firePalette = {{
    {0.0f, {255, 0, 0}},     // Red
    {0.5f, {255, 140, 0}},   // Dark orange
    {1.0f, {255, 255, 0}}    // Yellow
}};

const std::vector<GradientPoint> icePalette = {{
    {0.0f, {0, 255, 255}},   // Cyan
    {0.5f, {135, 206, 250}}, // Light sky blue
    {1.0f, {255, 250, 250}}  // Snow
}};

const std::vector<GradientPoint> rainbowPalette = {{
    {0.0f, {148, 0, 211}},   // Violet
    {0.2f, {75, 0, 130}},    // Indigo
    {0.4f, {0, 0, 255}},     // Blue
    {0.6f, {0, 255, 0}},     // Green
    {0.8f, {255, 255, 0}},   // Yellow
    {1.0f, {255, 127, 0}}    // Orange
}};

const std::vector<GradientPoint> purpleHazePalette = {{
    {0.0f, {75, 0, 130}},    // Indigo
    {0.5f, {138, 43, 226}},  // Blue violet
    {1.0f, {221, 160, 221}}  // Plum
}};

const std::vector<GradientPoint> desertPalette = {{
    {0.0f, {210, 180, 140}}, // Tan
    {0.5f, {244, 164, 96}},  // Sandy brown
    {1.0f, {255, 228, 181}}  // Moccasin
}};

const std::vector<GradientPoint> neonPalette = {{
    {0.0f, {57, 255, 20}},   // Neon green
    {0.5f, {0, 255, 255}},   // Neon cyan
    {1.0f, {255, 20, 147}}   // Neon pink
}};

const std::vector<GradientPoint> pastelPalette = {{
    {0.0f, {255, 182, 193}}, // Light pink
    {0.5f, {176, 224, 230}}, // Powder blue
    {1.0f, {152, 251, 152}}  // Pale green
}};

//give me a very complicated gradient with many colors
const std::vector<GradientPoint> complexGradientPalette = {{
    {0.0f, {255, 0, 0}},     // Red
    {0.15f, {255, 127, 0}},  // Orange
    {0.3f, {255, 255, 0}},   // Yellow
    {0.45f, {0, 255, 0}},    // Green   
    {0.6f, {0, 0, 255}},     // Blue
    {0.75f, {75, 0, 130}},   // Indigo
    {0.9f, {148, 0, 211}},   // Violet
    {1.0f, {255, 20, 147}}   // Deep pink
}};

const std::vector<GradientPoint> kingYnaPalette = {{
    // Color 1: Deep Blue (#1a2a6c)
    {0.0f, {26, 42, 108}},   
    
    // Color 2: Fiery Red (#b21f1f) - positioned at 50%
    {0.5f, {178, 31, 31}},   
    
    // Color 3: Vibrant Yellow/Orange (#fdbb2d)
    {1.0f, {253, 187, 45}}    
}};


const std::vector<GradientPoint> solidRedPalette = {{
    {0.0f, {255, 0, 0}},
    {1.0f, {255, 0, 0}}
}};
const std::vector<GradientPoint> solidGreenPalette = {{
    {0.0f, {0, 255, 0}},
    {1.0f, {0, 255, 0}}
}};
const std::vector<GradientPoint> solidBluePalette = {{
    {0.0f, {0, 0, 255}},
    {1.0f, {0, 0, 255}}
}};
const std::vector<GradientPoint> solidWhitePalette = {{
    {0.0f, {255, 255, 255}},
    {1.0f, {255, 255, 255}}
}};
const std::vector<GradientPoint> solidYellowPalette = {{
    {0.0f, {255, 255, 0}},
    {1.0f, {255, 255, 0}}
}};
const std::vector<GradientPoint> solidCyanPalette = {{
    {0.0f, {0, 255, 255}},
    {1.0f, {0, 255, 255}}   
}};



// Function to get the color at a specific position along the entire gradient
unsigned int getGradientColor(float position, const std::vector<GradientPoint>& palette, unsigned char brightness); 


unsigned int rgbToInt(const RGBColor& color);






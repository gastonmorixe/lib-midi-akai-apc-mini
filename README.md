# AKAI APC Mini Controller Library

A modern C++ library for controlling the AKAI APC Mini MIDI controller. This library provides a complete interface for handling button inputs, LED control, and fader movements, making it perfect for creating custom light shows, MIDI applications, or integrating the APC Mini into your C++ projects.

## Features

- 🎛 Complete MIDI input/output management
- 💡 Full LED control (8x8 grid + round buttons)
- 🎚 Fader input support
- 🎨 Built-in light pattern animations
- 🔄 Asynchronous event handling
- 🧵 Thread-safe implementation
- 🎯 Easy-to-use callback system

## Requirements

- CMake 3.15 or higher
- C++17 compatible compiler
- RtMidi library (automatically fetched via CMake)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/gastonmorixe/lib-midi-akai-apc-mini.git
cd lib-midi-akai-apc-mini
```

2. Build the project:
```bash
make build
```

Or manually:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Quick Start

Here's a simple example to get you started:

```cpp
#include "apc_mini_controller.hpp"
#include <iostream>

int main() {
    APCMiniController controller;
    
    if (!controller.connect()) {
        std::cerr << "Failed to connect to APC Mini" << std::endl;
        return 1;
    }

    // Set up button callback
    controller.setButtonCallback([](APCMiniController::ButtonType type, int note, bool isPressed) {
        std::cout << "Button " << note << (isPressed ? " pressed" : " released") << std::endl;
    });

    // Set up fader callback
    controller.setFaderCallback([](APCMiniController::Fader fader, int value) {
        std::cout << "Fader " << static_cast<int>(fader) << " value: " << value << std::endl;
    });

    // Light up some LEDs
    controller.setGridLED(0, APCMiniController::LedColor::GREEN);
    controller.setHorizontalLED(APCMiniController::HorizontalButton::STOP_ALL, 
                              APCMiniController::RoundLedState::ON);

    // Keep the program running
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
```

## API Reference

### APCMiniController Class

The main class for interacting with the APC Mini controller.

#### LED Control

```cpp
// Control grid LEDs (8x8 matrix)
void setGridLED(int index, LedColor color);

// Available colors:
enum class LedColor {
    OFF = 0,
    GREEN = 1,
    GREEN_BLINK = 2,
    RED = 3,
    RED_BLINK = 4,
    YELLOW = 5,
    YELLOW_BLINK = 6
};

// Control horizontal button LEDs (bottom row)
void setHorizontalLED(HorizontalButton button, RoundLedState state);

// Control vertical button LEDs (right column)
void setVerticalLED(VerticalButton button, RoundLedState state);

// LED states for round buttons:
enum class RoundLedState {
    OFF,
    ON,
    BLINK
};
```

#### Button and Fader Events

```cpp
// Set button callback
void setButtonCallback(ButtonCallback callback);
// ButtonCallback = std::function<void(ButtonType type, int note, bool isPressed)>

// Set fader callback
void setFaderCallback(FaderCallback callback);
// FaderCallback = std::function<void(Fader fader, int value)>
```

#### Button Types

```cpp
enum class ButtonType {
    GRID,       // 8x8 matrix (0-63)
    HORIZONTAL, // Bottom row (64-71)
    VERTICAL,   // Right column (82-89)
    SPECIAL     // Button 98
};
```

#### Horizontal Buttons

```cpp
enum class HorizontalButton {
    STOP_ALL = 64,
    LEFT = 65,
    RIGHT = 66,
    UP = 67,
    DOWN = 68,
    VOLUME = 69,
    PAN = 70,
    SEND = 71
};
```

#### Vertical Buttons

```cpp
enum class VerticalButton {
    SCENE_1 = 82,
    SCENE_2 = 83,
    SCENE_3 = 84,
    SCENE_4 = 85,
    SCENE_5 = 86,
    SCENE_6 = 87,
    SCENE_7 = 88,
    SCENE_8 = 89
};
```

#### Faders

```cpp
enum class Fader {
    TRACK_1 = 48,
    TRACK_2 = 49,
    TRACK_3 = 50,
    TRACK_4 = 51,
    TRACK_5 = 52,
    TRACK_6 = 53,
    TRACK_7 = 54,
    TRACK_8 = 55,
    MASTER = 56
};
```

## Light Patterns

The library includes a `LightPatternController` class that provides several pre-built light patterns:

1. Snake Pattern (Button 64)
2. Rainfall Pattern (Button 65)
3. Color Wave (Button 66)
4. Expanding Square (Button 67)
5. Sparkle Pattern (Button 68)
6. Checkerboard (Button 69)
7. Spiral Pattern (Button 70)
8. Binary Counter (Button 71)

To use the light patterns:

```cpp
#include "light_pattern_controller.hpp"

APCMiniController controller;
LightPatternController patternController(controller);

// Start a pattern
patternController.startPattern(64); // Start snake pattern

// Stop current pattern
patternController.stopCurrentPattern();
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the BSD 2-Clause License - see the LICENSE file for details.

## Author

Gaston Morixe (gaston@gastonmorixe.com)

## Acknowledgments

- Built with [RtMidi](https://github.com/thestk/rtmidi) for MIDI communication
- Thanks to the AKAI Professional team for the APC Mini hardware specifications

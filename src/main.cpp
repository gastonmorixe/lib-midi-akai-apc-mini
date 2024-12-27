#include "apc_mini_controller.hpp"
#include "light_pattern_controller.cpp"

#include <iostream>

int main() {
  try {
    APCMiniController controller;

    if (!controller.connect()) {
      std::cerr << "Failed to connect to APC Mini" << std::endl;
      return 1;
    }

    LightPatternController patternController(controller);

    // Set up button callback
    controller.setButtonCallback([&patternController](int note, bool isPressed) {
      std::cout << "Button " << note << (isPressed ? " pressed" : " released") << std::endl;

      // Only respond to press events for the round buttons
      if (isPressed && note >= 64 && note <= 71) {
        patternController.startPattern(note);
      }
    });

    // Start MIDI processing
    controller.start();

    std::cout << "APC Mini Light Show Ready!" << std::endl;
    std::cout << "Press round buttons (64-71) for different patterns:" << std::endl;
    std::cout << " 64: Snake Pattern" << std::endl;
    std::cout << " 65: Rainbow Wave" << std::endl;
    std::cout << " 66: Random Blink" << std::endl;
    std::cout << " 67: Spiral Pattern" << std::endl;
    std::cout << " 68: Raindrop Effect" << std::endl;
    std::cout << " 69: Color Pulse" << std::endl;
    std::cout << " 70: Chessboard" << std::endl;
    std::cout << " 71: Four Corners" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;

    std::cin.get();

    patternController.stopCurrentPattern();
    controller.stop();
    return 0;
  } catch (RtMidiError &error) {
    error.printMessage();
    return 1;
  }
}

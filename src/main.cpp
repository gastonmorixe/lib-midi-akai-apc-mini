#include "../include/apc_mini_controller.hpp"

int main() {
  try {
    APCMiniController controller;

    if (!controller.connect()) {
      std::cerr << "Failed to connect to APC Mini" << std::endl;
      return 1;
    }

    // Set up button callback
    controller.setButtonCallback(
        [](int note, bool isPressed) { std::cout << "Button " << note << (isPressed ? " pressed" : " released") << std::endl; });

    // Set up fader callback
    controller.setFaderCallback([](int index, int value) { std::cout << "Fader " << index << " value: " << value << std::endl; });

    // Start MIDI processing
    controller.start();

    // Example: Set some LED colors
    controller.setClipLED(0, APCMiniController::LedColor::GREEN);
    controller.setClipLED(1, APCMiniController::LedColor::RED);
    controller.setClipLED(2, APCMiniController::LedColor::YELLOW);

    // Keep program running
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    controller.stop();
    return 0;
  } catch (RtMidiError &error) {
    error.printMessage();
    return 1;
  }
}

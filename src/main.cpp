#include "apc_mini_controller.hpp"
#include "light_pattern_controller.cpp"

#include <iomanip>
#include <iostream>
#include <signal.h>

std::unique_ptr<LightPatternController> patternController;
volatile sig_atomic_t keep_running = 1;

void signalHandler(int) { keep_running = 0; }

int main() {
  try {
    signal(SIGINT, signalHandler);

    // Initialize controller
    APCMiniController controller;

    if (!controller.connect()) {
      std::cerr << "Failed to connect to APC Mini" << std::endl;
      return 1;
    }

    // Initialize pattern controller
    patternController = std::make_unique<LightPatternController>(controller);

    // Track button states
    std::unordered_map<int, bool> buttonStates;

    // Setup callbacks
    controller.setButtonCallback([&controller, &buttonStates](APCMiniController::ButtonType type, int note, bool isPressed) {
      if (type == APCMiniController::ButtonType::HORIZONTAL) {
        if (isPressed) {
          controller.setHorizontalLED(static_cast<APCMiniController::HorizontalButton>(note), APCMiniController::RoundLedState::ON);
          buttonStates[note] = true;
        } else if (buttonStates[note]) {
          controller.setHorizontalLED(static_cast<APCMiniController::HorizontalButton>(note), APCMiniController::RoundLedState::OFF);
          buttonStates[note] = false;
        }

        if (isPressed) {
          patternController->startPattern(note);
        }
      } else if (type == APCMiniController::ButtonType::VERTICAL) {
        if (isPressed) {
          controller.setVerticalLED(static_cast<APCMiniController::VerticalButton>(note), APCMiniController::RoundLedState::ON);
          buttonStates[note] = true;
        } else if (buttonStates[note]) {
          controller.setVerticalLED(static_cast<APCMiniController::VerticalButton>(note), APCMiniController::RoundLedState::OFF);
          buttonStates[note] = false;
        }
      }
    });

    // Start MIDI processing
    controller.start();

    // Print available patterns
    std::cout << "\nAPC Mini Light Controller Ready!\n"
              << "Available patterns (bottom round buttons):\n"
              << "64: Snake Pattern\n"
              << "65: Rainfall Pattern\n"
              << "66: Color Wave\n"
              << "67: Expanding Square\n"
              << "68: Sparkle Pattern\n"
              << "69: Checkerboard\n"
              << "70: Spiral Pattern\n"
              << "71: Binary Counter\n"
              << "\nPress Ctrl+C to exit...\n"
              << std::endl;

    // Main loop
    while (keep_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    std::cout << "\nShutting down..." << std::endl;
    patternController->stopCurrentPattern();
    controller.stop();

    return 0;
  } catch (RtMidiError &error) {
    error.printMessage();
    return 1;
  }
}

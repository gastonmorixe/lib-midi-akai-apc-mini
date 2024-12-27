#include "apc_mini_controller.hpp"
#include "light_pattern_controller.hpp"
#include <signal.h>

volatile sig_atomic_t keep_running = 1;
void signalHandler(int) { keep_running = 0; }

int main() {
  try {
    signal(SIGINT, signalHandler);
    APCMiniController controller;

    if (!controller.connect()) {
      std::cerr << "Failed to connect to APC Mini" << std::endl;
      return 1;
    }

    LightPatternController patternController(controller);

    std::unordered_map<int, bool> buttonStates;

    std::cout << "main() Thread ID: " << std::this_thread::get_id() << std::endl;

    controller.setButtonCallback(
        [&controller, &patternController, &buttonStates](APCMiniController::ButtonType type, int note, bool isPressed) {
          std::cout << "setButtonCallback Thread ID: " << std::this_thread::get_id() << std::endl;

          if (type == APCMiniController::ButtonType::HORIZONTAL) {
            if (isPressed) {
              controller.setHorizontalLED(static_cast<APCMiniController::HorizontalButton>(note), APCMiniController::RoundLedState::ON);
              buttonStates[note] = true;
              patternController.startPattern(note);
            } else {
              controller.setHorizontalLED(static_cast<APCMiniController::HorizontalButton>(note), APCMiniController::RoundLedState::OFF);
              buttonStates[note] = false;
            }
          } else if (type == APCMiniController::ButtonType::VERTICAL) {
            if (isPressed) {
              controller.setVerticalLED(static_cast<APCMiniController::VerticalButton>(note), APCMiniController::RoundLedState::ON);
              buttonStates[note] = true;
            } else {
              controller.setVerticalLED(static_cast<APCMiniController::VerticalButton>(note), APCMiniController::RoundLedState::OFF);
              buttonStates[note] = false;
            }
          }
        });

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

    while (keep_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "\nShutting down..." << std::endl;
    patternController.stopCurrentPattern();
    controller.disconnect();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}

#include "../include/apc_mini_controller.hpp"

APCMiniController::APCMiniController() {
  try {
    midiIn = std::make_unique<RtMidiIn>();
    midiOut = std::make_unique<RtMidiOut>();

    // Find AKAI APC MINI ports
    findAPCMiniPorts();
  } catch (RtMidiError &error) {
    error.printMessage();
    throw;
  }
}

APCMiniController::~APCMiniController() {
  if (isRunning) {
    stop();
  }
}

bool APCMiniController::connect() {
  if (inputPort == -1 || outputPort == -1) {
    std::cerr << "APC Mini ports not found!" << std::endl;
    return false;
  }

  try {
    midiIn->openPort(inputPort);
    midiOut->openPort(outputPort);

    // Don't ignore sysex, timing, or active sensing messages
    midiIn->ignoreTypes(false, false, false);

    return true;
  } catch (RtMidiError &error) {
    error.printMessage();
    return false;
  }
}

void APCMiniController::start() {
  if (isRunning)
    return;

  isRunning = true;
  midiThread = std::thread(&APCMiniController::midiInputLoop, this);
}

void APCMiniController::stop() {
  if (!isRunning)
    return;

  isRunning = false;
  if (midiThread.joinable()) {
    midiThread.join();
  }
}

void APCMiniController::setClipLED(int index, LedColor color) {
  if (index < 0 || index > 63)
    return;

  int row = index / 8;
  int col = index % 8;

  std::vector<unsigned char> message = {0x90, // Note On
                                        static_cast<unsigned char>(0x35 + col + (row * 8)), static_cast<unsigned char>(color)};

  midiOut->sendMessage(&message);
}

void APCMiniController::setFader(int index, int value) {
  if (index < 0 || index > 8)
    return;
  if (value < 0 || value > 127)
    return;

  std::vector<unsigned char> message = {0xB0, // Control Change
                                        static_cast<unsigned char>(48 + index), static_cast<unsigned char>(value)};

  midiOut->sendMessage(&message);
}

void APCMiniController::setButtonCallback(std::function<void(int, bool)> callback) { buttonCallback = callback; }

void APCMiniController::setFaderCallback(std::function<void(int, int)> callback) { faderCallback = callback; }

void APCMiniController::findAPCMiniPorts() {
  // Look for "APC MINI" in port names
  for (unsigned int i = 0; i < midiIn->getPortCount(); i++) {
    std::string portName = midiIn->getPortName(i);
    if (portName.find("APC MINI") != std::string::npos) {
      inputPort = i;
      break;
    }
  }

  for (unsigned int i = 0; i < midiOut->getPortCount(); i++) {
    std::string portName = midiOut->getPortName(i);
    if (portName.find("APC MINI") != std::string::npos) {
      outputPort = i;
      break;
    }
  }
}

void APCMiniController::midiInputLoop() {
  std::vector<unsigned char> message;

  while (isRunning) {
    message.clear();
    midiIn->getMessage(&message);

    if (message.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    // Process MIDI message
    if (message.size() >= 3) {
      unsigned char status = message[0] & 0xF0;
      unsigned char data1 = message[1];
      unsigned char data2 = message[2];

      switch (status) {
      case 0x90: // Note On/Off (buttons)
        if (buttonCallback) {
          buttonCallback(data1, data2 > 0);
        }
        break;

      case 0xB0: // Control Change (faders)
        if (faderCallback) {
          // Convert CC number to fader index (48-56)
          int faderIndex = data1 - 48;
          if (faderIndex >= 0 && faderIndex <= 8) {
            faderCallback(faderIndex, data2);
          }
        }
        break;
      }
    }
  }
}

#include "apc_mini_controller.hpp"
#include <cstdio>
#include <iostream>
#include <map>
#include <ostream>

const std::vector<std::vector<int>> APCMiniController::GRID_LAYOUT = {
    {56, 57, 58, 59, 60, 61, 62, 63}, // Row 1 (Top)
    {48, 49, 50, 51, 52, 53, 54, 55}, {40, 41, 42, 43, 44, 45, 46, 47}, {32, 33, 34, 35, 36, 37, 38, 39}, {24, 25, 26, 27, 28, 29, 30, 31},
    {16, 17, 18, 19, 20, 21, 22, 23}, {8, 9, 10, 11, 12, 13, 14, 15},   {0, 1, 2, 3, 4, 5, 6, 7} // Row 8 (Bottom)
};

APCMiniController::APCMiniController() {
  try {
    midiIn = std::make_unique<RtMidiIn>();
    midiOut = std::make_unique<RtMidiOut>();
  } catch (RtMidiError &error) {
    std::cerr << "RtMidi error: " << error.getMessage() << std::endl;
    throw;
  }
}

APCMiniController::~APCMiniController() { disconnect(); }

void APCMiniController::midiCallback(double, std::vector<unsigned char> *message, void *userData) {
  auto controller = static_cast<APCMiniController *>(userData);
  auto messageCopy = *message; // Copy the message

  std::unique_lock<std::mutex> lock(controller->callbackMutex);
  controller->pendingCallback = [controller, messageCopy]() { controller->handleMidiMessage(messageCopy); };
  // std::cout << "Notifying from midiCallback" << std::endl;
  controller->callbackCV.notify_one();
}

void APCMiniController::processCallback() {
  // std::cout << "ProcessCallback Thread ID: " << std::this_thread::get_id() << std::endl;
  while (isConnected()) {
    {
      std::unique_lock<std::mutex> lock(callbackMutex);
      // std::cout << "Waiting with isConnected: " << isConnected() << std::endl;
      if (callbackCV.wait_for(lock, std::chrono::seconds(1), [this]() { return pendingCallback != nullptr; })) {
        // std::cout << "Callback received, executing" << std::endl;
        pendingCallback();
        pendingCallback = nullptr;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  // std::cout << "ProcessCallback exiting" << std::endl;
}

bool APCMiniController::findAndOpenPorts() {
  // Find APC Mini ports
  int inputPort = -1, outputPort = -1;

  for (unsigned int i = 0; i < midiIn->getPortCount(); i++) {
    if (midiIn->getPortName(i).find("APC MINI") != std::string::npos) {
      inputPort = i;
      break;
    }
  }

  for (unsigned int i = 0; i < midiOut->getPortCount(); i++) {
    if (midiOut->getPortName(i).find("APC MINI") != std::string::npos) {
      outputPort = i;
      break;
    }
  }

  if (inputPort == -1 || outputPort == -1) {
    std::cerr << "APC Mini ports not found" << std::endl;
    return false;
  }

  try {
    midiIn->openPort(inputPort);
    midiOut->openPort(outputPort);
    return true;
  } catch (RtMidiError &error) {
    std::cerr << "Error opening ports: " << error.getMessage() << std::endl;
    return false;
  }
}

bool APCMiniController::connect() {
  if (!findAndOpenPorts())
    return false;

  // Set up callback thread first
  callbackThread = std::make_unique<std::thread>(&APCMiniController::processCallback, this);

  // Then set up MIDI callback
  midiIn->setCallback(&midiCallback, this);
  midiIn->ignoreTypes(false, false, false);

  return true;
}

void APCMiniController::disconnect() {
  if (midiIn)
    midiIn->closePort();
  if (midiOut)
    midiOut->closePort();
  if (callbackThread && callbackThread->joinable()) {
    callbackThread->join();
  }
}

// In sync
// void APCMiniController::midiCallback(double, std::vector<unsigned char> *message, void *userData) {
//   auto controller = static_cast<APCMiniController *>(userData);
//   controller->handleMidiMessage(*message);
// }

void APCMiniController::handleMidiMessage(const std::vector<unsigned char> &message) {
  if (message.size() < 3)
    return;

  unsigned char status = message[0] & 0xF0;
  unsigned char data1 = message[1];
  unsigned char data2 = message[2];

  if (status == 0x90 || status == 0x80) {
    bool isPressed = (status == 0x90 && data2 > 0);
    try {
      ButtonType type = getButtonType(data1);
      if (buttonCallback) {
        buttonCallback(type, data1, isPressed);
      }
      // Add logging
      std::cout << "Button: " << buttonTypeToString(type) << " [" << static_cast<int>(data1) << " " << getButtonName(data1) << "] "
                << (isPressed ? "pressed" : "released") << std::endl;
    } catch (const std::runtime_error &) {
    }
  } else if (status == 0xB0 && data1 >= 48 && data1 <= 56) {
    if (faderCallback) {
      faderCallback(static_cast<Fader>(data1), data2);
    }
    // Add logging
    std::cout << "Fader: (" << static_cast<int>(data1) << ") " << getFaderName(data1) << " = " << static_cast<int>(data2) << std::endl;
  }
}

void APCMiniController::sendMidiMessage(const std::vector<unsigned char> &message) {
  if (!midiOut || !midiOut->isPortOpen())
    return;
  try {
    midiOut->sendMessage(&message);
  } catch (RtMidiError &error) {
    std::cerr << "Error sending MIDI message: " << error.getMessage() << std::endl;
  }
}

void APCMiniController::setGridLED(int index, LedColor color) {
  if (index < 0 || index > 63)
    return;
  int row = index / 8;
  int col = index % 8;
  int note = GRID_LAYOUT[row][col];
  sendMidiMessage({0x90, static_cast<unsigned char>(note), static_cast<unsigned char>(color)});
}

void APCMiniController::setHorizontalLED(HorizontalButton button, RoundLedState state) {
  sendMidiMessage({0x90, static_cast<unsigned char>(button),
                   static_cast<unsigned char>(state == RoundLedState::BLINK ? 2 : (state == RoundLedState::ON ? 1 : 0))});
}

void APCMiniController::setVerticalLED(VerticalButton button, RoundLedState state) {
  sendMidiMessage({0x90, static_cast<unsigned char>(button),
                   static_cast<unsigned char>(state == RoundLedState::BLINK ? 2 : (state == RoundLedState::ON ? 1 : 0))});
}

APCMiniController::ButtonType APCMiniController::getButtonType(int note) {
  if (note >= 0 && note <= 63)
    return ButtonType::GRID;
  if (note >= 64 && note <= 71)
    return ButtonType::HORIZONTAL;
  if (note >= 82 && note <= 89)
    return ButtonType::VERTICAL;
  if (note == 98)
    return ButtonType::SPECIAL;
  throw std::runtime_error("Invalid note number");
}

std::string APCMiniController::buttonTypeToString(ButtonType type) {
  switch (type) {
  case ButtonType::GRID:
    return "Grid";
  case ButtonType::HORIZONTAL:
    return "Horizontal";
  case ButtonType::VERTICAL:
    return "Vertical";
  case ButtonType::SPECIAL:
    return "Special";
  default:
    return "Unknown";
  }
}

std::string APCMiniController::getButtonName(int note) {
  if (note >= 0 && note <= 63) {
    int row = note / 8;
    int col = note % 8;
    return "Grid[" + std::to_string(row) + "," + std::to_string(col) + "]";
  }

  static const std::map<int, std::string> buttonNames = {
      {64, "Stop All"}, {65, "Left"},    {66, "Right"},   {67, "Up"},      {68, "Down"},    {69, "Volume"},
      {70, "Pan"},      {71, "Send"},    {82, "Scene 1"}, {83, "Scene 2"}, {84, "Scene 3"}, {85, "Scene 4"},
      {86, "Scene 5"},  {87, "Scene 6"}, {88, "Scene 7"}, {89, "Scene 8"}, {98, "Shift"}};

  auto it = buttonNames.find(note);
  return it != buttonNames.end() ? it->second : "Unknown";
}

std::string APCMiniController::getFaderName(int fader) {
  if (fader == 56)
    return "Master";
  if (fader >= 48 && fader <= 55)
    return "Track " + std::to_string(fader - 47);
  return "Unknown Fader";
}

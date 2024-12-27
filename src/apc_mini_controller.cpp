#include "apc_mini_controller.hpp"
#include <sys/types.h>

const std::vector<std::vector<int>> APCMiniController::GRID_LAYOUT = {
    {56, 57, 58, 59, 60, 61, 62, 63}, // Row 1 (Top)
    {48, 49, 50, 51, 52, 53, 54, 55}, // Row 2
    {40, 41, 42, 43, 44, 45, 46, 47}, // Row 3
    {32, 33, 34, 35, 36, 37, 38, 39}, // Row 4
    {24, 25, 26, 27, 28, 29, 30, 31}, // Row 5
    {16, 17, 18, 19, 20, 21, 22, 23}, // Row 6
    {8, 9, 10, 11, 12, 13, 14, 15},   // Row 7
    {0, 1, 2, 3, 4, 5, 6, 7}          // Row 8 (Bottom)
};

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

APCMiniController::APCMiniController() {
  try {
    midiIn = std::make_unique<RtMidiIn>();
    midiOut = std::make_unique<RtMidiOut>();
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

bool APCMiniController::connect() {
  if (inputPort == -1 || outputPort == -1) {
    std::cerr << "APC Mini ports not found!" << std::endl;
    return false;
  }

  try {
    midiIn->openPort(inputPort);
    midiOut->openPort(outputPort);
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
  midiInThread = std::thread(&APCMiniController::midiInputLoop, this);
  midiOutThread = std::thread(&APCMiniController::midiOutputLoop, this);
}

void APCMiniController::stop() {
  if (!isRunning)
    return;
  isRunning = false;
  queueCV.notify_all();
  if (midiInThread.joinable())
    midiInThread.join();
  if (midiOutThread.joinable())
    midiOutThread.join();
}

void APCMiniController::setButtonCallback(ButtonCallback callback) { buttonCallback = callback; }

void APCMiniController::setFaderCallback(FaderCallback callback) { faderCallback = callback; }

void APCMiniController::findAPCMiniPorts() {
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

void APCMiniController::midiOutputLoop() {
  while (isRunning) {
    MidiMessage msg;
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      if (midiOutQueue.empty()) {
        queueCV.wait_for(lock, std::chrono::milliseconds(1));
        continue;
      }
      msg = std::move(midiOutQueue.front());
      midiOutQueue.pop();
    }
    midiOut->sendMessage(&msg.data);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

void APCMiniController::sendMidiMessage(const std::vector<unsigned char> &message) {
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    midiOutQueue.push({message});
  }
  queueCV.notify_one();
}

void APCMiniController::midiInputLoop() {
  std::vector<unsigned char> message;

  while (isRunning) {
    message.clear();
    midiIn->getMessage(&message);

    if (!message.empty() && message.size() >= 3) {
      unsigned char status = message[0] & 0xF0;
      unsigned char data1 = message[1];
      unsigned char data2 = message[2];

      if (status == 0x90 || status == 0x80) {
        bool isPressed = (status == 0x90 && data2 > 0);
        try {
          ButtonType type = getButtonType(data1);
          if (buttonCallback)
            buttonCallback(type, data1, isPressed);
          std::cout << "Button: " << buttonTypeToString(type) << " [" << static_cast<uint>(data1) << " " << getButtonName(data1) << "] "
                    << (isPressed ? "pressed" : "released") << std::endl;
        } catch (const std::runtime_error &) {
        }
      } else if (status == 0xB0 && data1 >= 48 && data1 <= 56) {
        if (faderCallback)
          faderCallback(static_cast<Fader>(data1), data2);
        std::cout << "Fader: (" << static_cast<uint>(data1) << ") " << getFaderName(data1) << " = " << static_cast<int>(data2) << std::endl;
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

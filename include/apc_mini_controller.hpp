#ifndef APC_MINI_CONTROLLER_HPP
#define APC_MINI_CONTROLLER_HPP

#include <RtMidi.h>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class APCMiniController {
public:
  enum class ButtonType {
    GRID,       // 8x8 matrix (0-63)
    HORIZONTAL, // Bottom row (64-71)
    VERTICAL,   // Right column (82-89)
    SPECIAL     // Button 98
  };

  enum class LedColor { OFF = 0, GREEN = 1, GREEN_BLINK = 2, RED = 3, RED_BLINK = 4, YELLOW = 5, YELLOW_BLINK = 6 };

  enum class HorizontalButton { STOP_ALL = 64, LEFT = 65, RIGHT = 66, UP = 67, DOWN = 68, VOLUME = 69, PAN = 70, SEND = 71 };

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

  enum class RoundLedState { OFF, ON, BLINK };

  using ButtonCallback = std::function<void(ButtonType type, int note, bool isPressed)>;
  using FaderCallback = std::function<void(Fader fader, int value)>;

  APCMiniController();
  ~APCMiniController();

  bool connect();
  void disconnect();
  bool isConnected() const { return midiIn && midiIn->isPortOpen(); }

  void setGridLED(int index, LedColor color);
  void setHorizontalLED(HorizontalButton button, RoundLedState state);
  void setVerticalLED(VerticalButton button, RoundLedState state);

  void setButtonCallback(ButtonCallback callback) { buttonCallback = callback; }
  void setFaderCallback(FaderCallback callback) { faderCallback = callback; }

  static ButtonType getButtonType(int note);
  static std::string buttonTypeToString(ButtonType type);
  static std::string getButtonName(int note);
  static std::string getFaderName(int fader);

private:
  std::unique_ptr<std::thread> callbackThread;
  std::function<void()> pendingCallback;
  std::mutex callbackMutex;
  std::condition_variable callbackCV;

  static void midiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData);
  void processCallback();

  std::unique_ptr<RtMidiIn> midiIn;
  std::unique_ptr<RtMidiOut> midiOut;
  ButtonCallback buttonCallback;
  FaderCallback faderCallback;

  void handleMidiMessage(const std::vector<unsigned char> &message);
  void sendMidiMessage(const std::vector<unsigned char> &message);
  bool findAndOpenPorts();

  static const std::vector<std::vector<int>> GRID_LAYOUT;
};

#endif

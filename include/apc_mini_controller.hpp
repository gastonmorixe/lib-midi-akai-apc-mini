#ifndef APC_MINI_CONTROLLER_HPP
#define APC_MINI_CONTROLLER_HPP

#include <RtMidi.h>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class APCMiniController {
public:
  // LED colors for the clip launch grid
  enum class LedColor {
    OFF = 0,
    GREEN = 1,
    GREEN_BLINK = 2,
    RED = 3,
    RED_BLINK = 4,
    YELLOW = 5,
    YELLOW_BLINK = 6
  };

  // Constructor & Destructor
  APCMiniController();
  ~APCMiniController();

  // Connection management
  bool connect();
  void start();
  void stop();

  // Control methods
  void setClipLED(int index, LedColor color);
  void setFader(int index, int value);

  // Callback registration
  void setButtonCallback(std::function<void(int, bool)> callback);
  void setFaderCallback(std::function<void(int, int)> callback);

private:
  std::unique_ptr<RtMidiIn> midiIn;
  std::unique_ptr<RtMidiOut> midiOut;
  int inputPort = -1;
  int outputPort = -1;

  std::atomic<bool> isRunning{false};
  std::thread midiThread;

  std::function<void(int, bool)> buttonCallback;
  std::function<void(int, int)> faderCallback;

  // Private helper methods
  void findAPCMiniPorts();
  void midiInputLoop();
};

#endif // APC_MINI_CONTROLLER_HPP

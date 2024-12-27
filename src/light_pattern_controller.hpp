#pragma once
#include "apc_mini_controller.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>

class LightPatternController {
private:
  APCMiniController &controller;
  std::atomic<int> currentPattern{-1};
  std::unique_ptr<std::thread> animationThread;
  std::mt19937 rng{std::random_device{}()};
  std::atomic<bool> isRunning{false};

  void clearGrid();
  void animationLoop();
  static const std::vector<std::pair<int, int>> SPIRAL_PATH;
  using PatternFunc = std::function<void(APCMiniController &, std::atomic<int> &, std::mt19937 &)>;
  static const std::unordered_map<int, PatternFunc> patterns;

public:
  explicit LightPatternController(APCMiniController &ctrl);
  ~LightPatternController();
  void startPattern(int buttonIndex);
  void stopCurrentPattern();
};

#include "light_pattern_controller.hpp"

#include <chrono>
#include <cmath>
#include <functional>
#include <random>
#include <thread>
#include <unordered_map>

const std::vector<std::pair<int, int>> LightPatternController::SPIRAL_PATH = {
    {3, 3}, {3, 4}, {4, 4}, {4, 3}, {3, 2}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {5, 4}, {5, 3}, {5, 2}, {5, 1},
    {4, 1}, {3, 1}, {2, 1}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}, {6, 5}, {6, 4},
    {6, 3}, {6, 2}, {6, 1}, {6, 0}, {5, 0}, {4, 0}, {3, 0}, {2, 0}, {1, 0}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6},
    {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}, {7, 7}, {7, 6}, {7, 5}, {7, 4}, {7, 3}, {7, 2}, {7, 1}, {7, 0}};

LightPatternController::LightPatternController(APCMiniController &ctrl) : controller(ctrl) {
  isRunning = true;
  animationThread = std::make_unique<std::thread>(&LightPatternController::animationLoop, this);
}

LightPatternController::~LightPatternController() {
  isRunning = false;
  if (animationThread && animationThread->joinable()) {
    animationThread->join();
  }
}

void LightPatternController::clearGrid() {
  for (int i = 0; i < 64; i++) {
    controller.setGridLED(i, APCMiniController::LedColor::OFF);
  }
}

void LightPatternController::startPattern(int buttonIndex) { currentPattern = buttonIndex; }

void LightPatternController::stopCurrentPattern() {
  clearGrid();
  currentPattern = -1;
}

void LightPatternController::animationLoop() {
  std::cout << "LightPatternController Thread ID: " << std::this_thread::get_id() << std::endl;

  using namespace std::chrono;
  auto nextFrame = steady_clock::now();

  while (isRunning) {
    auto now = steady_clock::now();
    if (now >= nextFrame) {
      if (auto pattern = currentPattern.load()) {
        if (auto it = patterns.find(pattern); it != patterns.end()) {
          it->second(controller, currentPattern, rng);
        }
      }
      nextFrame = now + milliseconds(100);
    }
    std::this_thread::yield();
  }
}

const std::unordered_map<int, LightPatternController::PatternFunc> 
LightPatternController::patterns = {
   // Snake Pattern
   {64, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static int pos = 0;
       for(int i = 0; i < 64; i++) {
           ctrl.setGridLED(i, i == pos ? 
               APCMiniController::LedColor::GREEN_BLINK : 
               APCMiniController::LedColor::OFF);
       }
       pos = (pos + 1) % 64;
   }},

   // Rainfall Pattern  
   {65, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937& rng) {
       static std::vector<int> drops(8, -1);
       static std::uniform_int_distribution<> dist(0, 7);

       for(int col = 0; col < 8; col++) {
           if(drops[col] == -1 && dist(rng) == 0) {
               drops[col] = 0;
           }
           if(drops[col] >= 0) {
               ctrl.setGridLED(drops[col] * 8 + col,
                   APCMiniController::LedColor::YELLOW_BLINK);
               drops[col]++;
               if(drops[col] >= 8) drops[col] = -1;
           }
       }
   }},

   // Color Wave
   {66, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static int wave = 0;
       for(int row = 0; row < 8; row++) {
           for(int col = 0; col < 8; col++) {
               int phase = (row + col + wave) % 3;
               APCMiniController::LedColor color;
               switch(phase) {
                   case 0: color = APCMiniController::LedColor::GREEN; break;
                   case 1: color = APCMiniController::LedColor::RED; break;
                   case 2: color = APCMiniController::LedColor::YELLOW; break;
                   default: color = APCMiniController::LedColor::OFF;
               }
               ctrl.setGridLED(row * 8 + col, color);
           }
       }
       wave = (wave + 1) % 16;
   }},

   // Expanding Square
   {67, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static int size = 0;
       for(int i = -size; i <= size; i++) {
           for(int j = -size; j <= size; j++) {
               if(abs(i) == size || abs(j) == size) {
                   int row = 3 + i;
                   int col = 3 + j;
                   if(row >= 0 && row < 8 && col >= 0 && col < 8) {
                       ctrl.setGridLED(row * 8 + col, 
                           APCMiniController::LedColor::RED_BLINK);
                   }
               }
           }
       }
       size = (size + 1) % 4;
   }},

   // Sparkle Pattern
   {68, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937& rng) {
       static std::uniform_int_distribution<> pos_dist(0, 63);
       static std::uniform_int_distribution<> color_dist(0, 2);

       for(int i = 0; i < 5; i++) {
           int pos = pos_dist(rng);
           APCMiniController::LedColor color;
           switch(color_dist(rng)) {
               case 0: color = APCMiniController::LedColor::GREEN_BLINK; break;
               case 1: color = APCMiniController::LedColor::RED_BLINK; break;
               default: color = APCMiniController::LedColor::YELLOW_BLINK;
           }
           ctrl.setGridLED(pos, color);
       }

       for(int i = 0; i < 3; i++) {
           ctrl.setGridLED(pos_dist(rng), APCMiniController::LedColor::OFF);
       }
   }},

   // Checkerboard Pattern
   {69, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static bool alternate = false;
       for(int row = 0; row < 8; row++) {
           for(int col = 0; col < 8; col++) {
               bool isEven = (row + col) % 2 == 0;
               ctrl.setGridLED(
                   row * 8 + col,
                   (isEven != alternate) ? 
                       APCMiniController::LedColor::GREEN : 
                       APCMiniController::LedColor::RED
               );
           }
       }
       alternate = !alternate;
   }},

   // Spiral Pattern
   {70, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static size_t currentPos = 0;
       if(currentPos < SPIRAL_PATH.size()) {
           const auto& pos = SPIRAL_PATH[currentPos];
           ctrl.setGridLED(
               pos.first * 8 + pos.second,
               APCMiniController::LedColor::YELLOW_BLINK
           );
           currentPos = (currentPos + 1) % SPIRAL_PATH.size();
       }
   }},

   // Binary Counter
   {71, [](APCMiniController& ctrl, std::atomic<int>& pattern, std::mt19937&) {
       static int count = 0;
       for(int bit = 0; bit < 8; bit++) {
           if(count & (1 << bit)) {
               for(int row = 0; row < 8; row++) {
                   ctrl.setGridLED(row * 8 + bit,
                       APCMiniController::LedColor::GREEN);
               }
           }
       }
       count = (count + 1) % 256;
   }}
};

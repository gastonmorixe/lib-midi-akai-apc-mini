#include "apc_mini_controller.hpp"

#include <chrono>
#include <random>

class LightPatternController {
private:
    APCMiniController& controller;
    std::random_device rd;
    std::mt19937 gen;
    bool isAnimating = false;
    std::thread animationThread;

    void clearGrid() {
        for (int i = 0; i < 64; i++) {
            controller.setGridLED(i, APCMiniController::LedColor::OFF);
        }
    }

    // Pattern 1: Snake Pattern (top to bottom, left to right)
    void snakePattern() {
        while (isAnimating) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    clearGrid();
                    controller.setGridLED(row * 8 + col, APCMiniController::LedColor::GREEN_BLINK);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (!isAnimating) return;
                }
            }
        }
    }

    // Pattern 2: Rainfall Pattern
    void rainfallPattern() {
        std::uniform_int_distribution<> col_dist(0, 7);
        std::vector<int> raindrops(8, -1); // Current position of drops in each column

        while (isAnimating) {
            // Randomly start new drops
            for (int col = 0; col < 8; col++) {
                if (raindrops[col] == -1 && col_dist(gen) == 0) {
                    raindrops[col] = 0;
                }
            }

            // Clear grid
            clearGrid();

            // Update and draw drops
            for (int col = 0; col < 8; col++) {
                if (raindrops[col] >= 0) {
                    int index = raindrops[col] * 8 + col;
                    controller.setGridLED(index, APCMiniController::LedColor::YELLOW_BLINK);
                    raindrops[col]++;
                    if (raindrops[col] >= 8) raindrops[col] = -1;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!isAnimating) return;
        }
    }

    // Pattern 3: Color Wave
    void colorWavePattern() {
        while (isAnimating) {
            for (int wave = 0; wave < 16; wave++) {
                for (int row = 0; row < 8; row++) {
                    for (int col = 0; col < 8; col++) {
                        int phase = (row + col + wave) % 3;
                        APCMiniController::LedColor color;
                        switch(phase) {
                            case 0: color = APCMiniController::LedColor::GREEN; break;
                            case 1: color = APCMiniController::LedColor::RED; break;
                            case 2: color = APCMiniController::LedColor::YELLOW; break;
                            default: color = APCMiniController::LedColor::OFF;
                        }
                        controller.setGridLED(row * 8 + col, color);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!isAnimating) return;
            }
        }
    }

    // Pattern 4: Expanding Square
    void expandingSquarePattern() {
        while (isAnimating) {
            for (int size = 0; size < 4; size++) {
                clearGrid();
                // Draw square border
                for (int i = -size; i <= size; i++) {
                    for (int j = -size; j <= size; j++) {
                        if (abs(i) == size || abs(j) == size) {
                            int row = 3 + i;
                            int col = 3 + j;
                            if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                                controller.setGridLED(row * 8 + col, APCMiniController::LedColor::RED_BLINK);
                            }
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!isAnimating) return;
            }
        }
    }

    // Pattern 5: Random Sparkles
    void sparklePattern() {
        std::uniform_int_distribution<> pos_dist(0, 63);
        std::uniform_int_distribution<> color_dist(0, 2);
        
        while (isAnimating) {
            for (int i = 0; i < 5; i++) {
                int pos = pos_dist(gen);
                APCMiniController::LedColor color;
                switch(color_dist(gen)) {
                    case 0: color = APCMiniController::LedColor::GREEN_BLINK; break;
                    case 1: color = APCMiniController::LedColor::RED_BLINK; break;
                    case 2: color = APCMiniController::LedColor::YELLOW_BLINK; break;
                    default: color = APCMiniController::LedColor::OFF;
                }
                controller.setGridLED(pos, color);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Randomly clear some LEDs
            for (int i = 0; i < 3; i++) {
                controller.setGridLED(pos_dist(gen), APCMiniController::LedColor::OFF);
            }
            
            if (!isAnimating) return;
        }
    }

    // Pattern 6: Checkerboard
    void checkerboardPattern() {
        bool alternate = false;
        while (isAnimating) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    bool isEven = (row + col) % 2 == 0;
                    controller.setGridLED(
                        row * 8 + col,
                        (isEven != alternate) ? APCMiniController::LedColor::GREEN : APCMiniController::LedColor::RED
                    );
                }
            }
            alternate = !alternate;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!isAnimating) return;
        }
    }

    // Pattern 7: Spiral Pattern
    void spiralPattern() {
        const std::vector<std::pair<int, int>> spiral_path = {
            {3,3}, {3,4}, {4,4}, {4,3}, // Center
            {3,2}, {2,2}, {2,3}, {2,4}, {2,5}, // Expanding
            {3,5}, {4,5}, {5,5}, {5,4}, {5,3},
            {5,2}, {5,1}, {4,1}, {3,1}, {2,1},
            {1,1}, {1,2}, {1,3}, {1,4}, {1,5},
            {1,6}, {2,6}, {3,6}, {4,6}, {5,6},
            {6,6}, {6,5}, {6,4}, {6,3}, {6,2},
            {6,1}, {6,0}, {5,0}, {4,0}, {3,0},
            {2,0}, {1,0}, {0,0}, {0,1}, {0,2},
            {0,3}, {0,4}, {0,5}, {0,6}, {0,7},
            {1,7}, {2,7}, {3,7}, {4,7}, {5,7},
            {6,7}, {7,7}, {7,6}, {7,5}, {7,4},
            {7,3}, {7,2}, {7,1}, {7,0}
        };
        
        while (isAnimating) {
            clearGrid();
            for (const auto& pos : spiral_path) {
                controller.setGridLED(
                    pos.first * 8 + pos.second,
                    APCMiniController::LedColor::YELLOW_BLINK
                );
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if (!isAnimating) return;
            }
        }
    }

    // Pattern 8: Binary Counter
    void binaryCounterPattern() {
        while (isAnimating) {
            for (int count = 0; count < 256 && isAnimating; count++) {
                clearGrid();
                for (int bit = 0; bit < 8; bit++) {
                    for (int row = 0; row < 8; row++) {
                        if (count & (1 << bit)) {
                            controller.setGridLED(
                                row * 8 + bit,
                                APCMiniController::LedColor::GREEN
                            );
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

public:
    LightPatternController(APCMiniController& ctrl) 
        : controller(ctrl), gen(rd()) {}

    void stopCurrentPattern() {
        if (isAnimating) {
            isAnimating = false;
            if (animationThread.joinable()) {
                animationThread.join();
            }
            clearGrid();
        }
    }

    void startPattern(int buttonIndex) {
        stopCurrentPattern();
        isAnimating = true;

        switch(buttonIndex) {
            case 64: // First horizontal button
                animationThread = std::thread(&LightPatternController::snakePattern, this);
                break;
            case 65:
                animationThread = std::thread(&LightPatternController::rainfallPattern, this);
                break;
            case 66:
                animationThread = std::thread(&LightPatternController::colorWavePattern, this);
                break;
            case 67:
                animationThread = std::thread(&LightPatternController::expandingSquarePattern, this);
                break;
            case 68:
                animationThread = std::thread(&LightPatternController::sparklePattern, this);
                break;
            case 69:
                animationThread = std::thread(&LightPatternController::checkerboardPattern, this);
                break;
            case 70:
                animationThread = std::thread(&LightPatternController::spiralPattern, this);
                break;
            case 71:
                animationThread = std::thread(&LightPatternController::binaryCounterPattern, this);
                break;
        }
    }

    ~LightPatternController() {
        stopCurrentPattern();
    }
};

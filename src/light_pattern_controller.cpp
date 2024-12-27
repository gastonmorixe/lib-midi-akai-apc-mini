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

    // Helper to convert row, col to button index (0-63)
    int gridIndex(int row, int col) {
        return row * 8 + col;  // 8x8 matrix, numbered from left to right, top to bottom
    }

    // Pattern 1: Snake
    void snakePattern() {
        while (isAnimating) {
            // Move left to right, top to bottom
            for (int i = 0; i < 64; i++) {
                if (i > 0) controller.setClipLED(i-1, APCMiniController::LedColor::OFF);
                controller.setClipLED(i, APCMiniController::LedColor::GREEN_BLINK);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (!isAnimating) return;
            }
            controller.setClipLED(63, APCMiniController::LedColor::OFF);
        }
    }

    // Pattern 2: Alternating rows
    void alternatingRows() {
        bool firstPattern = true;
        while (isAnimating) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    int index = gridIndex(row, col);
                    if (firstPattern) {
                        controller.setClipLED(index, (row % 2 == 0) ? 
                            APCMiniController::LedColor::RED : 
                            APCMiniController::LedColor::GREEN);
                    } else {
                        controller.setClipLED(index, (row % 2 == 0) ? 
                            APCMiniController::LedColor::GREEN : 
                            APCMiniController::LedColor::RED);
                    }
                }
            }
            firstPattern = !firstPattern;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!isAnimating) return;
        }
    }

    // Pattern 3: Random rain
    void randomRain() {
        std::uniform_int_distribution<> col_dist(0, 7);
        
        while (isAnimating) {
            // Start new drops in random columns
            int col = col_dist(gen);
            
            // Animate drop falling
            for (int row = 0; row < 8; row++) {
                int index = gridIndex(row, col);
                controller.setClipLED(index, APCMiniController::LedColor::YELLOW);
                
                if (row > 0) {
                    controller.setClipLED(gridIndex(row-1, col), APCMiniController::LedColor::OFF);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if (!isAnimating) return;
            }
            // Clear last LED
            controller.setClipLED(gridIndex(7, col), APCMiniController::LedColor::OFF);
        }
    }

    // Pattern 4: Expanding square
    void expandingSquare() {
        while (isAnimating) {
            // Expand from center
            for (int size = 0; size <= 4; size++) {
                for (int row = 3-size; row <= 4+size && row < 8; row++) {
                    for (int col = 3-size; col <= 4+size && col < 8; col++) {
                        if (row >= 0 && col >= 0) {
                            controller.setClipLED(gridIndex(row, col), 
                                APCMiniController::LedColor::RED_BLINK);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!isAnimating) return;
            }
            
            // Contract back
            for (int size = 3; size >= 0; size--) {
                // Clear all
                for (int i = 0; i < 64; i++) {
                    controller.setClipLED(i, APCMiniController::LedColor::OFF);
                }
                // Draw smaller square
                for (int row = 3-size; row <= 4+size; row++) {
                    for (int col = 3-size; col <= 4+size; col++) {
                        if (row >= 0 && col >= 0 && row < 8 && col < 8) {
                            controller.setClipLED(gridIndex(row, col), 
                                APCMiniController::LedColor::GREEN);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!isAnimating) return;
            }
        }
    }

    // Pattern 5: Diagonal waves
    void diagonalWaves() {
        while (isAnimating) {
            for (int wave = 0; wave < 16; wave++) {
                for (int row = 0; row < 8; row++) {
                    for (int col = 0; col < 8; col++) {
                        int phase = (row + col - wave) % 3;
                        APCMiniController::LedColor color;
                        switch(phase) {
                            case 0: color = APCMiniController::LedColor::GREEN; break;
                            case 1: color = APCMiniController::LedColor::YELLOW; break;
                            case 2: color = APCMiniController::LedColor::RED; break;
                        }
                        controller.setClipLED(gridIndex(row, col), color);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
                if (!isAnimating) return;
            }
        }
    }

    // Pattern 6: Random sparkles
    void randomSparkles() {
        std::uniform_int_distribution<> pos_dist(0, 63);
        std::uniform_int_distribution<> color_dist(0, 2);
        
        while (isAnimating) {
            int pos = pos_dist(gen);
            APCMiniController::LedColor color;
            switch(color_dist(gen)) {
                case 0: color = APCMiniController::LedColor::GREEN_BLINK; break;
                case 1: color = APCMiniController::LedColor::RED_BLINK; break;
                case 2: color = APCMiniController::LedColor::YELLOW_BLINK; break;
            }
            controller.setClipLED(pos, color);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if (!isAnimating) return;
        }
    }

    // Pattern 7: Checkerboard
    void checkerboard() {
        bool alternate = false;
        while (isAnimating) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    bool isEven = (row + col) % 2 == 0;
                    APCMiniController::LedColor color = (isEven != alternate) ? 
                        APCMiniController::LedColor::GREEN : 
                        APCMiniController::LedColor::OFF;
                    controller.setClipLED(gridIndex(row, col), color);
                }
            }
            alternate = !alternate;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!isAnimating) return;
        }
    }

    // Pattern 8: Rotating line
    void rotatingLine() {
        while (isAnimating) {
            for (int angle = 0; angle < 8; angle++) {
                // Clear previous
                for (int i = 0; i < 64; i++) {
                    controller.setClipLED(i, APCMiniController::LedColor::OFF);
                }
                
                // Draw new line
                for (int i = 0; i < 8; i++) {
                    int row, col;
                    switch(angle % 4) {
                        case 0: // Horizontal
                            row = i; col = i;
                            break;
                        case 1: // Diagonal
                            row = i; col = 7-i;
                            break;
                        case 2: // Vertical
                            row = 7-i; col = 7-i;
                            break;
                        case 3: // Other diagonal
                            row = 7-i; col = i;
                            break;
                        default:
                            continue;
                    }
                    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                        controller.setClipLED(gridIndex(row, col), 
                            APCMiniController::LedColor::YELLOW_BLINK);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!isAnimating) return;
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
            // Clear all LEDs
            for (int i = 0; i < 64; i++) {
                controller.setClipLED(i, APCMiniController::LedColor::OFF);
            }
        }
    }

    void startPattern(int patternIndex) {
        stopCurrentPattern();
        isAnimating = true;

        switch(patternIndex) {
            case 64: 
                animationThread = std::thread(&LightPatternController::snakePattern, this);
                break;
            case 65:
                animationThread = std::thread(&LightPatternController::alternatingRows, this);
                break;
            case 66:
                animationThread = std::thread(&LightPatternController::randomRain, this);
                break;
            case 67:
                animationThread = std::thread(&LightPatternController::expandingSquare, this);
                break;
            case 68:
                animationThread = std::thread(&LightPatternController::diagonalWaves, this);
                break;
            case 69:
                animationThread = std::thread(&LightPatternController::randomSparkles, this);
                break;
            case 70:
                animationThread = std::thread(&LightPatternController::checkerboard, this);
                break;
            case 71:
                animationThread = std::thread(&LightPatternController::rotatingLine, this);
                break;
        }
    }

    ~LightPatternController() {
        stopCurrentPattern();
    }
};

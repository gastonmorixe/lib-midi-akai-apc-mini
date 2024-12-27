# Makefile that leverages CMake

BUILD_DIR = build

.PHONY: all build clean rebuild fetch

all: build

# Just configure and fetch dependencies without building
fetch:
	mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target rtmidi

# Full build
build:
	mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

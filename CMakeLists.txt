cmake_minimum_required(VERSION 3.15)
project(apc_mini_controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set compiler flags based on compiler ID
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Using Clang compiler.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-gnu-anonymous-struct -Wno-padded -Wno-c++98-compat-pedantic")
else()
    message(STATUS "Using other compiler. (not clang)")
endif()

# Add RtMidi using FetchContent
include(FetchContent)
FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG        5.0.0
)
FetchContent_MakeAvailable(rtmidi)

# Find required system packages for RtMidi on MacOS
if(APPLE)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
endif()

# Create the executable
add_executable(apc_mini_controller 
    src/main.cpp
    src/apc_mini_controller.cpp
)

# Include directories
target_include_directories(apc_mini_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against RtMidi and system libraries
target_link_libraries(apc_mini_controller PRIVATE rtmidi)

if(APPLE)
    target_link_libraries(apc_mini_controller PRIVATE
        ${COREMIDI_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
endif()

# cmake_minimum_required(VERSION 3.15)
# project(apc_mini_controller)
#
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
# # Specify Clang as the compiler
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)
#
# # Add RtMidi using FetchContent
# include(FetchContent)
# FetchContent_Declare(
#     rtmidi
#     GIT_REPOSITORY https://github.com/thestk/rtmidi.git
#     GIT_TAG       6.0.0
# )
# FetchContent_MakeAvailable(rtmidi)
#
# # Find required system packages for RtMidi on MacOS
# find_library(COREMIDI_LIBRARY CoreMIDI)
# find_library(COREAUDIO_LIBRARY CoreAudio)
# find_library(COREFOUNDATION_LIBRARY CoreFoundation)
#
# # Create the executable
# add_executable(apc_mini_controller 
#     src/main.cpp
#     src/apc_mini_controller.cpp
# )
#
# # Include directories
# target_include_directories(apc_mini_controller PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
#
# # Link against RtMidi and system libraries
# target_link_libraries(apc_mini_controller PRIVATE 
#     rtmidi
#     ${COREMIDI_LIBRARY}
#     ${COREAUDIO_LIBRARY}
#     ${COREFOUNDATION_LIBRARY}
# )




# cmake_minimum_required(VERSION 3.15)
# project(apc_mini_controller)
#
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
# # Specify Clang as the compiler
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)
#
# # Add RtMidi dependency through FetchContent
# include(FetchContent)
# FetchContent_Declare(
#     rtmidi
#     GIT_REPOSITORY https://github.com/thestk/rtmidi.git
#     GIT_TAG        5.0.0
# )
# FetchContent_MakeAvailable(rtmidi)
#
# # Find required system packages for RtMidi
# if(APPLE)
#     find_library(COREMIDI_LIBRARY CoreMIDI)
#     find_library(COREAUDIO_LIBRARY CoreAudio)
#     find_library(COREFOUNDATION_LIBRARY CoreFoundation)
# elseif(UNIX)
#     find_package(ALSA REQUIRED)
# endif()
#
# # Create the executable
# add_executable(apc_mini_controller src/main.cpp src/apc_mini_controller.cpp)
#
# # Link against RtMidi and system libraries
# target_link_libraries(apc_mini_controller PRIVATE rtmidi)
#
# if(APPLE)
#     target_link_libraries(apc_mini_controller PRIVATE
#         ${COREMIDI_LIBRARY}
#         ${COREAUDIO_LIBRARY}
#         ${COREFOUNDATION_LIBRARY}
#     )
# elseif(UNIX)
#     target_link_libraries(apc_mini_controller PRIVATE ALSA::ALSA)
# endif()
#
# # Include directories
# target_include_directories(apc_mini_controller PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

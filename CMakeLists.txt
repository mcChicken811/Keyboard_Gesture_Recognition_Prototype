cmake_minimum_required(VERSION 3.10.0)
project(LCS_Gesture_Recognition VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)


add_executable(LCS_Gesture_Recognition
    main.cpp
    src/LCS.cpp
    src/GestureRecognizer.cpp
)

include_directories(src)

cmake_minimum_required(VERSION 3.16.0)
project(LCS_Gesture_Recognition VERSION 0.1.0 LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)


add_executable(LCS_Gesture_Recognition
    main.cpp
    src/LCS.cpp
    src/GestureRecognizer.cpp
    src/GestureWindow.mm
)

include_directories(src)
target_compile_options(LCS_Gesture_Recognition PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>")
target_link_libraries(LCS_Gesture_Recognition PRIVATE "-framework Cocoa")

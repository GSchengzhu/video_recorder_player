cmake_minimum_required(VERSION 3.16.3)

add_definitions("-g")
project(TEST)

include_directories(/usr/local/ffmpeg/include)
link_directories(/usr/local/ffmpeg/lib)

list(APPEND RGB2YUVSRC
  test_rgb2yuv.cpp
  ../screencapture/ScreenCapture.cpp
)

add_executable(TEST_RGB2YUV ${RGB2YUVSRC})
target_link_libraries(TEST_RGB2YUV -lavformat  -lswscale -lavutil -lX11 -lXext -ldl -lpthread)
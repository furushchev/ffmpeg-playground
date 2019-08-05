cmake_minimum_required(VERSION 2.8.3)
project(ffmpeg_playground)
add_compile_options(-std=c++11)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(OpenCV REQUIRED)
find_package(FFmpeg REQUIRED)


###########
## Build ##
###########

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${FFmpeg_INCLUDE_DIRS})

## Declare a C++ library
add_executable(encoder_h264_x264 src/encoder_h264_x264.cpp)

target_link_libraries(encoder_h264_x264
  ${OpenCV_LIBRARIES}
  ${FFmpeg_LIBRARIES})

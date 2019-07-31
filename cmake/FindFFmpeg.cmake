find_package(PkgConfig REQUIRED)

pkg_check_modules(FFmpeg_libavcodec  libavcodec)
if (FFmpeg_libavcodec_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libavcodec_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libavcodec_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libavcodec_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libavformat  libavformat)
if (FFmpeg_libavformat_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libavformat_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libavformat_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libavformat_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libavutil   libavutil)
if (FFmpeg_libavutil_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libavutil_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libavutil_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libavutil_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libavfilter   libavfilter)
if (FFmpeg_libavfilter_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libavfilter_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libavfilter_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libavfilter_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libavdevice   libavdevice)
if (FFmpeg_libavdevice_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libavdevice_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libavdevice_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libavdevice_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libswresample   libswresample)
if (FFmpeg_libswresample_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libswresample_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libswresample_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libswresample_LIBRARY_DIRS})
endif()

pkg_check_modules(FFmpeg_libswscale libswscale)
if (FFmpeg_libswscale_FOUND)
  list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_libswscale_INCLUDE_DIRS})
  list(APPEND FFmpeg_LIBRARIES ${FFmpeg_libswscale_LIBRARIES})
  list(APPEND FFmpeg_LIBRARY_DIRS ${FFmpeg_libswscale_LIBRARY_DIRS})
endif()

if (FFmpeg_libavcodec_FOUND OR
    FFmpeg_libavformat_FOUND OR
    FFmpeg_libavutil_FOUND OR
    FFmpeg_libavfilter_FOUND OR
    FFmpeg_libavdevice_FOUND OR
    FFmpeg_libswresample_FOUND OR
    FFmpeg_libswscale_FOUND)
  set(FFmpeg_FOUND TRUE)
  #
  list(REMOVE_DUPLICATES FFmpeg_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES FFmpeg_LIBRARIES)
  list(REMOVE_DUPLICATES FFmpeg_LIBRARY_DIRS)
  #
  message(STATUS "Found FFmpeg Library directories: ${FFmpeg_LIBRARY_DIRS}")
  link_directories(${FFmpeg_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FFmpeg
  FOUND_VAR FFmpeg_FOUND
  REQUIRED_VARS FFmpeg_LIBRARIES FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARY_DIRS
  HANDLE_COMPONENTS)

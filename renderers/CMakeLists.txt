cmake_minimum_required(VERSION 3.4.1)

  find_package(PkgConfig)

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g")

  pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
                                 gstreamer-sdp-1.0>=1.4
                                 gstreamer-video-1.0>=1.4
                                 gstreamer-app-1.0>=1.4)
  add_library( renderers
        STATIC
        audio_renderer_gstreamer.c video_renderer_gstreamer.c)
  include_directories ( renderers ${GST_INCLUDE_DIRS} )
  target_link_libraries ( renderers ${GST_LIBRARIES} )

  

  target_link_libraries (renderers airplay)

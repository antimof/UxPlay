cmake_minimum_required(VERSION 3.4.1)
  
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g")

if (APPLE )
  set( ENV{PKG_CONFIG_PATH} "/Library/FrameWorks/GStreamer.framework/Libraries/pkgconfig" ) # GStreamer.framework, preferred
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib/pkgconfig" ) # standard location, and Brew
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/local/lib/pkgconfig/" ) # MacPorts
  message( "PKG_CONFIG_PATH (Apple, renderers) = " $ENV{PKG_CONFIG_PATH} )
  find_program( PKG_CONFIG_EXECUTABLE  pkg-config  PATHS /Library/FrameWorks/GStreamer.framework/Commands )
endif()

find_package( PkgConfig REQUIRED )
pkg_check_modules(GST REQUIRED    gstreamer-1.0>=1.4
                                  gstreamer-sdp-1.0>=1.4
                                  gstreamer-video-1.0>=1.4
                                  gstreamer-app-1.0>=1.4
                                  )
message( "GST_LIBRARIES" ${GST_LIBRARIES} )
message( "GST_LIBRARY_DIRS " ${GST_LIBRARY_DIRS} )
message( "GST_CFLAGS " ${GST_CFLAGS} )
message( "GST_LDFLAGS " ${GST_LDFLAGS} )
message( "GST_INCLUDE_DIRS " ${GST_INCLUDE_DIRS} )

add_library( renderers
             STATIC
             audio_renderer_gstreamer.c
	     video_renderer_gstreamer.c )
if( UNIX AND NOT APPLE )
  include_directories ( renderers ${GST_INCLUDE_DIRS} )
else()
  include_directories (renderers
                    /Library/FrameWorks/GStreamer.framework/Headers
                    /usr/local/include
                    /usr/local/include/gstreamer-1.0
		    /usr/local/include/glib-2.0
		    /usr/local/lib/glib-2.0/include
                    /opt/local/include/gstreamer-1.0
		    /opt/local/include/glib-2.0
		    /opt/local/lib/glib-2.0/include
                    )
endif()

target_link_libraries ( renderers PUBLIC
                     ${GST_LIBRARIES}
                     airplay
                     )

if( APPLE )
  message( "APPLE ONLY: \"target_link_directories\" used here requires CMake >= 3.13 ")
  target_link_directories ( renderers  PUBLIC ${GST_LIBRARY_DIRS} )
endif()

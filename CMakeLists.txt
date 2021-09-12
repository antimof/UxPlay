cmake_minimum_required( VERSION 3.4.1 )

project( uxplay )

set ( CMAKE_CXX_STANDARD 11 )

if ( ZOOMFIX )
add_definitions( -DX_DISPLAY_FIX )
find_package( X11 REQUIRED )
link_libraries( ${X11_LIBRARIES} )
include_directories( ${X11_INCLUDE_DIR} )
endif ( ZOOMFIX )

if( UNIX AND NOT APPLE )
 add_definitions( -DSUPPRESS_AVAHI_COMPAT_WARNING )
endif()

add_subdirectory( lib/llhttp )
add_subdirectory( lib/playfair )
add_subdirectory( lib )
add_subdirectory( renderers )

add_executable( uxplay uxplay.cpp )

if( UNIX AND NOT APPLE )
  include_directories( uxplay ${GST_INCLUDE_DIRS} )
else()
  include_directories( uxplay
                  /Library/FrameWorks/GStreamer.framework/Headers/
                  /usr/local/include
		  /usr/local/include/glib-2.0
		  /usr/local/lib/glib-2.0/include
                  /opt/local/include
		  /opt/local/include/glib-2.0
		  /opt/local/lib/glib-2.0/include
                  )
endif()

target_link_libraries( uxplay
                   renderers
                   airplay
		   )

install( TARGETS  uxplay RUNTIME DESTINATION bin )

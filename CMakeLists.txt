cmake_minimum_required(VERSION 3.4.1)

project(uxplay)

set (CMAKE_CXX_STANDARD 11)


if (ZOOMFIX)
add_definitions(-DX_DISPLAY_FIX)
find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})
# link_directories(${X11_LIBRARIES})
endif (ZOOMFIX)

add_definitions(-DAVAHI_COMPAT_NOWARN)

add_subdirectory(lib/llhttp)
add_subdirectory(lib/playfair)
add_subdirectory(lib)
add_subdirectory(renderers)

add_executable( uxplay uxplay.cpp)
target_link_libraries ( uxplay renderers airplay ${X11_LIBRARIES})

install(TARGETS  uxplay RUNTIME DESTINATION bin)


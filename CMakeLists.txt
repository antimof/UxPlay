cmake_minimum_required(VERSION 3.4.1)

project(uxplay)

set (CMAKE_CXX_STANDARD 11)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})
# link_directories(${X11_LIBRARIES})

add_subdirectory(lib/curve25519)
add_subdirectory(lib/ed25519)
add_subdirectory(lib/playfair)
add_subdirectory(lib/plist)
add_subdirectory(lib)
add_subdirectory(renderers)

add_executable( uxplay uxplay.cpp)
target_link_libraries ( uxplay renderers airplay ${X11_LIBRARIES})

install(PROGRAMS uxplay DESTINATION bin)


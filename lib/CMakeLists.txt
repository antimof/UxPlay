cmake_minimum_required(VERSION 3.4.1)
include_directories( curve25519 ed25519 playfair plist/plist )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g")

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library( airplay
        STATIC
        ${DIR_SRCS}
        )

target_link_libraries( airplay
	pthread
        curve25519
        ed25519
        playfair
        plist )

  find_package(OpenSSL REQUIRED)
  target_link_libraries( airplay OpenSSL::Crypto )
  target_link_libraries( airplay dns_sd )
        

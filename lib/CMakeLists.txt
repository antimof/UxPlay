cmake_minimum_required(VERSION 3.4.1)
include_directories( playfair llhttp )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g")

if( APPLE )
  set( ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig" ) # standard location, and Brew
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/local/lib/pkgconfig/" ) # MacPorts
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/openssl@1.1/lib/pkgconfig" ) # Brew openssl
  message( "PKG_CONFIG_PATH (Apple, lib) = " $ENV{PKG_CONFIG_PATH} )
  find_program( PKG_CONFIG_EXECUTABLE  pkg-config  PATHS /Library/FrameWorks/GStreamer.framework/Commands )
  message( "PKG_CONFIG_EXECUTABLE " ${PKG_CONFIG_EXECUTABLE} )
endif()

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library( airplay
        STATIC
        ${DIR_SRCS}
        )

find_package(PkgConfig REQUIRED)

if( UNIX AND NOT APPLE )
   find_library( LIBPLIST NAMES plist plist-2.0 )
elseif( APPLE )
   pkg_check_modules( PLIST REQUIRED libplist-2.0 )
   find_library( LIBPLIST libplist-2.0.a REQUIRED )
   message( "LIBPLIST" ${LIBPLIST} )
   target_include_directories( airplay PRIVATE
                            /usr/local/include  # standard and MacPorts
			    /opt/local/include  # MacPorts
                            )
endif()
message( "LIBPLIST" ${LIBPLIST} )
target_link_libraries( airplay
        pthread
        playfair
        llhttp
        ${LIBPLIST} )

if( UNIX AND NOT APPLE )
  find_package(OpenSSL 1.1.1 REQUIRED)
  target_compile_definitions( airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L )
  target_link_libraries( airplay OpenSSL::Crypto )
  target_link_libraries( airplay dns_sd )
elseif( APPLE )
  # can either compile Openssl 1.1.1 from source (install_dev  to /usr/local) or use Macports or Brew
  # MacPorts needs zlib with it, Brew has a strange "keg-only" installation in usr/local/opt/openssl@1.1
  pkg_check_modules( OPENSSL REQUIRED Openssl>=1.1.1)
  message( "OPENSSL_LIBRARY_DIRS " ${OPENSSL_LIBRARY_DIRS} )
  message( "OPENSSL_INCLUDE_DIRS " ${OPENSSL_INCLUDE_DIRS} )
  find_library( LIBCRYPTO libcrypto.a PATHS ${OPENSSL_LIBRARY_DIRS} REQUIRED )
  find_library( LIBZ libz.a)  # needed by MacPorts openssl
  message( "LIBCRYPTO "  ${LIBCRYPTO}  )
  target_include_directories( airplay PRIVATE
                             ${OPENSSL_INCLUDE_DIRS}
			     )

  target_link_libraries( airplay
                      ${LIBCRYPTO}
		      ${LIBZ}
		       )
endif()


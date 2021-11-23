cmake_minimum_required(VERSION 3.4.1)
include_directories( playfair llhttp )

INCLUDE (CheckIncludeFiles)
# for BSD Unix (e.g. FreeBSD)
CHECK_INCLUDE_FILES ("sys/endian.h" BSD )
if ( BSD )
add_definitions( -DSYS_ENDIAN_H )
endif ( BSD )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g")

if( APPLE )
  set( ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig" ) # standard location, and Brew
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/local/lib/pkgconfig/" ) # MacPorts
  set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/openssl@1.1/lib/pkgconfig" ) # Brew openssl
  message( "PKG_CONFIG_PATH (Apple, lib) = " $ENV{PKG_CONFIG_PATH} )
  find_program( PKG_CONFIG_EXECUTABLE  pkg-config  PATHS /Library/FrameWorks/GStreamer.framework/Commands )
  message( "PKG_CONFIG_EXECUTABLE " ${PKG_CONFIG_EXECUTABLE} )
endif()
find_package(PkgConfig REQUIRED)

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library( airplay
        STATIC
        ${DIR_SRCS}
        )
target_link_libraries( airplay
        pthread
        playfair
        llhttp )

# libplist
if( UNIX AND NOT APPLE )
     target_include_directories( airplay PUBLIC ${PLIST_INCLUDE_DIRS} )
     link_directories( ${PLIST_LIBRARY_DIRS} )
     target_link_libraries ( airplay ${PLIST_LBRARIES} )
elseif( APPLE )
   # use static linking
   pkg_check_modules( PLIST REQUIRED libplist-2.0 )
   find_library( LIBPLIST libplist-2.0.a REQUIRED )
   target_include_directories( airplay PRIVATE
                            /usr/local/include  # standard and Brew
                            /opt/local/include  # MacPorts
                            )
   message( STATUS "LIBPLIST " ${LIBPLIST} )
   target_link_libraries ( airplay ${LIBPLIST} )
endif()

#libcrypto
if( UNIX AND NOT APPLE )
  find_package(OpenSSL 1.1.1 REQUIRED)
  target_compile_definitions( airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L )
  target_link_libraries( airplay OpenSSL::Crypto )
elseif( APPLE )
  # use static linking
  # can either compile Openssl 1.1.1 from source (install_dev  to /usr/local) or use Macports or Brew
  # MacPorts needs zlib with it, Brew has a strange "keg-only" installation in usr/local/opt/openssl@1.1
  pkg_check_modules( OPENSSL REQUIRED openssl>=1.1.1)
  message( "OPENSSL_LIBRARY_DIRS " ${OPENSSL_LIBRARY_DIRS} )
  message( "OPENSSL_INCLUDE_DIRS " ${OPENSSL_INCLUDE_DIRS} )
  find_library( LIBCRYPTO libcrypto.a PATHS ${OPENSSL_LIBRARY_DIRS} REQUIRED )
  message( "LIBCRYPTO "  ${LIBCRYPTO}  )
  target_link_libraries( airplay ${LIBCRYPTO} )
  if( LIBCRYPTO MATCHES "/opt/local/lib/libcrypto.a" ) #MacPorts openssl
    find_library( LIBZ libz.a)  # needed by MacPorts openssl
    message("(MacPorts) LIBZ= " ${LIBZ} )
    target_link_libraries( airplay ${LIBZ} )
  endif()
  target_include_directories( airplay PRIVATE
                             ${OPENSSL_INCLUDE_DIRS}
			     )
endif()

#dns_sd (we only need to find the  dns_sd.h include file for non_Apple systems)
if ( UNIX AND NOT APPLE )
  pkg_check_modules(AVAHI_COMPAT avahi-compat-libdns_sd)
  if( AVAHI_COMPAT_FOUND )
        message( STATUS "AVAHI_COMPAT_INCLUDE_DIRS " ${AVAHI_COMPAT_INCLUDE_DIRS} )
        target_include_directories( airplay PUBLIC ${AVAHI_COMPAT_INCLUDE_DIRS} )
        message( STATUS "AVAHI_COMPAT_LIBRARIES " ${AVAHI_COMPAT_LIBRARIES} )
	target_link_libraries( airplay ${AVAHI_COMPAT_LIBRARIES} )
  else()  # dns_sd can be provided by mdnsResponder instead of Avahi.
      find_library(DNS_SD dns_sd )
      if( NOT DNS_SD )
          message( FATAL_ERROR "libdns_sd not found; suggestion: install avahi-compat-libdns-sd ")
      else()
         message( STATUS "libdns_sd found at " ${DNS_SD} )
      endif()
      set ( INCLUDE_PATHS " /usr/include/ /usr/include/* /usr/local/include/ /usr/local/include/* " )
      find_file( DNS_SD_H dns_sd.h PATHS ${INCLUDE_PATHS} )
      if( NOT DNS_SD_H )
          message( FATAL_ERROR " dns_sd.h not found in PATHS " ${INCLUDE_PATHS})
      endif()
      string( REPLACE "dns_sd.h" "" DNSSD_INCLUDE_DIR  ${DNS_SD_H} )
      message( STATUS "found dns_sd.h in " ${DNSSD_INCLUDE_DIR} )
      target_include_directories( airplay PRIVATE ${DNSSD_INCLUDE_DIR} )
      target_link_libraries( airplay dns_sd )
  endif()
endif()

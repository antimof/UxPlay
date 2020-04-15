cmake_minimum_required(VERSION 3.4.1)
aux_source_directory(. plist_src)
set(DIR_SRCS ${plist_src})
add_library( plist
        STATIC
        ${DIR_SRCS})
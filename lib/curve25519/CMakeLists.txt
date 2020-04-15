cmake_minimum_required(VERSION 3.4.1)
aux_source_directory(. curve25519_src)
set(DIR_SRCS ${curve25519_src})
add_library( curve25519
        STATIC
        ${DIR_SRCS})
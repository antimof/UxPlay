cmake_minimum_required(VERSION 3.4.1)
aux_source_directory(. ed25519_src)
set(DIR_SRCS ${ed25519_src})
add_library( ed25519
        STATIC
        ${DIR_SRCS})
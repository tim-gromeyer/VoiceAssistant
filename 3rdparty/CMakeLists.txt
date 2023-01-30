cmake_minimum_required(VERSION 3.13)

cmake_policy(SET CMP0077 NEW)

set(MZ_LIBCOMP OFF)
set(MZ_PKCRYPT OFF)
set(MZ_WZAES OFF)
set(MZ_OPENSSL OFF)
set(MZ_LIBBSD OFF)
set(MZ_SIGNING OFF)

set(MZ_DECOMPRESS_ONLY ON)

add_subdirectory(11Zip)

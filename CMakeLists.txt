cmake_minimum_required(VERSION 3.19)
project(lzwV2)

set(CMAKE_CXX_STANDARD 20)

add_executable(lzwV2 main.cpp LzwCompressService.h LzwDecompressService.h)
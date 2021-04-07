cmake_minimum_required(VERSION 3.17)
project(gif_maker C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

find_package(OpenMP REQUIRED)

add_executable(gif_maker
        c_img.c
        crop.c
        seamcarving.c)

target_link_libraries(gif_maker PRIVATE m)
target_link_libraries(gif_maker PRIVATE OpenMP::OpenMP_C)

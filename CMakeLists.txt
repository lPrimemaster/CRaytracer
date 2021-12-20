cmake_minimum_required (VERSION 3.8)

project(Raytracer C)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/GLFW/lib/cmake/glfw3")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

find_package(glfw3 3.3 REQUIRED)

include_directories(Raytracer ${GLFW3_INCLUDE_DIRS})

add_executable (Raytracer

    glad/khrplatform.h
    glad/glad.c
    glad/glad.h

    src/main.c

    src/common.c
    inc/common.h

    src/caster.c
    inc/caster.h

    src/material.c
    inc/material.h

    src/image.c
    inc/image.h

    src/camera.c
    inc/camera.h

    src/dispatcher.c
    inc/dispatcher.h

    src/display.c
    inc/display.h

    src/display_acc.c
    inc/display_acc.h

    src/noise.c
    inc/noise.h

    src/profiler.c
    inc/profiler.h
    )

target_link_libraries(Raytracer glfw)

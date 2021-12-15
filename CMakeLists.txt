cmake_minimum_required (VERSION 3.8)

project(Raytracer C)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable (Raytracer 
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
    )
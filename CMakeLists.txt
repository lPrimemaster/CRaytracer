cmake_minimum_required (VERSION 3.8)

project(Raytracer C)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable (Raytracer src/main.c)
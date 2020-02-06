cmake_minimum_required(VERSION 3.10)
project(Projet_IA_Puissance_4)

set(CMAKE_C_STANDARD 11)

include_directories(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB SOURCES
        "${SRC_DIR}/*.h"
        "${SRC_DIR}/*.c"
        )

add_executable(Projet_IA_Puissance_4
        src/jeu.c
        src/ia.c)

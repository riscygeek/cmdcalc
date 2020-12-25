cmake_minimum_required(VERSION 3.17)
project(cmdcalc2 C)

set(CMAKE_C_STANDARD 11)
include_directories(include)

add_executable(cmdcalc2 src/main.c include/token.h src/token.c include/lexer.h src/lexer.c include/input.h src/input.c
               include/strint.h src/strint.c include/parser.h src/parser.c include/evaluater.h src/evaluater.c)
target_link_libraries(cmdcalc2 m readline)

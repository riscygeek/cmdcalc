cmake_minimum_required(VERSION 3.17)
project(cmdcalc2 C)

set(CMAKE_C_STANDARD 11)
include_directories(include)

add_executable(cmdcalc2 src/main.c include/token.h src/token.c include/lexer.h src/lexer.c include/input.h src/input.c
               include/strint.h src/strint.c include/parser.h src/parser.c include/evaluater.h src/evaluater.c include/value.h src/value.c src/builtin_funcs.c include/error.h src/error.c include/cmdcalc.h src/parse_args.c src/shell.c)
target_link_libraries(cmdcalc2 m readline)

cmake_minimum_required(VERSION 4.0)

project(emzasm DESCRIPTION "An assembler written for the Iskra EMZ-1001 Processor." LANGUAGES C)

add_library(emzasmlib STATIC lib/functions.h lib/globalvars.h lib/lexer.h lib/parser.h lib/typedef.h)
set_target_properties(emzasmlib PROPERTIES LINKER_LANGUAGE C)
target_include_directories(emzasmlib PUBLIC lib)

add_executable(emzasm src/main.c)
target_link_libraries(emzasm PUBLIC emzasmlib)

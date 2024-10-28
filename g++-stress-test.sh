#!/bin/bash

g++ -std=c++23 \
    -g -O3 \
    \
    -pedantic -pedantic-errors -Wall -Wall -Wcast-align -Wcast-align -Wcast-qual -Wcast-qual -Wconversion -Wconversion -Wdisabled-optimization -Wdisabled-optimization -Wdouble-promotion -Weffc++ -Werror -Wextra -Wextra -Wfloat-equal -Wfloat-equal -Wformat=2 -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wlogical-op -Wlong-long -Wmissing-declarations -Wmissing-declarations -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wno-aggregate-return -Wno-inline -Wno-non-virtual-dtor -Wnon-virtual-dtor -Wno-padded -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpedantic -Wpointer-arith -Wpointer-arith -Wredundant-decls -Wredundant-decls -Wshadow -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wstrict-overflow=5 -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunreachable-code -Wunused -Wunused-parameter -Wuseless-cast -Wvariadic-macros -Wwrite-strings -Wwrite-strings \
    \
    -ISilvaCollections/ \
    -I./ \
    \
    -DDEBUG \
    \
	-lspdlog -lfmt \
    \
    ${@} \
    \
	example.cpp -o example.out 2>&1

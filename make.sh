#!/bin/bash

mkdir bin 2> /dev/null
cc -W src/main.c -o bin/emzasm "$@"

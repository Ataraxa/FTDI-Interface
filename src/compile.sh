#!/bin/bash

FILENAME="${1:-"interface.cpp"}"
echo "Compiling $FILENAME..."
g++ -I../include -L../lib -o ../bin/${FILENAME%.*}.exe $FILENAME -llibmpsse -lftd2xx64
#!/bin/bash

FILENAME="${1:-"interface.cpp"}"
echo "Running executable..."
./../bin/${FILENAME%.*}
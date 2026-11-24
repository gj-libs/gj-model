#!/bin/bash

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DGJ_MODEL_BUILD_TESTS=ON
cmake --build build
./build/gj-model-test

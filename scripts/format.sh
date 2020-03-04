#!/bin/bash

clang-format -i "${MESON_SOURCE_ROOT}"/src/*/*.c
clang-format -i "${MESON_SOURCE_ROOT}"/include/*/*/*.h

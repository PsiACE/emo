#!/bin/bash

ninja -C "${MESON_BUILD_ROOT}"
"${MESON_BUILD_ROOT}"/src/emo

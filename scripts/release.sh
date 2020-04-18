#!/bin/bash

if ! [ "$MESON_BUILD_ROOT" ]; then
    echo "This can only be run via meson, exiting!"
    exit 1
fi

PKGVER=$1-$2
DEST=${MESON_BUILD_ROOT}
DIST=$DEST/dist/$PKGVER
SRC=${MESON_SOURCE_ROOT}

cd "${SRC}" || exit 1
mkdir -p "${DIST}"

ginst() {
	cp -rf "$@" "${DIST}"
}

ginst \
	meson.build \
	LICENSE \
	README.md \
	scripts \
	src \
    include \

# packaging
cd "${DEST}"/dist || exit 1
tar cJvf "$PKGVER.tar.xz" "$PKGVER"

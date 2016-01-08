#!/bin/bash

SCRIPT_DIR="$( cd $(dirname $0) ; pwd )"

if [ -z "$1" ] || [ ${1:0:2} == "-D" ] ;
then
    TARGET=$(uname -sm)
    echo "Building for current host target: $TARGET"
	CMAKE_PARAMS=${@:1}
else
    if [ ! -f "$SCRIPT_DIR/platforms/$1/toolchain.cmake" ]; then
        echo "Provided target \"$1\" is not supported"
        exit
    fi
    echo "Cross compiling for target: " $1
    if [ -z "$TOOLCHAIN_DIR" ]; then
        export TOOLCHAIN_DIR=$SCRIPT_DIR/build/toolchain
        #export STAGING_DIR=$SCRIPT_DIR/build/toolchain
        echo "\$TOOLCHAIN_DIR is not set, build process will download a toolchain itself to $TOOLCHAIN_DIR"
    else
        echo "\$TOOLCHAIN_DIR is set to $TOOLCHAIN_DIR, build process will use it"
    fi
    TOOLCHAIN_DEFINE="-DCMAKE_TOOLCHAIN_FILE=$SCRIPT_DIR/platforms/$1/toolchain.cmake"
    TARGET_DEFINE="-DTARGET_DEVICE=$1"
	CMAKE_PARAMS=${@:2}
fi

rm -rf ./build
mkdir build
cd build

cmake $TOOLCHAIN_DEFINE $TARGET_DEFINE "$CMAKE_PARAMS" ..
make
make package
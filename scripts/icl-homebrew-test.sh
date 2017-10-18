#!/bin/sh

HOMEBREW_PREFIX=/usr/local

brew install iclcv/homebrew-formulas/icl --with-full --only-dependencies

cd build

cmake -DBUILD_APPS=On \
      -DBUILD_DEMOS=On \
      -DBUILD_EXAMPLES=On \
      -DCMAKE_INSTALL_PREFIX="${HOMEBREW_PREFIX}" \
      -DBUILD_WITH_OPENCV=On \
      -DBUILD_WITH_QT=On \
      -DQT_ROOT="${HOMEBREW_PREFIX}/opt/qt5" \
      -DBUILD_WITH_OPENGL=On \
      -DBUILD_WITH_IMAGEMAGICK=On \
      -DBUILD_WITH_LIBAV=On \
      -DBUILD_WITH_LIBDC=On \
      -DBUILD_WITH_EIGEN3=On \
      -DBUILD_WITH_RSB=On \
      -DBUILD_WITH_RSC=On \
      -DBUILD_WITH_PROTOBUF=On \
      -DBUILD_WITH_PCL=On \
      -DBUILD_WITH_BULLET=On \
      -DBUILD_WITH_LIBFREENECT=On \
      -DBUILD_WITH_LIBUSB=On \
      -DBUILD_WITH_ZMQ=On \
      -DBUILD_WITH_OPENNI=On \
      ..
sleep 5
make -j5

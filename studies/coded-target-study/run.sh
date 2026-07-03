#!/usr/bin/env bash
# Build + run the calibration-target study against the already-built ICL dylibs.
# Run from the repo root:  studies/coded-target-study/run.sh
set -e
BD=builddir
QT=/Users/celbrech/Qt/6.11.0/macos
INC="-std=c++20 -I$BD -I. -I3rdparty/sse2neon -F$QT/lib -I$QT/include \
 -I/opt/homebrew/opt/jpeg-turbo/include -I/opt/homebrew/opt/libpng/include/libpng16 \
 -I/opt/homebrew/opt/opencv/include/opencv4 -Iicl/markers"
LIBS="-Licl/markers -Licl/filter -Licl/core -Licl/math -Licl/utils -Licl/cv -Licl/io -Licl/geom \
 -licl-markers -licl-filter -licl-core -licl-math -licl-utils -licl-cv -licl-io -licl-geom"
RPATH=""
for d in markers filter core math utils cv io geom geom2 qt; do RPATH="$RPATH -Wl,-rpath,$PWD/$BD/icl/$d"; done
clang++ $INC -c studies/coded-target-study/study.cpp -o /tmp/study.o
( cd $BD && clang++ /tmp/study.o $LIBS $RPATH -o /tmp/study )
/tmp/study

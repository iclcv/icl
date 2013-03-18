#!/bin/bash

rm -rf /media/local_data1/mgoettin/_temp_/*
rm -rf /homes/mgoettin/src/cor-lab/icl/branches/icl_cmake_revise/build/*

cd /homes/mgoettin/src/cor-lab/icl/branches/icl_cmake_revise/build

cmake -DBUILD_WITH_IPP=TRUE -DIPP_ROOT=/vol/nivision/share/IPP/7.0 \
      -DBUILD_WITH_MKL=TRUE -DMKL_ROOT=/vol/nivision/share/MKL/10.3 \
      -DBUILD_WITH_EIGEN3=TRUE \
      -DBUILD_WITH_OPENCV=TRUE -DOpenCV_DIR=/usr/share/OpenCV \
      -DBUILD_WITH_OPENNI=TRUE -DOPENNI_ROOT=/vol/nivision/linx86_64_precise/usr \
      -DBUILD_WITH_LIBDC=TRUE \
      -DBUILD_WITH_V4L=TRUE \
      -DBUILD_WITH_XINE=TRUE \
      -DBUILD_WITH_QT=TRUE \
      -DBUILD_WITH_MESASR=TRUE -DMESASR_ROOT=/vol/nivision/linx86_64_precise/usr \
      -DBUILD_WITH_IMAGEMAGICK=TRUE \
      -DBUILD_WITH_LIBFREENECT=TRUE \
      -DBUILD_WITH_PCL=TRUE -DPCL_SEARCH_PATH=/vol/nivision/linx86_64_precise/share/pcl-1.6 \
      -DCMAKE_INSTALL_PREFIX=/media/local_data1/mgoettin/_temp_ \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_DEMOS=OFF \
      -DBUILD_APPS=OFF \
      ..

make VERBOSE=1 -j10
make install

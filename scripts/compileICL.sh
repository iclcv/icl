#!/bin/bash

B=$HOME/projects/ICL/build
if [ "$1" = "new" ] ; then
    rm -rf ${B} ;
    mkdir ${B} ;
fi
    
cd ${B}

cmake -DBUILD_WITH_IPP=TRUE -DIPP_ROOT=/vol/nivision/share/IPP/7.07 \
      -DBUILD_WITH_MKL=FALSE -DMKL_ROOT=/vol/nivision/share/MKL/10.3.11 \
      -DBUILD_WITH_EIGEN3=TRUE \
      -DBUILD_WITH_V4L=TRUE \
      -DBUILD_WITH_XINE=TRUE \
      -DBUILD_WITH_LIBFREENECT=TRUE \
      -DBUILD_WITH_LIBFREENECT2=TRUE \
      -DLIBFREENECT2_ROOT=/vol/nivision/share/libfreenect2/freenect2 \
      -DBUILD_WITH_MESASR=TRUE \
      -DBUILD_WITH_QT=TRUE \
      -DQT_ROOT=/vol/nivision/share/qt5/5.2.1/gcc_64 \
      -DBUILD_WITH_OPENCL=TRUE \
      -DBUILD_WITH_LIBDC=TRUE \
      -DBUILD_WITH_IMAGEMAGICK=TRUE \
      -DCMAKE_INSTALL_PREFIX=/vol/nivision/share/ICL-8.0 \
      -DBUILD_EXAMPLES=ON \
      -DBUILD_DEMOS=ON \
      -DBUILD_WITH_OPENCV=TRUE OpenCV_DIR=/usr \
      -DBUILD_APPS=ON \
      ..



#      -DBUILD_WITH_PCL=TRUE -DPCL_DIR=/usr/local/share/pcl-1.6 \


make -j6 install
#make install
#make manual

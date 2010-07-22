#!/bin/bash

VERSION=release
VERSION_POSTFIX=
DEBUG_VALUE=FALSE
PROG=cmake
for T in "$@" ; do
    if [ "$T" = "--ccmake" ] ; then
        PROG=ccmake ;
    elif [ "$T" = "--debug" ] ; then 
        VERSION=debug ;
        VERSION_POSTFIX=-debug ;
        DEBUG_VALUE=TRUE ;
    elif [ "$T" = "--help" ] ; then
        echo "usage:"
        echo "  scripts/icl-cmake.sh [--debug] [--ccmake] [--help]" ;
        echo "    --debug : enables debug mode and attaches '-debug' to install prefix" ;
        echo "    --ccmake: uses ccmake instead of cmake" ;
        echo "    --help  : guess what!" ;
        exit 0 ;
    else
        echo "unknown arg: $T" ;
        exit -1 ;
    fi
done

if [ -e CMakeCache.txt ] ; then
    echo "The current directory already contains a CMakeChache.txt"
    echo "This will force cmake and ccmake to create and in-source-build"
    echo "As this script was implemented to create out-of-source builds,"
    echo "the script will exit now without doing anything"
    exit -1;
fi

INSTALL_PREFIX=/vol/nivision/ICL/${ICLVER}${VERSION_POSTSFIX}
IPP_PATH=/vol/nivision/share/IPP/6.0
MKL_PATH=/vol/nivision/share/MKL/10.2
LIBDC_PATH=/usr
LIBMESASR_PATH=/usr
XCF_PATH=/vol/xcf
OPENCV_PATH=/vol/nivision/OpenCV2
USE_OPENCV=TRUE
USE_XCF=TRUE



IP_PREFIX=$(ping -c 1 $HOSTNAME | grep PING | cut -d ' ' -f 3 | tr -d '()' | cut -d '.' -f 1-2)
if [ "$IP_PREFIX" = "129.70" ] ; then
   LIBMESASR_PATH=/vol/nivision
   LIBDC_PATH=/vol/nivision
   OPENCV_PATH=/vol/nivision
fi

ICLVER=$(./VERSION.sh)

mkdir -p build/${VERSION}
cd build/${VERSION}
${PROG} -D ALL_ON:BOOL=TRUE -D IPP_PATH:STRING=${IPP_PATH} -D MKL_PATH:STRING=${MKL_PATH} -D XCF_PATH:STRING=${XCF_PATH} -D ICL_INSTALL_PREFIX:STRING=${INSTALL_PREFIX} -D ICL_DEBUG_MODE:BOOL=${DEBUG_VALUE} -D OPENCV_PATH:STRING=${OPENCV_PATH} -D LIBDC_PATH:STRING=${LIBDC_PATH} -D LIBMESASR_PATH:STRING=${LIBMESASR_PATH} -D USE_OPENCV:BOOL=${USE_OPENCV} -D USE_XCF:BOOL=${USE_XCF} ../../

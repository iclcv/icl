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

ICLVER=$(./VERSION.sh)

mkdir -p build/${VERSION}
cd build/${VERSION}
${PROG} -D ALL_ON:BOOL=TRUE -D IPP_PATH:STRING=/vol/nivision/share/IPP/6.0 -D MKL_PATH:STRING=/vol/nivision/share/MKL/10.2 -D XCF_PATH:STRING=/vol/xcf -D ICL_INSTALL_PREFIX:STRING=/vol/nivision/ICL/${ICLVER}${VERSION_POSTSFIX} -D ICL_DEBUG_MODE:BOOL=${DEBUG_VALUE} -D OPENCV_PATH:STRING=/vol/nivision/OpenCV2 -D USE_OPENCV:BOOL=TRUE -D USE_XCF:BOOL=TRUE ../../

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

ICL_VAR_INSTALL_PREFIX=/vol/nivision
#ICL/${ICLVER}${VERSION_POSTFIX}
ICL_XDEP_IPP_PATH=/vol/nivision/share/IPP/6.0
ICL_XDEP_MKL_PATH=/vol/nivision/share/MKL/10.2-ia32
ICL_XDEP_LIBDC_PATH=/usr
ICL_XDEP_QT_PATH=/usr
ICL_XDEP_LIBMESASR_PATH=/usr
ICL_XDEP_XCF_PATH=/vol/xcf
ICL_XDEP_OPENCV_PATH=/usr
ICL_XDEP_OPENCV_ON=TRUE
ICL_XDEP_XCF_ON=TRUE
ICL_XDEP_GTEST_PATH=/usr






IP_PREFIX=$(ping -c 1 $HOSTNAME | grep PING | cut -d ' ' -f 3 | tr -d '()' | cut -d '.' -f 1-2)
if [ "$IP_PREFIX" = "129.70" ] ; then
   ICL_XDEP_LIBMESASR_PATH=/vol/nivision
#   ICL_XDEP_LIBDC_PATH=/vol/nivision
#   ICL_XDEP_OPENCV_PATH=/vol/nivision
   ICL_XDEP_GTEST_PATH=/vol/nivision/share
fi



mkdir -p build/${VERSION}
cd build/${VERSION}
${PROG} -D ICL_XDEP_ALL_ON:BOOL=TRUE -D ICL_XDEP_IPP_PATH:STRING=${ICL_XDEP_IPP_PATH} -D ICL_XDEP_MKL_PATH:STRING=${ICL_XDEP_MKL_PATH} -D ICL_XDEP_XCF_PATH:STRING=${ICL_XDEP_XCF_PATH} -D ICL_VAR_INSTALL_PREFIX:STRING=${ICL_VAR_INSTALL_PREFIX} -D ICL_VAR_DEBUG_MODE:BOOL=${DEBUG_VALUE} -D ICL_XDEP_OPENCV_PATH:STRING=${ICL_XDEP_OPENCV_PATH} -D ICL_XDEP_LIBDC_PATH:STRING=${ICL_XDEP_LIBDC_PATH} -D ICL_XDEP_LIBMESASR_PATH:STRING=${ICL_XDEP_LIBMESASR_PATH} -D ICL_XDEP_OPENCV_ON:BOOL=${ICL_XDEP_OPENCV_ON} -D ICL_XDEP_XCF_ON:BOOL=${ICL_XDEP_XCF_ON}  -D ICL_XDEP_GTEST_PATH:STRING=${ICL_XDEP_GTEST_PATH} -D ICL_XDEP_QT_ON:BOOL=TRUE -D ICL_XDEP_QT_PATH:STRING=${ICL_XDEP_QT_PATH} ../../

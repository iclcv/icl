#                                             -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

####################################################################################
# Set some default project values
#   linux: -D SYSTEM_LINUX, macosx: -D SYSTEM_APPLE, windows: -D SYSTEM_WINDOWS
####################################################################################

AC_PREREQ(2.61)
AC_INIT([ICL],[5.0.2],[nivision@techfak.uni-bielefeld.de])
AM_INIT_AUTOMAKE([foreign nostdinc])

echo -n "checking system architecture..."
ARCH=`./config.guess | cut -f 3 -d '-' | tr [[:lower:]] [[:upper:]]`
if [[ `echo $ARCH | grep -e '^DARWIN'` ]] ; then 
   ARCH=APPLE
elif test "$ARCH" = "CYGWIN" ; then
   ARCH=LINUX
fi
echo " $ARCH (adding -DSYSTEM_$ARCH to CXXFLAGS)"

# checking system architecture 64 or 32 bit
MACHINE=`uname -m`
BITS=32
if [[ "$MACHINE" = "x86_64" ]] ; then
   BITS=64
fi

CXXFLAGS="$CXXFLAGS -DSYSTEM_${ARCH} -Wno-long-long -Wall -I../../include"
#Workaround for missing pthread symbols on some systems ?!
if [[ "$ARCH" = "APPLE" ]] ; then
LDFLAGS="$LDFLAGS -lpthread"
else
LDFLAGS="$LDFLAGS -pthread"
fi

#TODO where is -lpthread missing?
#LDFLAGS="$LDFLAGS -lpthread"
#ICL_EXTEND_PC_FLAGS([],[-DSYSTEM_${ARCH}],[])

AM_CONDITIONAL([SYSTEM_LINUX_COND],[test x$ARCH = xLINUX])
AM_CONDITIONAL([SYSTEM_APPLE_COND],[test x$ARCH = xAPPLE])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([include/ICLUtils/config.h])       
AC_LANG([C++])

####################################################################################
# Checks for generic programs.
####################################################################################
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_SED

AC_CHECK_PROG([HAVE_PKG_CONFIG],[pkg-config],[TRUE],[FALSE])
if test "$HAVE_PKG_CONFIG" = "FALSE" ; then
AC_MSG_ERROR([unable to find 'pkg-config' please install and/or adapt path variable])
fi

###############################################################################
# NEW ICLV5: out-sourced dependency checks -> search in ./m4/check_*.m4
###############################################################################

ICL_CHECK_IPP()
ICL_CHECK_MKL()
ICL_CHECK_LIBDC()
ICL_CHECK_OPENCV()
ICL_CHECK_OPENGL()
ICL_CHECK_UNICAP()
ICL_CHECK_LIBMAGICK()
ICL_CHECK_LIBJPEG()
ICL_CHECK_LIBZ()
ICL_CHECK_LIBMESASR()
ICL_CHECK_VIDEODEV()
ICL_CHECK_SVS()
ICL_CHECK_XINE()
ICL_CHECK_QT()
ICL_CHECK_XCF()

echo "LDFLAGS ARE: $LDFLAGS"
echo "CXXFLAGS ARE: $CXXFLAGS"
###############################################################################
# NEW ICLV5: create dedicated flags for all icl packages
###############################################################################

# ICLUtils
ICL_USE_EXTERNAL_PACKAGE_IN([ICLUtils],[IPP])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLUtils],[MKL])

# ICLCore
ICL_USE_INTERNAL_PACKAGE_IN([ICLCore],[ICLUtils])

# ICLCC
ICL_USE_INTERNAL_PACKAGE_IN([ICLCC],[ICLCore]) 

# ICLFilter
ICL_USE_INTERNAL_PACKAGE_IN([ICLFilter],[ICLCC]) 

# ICLBlob
ICL_USE_INTERNAL_PACKAGE_IN([ICLBlob],[ICLCC]) 

# ICLOpenCV
if [[ "$HAVE_OPENCV" = "TRUE" ]] ; then
ICL_USE_INTERNAL_PACKAGE_IN([ICLOpenCV],[ICLCore])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLOpenCV],[OPENCV])
fi

# ICLIO
ICL_USE_INTERNAL_PACKAGE_IN([ICLIO],[ICLFilter])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBZ])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBJPEG])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBMESASR])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBDC])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[XINE])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[UNICAP])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBXINE])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[LIBMAGICK])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[VIDEODEV])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLIO],[XCF])
if [[ "$HAVE_OPENCV" = "TRUE" ]] ; then
ICL_USE_INTERNAL_PACKAGE_IN([ICLIO],[ICLOpenCV])
fi

# ICLQt
if [[ "$HAVE_QT" = "TRUE" ]] ; then
ICL_USE_INTERNAL_PACKAGE_IN([ICLQt],[ICLFilter]) 
ICL_USE_INTERNAL_PACKAGE_IN([ICLQt],[ICLIO]) 
ICL_USE_EXTERNAL_PACKAGE_IN([ICLQt],[QT])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLQt],[OPENGL])
fi

# ICLQuick
if [[ "$HAVE_QT" = "TRUE" ]] ; then
ICL_USE_INTERNAL_PACKAGE_IN([ICLQuick],[ICLQt]) 
else
ICL_USE_INTERNAL_PACKAGE_IN([ICLQuick],[ICLFilter]) 
ICL_USE_INTERNAL_PACKAGE_IN([ICLQuick],[ICLIO]) 
fi
ICL_USE_INTERNAL_PACKAGE_IN([ICLQuick],[ICLBlob]) 

# ICLGeom
ICL_USE_INTERNAL_PACKAGE_IN([ICLGeom],[ICLQuick]) 



# ICLAlgorithms
if [[ "$HAVE_OPENCV" = "TRUE" ]] ; then
ICL_USE_INTERNAL_PACKAGE_IN([ICLAlgorithms],[ICLOpenCV])
fi
ICL_USE_INTERNAL_PACKAGE_IN([ICLAlgorithms],[ICLQuick])
ICL_USE_INTERNAL_PACKAGE_IN([ICLAlgorithms],[ICLGeom])
ICL_USE_EXTERNAL_PACKAGE_IN([ICLAlgorithms],[SVS])

###############################################################################
# NEW ICLV5: subst make variables for all packages
###############################################################################

ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLUtils)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLCore)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLCC)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLIO)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLBlob)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLFilter)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLGeom)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLQt)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLQuick)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLOpenCV)
ICL_STRIP_AND_SUBST_FLAGS_FOR(ICLAlgorithms)



###############################################################################
# Summary
###############################################################################
AC_MSG_NOTICE([+---------------------------------------+])
AC_MSG_NOTICE([| Summary                               |])
AC_MSG_NOTICE([+---------------------------------------+])
ICL_NOTIFY_SUPPORT([IPP],          [IPP      ])
ICL_NOTIFY_SUPPORT([MKL],          [MKL      ])
ICL_NOTIFY_SUPPORT([LIBDC],        [libdc1394])
ICL_NOTIFY_SUPPORT([UNICAP],       [Unicap   ])
ICL_NOTIFY_SUPPORT([OPENGL],       [OpenGL   ])
ICL_NOTIFY_SUPPORT([XINE],         [Xine     ])
ICL_NOTIFY_SUPPORT([QT],           [Qt       ])
ICL_NOTIFY_SUPPORT([LIBJPEG],      [libjpeg  ])
ICL_NOTIFY_SUPPORT([LIBZ],         [libz     ])
ICL_NOTIFY_SUPPORT([VIDEODEV],     [videodev ])
ICL_NOTIFY_SUPPORT([SVS],          [SVS      ])
ICL_NOTIFY_SUPPORT([XCF],          [XCF      ])
ICL_NOTIFY_SUPPORT([LIBMAGICK],    [Magick++ ])
ICL_NOTIFY_SUPPORT([LIBMESASR],    [libmesasr])
ICL_NOTIFY_SUPPORT([OPENCV],       [OpenCV   ])
AC_MSG_NOTICE([+---------------------------------------+])


###############################################################################
# Create list of packages to build (in correct order)
###############################################################################
ICL_BUILD_PACKAGES="ICLUtils ICLCore ICLIO ICLCC ICLBlob ICLFilter ICLQuick ICLGeom ICLAlgorithms"

# Add optional packages
ICL_ADD_PACKAGE_IF([ICLQt],["$HAVE_QT" = "TRUE"])
ICL_ADD_PACKAGE_IF([ICLOpenCV],["$HAVE_OPENCV" = "TRUE"])

for T in $ICL_BUILD_PACKAGES ; do
    ICL_BUILD_SUBDIRS="$ICL_BUILD_SUBDIRS $T/lib"
    ICL_BUILD_DOCDIRS="$ICL_BUILD_DOCDIRS $T"
    ICL_DOC_MAKEFILES_DIST="$ICL_DOC_MAKEFILES_DIST $T/Makefile.am $T/Makefile.in"
done

# Add all bin dirs after lib dirs
for T in $ICL_BUILD_PACKAGES ; do
    ICL_BUILD_SUBDIRS="$ICL_BUILD_SUBDIRS $T/bin"
done

# substituted variables become visible in Makefile.am's
AC_SUBST([ICL_BUILD_PACKAGES])
AC_SUBST([ICL_BUILD_SUBDIRS])
AC_SUBST([ICL_BUILD_DOCDIRS])
AC_SUBST([ICL_MAKEFILE_LIST])
AC_SUBST([ICL_DOC_MAKEFILES_DIST])

echo "Build Subdirs are: - $ICL_BUILD_SUBDIRS -"

###############################################################################
# NEW ICLV5: create package config files
###############################################################################

# if prefix was not explicitly set, it must be adapted to /usr
# otherwise, the pc-files get the prefix name 'NONE'
if [[ "$prefix" = "NONE" ]] ; then
   prefix=/usr
fi 

ICL_CREATE_PC_FOR([ICLUtils],["TRUE"])
ICL_CREATE_PC_FOR([ICLCore],["TRUE"])
ICL_CREATE_PC_FOR([ICLCC],["TRUE"])
ICL_CREATE_PC_FOR([ICLIO],["TRUE"])
ICL_CREATE_PC_FOR([ICLFilter],["TRUE"])
ICL_CREATE_PC_FOR([ICLBlob],["TRUE"])
ICL_CREATE_PC_FOR([ICLQt],["$HAVE_QT"]) 
ICL_CREATE_PC_FOR([ICLQuick],["TRUE"])
ICL_CREATE_PC_FOR([ICLGeom],["TRUE"])
ICL_CREATE_PC_FOR([ICLOpenCV],["$HAVE_OPENCV"])
ICL_CREATE_PC_FOR([ICLAlgorithms],["TRUE"])

ICL_CREATE_ICL_PC()

###############################################################################
# NEW ICLV5: create and subst variables for Makefile.bin.am's
###############################################################################

ICL_DEF_EXAMPLE_FLAGS_FOR([ICLUtils],[ICLUtils])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLCore],[ICLCore])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLCC],[ICLQuick])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLIO],[ICLQuick])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLBlob],[ICLQuick])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLFilter],[ICLQuick])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLGeom],[ICLAlgorithms])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLQt],[ICLAlgorithms])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLQuick],[ICLQuick])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLOpenCV],[ICLAlgorithms])
ICL_DEF_EXAMPLE_FLAGS_FOR([ICLAlgorithms],[ICLAlgorithms])


echo $IC

###############################################################################
# Create makefiles ...
###############################################################################
# make autoconf create makefile 
# note: this macros is not able to work with variables,
#       so all makefiles must be added here manually

AC_DEFUN([ICL_CONFIG_FILES_FOR_PACKAGE],
[AC_CONFIG_FILES([$1/Makefile $1/lib/Makefile:$1/Makefile.lib.in $1/bin/Makefile:$1/Makefile.bin.in])])

AC_CONFIG_FILES([Makefile])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLUtils])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLCore])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLFilter])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLCC])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLBlob])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLIO])
if [[ "$HAVE_QT" = "TRUE" ]] ; then
ICL_CONFIG_FILES_FOR_PACKAGE([ICLQt])
fi
ICL_CONFIG_FILES_FOR_PACKAGE([ICLQuick])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLGeom])
ICL_CONFIG_FILES_FOR_PACKAGE([ICLAlgorithms])
if [[ "$HAVE_OPENCV" = "TRUE" ]] ; then
ICL_CONFIG_FILES_FOR_PACKAGE([ICLOpenCV])
fi
AC_OUTPUT

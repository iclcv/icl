AC_DEFUN([ICL_CHECK_OPENCV],[
AC_ARG_WITH([OPENCV],
        [AS_HELP_STRING([--without-OPENCV],
                        [disable support for OPENCV])],
        [HAVE_OPENCV=FALSE],
        [ICL_NOTIFY_CHECK([OpenCV        ])
        ICL_WITH_ROOT([OPENCV],[/usr])
        HAVE_OPENCV=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(OPENCV,lib,include/opencv)

        AC_CHECK_FILE([$OPENCV_ROOT/lib/pkgconfig/opencv.pc],[],[HAVE_OPENCV=FALSE])  

        AC_CHECK_HEADER([cv.h],[],[HAVE_OPENCV=FALSE])
        AC_CHECK_HEADER([cvaux.h],[],[HAVE_OPENCV=FALSE])
        AC_CHECK_HEADER([cxcore.h],[],[HAVE_OPENCV=FALSE])
        AC_CHECK_LIB([cv],[cvSmooth],[],[HAVE_OPENCV=FALSE],[-lcvaux])
        AC_CHECK_LIB([cvaux],[cvSegmentImage],[],[HAVE_OPENCV=FALSE])
        AC_CHECK_LIB([cxcore],[cvAlloc],[],[HAVE_OPENCV=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_OPENCV" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([OPENCV],[opencv])
        fi])
AM_CONDITIONAL([HAVE_OPENCV_COND],[test x$HAVE_OPENCV = xTRUE])
])

AC_DEFUN([ICL_CHECK_LIBDC],[
AC_ARG_WITH([LIBDC],
        [AS_HELP_STRING([--without-LIBDC],
                        [disable support for LIBDC])],          
        [HAVE_LIBDC=FALSE],
        [ICL_NOTIFY_CHECK([libdc1394-2   ])
        ICL_WITH_ROOT([LIBDC],[/usr])
        HAVE_LIBDC=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBDC,lib,include)
        AC_CHECK_FILE([$LIBDC_ROOT/lib/pkgconfig/libdc1394-2.pc],[],[HAVE_LIBDC=FALSE])  
        AC_CHECK_HEADER([dc1394/dc1394.h],[],[HAVE_LIBDC=FALSE])
        AC_CHECK_LIB([dc1394],[dc1394_camera_enumerate],[],[HAVE_LIBDC=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_LIBDC" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([LIBDC],[libdc1394-2])
        fi])
AM_CONDITIONAL([HAVE_LIBDC_COND],[test x$HAVE_LIBDC = xTRUE])
])
AC_DEFUN([ICL_CHECK_VIDEODEV],[
AC_ARG_WITH([VIDEODEV],
        [AS_HELP_STRING([--without-VIDEODEV],
                        [disable support for VIDEODEV])],
        [HAVE_VIDEODEV=FALSE],
        [ICL_NOTIFY_CHECK([videodev       ])
        ICL_WITH_ROOT([VIDEODEV],[/usr])
        HAVE_VIDEODEV=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(VIDEODEV,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([linux/videodev.h],[],[HAVE_VIDEODEV=FALSE])
        AC_CHECK_HEADER([sys/ioctl.h],[],[HAVE_VIDEODEV=FALSE])
        AC_CHECK_HEADER([sys/mman.h],[],[HAVE_VIDEODEV=FALSE])
        AC_CHECK_HEADER([fcntl.h],[],[HAVE_VIDEODEV=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_VIDEODEV" = "TRUE" ; then
          ICL_DEF_VARS(
            [VIDEODEV],
            [],
            [],
            [-I$VIDEODEV_ROOT/include],
            [-DHAVE_VIDEODEV])
        fi])
AM_CONDITIONAL([HAVE_VIDEODEV_COND],[test x$HAVE_VIDEODEV = xTRUE])
])
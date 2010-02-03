AC_DEFUN([ICL_CHECK_LIBJPEG],[
AC_ARG_WITH([LIBJPEG],
        [AS_HELP_STRING([--without-LIBJPEG],
                        [disable support for LIBJPEG])],
        [HAVE_LIBJPEG=FALSE],
        [ICL_NOTIFY_CHECK([libjpeg       ])
        ICL_WITH_ROOT([LIBJPEG],[/usr])
        
        HAVE_LIBJPEG=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBJPEG,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([jpeglib.h],[],[HAVE_LIBJPEG=FALSE])
        AC_CHECK_LIB([jpeg],[jpeg_std_error],[],[HAVE_LIBJPEG=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_LIBJPEG" = "TRUE" ; then
           ICL_DEF_VARS(
                [LIBJPEG],
                [-L$LIBJPEG_ROOT/lib -ljpeg],
                [-Wl,-rpath -Wl,$LIBJPEG_ROOT/lib],
                [-I$LIBJPEG_ROOT/include],
                [-DHAVE_LIBJPEG])
        fi])

AM_CONDITIONAL([HAVE_LIBJPEG_COND],[test x$HAVE_LIBJPEG = xTRUE])
])
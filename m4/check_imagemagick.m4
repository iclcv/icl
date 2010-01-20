AC_DEFUN([ICL_CHECK_IMAGEMAGICK],[
AC_ARG_WITH([IMAGEMAGICK],
        [AS_HELP_STRING([--without-IMAGEMAGICK],
                        [disable support for IMAGEMAGICK])],
        [HAVE_IMAGEMAGICK=FALSE],
        [ICL_NOTIFY_CHECK([Magick++      ])
        ICL_WITH_ROOT([IMAGEMAGICK],[/usr])

        HAVE_IMAGEMAGICK=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(IMAGEMAGICK,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([Magick++.h],[],[HAVE_IMAGEMAGICK=FALSE])
        AC_CHECK_LIB([Magick++],[InitializeMagick],[],[HAVE_IMAGEMAGICK=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_IMAGEMAGICK" = "TRUE" ; then
           ICL_DEF_VARS(
                [IMAGEMAGICK],
                [-L$IMAGEMAGICK_ROOT/lib -lMagick++],
                [-Wl,-rpath=$IMAGEMGAGICK_ROOT/lib],
                [-I$IMAGEMAGICK_ROOT/include],
                [-DHAVE_IMAGEMAGICK])
        fi])             
AM_CONDITIONAL([HAVE_IMAGEMAGICK_COND],[test x$HAVE_IMAGEMAGICK = xTRUE])
])
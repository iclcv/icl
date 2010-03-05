AC_DEFUN([ICL_CHECK_LIBMAGICK],[
AC_ARG_WITH([LIBMAGICK],
        [AS_HELP_STRING([--without-LIBMAGICK],
                        [disable support for LIBMAGICK])],
        [HAVE_LIBMAGICK=FALSE],
        [ICL_NOTIFY_CHECK([Magick++      ])
        ICL_WITH_ROOT([LIBMAGICK],[/usr])

        HAVE_LIBMAGICK=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBMAGICK,lib,include/GraphicsMagick)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([Magick++.h],[],[HAVE_LIBMAGICK=FALSE])
        AC_CHECK_LIB([GraphicsMagick++],[InitializeMagick],[],[HAVE_LIBMAGICK=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_LIBMAGICK" = "TRUE" ; then
           ICL_DEF_VARS(
                [LIBMAGICK],
                [-L$LIBMAGICK_ROOT/lib -lGraphicsMagick++],
                [-Wl,-rpath -Wl,$IMAGEMGAGICK_ROOT/lib],
                [-I$LIBMAGICK_ROOT/include],
                [-DHAVE_LIBMAGICK])
        fi])             
AM_CONDITIONAL([HAVE_LIBMAGICK_COND],[test x$HAVE_LIBMAGICK = xTRUE])
])
AC_DEFUN([ICL_CHECK_LIBMAGICK],[
AC_ARG_WITH([LIBMAGICK],
        [AS_HELP_STRING([--without-LIBMAGICK],
                        [disable support for LIBMAGICK])],
        [HAVE_LIBMAGICK=FALSE],
        [ICL_NOTIFY_CHECK([Magick++      ])
        ICL_WITH_ROOT([LIBMAGICK],[/usr])

        AC_LANG([C++])

        HAVE_LIBMAGICK=TRUE
		  cache_var=AS_TR_SH([ac_cv_header_Magick++.h])

		  # first check for libGraphicsMagick++
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBMAGICK,lib,include/GraphicsMagick)
        
        AC_CHECK_HEADER([Magick++.h],[],[HAVE_LIBMAGICK=FALSE])
        AC_CHECK_LIB([GraphicsMagick++],[InitializeMagick],,[HAVE_LIBMAGICK=FALSE])
		  MAGICK_LIBS=$LIBS
        ICL_POP_FLAG_VARS

		  # alternatively check for old libMagick++
        if test "$HAVE_LIBMAGICK" = "FALSE" ; then
		          HAVE_LIBMAGICK=TRUE
					 $as_unset $cache_var

		          ICL_PUSH_FLAG_VARS
        			 ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBMAGICK,lib,include/ImageMagick)
        
		          AC_CHECK_HEADER([Magick++.h],[],[HAVE_LIBMAGICK=FALSE])
        			 AC_CHECK_LIB([Magick++],[InitializeMagick],,[HAVE_LIBMAGICK=FALSE])
		  			 MAGICK_LIBS=$LIBS
        			 ICL_POP_FLAG_VARS
		  fi

        if test "$HAVE_LIBMAGICK" = "TRUE" ; then
           ICL_DEF_VARS(
                [LIBMAGICK],
                [-L$LIBMAGICK_ROOT/lib $MAGICK_LIBS],
                [-Wl,-rpath -Wl,$IMAGEMGAGICK_ROOT/lib],
                [-I$LIBMAGICK_ROOT/include],
                [-DHAVE_LIBMAGICK])
        fi])             
AM_CONDITIONAL([HAVE_LIBMAGICK_COND],[test x$HAVE_LIBMAGICK = xTRUE])
])

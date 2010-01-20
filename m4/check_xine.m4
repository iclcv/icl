AC_DEFUN([ICL_CHECK_XINE],[
AC_ARG_WITH([XINE],
        [AS_HELP_STRING([--without-XINE],
                        [disable support for XINE])],
        [HAVE_XINE=FALSE],
        [ICL_NOTIFY_CHECK([XINE          ])
        ICL_WITH_ROOT([XINE],[/usr])
        HAVE_XINE=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(XINE,lib,include)
        AC_CHECK_FILE([$XINE_ROOT/lib/pkgconfig/libxine.pc],[],[HAVE_XINE=FALSE])  
        AC_CHECK_HEADER([xine.h],[],[HAVE_XINE=FALSE])
        AC_CHECK_HEADER([xine/xineutils.h],[],[HAVE_XINE=FALSE])
        AC_CHECK_LIB([xine],[xine_new],[],[HAVE_XINE=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_XINE" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([XINE],[libxine])
        fi])
AM_CONDITIONAL([HAVE_XINE_COND],[test x$HAVE_XINE = xTRUE])
])
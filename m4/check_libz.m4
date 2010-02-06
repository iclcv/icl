
+AC_DEFUN([ICL_CHECK_LIBZ],[
AC_ARG_WITH([LIBZ],
        [AS_HELP_STRING([--without-LIBZ],
                        [disable support for LIBZ])],
        [HAVE_LIBZ=FALSE],
        [ICL_NOTIFY_CHECK([libz          ])
        ICL_WITH_ROOT([LIBZ],[/usr])
        
        HAVE_LIBZ=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBZ,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([zlib.h],[],[HAVE_LIBZ=FALSE])
        AC_CHECK_LIB([z],[zlibVersion],[],[HAVE_LIBZ=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_LIBZ" = "TRUE" ; then
           ICL_DEF_VARS(
                [LIBZ],
                [-L$LIBZ_ROOT/lib -lz],
                [-Wl,-rpath -Wl,$LIBZ_ROOT/lib],
                [-I$LIBZ_ROOT/include],
                [-DHAVE_LIBZ])
        fi])
AM_CONDITIONAL([HAVE_LIBZ_COND],[test x$HAVE_LIBZ = xTRUE])
])
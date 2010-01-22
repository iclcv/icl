AC_DEFUN([ICL_CHECK_SVS],[
AC_ARG_WITH([SVS],
        [AS_HELP_STRING([--without-SVS],
                        [disable support for SVS])],
        [HAVE_SVS=FALSE],
        [ICL_NOTIFY_CHECK([SVS           ])
        ICL_WITH_ROOT([SVS],[/usr])
        HAVE_SVS=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(SVS,bin,src)                
        
        AC_CHECK_HEADER([svs.h],[],[HAVE_SVS=FALSE])
        AC_CHECK_LIB([svs],[svsEngineVersion],[],[HAVE_SVS=FALSE],[-lsvscalc])
        AC_CHECK_LIB([svscalc],[main],[],[HAVE_SVS=FALSE],[-lsvs])

        ICL_POP_FLAG_VARS

        if test "$HAVE_SVS" = "TRUE" ; then
           ICL_DEF_VARS(
                [SVS],
                [-L$SVS_ROOT/bin -lsvs -lsvscalc],
                [-Wl,-rpath=$SVS_ROOT/bin],
                [-I$SVS_ROOT/src],
                [-DHAVE_SVS])
        fi])
AM_CONDITIONAL([HAVE_SVS_COND],[test x$HAVE_SVS = xTRUE])
])

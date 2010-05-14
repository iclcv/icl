AC_DEFUN([ICL_CHECK_OPENSURF],[
AC_ARG_WITH([OPENSURF],
        [AS_HELP_STRING([--without-OPENSURF],
                        [disable support for OPENSURF])],
        [HAVE_OPENSURF=FALSE],
        [ICL_NOTIFY_CHECK([OpenSURF        ])
        ICL_WITH_ROOT([OPENSURF],[/vol/nivision/opensurf/OpenSURFcpp])
        HAVE_OPENCV=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(OPENSURF,src,src)

        AC_CHECK_FILE([$OPENSURF_ROOT/src/pkgconfig/opensurf.pc],[],[HAVE_OPENSURF=FALSE])  

        AC_CHECK_HEADER([fasthessian.h],[],[HAVE_OPENSURF=FALSE])
        #AC_CHECK_HEADER([integral.h],[],[HAVE_OPENSURF=FALSE])
        #AC_CHECK_HEADER([ipoint.h],[],[HAVE_OPENSURF=FALSE])
	#AC_CHECK_HEADER([kmeans.h],[],[HAVE_OPENSURF=FALSE])
        #AC_CHECK_HEADER([surf.h],[],[HAVE_OPENSURF=FALSE])
	#AC_CHECK_HEADER([surflib.h],[],[HAVE_OPENSURF=FALSE])
	#AC_CHECK_HEADER([utils.h],[],[HAVE_OPENSURF=FALSE])

	AC_CHECK_LIB([opensurf],[main],[],[HAVE_OPENSURF=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_OPENSURF" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([OPENSURF],[opensurf])
        fi])
AM_CONDITIONAL([HAVE_OPENSURF_COND],[test x$HAVE_OPENSURF = xTRUE])
])

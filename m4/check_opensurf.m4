AC_DEFUN([ICL_CHECK_OPENSURF],[
AC_ARG_WITH([OPENSURF],
        [AS_HELP_STRING([--without-OPENSURF],
                        [disable support for OPENSURF])],
        [HAVE_OPENSURF=FALSE],
        [ICL_NOTIFY_CHECK([OpenSURF        ])
        ICL_WITH_ROOT([OPENSURF],[/vol/nivision])
        HAVE_OPENSURF=TRUE

        ICL_PUSH_FLAG_VARS
  	AC_CHECK_FILE([$OPENSURF_ROOT/lib/pkgconfig/opensurf.pc],[],[HAVE_OPENSURF=FALSE])
	ICL_EXTEND_FLAG_VARS_TMP_FROM_PC_FOR(opensurf,OPENSURF)

        AC_CHECK_HEADER([opensurf/fasthessian.h],[],[HAVE_OPENSURF=FALSE])
        AC_CHECK_HEADER([opensurf/integral.h],[],[HAVE_OPENSURF=FALSE])
        AC_CHECK_HEADER([opensurf/surf.h],[],[HAVE_OPENSURF=FALSE])
	AC_CHECK_HEADER([opensurf/surflib.h],[],[HAVE_OPENSURF=FALSE])
	AC_CHECK_HEADER([opensurf/utils.h],[],[HAVE_OPENSURF=FALSE])
        
	# These two files are not self-contained, so we cannot check
	# them here correctly. Maybe a later opensurf-release will fix this
    	#AC_CHECK_HEADER([opensurf/ipoint.h],[],[HAVE_OPENSURF=FALSE]) 
	#AC_CHECK_HEADER([opensurf/kmeans.h],[],[HAVE_OPENSURF=FALSE])

	AC_CHECK_LIB([opensurf],[main],[],[HAVE_OPENSURF=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_OPENSURF" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([OPENSURF],[opensurf])
        fi])
AM_CONDITIONAL([HAVE_OPENSURF_COND],[test x$HAVE_OPENSURF = xTRUE])
])

AC_DEFUN([ICL_CHECK_OPENGL],[
AC_ARG_WITH([OPENGL],
        [AS_HELP_STRING([--without-OPENGL],
                        [disable support for OPENGL])],
        [HAVE_OPENGL=FALSE],
        [ICL_NOTIFY_CHECK([OpenGL        ])
        ICL_WITH_ROOT([OPENGL],[/usr])
        HAVE_OPENGL=TRUE

        ICL_PUSH_FLAG_VARS

        if [[ "$ARCH" = "APPLE" ]] ; then
        # on mac, we use -framework OpenGL
        
        AC_CHECK_FRAMEWORK([OpenGL],[main],[],[HAVE_OPENGL=FALSE])

        if test "$HAVE_OPENGL" = "TRUE" ; then
           ICL_DEF_VARS(
                [OPENGL],
                [-F$OPENGL_ROOT -framework OpenGL],
                [],
                [],
                [-DHAVE_OPENGL])
        fi

        
        LDFLAGS="$LDFLAGS -F$OPENGL_ROOT"

        else

        ICL_EXTEND_FLAG_VARS_TMP_FOR(OPENGL,lib,include)
        
        AC_CHECK_HEADER([GL/gl.h],[],[HAVE_OPENGL=FALSE])
        AC_CHECK_HEADER([GL/glu.h],[],[HAVE_OPENGL=FALSE])
        AC_CHECK_LIB([GL],[glBegin],[],[HAVE_OPENGL=FALSE])
        AC_CHECK_LIB([GLU],[gluLookAt],[],[HAVE_OPENGL=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_OPENGL" = "TRUE" ; then
           ICL_DEF_VARS(
                [OPENGL],
                [-L$OPENGL_ROOT/lib -lGL -lGLU],
                [-Wl,-rpath -Wl,${OPENGL_ROOT}/lib],
                [-I$OPENGL_ROOT/include],
                [-DHAVE_OPENGL])
        fi
        
        fi])
AM_CONDITIONAL([HAVE_OPENGL_COND],[test x$HAVE_OPENGL = xTRUE])
])
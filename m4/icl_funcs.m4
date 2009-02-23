AC_DEFUN([ICL_PUSH_FLAG_VARS],
        [LIBS_SAVE="$LIBS"
        LDFLAGS_SAVE="$LDFLAGS"
        CXXFLAGS_SAVE="$CXXFLAGS"
        CFLAGS_SAVE="$CFLAGS"
        CPPFLAGS_SAVE="$CPPFLAGS"
        CXXCPP_SAVE="$CXXCPP"
        ICL_PC_LIBS_SAVE="$ICL_PC_LIBS"
        ICL_PC_CFLAGS_SAVE="$ICL_PC_CFLAGS"
        ICL_PC_REQ_SAVE="$ICL_PC_REQ"
        ])
AC_DEFUN([ICL_POP_FLAG_VARS],
        [LIBS=$LIBS_SAVE
        LDFLAGS=$LDFLAGS_SAVE
        CXXFLAGS=$CXXFLAGS_SAVE
        CFLAGS=$CFLAGS_SAVE
        CPPFLAGS=$CPPFLAGS_SAVE
        CXXCPP=$CXXCPP_SAVE
        ICL_PC_LIBS=$ICL_PC_LIBS_SAVE
        ICL_PC_CFLAGS=$ICL_PC_CFLAGS_SAVE
        ICL_PC_REQ=$ICL_PC_REQ_SAVE
        ])

#ORDER LIBS, LDFLAGS, CXXFLAGS, CXXCPP, CFLAGS, CPPFLAGS 
# HACK HERE!
AC_DEFUN([ICL_EXTEND_FLAG_VARS],
        [LIBS="${LIBS} $1"
        LDFLAGS="$LDFLAGS $2"
        CXXFLAGS="$CXXFLAGS $3 $4"
        CXXCPP="$CXXCPP $4"
        CFLAGS="$CFLAGS $5"
        CPPFLAGS="$CPPFLAGS $6"])



AC_DEFUN([ICL_WITH_ROOT],
        [AC_ARG_WITH([$1-Root],
            [AS_HELP_STRING([--with-$1-Root],
                            [Set $1-Root directory (default=$2)])],
            [$1_ROOT=$with_$1_Root],
            [$1_ROOT=$2])
        ])

AC_DEFUN([ICL_EXTEND_FLAG_VARS_TMP_FOR],
[ICL_EXTEND_FLAG_VARS(
        [-L${$1_ROOT}/$2],
        [-Wl,-rpath=${$1_ROOT}/$2],
        [-I${$1_ROOT}/$3],
        [-I${$1_ROOT}/$3],
        [-I${$1_ROOT}/$3],
        [-I${$1_ROOT}/$3])
])
AC_DEFUN([SHOW_VAR],[AC_MSG_NOTICE([$1 is $$1])])

AC_DEFUN([ICL_NOTIFY_CHECK],[
AC_MSG_NOTICE([+------------------------------------+])
AC_MSG_NOTICE([| Testing for $1         |])
AC_MSG_NOTICE([+------------------------------------+])
])

AC_DEFUN([ICL_NOTIFY_SUPPORT],[
        if test "${HAVE_$1}" = "TRUE" ; then
          $1_SUPPORTED="enabled "
        else
          $1_SUPPORTED="disabled"
        fi
        AC_MSG_NOTICE([| $2 : ${$1_SUPPORTED}                  |])
        ])

AC_DEFUN([ICL_ADD_PACKAGE_IF],
        [if test $2 ; then export ICL_BUILD_PACKAGES="$ICL_BUILD_PACKAGES $1" ; fi])


AC_DEFUN([ICL_EXTEND_PC_FLAGS],[
ICL_PC_LIBS="$ICL_PC_LIBS $1"
ICL_PC_CFLAGS="$ICL_PC_CFLAGS $2"
ICL_PC_REQ="$ICL_PC_REQ $3"])


AC_DEFUN([ICL_USE_PC_INPUT],[
export PKG_CONFIG_PATH="$$1_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH"
ICL_$1_LIBS=`pkg-config --libs-only-L $2`
ICL_$1_LDFLAGS=`echo $ICL_$1_LIBS | sed "s|-L|-Wl,-rpath=|g"`
ICL_$1_LIBS="$ICL_$1_LIBS `pkg-config --libs-only-l $2`"
ICL_$1_CXXFLAGS=`pkg-config --cflags-only-I $2`
ICL_$1_CXXCPP="`pkg-config --cflags-only-other $2` -DHAVE_$1"
ICL_EXTEND_FLAG_VARS([$ICL_$1_LIBS],[$ICL_$1_LDFLAGS],[$ICL_$1_CXXCPP],[$ICL_$1_CXXFLAGS])
ICL_EXTEND_PC_FLAGS([$ICL_$1_LDFLAGS],[-DHAVE_$1],[$2])
])


AC_DEFUN([ICL_EXTEND_FLAG_VARS_2],
        [ICL_$1_LIBS="$2"
        ICL_$1_LDFLAGS="$3"
        ICL_$1_CXXFLAGS="$4 $5"
        ICL_$1_CXXCPP="$5"
        ICL_$1_CFLAG="$6"
        ICL_$1_CPPFLAGS="$7"
        LIBS="${LIBS} $ICL_$1_LIBS"
        LDFLAGS="$LDFLAGS $ICL_$1_LDFLAGS"
        CXXFLAGS="$CXXFLAGS $ICL_$1_CXXFLAGS"
        CXXCPP="$CXXCPP $ICL_$1_CXXCPP"
        CFLAGS="$CFLAGS $ICL_$1_CFLAGS"
        CPPFLAGS="$CPPFLAGS $ICL_$1_CPPFLAGS"
        ICL_EXTEND_PC_FLAGS([$ICL_$1_LIBS $ICL_$1_LDFLAGS],[$ICL_$1_CXXFLAGS $ICL_$1_CXXCPP])
])



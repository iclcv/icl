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
        CPPFLAGS="$CPPFLAGS $6"
        # this is a hack here, that allows to link dynamically against other libs 
        LIBS=`echo $LIBS | sed 's|-L\(.*\) |-L\1 -Wl,-rpath=\1|g' | sed 's|-L\(.*\)$|-L\1 -Wl,-rpath=\1|g'`
])

AC_DEFUN([ICL_WITH_ROOT],
        [AC_ARG_WITH([$1-Root],
            [AS_HELP_STRING([--with-$1-Root],
                            [Set $1-Root directory (default=$2)])],
            [$1_ROOT=$with_$1_Root],
            [$1_ROOT=$2])
        ])


# parameters $1 = package config file name
#            $2 = for which icl-arg-with package
AC_DEFUN([ICL_EXTEND_FLAG_VARS_TMP_FROM_PC_FOR],
[ICL_EXTEND_FLAG_VARS(
        [`PKG_CONFIG_PATH=${$2_ROOT}/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --libs-only-L $1`],
        [`PKG_CONFIG_PATH=${$2_ROOT}/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --libs-only-other $1`],
        [`PKG_CONFIG_PATH=${$2_ROOT}/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --cflags $1`],
        [`PKG_CONFIG_PATH=${$2_ROOT}/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --cflags $1`],
        [],
        [])
])

AC_DEFUN([ICL_EXTEND_FLAG_VARS_TMP_FOR],
[ICL_EXTEND_FLAG_VARS(
        [-L${$1_ROOT}/$2],
        [-Wl,-rpath -Wl,${$1_ROOT}/$2],
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
	

# ICL_DEF_VARS(package,libs,ldflags,cxxflags,cxxcpp)
# Registers libs,cflags,... for given external
# package name.
# created variables: e.g.  ICL_${package}_LIBS 
AC_DEFUN([ICL_DEF_VARS],[
ICL_$1_LIBS="$2"
ICL_$1_LDFLAGS="$3"
#ICL_$1_CXXFLAGS="$4 $5"
ICL_$1_CXXFLAGS="$4"
ICL_$1_CXXCPP="$5"
ICL_$1_LIBS_PC="$2"
ICL_$1_LDFLAGS_PC="$3"
ICL_$1_CXXFLAGS_PC="$4 $5"
ICL_$1_CXXCPP_PC="$5"
])



# ICL_DEF_VARS_FROM_PC(package,pc-name)
# extracts xcflags and libs from given package config
# package name and create appropriate configuration
# variables from this. Furthermore -Wl,-rpath= 
# LDFLAGS are automatically created
AC_DEFUN([ICL_DEF_VARS_FROM_PC],[
export PKG_CONFIG_PATH="$$1_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH"
ICL_$1_LIBS=`pkg-config --libs-only-L $2`
ICL_$1_LDFLAGS=`echo $ICL_$1_LIBS | sed "s|-L|-Wl,-rpath -Wl,|g"`
ICL_$1_LDFLAGS_PC=`echo $ICL_$1_LIBS | sed "s|-L|-Wl,-rpath -Wl,|g"`
ICL_$1_LIBS="$ICL_$1_LIBS `pkg-config --libs-only-l $2`"
ICL_$1_CXXFLAGS=`pkg-config --cflags-only-I $2`
ICL_$1_CXXCPP="`pkg-config --cflags-only-other $2` -DHAVE_$1"
ICL_$1_REQUIRES_PC="$ICL_$1_REQUIRES_PC $2"
ICL_$1_CXXCPP_PC="$ICL_$1_CXXCPP_PC -DHAVE_$1"
])

# ICL_USE_EXTERNAL_PACKAGE_IN(icl-package,external-package)
# This macro is use if an icl-package $1
# actually needs to be compiled and linked agains an
# external package $1
# Internally the icl-package name specific variables are extended
AC_DEFUN([ICL_USE_EXTERNAL_PACKAGE_IN],[
$1_LIBS="$$1_LIBS $ICL_$2_LIBS"
$1_CXXFLAGS="$$1_CXXFLAGS $ICL_$2_CXXFLAGS"
$1_LDFLAGS="$$1_LDFLAGS $ICL_$2_LDFLAGS"
$1_CXXCPP="$$1_CXXCPP $ICL_$2_CXXCPP"
$1_LIBS_PC="$$1_LIBS_PC $ICL_$2_LIBS_PC"
$1_CXXFLAGS_PC="$$1_CXXFLAGS_PC $ICL_$2_CXXFLAGS_PC"
$1_LDFLAGS_PC="$$1_LDFLAGS_PC $ICL_$2_LDFLAGS_PC"
$1_CXXCPP_PC="$$1_CXXCPP_PC $ICL_$2_CXXCPP_PC"
$1_REQUIRES_PC="$$1_REQUIRES_PC $ICL_$2_REQUIRES_PC"
])

# ICL_USE_EXTERNAL_PACKAGE_IN(icl-package,input-package)
AC_DEFUN([ICL_USE_INTERNAL_PACKAGE_IN],[
$1_LIBS="$$1_LIBS $$2_LIBS"
$1_CXXFLAGS="$$1_CXXFLAGS $$2_CXXFLAGS"
$1_LDFLAGS="$$1_LDFLAGS $$2_LDFLAGS"
$1_CXXCPP="$$1_CXXCPP $$2_CXXCPP"
$1_REQUIRES_PC="$$1_REQUIRES_PC $2"
])

# ICL_SUBST_VARIABLES_FOR(icl-package)
AC_DEFUN([ICL_SUBST_VARIABLES_FOR],[
AC_SUBST([$1_CXXFLAGS])
AC_SUBST([$1_CXXCPP])
AC_SUBST([$1_LDFLAGS])
AC_SUBST([$1_LIBS])
])

# ICL_STRIP_FLAGS_FOR(icl-package)
AC_DEFUN([ICL_STRIP_FLAGS_FOR],[
$1_LIBS=`echo ${$1_LIBS} | sed "s|-L/usr/lib | |g"`
$1_LIBS=`echo ${$1_LIBS} | sed "s|-L/lib | |g"`
$1_LDFLAGS=`echo ${$1_LDFLAGS} | sed "s|-Wl,-rpath -Wl,/usr/lib||g"`
$1_LDFLAGS=`echo ${$1_LDFLAGS} | sed "s|-Wl,-rpath -Wl,/lib||g"`
$1_CXXFLAGS=`echo ${$1_CXXFLAGS} | sed "s|-I/usr/include | |g"`
$1_CXXFLAGS=`echo ${$1_CXXFLAGS} | sed "s|-I/include | |g"`
$1_LIBS_PC=`echo ${$1_LIBS_PC} | sed "s|-L/usr/lib||g"`
$1_LIBS_PC=`echo ${$1_LIBS_PC} | sed "s|-L/lib||g"`
$1_LDFLAGS_PC=`echo ${$1_LDFLAGS_PC} | sed "s|-Wl,-rpath -Wl,/usr/lib||g"`
$1_LDFLAGS_PC=`echo ${$1_LDFLAGS_PC} | sed "s|-Wl,-rpath -Wl,/lib||g"`
$1_CXXFLAGS_PC=`echo ${$1_CXXFLAGS_PC} | sed "s|-I/usr/include | |g"`
$1_CXXFLAGS_PC=`echo ${$1_CXXFLAGS_PC} | sed "s|-I/include | |g"`
])

# ICL_STRIP_AND_SUBST_FLAGS_FOR(icl-package)
AC_DEFUN([ICL_STRIP_AND_SUBST_FLAGS_FOR],[
ICL_STRIP_FLAGS_FOR($1)
ICL_SUBST_VARIABLES_FOR($1)
])


# ICL_PC_ENTRY_FOR(icl-package, text)
AC_DEFUN([ICL_PC_ENTRY_FOR],[echo "$2" >> $1.pc])

# ICL_PC_ROOT_ENTRY_FOR(icl-package, external-package)
AC_DEFUN([ICL_PC_ROOT_ENTRY_FOR],[
if test "${HAVE_$2}" = "TRUE" ; then
   ICL_PC_ENTRY_FOR([$1],[$2_ROOT=${$2_ROOT}])
else
   ICL_PC_ENTRY_FOR([$1],[$2_ROOT=DISABLED])
fi  
])

# ICL_PC_ROOT_ENTRIES_FOR(icl-package)
AC_DEFUN([ICL_PC_ROOT_ENTRIES_FOR],[
ICL_PC_ROOT_ENTRY_FOR([$1],[IPP])
ICL_PC_ROOT_ENTRY_FOR([$1],[MKL])
ICL_PC_ROOT_ENTRY_FOR([$1],[LIBDC1349])
ICL_PC_ROOT_ENTRY_FOR([$1],[UNICAP])
ICL_PC_ROOT_ENTRY_FOR([$1],[OPENGL])
ICL_PC_ROOT_ENTRY_FOR([$1],[XINE])
ICL_PC_ROOT_ENTRY_FOR([$1],[QT])
ICL_PC_ROOT_ENTRY_FOR([$1],[LIBJPEG])
ICL_PC_ROOT_ENTRY_FOR([$1],[LIBZ])
ICL_PC_ROOT_ENTRY_FOR([$1],[VIDEODEV])
ICL_PC_ROOT_ENTRY_FOR([$1],[SVS])
ICL_PC_ROOT_ENTRY_FOR([$1],[XCF])
ICL_PC_ROOT_ENTRY_FOR([$1],[IMAGEMAGICK])
ICL_PC_ROOT_ENTRY_FOR([$1],[LIBMESASR])
ICL_PC_ROOT_ENTRY_FOR([$1],[OPENCV])
])

# ICL_CREATE_PC_FOR(icl-package,what has to be true)
AC_DEFUN([ICL_CREATE_PC_FOR],[
rm -rf $1.pc
if [[ $2 == "TRUE" ]] ; then
ICL_PC_ENTRY_FOR([$1],[prefix=${prefix}])
ICL_PC_ENTRY_FOR([$1],[exec_prefix=\${prefix}])
ICL_PC_ENTRY_FOR([$1],[bindir=\${prefix}/bin])
ICL_PC_ENTRY_FOR([$1],[libdir=\${prefix}/lib])
ICL_PC_ENTRY_FOR([$1],[datadir\${prefix}/share/data])
ICL_PC_ENTRY_FOR([$1],[includedir=\${prefix}/include])
ICL_PC_ENTRY_FOR([$1],[package=$1])
ICL_PC_ENTRY_FOR([$1],[])
ICL_PC_ROOT_ENTRIES_FOR([$1])
ICL_PC_ENTRY_FOR([$1],[])
ICL_PC_ENTRY_FOR([$1],[Name: $1])
ICL_PC_ENTRY_FOR([$1],[Description: ICL's $1 package])
ICL_PC_ENTRY_FOR([$1],[Version: $PACKAGE_VERSION])
ICL_PC_ENTRY_FOR([$1],[])
ICL_PC_ENTRY_FOR([$1],[Requires:${$1_REQUIRES_PC}])
ICL_PC_ENTRY_FOR([$1],[])
ICL_PC_ENTRY_FOR([$1],[Libs: -L\${libdir} -l$1 -Wl,-rpath -Wl,\${libdir} ${$1_LIBS_PC} ${$1_LDFLAGS_PC}])
ICL_PC_ENTRY_FOR([$1],[])
ICL_PC_ENTRY_FOR([$1],[Cflags: -I\${includedir}/ICL ${$1_CXXFLAGS_PC} ${$1_CXXCPP_PC}])
cat $1.pc | sed "s|\(-Wl,-rpath -Wl,.*\)|'\1'|g" > $1.pc.tmp && mv $1.pc.tmp $1.pc
fi
])

# ICL_CREATE_ICL_PC() no parameters
AC_DEFUN([ICL_CREATE_ICL_PC],[
rm -rf icl.pc
ICL_PC_ENTRY_FOR([icl],[prefix=${prefix}])
ICL_PC_ENTRY_FOR([icl],[exec_prefix=\${prefix}])
ICL_PC_ENTRY_FOR([icl],[bindir=\${prefix}/bin])
ICL_PC_ENTRY_FOR([icl],[libdir=\${prefix}/lib])
ICL_PC_ENTRY_FOR([icl],[datadir\${prefix}/share/data])
ICL_PC_ENTRY_FOR([icl],[includedir=\${prefix}/include])
ICL_PC_ENTRY_FOR([icl],[package=icl])
ICL_PC_ENTRY_FOR([icl],[])
ICL_PC_ROOT_ENTRIES_FOR([icl])
ICL_PC_ENTRY_FOR([icl],[])
ICL_PC_ENTRY_FOR([icl],[Name: icl])
ICL_PC_ENTRY_FOR([icl],[Description: Image Component Library (ICL)])
ICL_PC_ENTRY_FOR([icl],[Version: $PACKAGE_VERSION])
ICL_PC_ENTRY_FOR([icl],[])
ICL_PC_ENTRY_FOR([icl],[Requires:$ICL_BUILD_PACKAGES])
ICL_PC_ENTRY_FOR([icl],[])
ICL_PC_ENTRY_FOR([icl],[Libs:])
ICL_PC_ENTRY_FOR([icl],[])
ICL_PC_ENTRY_FOR([icl],[Cflags:])
])

# ICL_DEF_EXAMPLE_FLAGS_FOR(for which package, include which other packages)
AC_DEFUN([ICL_DEF_EXAMPLE_FLAGS_FOR],[
$1_EXAMPLE_LDFLAGS=`PKG_CONFIG_PATH=.:$PKG_CONFIG_PATH pkg-config --libs $2`
$1_EXAMPLE_CXXFLAGS=`PKG_CONFIG_PATH=.:$PKG_CONFIG_PATH pkg-config --cflags $2`
$1_EXAMPLE_CXXFLAGS="-I../../include $$1_EXAMPLE_CXXFLAGS"
for T in ${ICL_BUILD_PACKAGES} ; do 
  $1_EXAMPLE_LDFLAGS="-L../../$T/lib $$1_EXAMPLE_LDFLAGS" ;
done
AC_SUBST($1_EXAMPLE_LDFLAGS)
AC_SUBST($1_EXAMPLE_CXXFLAGS)
])


# AC_CHECK_FRAMEWORK(FRAMEWORK, FUNCTION,
#              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#              [OTHER-LIBRARIES])
# ------------------------------------------------------
#
# Use a cache variable name containing both the framework and function name,
# because the test really is for framework $1 defining function $2, not
# just for framework $1.  Separate tests with the same $1 and different $2s
# may have different results.
#
# Note that using directly AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1_$2])
# is asking for troubles, since AC_CHECK_FRAMEWORK($framework, fun) would give
# ac_cv_framework_$framework_fun, which is definitely not what was meant.  Hence
# the AS_LITERAL_IF indirection.
#
# FIXME: This macro is extremely suspicious.  It DEFINEs unconditionally,
# whatever the FUNCTION, in addition to not being a *S macro.  Note
# that the cache does depend upon the function we are looking for.
#
# It is on purpose we used `ac_check_framework_save_LIBS' and not just
# `ac_save_LIBS': there are many macros which don't want to see `LIBS'
# changed but still want to use AC_CHECK_FRAMEWORK, so they save `LIBS'.
# And ``ac_save_LIBS' is too tempting a name, so let's leave them some
# freedom.
AC_DEFUN([AC_CHECK_FRAMEWORK],
[m4_ifval([$3], , [AH_CHECK_FRAMEWORK([$1])])dnl
AS_LITERAL_IF([$1],
         [AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1_$2])],
         [AS_VAR_PUSHDEF([ac_Framework], [ac_cv_framework_$1''_$2])])dnl
AC_CACHE_CHECK([for $2 in $1 framework], ac_Framework,
[ac_check_framework_save_LIBS=$LIBS
LIBS="-framework $1 $5 $LIBS"
AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
          [AS_VAR_SET(ac_Framework, yes)],
          [AS_VAR_SET(ac_Framework, no)])
LIBS=$ac_check_framework_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Framework) = yes],
      [m4_default([$3], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_FRAMEWORK_$1))
  LIBS="-framework $1 $LIBS"
])],
      [$4])dnl
AS_VAR_POPDEF([ac_Framework])dnl
])# AC_CHECK_FRAMEWORK

# AH_CHECK_FRAMEWORK(FRAMEWORK)
# ---------------------
m4_define([AH_CHECK_FRAMEWORK],
[AH_TEMPLATE(AS_TR_CPP(HAVE_FRAMEWORK_$1),
        [Define to 1 if you have the `]$1[' framework (-framework ]$1[).])])

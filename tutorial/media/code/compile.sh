if [ ! "$1" ] ; then
    echo "$1 is the compilation target e.g. test for compiling test.cpp" ;
    exit -1 ;
fi

CXXFLAGS="$(pkg-config --cflags icl) -I." LDFLAGS="$(pkg-config --libs icl)" make $1
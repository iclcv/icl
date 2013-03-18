##!1XLDFLAGS= ${LDFLAGS} \
##!1	-L../../ICLUtils/lib/ -lICLUtils \
##!1	-L../../ICLCore/lib/ -lICLCore \
##!1	-L../../ICLCC/lib/ -lICLCC \
##!1	-L../../ICLQt/lib/ -lICLQt \
##!1	-L../../ICLQuick/lib/ -lICLQuick \
##!1	-L../../ICLFilter/lib -lICLFilter \
##!1	-L../../ICLIO/lib -lICLIO
##!1XCXXFLAGS= ${CXXFLAGS} \
##!1	-I../../ICLUtils/src \
##!1	-I../../ICLCore/src \
##!1	-I../../ICLCC/src \
##!1	-I../../ICLQt/src \
##!1	-I../../ICLQuick/src \
##!1	-I../../ICLFilter/src \
##!1	-I../../ICLIO/src
##!1
##!1#################################################################################
##!1## Applications that depend on noting ###########################################
##!1#################################################################################


function usage {
    echo "usage: $0 [FLAGS]"
    echo "  flags are:"
    echo "  -s show created file on std out"
    echo "  -c create conditional sections"
}

FLAG_S=NOT_GIVEN
FLAG_C=NOT_GIVEN

for T in $1 $2 $3 $4 $5 ; do
    if [ "$T" = "-s" ] ; then
        FLAG_S=GIVEN
    fi
    if [ "$T" = "-c" ] ; then
        FLAG_C=GIVEN
    fi
    if [ "$T" = "-h" ] ; then
        usage
        exit
    fi
    if [ "$T" = "--help" ] ; then
        usage
        exit
    fi

done



M=Makefile.bin.am
APPLICATION_PREFIX2=icl-
APPLICATION_PREFIX=`echo $APPLICATION_PREFIX2 | sed "s|-|_|g"`

if [ -f $M ] ; then
    mv $M $M.bak
fi
 
cat $0 | grep "##!1" | grep -v "grep" | sed "s|##!1||g" > $M
echo -n "bin_PROGRAMS=" >> $M
FILES=`ls examples/*.cpp | sed "s|.cpp||g" | sed "s|examples\/||g"`

for T in $FILES ; do
    echo " \\" >> $M
    echo -e -n $APPLICATION_PREFIX2$T >> $M
done

echo "" >> $M
echo "" >> $M
echo "# explicit sources, ldflags and cxxflags" >> $M

S="_SOURCES"
L="_LDFLAGS=\${XLDFLAGS}"
C="_CXXFLAGS=\${XCXXFLAGS}"
for T in $FILES ; do
    NAME=`echo $T | sed "s|examples\/||g" | sed "s|-|_|g"`
#    echo File T is --$T--
#    echo Name is   --$NAME--
    echo $APPLICATION_PREFIX$NAME$S=../examples/$T.cpp >> $M
    echo $APPLICATION_PREFIX$NAME$L >> $M
    echo $APPLICATION_PREFIX$NAME$C >> $M
    echo "" >> $M
done

if [ "$FLAG_C" = "GIVEN" ] ; then
    echo "" >> $M
    COND="_COND"
    for T in LIBDC IPP QT UNICAP IMAGEMAGICK LIBJPEG LIBZ XCF SVS ; do
        echo "if HAVE_$T$COND" >> $M
        echo "#bin_PROGRAMS+= " >> $M
        echo "endif" >> $M
        echo "" >> $M
    done
fi

if [ "$FLAG_S" = "GIVEN" ] ; then
    echo wrote this file:
    echo "-------------------------------------->8------------------"
    cat $M
    echo "-------8<-------------------------------------------------"
fi


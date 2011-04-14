DEPS="ipp mkl opencv libz libfreenect libmesasr libjpeg unicap xine qt opengl glx opensurf gtest xcf imagemagick libdc videodev"

function usage {
    echo "usage: config-generator.sh <full-config-file> <build-dir>"
    exit -1
}

if [ "$#" != "2" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then 
    usage ;
fi

FULL_CFG=$1
BUILD_DIR=$2

# ensure absolute build dir
case $BUILD_DIR in
     /*) ;;
     *) BUILD_DIR=$PWD/$BUILD_DIR ;;
esac


if [ ! -e $FULL_CFG ] ; then 
    echo "unable to find given config file $FULL_CFG" ;
    usage ;
fi

if [ ! -d $BUILD_DIR ] ; then
    mkdir -p $BUILD_DIR
    if [ ! -d $BUILD_DIR ] ; then
        echo unable to create build dir $BUILD_DIR
        usage ;
    fi
fi

rm -rf $BUILD_DIR/run.sh
touch $BUILD_DIR/run.sh
chmod +x $BUILD_DIR/run.sh

NUM=$(echo "$DEPS" | wc -w)
NUM=$(echo "$NUM*2" | bc -l)
NUM=$(echo "$NUM+2" | bc -l)
CUR=3

echo build $BUILD_DIR/full > $BUILD_DIR/full.cfg
cat $FULL_CFG | grep -v "^build" >> $BUILD_DIR/full.cfg
echo "# always have a new line at the end of file" >> $BUILD_DIR/full.cfg
echo "configuring full build (1/$NUM)"
yes | ./configure $BUILD_DIR/full.cfg > $BUILD_DIR/full-config.log &
echo "make VERBOSE=1 all &> $BUILD_DIR/full-build.log && touch $BUILD_DIR/success-full || touch $BUILD_DIR/error-full" > $BUILD_DIR/full-build.sh
chmod +x $BUILD_DIR/full-build.sh
echo "cd $BUILD_DIR/full" >> $BUILD_DIR/run.sh
echo "../full-build.sh&" >> $BUILD_DIR/run.sh

echo build $BUILD_DIR/empty > $BUILD_DIR/empty.cfg
echo "# always have a new line at the end of file" >> $BUILD_DIR/empty.cfg
echo "configuring empty build (2/$NUM)"
yes | ./configure $BUILD_DIR/empty.cfg > $BUILD_DIR/empty-config.log &
echo "make VERBOSE=1 all &> $BUILD_DIR/empty-build.log && touch $BUILD_DIR/success-empty || touch $BUILD_DIR/error-empty" > $BUILD_DIR/empty-build.sh
chmod +x $BUILD_DIR/empty-build.sh
echo "cd $BUILD_DIR/empty" >> $BUILD_DIR/run.sh
echo "../empty-build.sh&" >> $BUILD_DIR/run.sh




for D in $DEPS ; do
echo build $BUILD_DIR/only_$D > $BUILD_DIR/only_$D.cfg
echo build $BUILD_DIR/no_$D > $BUILD_DIR/no_$D.cfg
cat $FULL_CFG | grep -v "^build" | grep -v "^$D" >> $BUILD_DIR/no_$D.cfg
cat $FULL_CFG | grep -v "^build" | grep "^$D" >> $BUILD_DIR/only_$D.cfg
echo '#'$D >> $BUILD_DIR/no_$D.cfg
echo "# always have a new line at the end of file" >> $BUILD_DIR/no_$D.cfg
echo "# always have a new line at the end of file" >> $BUILD_DIR/only_$D.cfg

echo "configuring only $D build ($CUR/$NUM)"
CUR=$(echo "$CUR+1" | bc -l)
yes | ./configure $BUILD_DIR/only_$D.cfg > $BUILD_DIR/only_$D-config.log &

echo "configuring no $D build ($CUR/$NUM)"
CUR=$(echo "$CUR+1" | bc -l)

yes | ./configure $BUILD_DIR/no_$D.cfg > $BUILD_DIR/no_$D-config.log &
echo "make VERBOSE=1 all &> $BUILD_DIR/only_$D-build.log && touch $BUILD_DIR/success-only_$D || touch $BUILD_DIR/error-only_$D" > $BUILD_DIR/only_$D-build.sh
chmod +x $BUILD_DIR/only_$D-build.sh
echo "make VERBOSE=1 all &> $BUILD_DIR/no_$D-build.log && touch $BUILD_DIR/success-no_$D || touch $BUILD_DIR/error-no_$D" > $BUILD_DIR/no_$D-build.sh
chmod +x $BUILD_DIR/no_$D-build.sh

echo "cd $BUILD_DIR/only_$D" >> $BUILD_DIR/run.sh
echo "../only_$D-build.sh&" >> $BUILD_DIR/run.sh
echo "cd $BUILD_DIR/no_$D" >> $BUILD_DIR/run.sh
echo "../no_$D-build.sh&" >> $BUILD_DIR/run.sh
done


#rm -rf run.sh
#touch run.sh
#chmod +x run.sh
#echo "for i in $(ls) ; do" >> run.sh
#echo "  screen"
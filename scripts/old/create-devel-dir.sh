#!/bin/bash

if [ ! -d devel ] ; then
    echo "creating devel directory in $PWD"
    mkdir devel
fi

for T in ICL* ; do 
    if [ -d $T ] ; then 
        echo "creating links for $T" ; 
        echo "headers ..." ;
        for H in include/$T/*.h ; do
            rm -f $PWD/devel/`basename $H` 
            ln -s $PWD/$H $PWD/devel/`basename $H` 
        done
        echo "sources ..."
        for S in $T/src/*.cpp ; do
            rm -f $PWD/devel/`basename $S` 
            ln -s $PWD/$S $PWD/devel/`basename $S` 
        done
        echo "examples ..."
        for E in $T/examples/*.cpp ; do
            rm -f $PWD/devel/`basename $E` 
            ln -s $PWD/$E $PWD/devel/`basename $E` 
        done
    fi ; 
done

echo `ls -1 devel | wc -l` links created. done!


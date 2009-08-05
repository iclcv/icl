#!/bin/bash

MYCONFIGURE="configure.ac-`uname -m`"

if test -f $MYCONFIGURE; then  
    echo "Preparing for `uname -m` architecture"
    if test -L configure.ac; then
        rm -f configure.ac
    fi
    ln -s $MYCONFIGURE configure.ac
    autoreconf --install --force 
else
    echo "No configure.ac for `uname -m` architecture available"
    echo "Aborting ..."
    exit 1
fi

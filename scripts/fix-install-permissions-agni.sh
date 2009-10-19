#!/bin/bash

V=`./VERSION.sh`
P=`cat Makefile | grep 'prefix = ' | cut -d' ' -f3 | grep -v '${prefix}'`

echo "VERSION is $V"
echo "PREFIX is $P"
echo ""
echo "adding write permissions for group nivision using:"
echo "> chmod -R g+w $P" 
echo ""
echo "please press enter if you want to succeed!"

read ENTER

if [ "$ENTER" = "" ] ; then
    echo "please wait a sec ..."
    chmod -R g+w $P 2>&1 | grep "alle meine entchen schwimmen auf dem see" ;
    echo "permissions changed!" ;
else
    echo "aborted!" ;
fi

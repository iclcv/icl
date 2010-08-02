#!/bin/bash

ICL_TOP_DIR=/vol/nivision/ICL

if [ -d "$ICL_TOP_DIR/cur" ] ; then
    CURRENT_VERSION=`ls -lA $ICL_TOP_DIR/cur | cut -d ">" -f 2 | tr -d " "`
    echo "\"$ICL_TOP_DIR/cur\" links to \"ICL_TOP_DIR/$CURRENT_VERSION\""
    echo "Should link to \"$ICL_TOP_DIR/$ICL_VER\""

    if [ $CURRENT_VERSION == $ICL_VER ] ; then
	echo "Nothing to do"
    else
	if [ -d $ICL_TOP_DIR/$ICL_VER ] ; then
	    cd $ICL_TOP_DIR
	    rm cur
	    ln -s $ICL_VER cur
	    cd - > /dev/null
	    echo "Soft link \"$ICL_TOP_DIR/cur\" updated to \"$ICL_TOP_DIR/$ICL_VER\""
	else
	    echo "$ICL_TOP_DIR/$ICL_VER not found. Please install ICL first."
	fi
    fi
else
    cd $ICL_TOP_DIR
    ln -s $ICL_VER cur
    cd - > /dev/null
    echo "Soft link \"$ICL_TOP_DIR/cur\" created. Links to \"$ICL_TOP_DIR/$ICL_VER\""
fi

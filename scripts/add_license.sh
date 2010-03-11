#!/bin/bash

# this script is used to add and to remove the ICL-license text from all files, 
# IMPORTANT:
# In case of removing the license text in order to change it before re-adding
# the new text, always remove the text before changing it. The remove step is
# a little simple: is simply counts the lines of the current license text and 
# removes that much preceding lines from each file. If you already changed the
# current license text, and the new version has more or less lines, you corrupt
# all files without being able to fix it afterwards with this script

#LICENSE_TEXT/*************************************************************************** 
#LICENSE_TEXT**                                                                        **
#LICENSE_TEXT** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
#LICENSE_TEXT**                         University of Bielefeld                        **
#LICENSE_TEXT**                         nivision@techfak.uni-bielefeld.de              **
#LICENSE_TEXT**                                                                        **
#LICENSE_TEXT** This file is part of the __ICL_PACKAGE__ module of ICL __ICL_SPACE__   **
#LICENSE_TEXT**                                                                        **
#LICENSE_TEXT** Commercial License                                                     **
#LICENSE_TEXT** Commercial usage of ICL is possible and must be negotiated with us.    **
#LICENSE_TEXT** See our website www.iclcv.org for more details                         **
#LICENSE_TEXT**                                                                        **
#LICENSE_TEXT** GNU General Public License Usage                                       **
#LICENSE_TEXT** Alternatively, this file may be used under the terms of the GNU        **
#LICENSE_TEXT** General Public License version 3.0 as published by the Free Software   **
#LICENSE_TEXT** Foundation and appearing in the file LICENSE.GPL included in the       **
#LICENSE_TEXT** packaging of this file.  Please review the following information to    **
#LICENSE_TEXT** ensure the GNU General Public License version 3.0 requirements will be **
#LICENSE_TEXT** met: http://www.gnu.org/copyleft/gpl.html.                             **
#LICENSE_TEXT**                                                                        **
#LICENSE_TEXT***************************************************************************/ 
#LICENSE_TEXT

function usage () {
    echo "usage: " ;
    echo " scripts/add_license.sh <-add|-remove> [-doit]" ;
    echo ""
    echo "this script can be used to add/remove the ICL license text. The license text"
    echo "is embedded into this script, so you have to change the script itself if you"
    echo "want to adapt the license text. "
    exit -1;
}

for ARG in $@ ; do
    if [ "$ARG" = "-doit" ] ; then
        DOIT="TRUE" ;
    elif [ "$ARG" = "-add" ] ; then
        ADD="TRUE" ;
    elif [ "$ARG" = "-remove" ] ; then
        REMOVE="TRUE" ;
    else
        usage
    fi
done

if [ "$ADD" = "" ] ; then
    if [ "$REMOVE" = "" ] ; then
        usage
    fi
fi


cat $0 | grep -e '^#LICENSE_TEXT' | sed 's|#LICENSE_TEXT||g' > ./.license_text.txt
echo "adding this license text:"
cat ./.license_text.txt

ALL_NUM_FILES=0
for PACKAGE in ICLAlgorithms ICLBlob ICLCC ICLCore ICLFilter ICLGeom ICLIO ICLQt ICLQuick ICLUtils ICLOpenCV ; do
    CHARS=`echo $PACKAGE | wc -c`
    MISSING=`echo 29-$CHARS | bc -l`
    unset BEAUTY
    for (( i=0 ; $i < $MISSING ; i++ )) ; do
        BEAUTY="$BEAUTY " ;
    done
    cat $0 | grep -e '^#LICENSE_TEXT' | sed 's|#LICENSE_TEXT||g' | sed "s|__ICL_PACKAGE__|$PACKAGE|g" | sed "s|__ICL_SPACE__|$BEAUTY|g"> ./.license_text.txt
    echo "processing package $PACKAGE"
    HEADERS=`find include/$PACKAGE -iname *.h` ;
    SOURCES=`find $PACKAGE/src -iname *.cpp` ;
    EXAMPLES=`find $PACKAGE/examples -iname *.cpp` ;
    NUMFILES=`echo $HEADERS $SOURCES $EXAMPLES | wc -w`
    for FILE in $HEADERS $SOURCES $EXAMPLES ; do
        if [ "$ADD" == "TRUE" ] ; then
            if [ "$DOIT" = "TRUE" ] ;then
                echo "  adding license to $FILE"
                ENSURE=`cat $FILE | grep 'GNU General Public License Usage'`
                if [ "$ENSHURE" = "" ] ; then
                    cat ./.license_text.txt $FILE > ./.tmp_file.tmp ;
                    mv ./.tmp_file.tmp $FILE ;
                else
                    echo "  error token \"GNU General Public License Usage\" was already found in file (skipping)" ;
                fi
            else
                echo "  doing as if adding license to $FILE"
                echo "  cat ./.license_text.txt $FILE > ./.tmp_file.tmp " ;
                echo "  mv ./.tmp_file.tmp $FILE " ;
            fi
        else # REMOVE is TRUE
            if [ "$DOIT" = "TRUE" ] ; then
                echo "  removing license from $FILE"
                ENSURE=`cat $FILE | grep 'GNU General Public License Usage'`
                if [ "$ENSURE" = "" ] ; then
                    echo "  error missing token \"GNU General Public License Usage\" (skipping)" ;
                else
                    LINES=`cat ./.license_text.txt | wc -l` ;
                    LINES=`echo "$LINES+1" | bc -l` ;
                    tail -n +$LINES $FILE > ./.tmp_file.tmp ;
                    mv ./.tmp_file.tmp $FILE ;
                fi
            else
                echo "  doing as if removing license from $FILE"
                echo "  tail -n +$LINES $FILE > ./.tmp_file.tmp" ;
                echo "  mv ./.tmp_file.tmp $FILE " ;
            fi
        fi
    done
    ALL_NUM_FILES=`echo "$ALL_NUM_FILES+$NUMFILES" | bc -l`
done

echo processed $ALL_NUM_FILES files

rm -rf ./.license_text.txt ./.tmp_file.tmp

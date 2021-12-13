#!/bin/bash

# this script is used to add and to remove the ICL-license text from all files,
# IMPORTANT:
# In case of removing the license text in order to change it before re-adding
# the new text, always remove the text before changing it. The remove step is
# a little simple: is simply counts the lines of the current license text and
# removes that much preceding lines from each file. If you already changed the
# current license text, and the new version has more or less lines, you corrupt
# all files without being able to fix it afterwards with this script

#LICENSE_TEXT/********************************************************************
#LICENSE_TEXT**                Image Component Library (ICL)                    **
#LICENSE_TEXT**                                                                 **
#LICENSE_TEXT** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
#LICENSE_TEXT**                         University of Bielefeld                 **
#LICENSE_TEXT**                Contact: nivision@techfak.uni-bielefeld.de       **
#LICENSE_TEXT**                Website: www.iclcv.org                           **
#LICENSE_TEXT**                                                                 **
#LICENSE_TEXT** File   : __ICL_FILE__ __ICL_FILE_SPACE__ **
#LICENSE_TEXT** Module : __ICL_MODULE__ __ICL_MODULE_SPACE__ **
#LICENSE_TEXT** Authors: __ICL_AUTHORS_A__ __ICL_AUTHORS_A_SPACE__ **
#LICENSE_TEXT**          __ICL_AUTHORS_B__ __ICL_AUTHORS_B_SPACE__ **
#LICENSE_TEXT**                                                                 **
#LICENSE_TEXT** Commercial License                                              **
#LICENSE_TEXT** ICL can be used commercially, please refer to our website       **
#LICENSE_TEXT** www.iclcv.org for more details.                                 **
#LICENSE_TEXT**                                                                 **
#LICENSE_TEXT** GNU General Public License Usage                                **
#LICENSE_TEXT** Alternatively, this file may be used under the terms of the     **
#LICENSE_TEXT** GNU General Public License version 3.0 as published by the      **
#LICENSE_TEXT** Free Software Foundation and appearing in the file LICENSE.GPL  **
#LICENSE_TEXT** included in the packaging of this file.  Please review the      **
#LICENSE_TEXT** following information to ensure the GNU General Public License  **
#LICENSE_TEXT** version 3.0 requirements will be met:                           **
#LICENSE_TEXT** http://www.gnu.org/copyleft/gpl.html.                           **
#LICENSE_TEXT**                                                                 **
#LICENSE_TEXT*********************************************************************/
#LICENSE_TEXT

function usage () {
    echo "usage: " ;
    echo " scripts/add_license.sh <-add|-remove> [-doit] <contrib-file-name>" ;
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
    elif [ -e "$ARG" ] ; then
        echo "using contrib-file: $ARG"
        CONTRIB_FILE=$ARG
    else
        usage
    fi
done

if [ "$ADD" = "" ] ; then
    if [ "$REMOVE" = "" ] ; then
        usage
    fi
fi

if [ "$CONTRIB_FILE" = "" ] ; then
    usage
fi

cat $0 | grep -e '^#LICENSE_TEXT' | sed 's|#LICENSE_TEXT||g' > ./.license_text.txt
echo "adding this license text:"
cat ./.license_text.txt

ALL_NUM_FILES=0
#for MODULE in ICLUtils ; do
for MODULE in ICLAlgorithms ICLBlob ICLCC ICLCore ICLFilter ICLGeom ICLIO ICLQt ICLQuick ICLUtils ICLOpenCV ; do
    MODULE_CHAR_COUNT=`echo $MODULE | wc -c`
    MODULE_SPACE_NUM=`echo 54-$MODULE_CHAR_COUNT | bc -l`
    unset MODULE_SPACE
    for (( i=0 ; $i < $MODULE_SPACE_NUM ; i++ )) ; do
        MODULE_SPACE="$MODULE_SPACE " ;
    done

    echo "processing package $MODULE"
    HEADERS=`find include/$MODULE -iname *.h` ;
    SOURCES=`find $MODULE/src -iname *.cpp` ;
    EXAMPLES=`find $MODULE/examples -iname *.cpp` ;
    NUMFILES=`echo $HEADERS $SOURCES $EXAMPLES | wc -w`
    for FILE in $HEADERS $SOURCES $EXAMPLES ; do
        if [ "$ADD" == "TRUE" ] ; then
            FILE_CHAR_COUNT=`echo $FILE | wc -c`
            FILE_SPACE_NUM=`echo 54-$FILE_CHAR_COUNT | bc -l`
            unset FILE_SPACE
            for (( i=0 ; $i < $FILE_SPACE_NUM ; i++ )) ; do
                FILE_SPACE="$FILE_SPACE " ;
            done
            CE=`cat $CONTRIB_FILE | grep $FILE | grep celbrech`
            MG=`cat $CONTRIB_FILE | grep $FILE | grep mgoettin`
            RH=`cat $CONTRIB_FILE | grep $FILE | grep rhaschke`
            AJ=`cat $CONTRIB_FILE | grep $FILE | grep ajustus`
            FR=`cat $CONTRIB_FILE | grep $FILE | grep freinhar`

            AUTHOR_COUNT=0
            unset AUTHORS
            unset AUTHORS_A
            unset AUTHORS_B

            if [ "$CE" != "" ] ; then
                if [ "$AUTHORS" = "" ] ; then
                    AUTHORS="Christof Elbrechter"
                else
                    AUTHORS="$AUTHORS, Christof Elbrechter"
                fi
                AUTHOR_COUNT=`echo $AUTHOR_COUNT+1 | bc -l`
            fi

            if [ "$MG" != "" ] ; then
                if [ "$AUTHORS" = "" ] ; then
                    AUTHORS="Michael Götting"
                else
                    AUTHORS="$AUTHORS, Michael Götting"
                fi
                AUTHOR_COUNT=`echo $AUTHOR_COUNT+1 | bc -l`
            fi

            if [ "$RH" != "" ] ; then
                if [ "$AUTHORS" = "" ] ; then
                    AUTHORS="Robert Haschke"
                else
                    AUTHORS="$AUTHORS, Robert Haschke"
                fi
                AUTHOR_COUNT=`echo $AUTHOR_COUNT+1 | bc -l`
            fi

            if [ "$AJ" != "" ] ; then
                if [ "$AUTHORS" = "" ] ; then
                    AUTHORS="Andre Justus"
                else
                    AUTHORS="$AUTHORS, Andre Justus"
                fi
                AUTHOR_COUNT=`echo $AUTHOR_COUNT+1 | bc -l`
            fi

            if [ "$FR" != "" ] ; then
                if [ "$AUTHORS" = "" ] ; then
                    AUTHORS="Felix Reinhard"
                else
                    AUTHORS="$AUTHORS, Felix Reinhard"
                fi
                AUTHOR_COUNT=`echo $AUTHOR_COUNT+1 | bc -l`
            fi

            if [ "$AUTHOR_COUNT" > "6" ] ; then
                AUTHORS_B=`echo $AUTHORS | cut -d ' ' -f '7-'`
                AUTHORS_A=`echo $AUTHORS | cut -d ' ' -f '1-6'`
            else
                AUTHORS_A=$AUTHORS
            fi

            LINE_LENGTH_A=54
            if [ "$AUTHORS_B" = "" ] ; then
                AUTHORS_A=`echo $AUTHORS_A | sed 's|,$||g'`
                if [ "$AUTHOR_COUNT" -lt 3 ] ; then
                    #echo 'B is empty: #authors < 3: ll=54'
                    LINE_LENGTH_A=54
                else
                    #echo 'B is empty: #authors >= 3: ll=55'
                    LINE_LENGTH_A=55
                fi
            else
                AUTHORS_B=`echo $AUTHORS_B | sed 's|,$||g'`
            fi

            AUTHORS_A_CHAR_COUNT=`echo $AUTHORS_A | wc -c`
            AUTHORS_A_SPACE_NUM=`echo $LINE_LENGTH_A-$AUTHORS_A_CHAR_COUNT | bc -l`
            unset AUTHORS_A_SPACE
            for (( i=0 ; $i < $AUTHORS_A_SPACE_NUM ; i++ )) ; do
                AUTHORS_A_SPACE="$AUTHORS_A_SPACE " ;
            done

            AUTHORS_B_CHAR_COUNT=`echo $AUTHORS_B | wc -c`
            AUTHORS_B_SPACE_NUM=`echo 54-$AUTHORS_B_CHAR_COUNT | bc -l`
            unset AUTHORS_B_SPACE
            for (( i=0 ; $i < $AUTHORS_B_SPACE_NUM ; i++ )) ; do
                AUTHORS_B_SPACE="$AUTHORS_B_SPACE " ;
            done



            cat $0 | grep -e '^#LICENSE_TEXT' | \
                sed 's|#LICENSE_TEXT||g' | \
                sed "s|__ICL_FILE__|$FILE|g" | \
                sed "s|__ICL_FILE_SPACE__|$FILE_SPACE|g" | \
                sed "s|__ICL_MODULE__|$MODULE|g" | \
                sed "s|__ICL_MODULE_SPACE__|$MODULE_SPACE|g" | \
                sed "s|__ICL_AUTHORS_A__|$AUTHORS_A|g" | \
                sed "s|__ICL_AUTHORS_A_SPACE__|$AUTHORS_A_SPACE|g" | \
                sed "s|__ICL_AUTHORS_B__|$AUTHORS_B|g" | \
                sed "s|__ICL_AUTHORS_B_SPACE__|$AUTHORS_B_SPACE|g" \
                > ./.license_text.txt

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
            if [ "$LINES" = "" ] ; then
                echo "please define number of lines to remove from all files"
                echo "(default is number of the lines of the current header)"
                echo -n "lines: ";
                read LINES

                if [ "$LINES" = "" ] ; then
                    LINES=`cat ./.license_text.txt | wc -l` ;
                    echo "using default: $LINES lines"
                    # we need +1 for the tail command
                    LINES=`echo "$LINES+1" | bc -l` ;
                else
                    echo "using input: $LINES lines"
                    LINES=`echo "$LINES+1" | bc -l` ;
                fi
            fi


            if [ "$DOIT" = "TRUE" ] ; then
                echo "  removing license from $FILE"
                ENSURE=`cat $FILE | grep 'GNU General Public License Usage'`
                if [ "$ENSURE" = "" ] ; then
                    echo "  error missing token \"GNU General Public License Usage\" (skipping)" ;
                else
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

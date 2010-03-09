#!/bin/bash

echo $0

A=`find . -iname *.h`
A="${A} `find . -iname *.cpp`"

if [ "$ICL_PACKAGE" = "" ] ; then
    echo "variable ICL_PACKAGE is empty (aborting)" ;
    exit -1 ;
fi



#LICENSE_TEXT/********************************************************************************* 
#LICENSE_TEXT** Copyright (C) 2006-2010 neuroinformatics (vision) group
#LICENSE_TEXT** University of Bielefeld
#LICENSE_TEXT** Contact: nivision@techfak.uni-bielefeld.de
#LICENSE_TEXT**
#LICENSE_TEXT** This file is part of the $ICL_PACKAGE module of ICL
#LICENSE_TEXT**
#LICENSE_TEXT** Commercial License
#LICENSE_TEXT** Commercial usage of ICL is possible and must be negotiated with us.
#LICENSE_TEXT** See our website www.iclcv.org for more details
#LICENSE_TEXT**
#LICENSE_TEXT** GNU General Public License Usage
#LICENSE_TEXT** Alternatively, this file may be used under the terms of the GNU
#LICENSE_TEXT** General Public License version 3.0 as published by the Free Software
#LICENSE_TEXT** Foundation and appearing in the file LICENSE.GPL included in the
#LICENSE_TEXT** packaging of this file.  Please review the following information to
#LICENSE_TEXT** ensure the GNU General Public License version 3.0 requirements will be
#LICENSE_TEXT** met: http://www.gnu.org/copyleft/gpl.html.
#LICENSE_TEXT**
#LICENSE_TEXT***********************************************************************************/ 


cat $0 | grep -e '^#LICENSE_TEXT' | sed 's|#LICENSE_TEXT||g' > ./.license_text.txt
cat ./.license_text.txt
rm -rf ./.license_text.txt


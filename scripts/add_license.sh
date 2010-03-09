#!/bin/bash

A=`find . -iname *.h`
A="${A} `find . -iname *.cpp`"

echo "$A"

if [ "$ICL_PACKAGE" = "" ] ; then
    echo "variable ICL_PACKAGE is empty (aborting)" ;
    exit -1 ;
fi

LICENSE_PLACE_HOLDER='/******** ICL_LICENSE_START_HERE *******\n *********** ICL_LICENSE_END_HERE *************/'

LICENSE_TEXT= '** \n' \
'** Copyright (C) 2006-2010 Neuroinformatics/vision-group University of Bielefeld\n' \
'** Contact: nivision@techfak.uni-bielefeld.de\n' \
'** \n' \
'** This file is part of the $ICL_PACKAGE module of ICL\n' \
'** \n' \
'** Commercial Usage\n' \
'** Licensees holding valid ICL Commercial licenses may use this file in\n' \
'** accordance with the ICL Commercial License Agreement available at\n' \
'** www.iclcv.org\n' \
'** \n' \
'** GNU General Public License Usage\n' \
'** Alternatively, this file may be used under the terms of the GNU\n' \
'** General Public License version 3.0 as published by the Free Software\n' \
'** Foundation and appearing in the file LICENSE.GPL included in the\n' \
'** packaging of this file.  Please review the following information to\n' \
'** ensure the GNU General Public License version 3.0 requirements will be\n' \
'** met: http://www.gnu.org/copyleft/gpl.html.\n' \
'** \n' \

echo $LICENSE_PLACE_HOLDER 


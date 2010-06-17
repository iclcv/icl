#!/bin/bash


TEXT=\
'**                                                                 **\
** The development of this software was supported by the           **\
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **\
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **\
** Forschungsgemeinschaft (DFG) in the context of the German       **\
** Excellence Initiative.                                          **'

for FILE in `find . -name *.cpp -name *.h` ; do 
#for FILE in test.h ; do
    echo "processing file $FILE" ;
    sed -i "s/Neuroinformatics, CITEC                 \*\*/CITEC, University of Bielefeld          **/g" $FILE ;
    sed -i "s/University of Bielefeld                 \*\*/Neuroinformatics Group                  **/g" $FILE ;
    sed -i "s/\*\*                Contact: nivision@techfak.uni-bielefeld.de       \*\*/** Website: www.iclcv.org and                                      **/g" $FILE ;
    sed -i "s/\*\*                Website: www.iclcv.org                           \*\*/**          http:\/\/opensource.cit-ec.de\/projects\/icl               **/g" $FILE ;
    sed -i "s/\*\* http:\/\/www.gnu.org\/copyleft\/gpl.html.                           \*\*/** http:\/\/www.gnu.org\/copyleft\/gpl.html.                           **\n$TEXT/g" $FILE ;
#    sed -i "s///g" $FILE ;
done 
    




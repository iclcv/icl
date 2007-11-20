#!/usr/local/bin/bash

echo "replacing shell variables ..."
ICL_OPTIMIZATION_SAVE=$ICL_OPTIMIZATION
export ICL_OPTIMIZATION=""

ICL_INSTALL_ROOT_SAVE=$ICL_INSTALL_ROOT
export ICL_INSTALL_ROOT=/vol/vision/


cd $HOME/projects/ICL/
#svn up
if [ "$1" = "noclean" ] ; then
    echo "re-building ICL... (targets all, install, installdoc)"
    make all install installdoc
else
    echo "re-building ICL... (targets clean, depend, all install doc and isntalldoc"
    make clean depend all install doc installdoc 
fi

echo "restoring shell variables..."
export ICL_OPTIMIZATION=$ICL_OPTIMIZATION_SAVE
export ICL_INSTALL_ROOT=$ICL_INSTALL_ROOT_SAVE

echo "setting up rights..."
chmod -R o+rX /vol/vision/ICL/default/*  
chmod -R o+x /vol/vision/ICL/default/bin/*  
chmod 664 /vol/vision/ICL/default/bin/*.cpp
chmod 664 /vol/vision/ICL/default/include/*.h

echo "done..."

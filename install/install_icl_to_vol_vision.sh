#!/usr/local/bin/bash

echo "replacing shell variables ..."
ICL_OPTIMIZATION_SAVE=$ICL_OPTIMIZATION
export ICL_OPTIMIZATION=-march=prescott

ICL_INSTALL_ROOT_SAVE=$ICL_INSTALL_ROOT
export ICL_INSTALL_ROOT=/vol/vision/

echo "re-building ICL..."
cd $HOME/projects/ICL/
#svn up
make clean depend all install doc installdoc
#make install installdoc

echo "restoring shell variables..."
export ICL_OPTIMIZATION=$ICL_OPTIMIZATION_SAVE
export ICL_INSTALL_ROOT=$ICL_INSTALL_ROOT_SAVE

echo "setting up rights..."
chmod -R o+rX /vol/vision/ICL/default/*  
chmod -R o+x /vol/vision/ICL/default/bin/*  
chmod 664 /vol/vision/ICL/default/bin/*.cpp
chmod 664 /vol/vision/ICL/default/include/*.h

echo "done..."

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/dcclearisochannels.cpp                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/DCGrabber.h>
#include <ICLIO/DC.h>
#include <map>
#include <vector>

using namespace std;
using namespace icl;
typedef std::map<int,vector<dc1394camera_t*> > cammap;

int main(int n, char **ppc){
  cammap m;
  
  std::vector<DCDevice> devs = DCGrabber::getDCDeviceList();
  printf("found %d cameras \n",(unsigned int)devs.size());
  
  
  for(unsigned int i=0;i<devs.size();i++){
    m[devs[i].getUnit()].push_back(devs[i].getCam());
    dc1394_camera_reset(devs[i].getCam());
    printf("calling reset for camera %d [%s] \n",i,devs[i].getModelID().c_str());
  }
  printf("\n");
  for(cammap::iterator it = m.begin();it != m.end();++it){
    dc1394_iso_release_all((it->second)[0]);
    //    dc1394_cleanup_iso_channels_and_bandwidth((it->second)[0]);
    printf("cleaning iso channel for port %d \n",it->first);
  }
  printf("\n");
  for(unsigned int i=0;i<devs.size();i++){
    dc1394_camera_free(devs[i].getCam());
    printf("releasing camera %d [%s] \n",i,devs[i].getModelID().c_str());
  }
  printf("\n");
  printf("iso channels cleared successfully !\n");
  
  
  return 0;
}


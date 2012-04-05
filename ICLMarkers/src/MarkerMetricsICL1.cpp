/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/MarkerMetricsICL1.cpp                   **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/MarkerMetricsICL1.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  MarkerMetricsICL1::MarkerMetricsICL1(const MarkerCodeICL1 &c, const Size32f &markerSizeMM):
    MarkerCodeICL1(c),root(markerSizeMM){

    const float u = root.width/13.0;
    const float v = root.height/17.0;
    
    for(int i=0;i<4;++i){
      CR &cr = crs[i];
      cr.x = u;
      cr.y = v * (i*4+1);
      cr.width = 11*u;
      cr.height = 3*v;
      cr.ccrs.resize(c[i]);

      float ccry = (i*4+2)*v;
      for(int j=0;j<c[i];++j){
        Rect32f &ccr = cr.ccrs[j];

        switch(c[i]){
          case 1: ccr.x = 6*u; break;
          case 2: ccr.x = (2+j*8)*u; break;
          case 3: ccr.x = (2+j*4)*u; break;
          case 4: ccr.x = (j<2?(2+j*2):(4+j*2))*u; break;
          case 5: ccr.x = (2+j*2)*u; break;
          default:
            throw ICLException(":MarkerMetricsICL1: invalid code found! " + str(c));
        }
        ccr.y = ccry;
        ccr.width = u;
        ccr.height = v;
      }
    }
  }
}

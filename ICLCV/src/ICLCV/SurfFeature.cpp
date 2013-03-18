/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/SurfFeature.cpp                        **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCV/SurfFeature.h>

#include <ICLUtils/Macros.h>

namespace icl{
  namespace cv{
    
    float SurfFeature::operator-(const SurfFeature &other) const{
      float sum=0.f;
      for(int i=0; i < 64; ++i){
        sum += utils::sqr(descriptor[i]-other.descriptor[i]);
      }
      return sum;
    }
    
    utils::VisualizationDescription SurfFeature::vis(int dx, int dy) const {
      utils::VisualizationDescription d;
      
      float s = 2 * scale;
      
      int cx = x + dx;    
      int cy = y + dy;
      
      d.color(0,255,0,255);
      d.linewidth(2);
      int cx2 = cx + cos(orientation) * s;
      int cy2 = cy + sin(orientation) * s;
      d.line(cx,cy,cx2,cy2);

      d.linewidth(1);
      if(laplacian == 1) d.color(0,100,255,255);
      else if(laplacian == 0) d.color(255,0,0,255);
      else d.color(0,255,0,255); //  ??
      
      d.fill(255,0,0,30);
      d.circle(cx,cy,s);
      
      return d;
    }
  }
}

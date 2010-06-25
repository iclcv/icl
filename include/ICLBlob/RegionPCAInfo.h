/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RegionPCAInfo.h                        **
** Module : ICLBlob                                                **
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

#ifndef ICL_REGION_PCA_INFO_H
#define ICL_REGION_PCA_INFO_H

#include <cmath>

namespace icl{
  /// data-struct to represent local PCA information \ingroup G_RD
  class RegionPCAInfo{
    public:
    
    /// Default Constructor
    RegionPCAInfo(float len1=0, float len2=0, float arc1=0, int cx=0, int cy=0):
    len1(len1),len2(len2),arc1(arc1),arc2(arc1+M_PI/2),cx(cx),cy(cy){}
    
    /// length of first major axis
    float len1; 
    /// length of second major axis
    float len2; 
    /// angle of the first major axis
    float arc1; 
    /// angle of the second major axis (arc1+PI/2)
    float arc2; 
    /// x center of the region
    int cx;
    /// y center of the region
    int cy;
    
    /// null PCAInfo
    static const RegionPCAInfo null;
  };
}

#endif

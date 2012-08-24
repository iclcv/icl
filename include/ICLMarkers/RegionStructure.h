/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/RegionStructure.h                   **
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

#pragma once

#include <ICLUtils/SmartPtr.h>
#include <string>

namespace icl{
  /** \cond */
  struct ImageRegion;
  /** \encond */
  
  /// region structure interface class
  /** A region structure can be defined arbitrarily, It defines
      how a single image region is matched agains a given structure
      instance */
  struct RegionStructure{
    /// answers the question whether a given region matches a region structure
    /** Usually, this method is called for every region in an image. Therefore,
        a particular match-implementation should try to reject a match as fast 
        as possible. E.g. by first checking whether the root region has a 
        correct color value */
    virtual bool match(const ImageRegion &r) const = 0;
  };
  
  /// Managed pointer type definition
  typedef SmartPtr<RegionStructure> RegionStructurePtr;
  
  
  
}


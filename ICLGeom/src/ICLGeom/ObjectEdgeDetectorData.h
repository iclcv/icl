/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorData.h           **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#pragma once

#include <ICLMath/DynMatrix.h>

namespace icl{
  namespace geom{
    
    /**
     This class is used in the ObjectEdgeDetector implementations */
    class ICLGeom_API ObjectEdgeDetectorData{
     
           
     public:
      typedef struct{
        int medianFilterSize;
        int normalRange;
        int normalAveragingRange;
        int neighborhoodMode;
        int neighborhoodRange;
        float binarizationThreshold;
        bool useNormalAveraging;
        bool useGaussSmoothing;
      }m_params;
     
      typedef struct{
        float norm;
	    math::DynMatrix<float> kernel;
	    int l;
	    int kSize;
	    int rowSize;
      }m_kernel;
     
     
      /// Create new ObjectEdgeDetectorData
      ObjectEdgeDetectorData(); 
  	
      ///Destructor
      virtual ~ObjectEdgeDetectorData();
      
      /// Returns Kernel of a given size
      /** @param size the kernel size
          @return the kernel */
      m_kernel getKernel(int size);
	
      /// Returns the initial parameters.
      /** @return the inital parameters */
      m_params getParameters();	  
    };
  }
}

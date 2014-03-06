/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/SOM2D.h                            **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/SOM.h>

namespace icl{
  namespace math{
  
    /// Simple Wrapper class of the generic SOM Implementation for 2D SOMs
    /** As the SOM class provides a <em>very</em> abstract interface for 
        generic K-D Self Organizing Maps, the SOM2D class offers a more convenient
        interface for exactly 2D SOMs.\n
        Therefore it has a specialized constructor (now receiving the count
        of x-Cells and y-Cells directly as well as a specialized getNeuron()-
        function.
    */
    class ICLMath_API SOM2D : public SOM{
  
      /** \cond this function is just used here for an inline vector creation*/
      template<class T>
      static inline std::vector<T> vec2(const T &t1, const T &t2){ 
        std::vector<T> v(2);
        v[0] = t1;
        v[1] = t2;
        return v;
      }
      /** \endcond */
      
      public:
      /// Wrapper constructor for 2D SOMs
      /** @param dataDim dimension of data elements and therewith dimension of prototype vectors
          @param nXCells cell count of the SOM in x-direction
          @param nYCells cell count of the SOM in y-direction
          @param prototypeBounds @see SOM
          @param epsilon @see SOM
          @param sigma @see SOM
      */
      SOM2D(unsigned int dataDim, 
            unsigned int nXCells, 
            unsigned int nYCells, 
            const std::vector<utils::Range<float> >& prototypeBounds, 
            float epsilon=0.1, 
            float sigma=1):SOM(dataDim,vec2(nXCells,nYCells),prototypeBounds,epsilon,sigma){}
  
      /// Wrapper function to access a neuron at a certain grid position
      /** @param x x-grid locatio of the neuron
          @param y y-grid locatio of the neuron
      **/
      const Neuron &getNeuron(int x, int y) const { return m_vecNeurons[x+m_vecDimOffsets[1]*y]; }
  
    };
  } // namespace math
}


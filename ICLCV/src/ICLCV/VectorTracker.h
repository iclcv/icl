/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/VectorTracker.h                        **
** Module : ICLCV                                                  **
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
#include <ICLUtils/Function.h>
#include <vector>

namespace icl{
  namespace cv{
  
    /// Extension of the position tracker class for N-dimensional positions
    /** Here's a copy of the PositionTracker documentation, which assumes 2D-input data:
        \copydoc icl::cv::PositionTracker
    */
    class ICLCV_API VectorTracker{
      public:
      /// Determines how ids are allocated internally
      enum IDmode{
        firstFree, // old unused IDs are re-used
        brandNew   // each new element gets a brand new ID (untils int-range ends)
      };
      
      /// Vector Type
      typedef std::vector<float> Vec;

      typedef utils::Function<float,const Vec&,const Vec&> DistanceFunction;
  
      /// Creates an empty (null) vector tracker (isNull() returns true then)
      VectorTracker();
      
      /// Creates new VectorTracker instance with given parameters
      /** @param dim data dimension (this must not changed at runtime)
          @param largeDistance to tackle element count changes, the distance matrix is padded with
                 largeDistnace values, this must be much (e.g. 100 times ) larger then the largest real distance, that
                 can be expected from the data. We can't use some fixed value here, as too large values lead to
                 numerical problems
          @param normFactors Internally the euclidian distance metric can be normalized in differenct dimensions seperately:
                            \f[ d(a,b) = \sqrt{ \sum\limits_{i=1}^D \left( \frac{a_i - b_i}{\sigma_i}\right)^2 } \f]
                            In literature this is norm is reference as normalized euclidian distance. Actually we use an
                            adapted instance of this norm:
                            \f[ d(a,b) = \sqrt[4]{ \sum\limits_{i=1}^D \left( \frac{a_i - b_i}{\sigma_i}\right)^2 } \f]
                            As mentioned in the documentation of the PositionTracker, it's compulsory to use the 
                            root of the actual norm to avoid new entries are mixed up with old ones.
                            The norm factor vector contains the \f$\sigma_i\f$ that are used in the formula above. If norm-factor
                            is empty or all entries are set to 1, the default euclidian norm (more precisely the square 
                            root of it) is used, which increases performance slightly. If normFactor contains zeros,
                            div-0 errors would occur, so this is checked during initialization.
          @param idMode This feature is taken from the recent PositionTracker update. It decides whether to re-use
                        old IDs, that got free again due to the disappearing of the associated entry or to assign a
                        brand new ID each time a new entry is found
          @param distanceThreshold As a first optimization a trivial assignment is tested. If entry count hasn't changed
                                   and each old entry can be assigned indisputably to a single new entry and each distance
                                   between estimation and current observation is beyond that threshold, the trivial
                                   assignment is used.
          @param tryOpt enables/disables whether to test for trivial assignment. If a trivial assignment can be expected, this
                        will increase performance significantly. If it's more likely, that trivial assignment will fail, this 
                        also reduce performance a little bit.
          @param df can be set to a custom function that is used to compute the distance value between two values
                    by default a pearson distance is used, for the normalization factors, normFactors is used
      */
      VectorTracker(int dim, float largeDistance, const std::vector<float> &normFactors=std::vector<float>(),
                    IDmode idMode=firstFree, float distanceThreshold=0, bool tryOpt=true,
                    DistanceFunction df=DistanceFunction(), bool dfIsQualityFunction=false); 
  
      /// Deep copy constructor (all data and current state is copied deeply)
      /** New instance is absolutely independent from the source instance */
      VectorTracker(const VectorTracker &other);
      
      /// assignment (all data and current state is copied deeply)
      /** New instance is absolutely independent from the source instance */
      VectorTracker &operator=(const VectorTracker &other);
      
      /// Destructor
      ~VectorTracker();
      
      /// next step function most efficient version
      void pushData(const std::vector<Vec> &newData);
  
      /// returns runtime id of last pushed data element index
      int getID(int index, float *lastErrorOrScore=0) const;
      
      /// returns whether VectorTracker instance is currently null (created with default constructor)
      bool isNull() const;
  
      /// return current data dimension
      int getDim() const;
      
      /// internally sets the extrapolation mask
      /** By default, the mask contains only true-entries. All dimensions of 
          current date, that have a true-entry in the mask are extrapolated
          using a linear model before the internal cost-matrix is created (and if
          this entry has an age of at least 2 -- so two former entries are available).
          Dimensions that have a false-entry in the given mask are not-extrapolated
          over time (which is identical to using a constant extrapolation model) */
      void setExtrapolationMask(const std::vector<bool> &mask);

      /// sets a custom distance function
      void setDistanceFunction(DistanceFunction df);
      
      /// returns current extrapolation mask
      const std::vector<bool> &getExtrapolationMask() const;
      
      private:
      
      /// internal data structure (declared and used in iclVectorTracker.cpp only)
      struct Data;
      
      /// internal data pointer
      Data *m_data;
    };
    
    
  } // namespace cv
}

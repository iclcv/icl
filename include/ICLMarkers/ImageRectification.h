/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/ImageRectification.h                **
** Module : ICLCV                                                **
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

#include <ICLCore/Img.h>
#include <ICLMath/FixedMatrix.h>

namespace icl{
  namespace markers{
  
    /// The ImageRectification class can be use to rectify an image quadrangle (IPP accellerated)
    /** The ImageRectification can be set up for automatic sorting of the given points (which is default).
        If this behaviour is disabled, the given points are not checked for their correct order. */
    template<class T>
    class ImageRectification{
      Img<T> buffer; //!< internal image buffer
      bool validateAndSortPoints; //!< internal flag
      
      public:
      
      /// create an ImageRectification instance with given value for validateAndSortPoints
      ImageRectification(bool validateAndSortPoints=true):validateAndSortPoints(validateAndSortPoints){}
      
      /** \cond  this method is not implemented yet!*/
      const Img<T> &apply(const FixedMatrix<float,3,3> &transform, const Img<T> &src, const Size &resultSize);
      /** \endcond */
      
      /// applies the image rectification from given source image quadrangle into a rectangular image
      /** @param ps source image points (must define a linestrip around the
                    source image quadrangular regions that needs to be rectified)
          @param src source image
          @param resultSize resulting rectified image size
          @param hom optionally, you can pass a matrix here, that is filled with the
                     homography matrix that is used to transform the pixels
          @param Q this can optionally be filled with the Q part of the QR-decomposition of hom
                 The Q part (which is an othogonal matrix) usually contains the homographies rotation
          @param R this can optionally be filled with the R part of the QR-decomposition of hom
                 The R part (which is an upper right triangluar matrix) usually contains the homographies
                 scale and shear information)
          @param maxTilt can optionally be given. If it is greater than zero, it is used as
                         maximum ratio between R's diagonal elements. 
          @param advanedAlgorithm selects the algorithm that is used for the creation of
                                  the homography internally (@see Homography2D)
          @param resultROI if this parameter is given, the final homography is only evaluated within
                           the resulting images ROI
          */
      const Img<T> &apply(const Point32f ps[4], const Img<T> &src,
                          const Size &resultSize, FixedMatrix<float,3,3> *hom=0,
                          FixedMatrix<float,2,2> *Q=0, FixedMatrix<float,2,2> *R=0,
                          float maxTilt=0, bool advanedAlgorithm=true, 
                          const Rect *resultROI=0);
  
  
      //    TODO: use result roi in order to speed up
    };
  
  } // namespace markers
}


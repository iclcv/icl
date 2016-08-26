/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/Projective4PointTransform.h        **
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

#include <ICLMath/HomogeneousMath.h>
#include <ICLUtils/Point32f.h>
#include <vector>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>

namespace icl{
  namespace math{

    /// Utility class that provides functions to perspectively map 4 points of a source frame into 4 points of a destination frame
    /** In contrast to a standard bilinear interpolation, a 4-point projective transform also implements perspective
        vanishing point effencts. The algorithm was explained well in 
        
        http://math.stackexchange.com/questions/296794/finding-the-transform-matrix-from-4-projected-points-with-javascript

    */
	  class ICLMath_API Projective4PointTransform{
      Mat3 m_Ainv; //!< internally held inverse source transform
      Mat3 m_B;    //!< internally held destination transform
      Mat3 m_C;    //!< internally held result-transform C = B * A_inv
      utils::Point32f m_srcQuad[4], m_dstQuad[4];   //!< source and destination quadrangles

      
      public:

      /// creates an empty ID-transform
      Projective4PointTransform();

      /// creates a Projective4PointTransform instance with given src and dst quadrangle
      Projective4PointTransform(const utils::Point32f srcQuad[4], const utils::Point32f dstQuad[4]);

      /// creates a Projective4PointTransform instance with given src and dst quadrangle (vector version)
      Projective4PointTransform(const std::vector<utils::Point32f> &srcQuad,
                                const std::vector<utils::Point32f> dstQuad);

      /// creates a Projective4PointTransform instance directly from a given 3by3 projective transform
      /** In this case the intenally held source and destination quadrangles as well as the matrices Ainv and B
          will not be initialized */
      Projective4PointTransform(const Mat3 &C);

      /// intializes the Projective4PointTransform from given source and destination quadrangle
      /** if either sourceQuad or destinationQuad is null, the internally held-quadrangles will be used
          instead. This allows this function to be used for updating the source or the destination quadrangle
          separately, if needed. */
      void init(const utils::Point32f srcQuad[4], const utils::Point32f dstQuad[4]);

      /// intializes the Projective4PointTransform from given source and destination rectangles
      /** For simplicity, also rectangular source and destination quadrangles can be used
          The default/null arguments Rect32f::null (for either srcRect or dstRect) are internally 
          replaced with the current rectangles. By these means also srcRect or dstRect can also 
          be updated separetely */
      void init(const utils::Rect32f &srcRect, const utils::Rect32f &dstRect=utils::Rect32f::null);

      /// intializes the Projective4PointTransform from given source and destination rectangles (int-version)
      /** @see Projective4PointTransform::init(const utils::Rect32f &, const utils::Rect32f &) */
      void init(const utils::Rect &srcRect, const utils::Rect &dstRect=utils::Rect::null);

      /// returns the internally stored inverse source transform matrix
      const Mat3 &getAInv() const { return m_Ainv; }

      /// returns the internally stored inverse destination transform matrix
      const Mat3 &getB() const { return m_B; }

      /// returns the internally stored combined transform matrix
      /** The actual point mapping of a point p = (x,y) is performed by
          homogenize( C * (x,y,1) ), where homogenize(x,y,k) = (x/k,y/k) */
      const Mat3 &getC() const { return m_C; }

      /// returns a copy of the internally held source quadrangle
      std::vector<utils::Point32f> getSrcQuad() const;

      /// returns a copy of the internally held destiation quadrangle
      std::vector<utils::Point32f> getDstQuad() const;

      /// maps a single point from the source quadrangle into the destination quadrangle
      inline utils::Point32f mapPoint(const utils::Point32f &p) const{
        Vec3 m = m_C * Vec3(p.x,p.y,1);
        const float norm = m[2] ? 1/m[2] : 1;
        return utils::Point32f(m[0]*norm,m[1]*norm);
      }

      /// updates the source quad (the other transforms are automatically updated)
      void setSrcQuad(const utils::Point32f srcQuad[4]);

      /// updates the destination quad (the other transforms are automatically updated)
      void setDstQuad(const utils::Point32f dstQuad[4]);

      /// updates the source quad (the other transforms are automatically updated, point-version)
      void setSrcQuad(const utils::Point32f &a, const utils::Point32f &b,
                      const utils::Point32f &c, const utils::Point32f &d);

      /// updates the destination quad (the other transforms are automatically updated, point-version)
      void setDstQuad(const utils::Point32f &a, const utils::Point32f &b,
                      const utils::Point32f &c, const utils::Point32f &d);


      /// maps a whole set of points at once
      /** Please note that this is <b>not</b> more efficient then mapping all manually in a loop */
      std::vector<utils::Point32f> map(const std::vector<utils::Point32f> &src);

      /// maps points in an iterator based fashion
      template<class SrcForwardIterator, class DstForwardIterator>
      inline void map(SrcForwardIterator begin, SrcForwardIterator end, DstForwardIterator dst){
        while(begin != end){
          *dst++ = mapPoint(*begin++);
        }
      }
      
    };
  }
}

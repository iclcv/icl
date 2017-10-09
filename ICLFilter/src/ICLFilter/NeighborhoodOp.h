/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/NeighborhoodOp.h               **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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
#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{
    /// unary operators that work on each pixels neighborhood \ingroup UNARY \ingroup NBH
    /** TODO:: check!!
        The NeighborhoodOp class builds a base class for unary operations employing
        a filter mask which is moved over the ROI of the source image(s),
        e.g. convolution filters use some convolution masks.
        To this end the class provides members to store the size and anchor
        of the filter mask.

        Special care has to be taken, when applying a filter mask to the border
        of an image, e.g. in case of full-image ROI. In this case the filter
        might access undefined pixel values outside the image, actually causing
        a segfault in most cases.

        Hence, the used ROI size of the source image is <em> shrinked </em> if
        neccessary, such that the filter mask always fits into the image, when
        moved over the ROI. For this purpose the method adaptROI is provided,
        which computes the to-be-used ROI. The ROI of the source image is not
        actually changed, which makes the filter operation thread safe, because
        simultaneous operations running on the source image within different
        threads do not interfere.
    */
    class ICLFilter_API NeighborhoodOp : public UnaryOp {
      public:

      ///Destructor
      virtual ~NeighborhoodOp(){}

      /// compute neccessary ROI offset and size
      /** This functions computes the to-be-used ROI for the source image,
          such that the filter mask of given size (oMaskSize) fits everywhere
          into the image if placed arbitrarily within the ROI.
          The original ROI of the source image is not changed, instead the
          adapted ROI is returned in parameters oROIsize and oROIoffset.
          @param poSrc  image whose ROI is adapted
          @param oROIoffset  new ROI offset
          @param oROIsize    new ROI size
          @return whether a valid ROI remains
          */
      bool computeROI(const core::ImgBase *poSrc, utils::Point& oROIoffset, utils::Size& oROIsize);

      /// *NEW* apply function for multithreaded filtering (reimplemented here for special roi handling!)
      virtual void applyMT(const core::ImgBase *operand1, core::ImgBase **dst, unsigned int nThreads);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      protected:
      NeighborhoodOp() : m_oMaskSize(1,1), m_oAnchor (0,0) {}
      NeighborhoodOp(const utils::Size &size) {
        setMask (size);
      }

      void setMask(const utils::Size &size) {
          m_oMaskSize = adaptSize(size);
          m_oAnchor   = utils::Point (m_oMaskSize.width/2, m_oMaskSize.height/2);
      }
      void setMask(const utils::Size &size, const utils::Point &anchor){
        m_oMaskSize = adaptSize(size);
        m_oAnchor = anchor;
      }
      void setROIOffset(const utils::Point &offs){
        m_oROIOffset = offs;
      }
      public:
      const utils::Size &getMaskSize() const{
        return m_oMaskSize;
      }
      const utils::Point &getAnchor() const {
        return m_oAnchor;
      }
      const utils::Point &getROIOffset() const{
        return m_oROIOffset;
      }
      protected:

      /// prepare filter operation: ensure compatible image format and size
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc);

      /// prepare filter operation: as above, but with depth parameter
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepht);

      /// this function can be reimplemented e.g to enshure an odd mask width and height
       /** E.g. some implementations of Neighborhood-operation could demand odd or even
           mask size parameters. In this case, this function can be implemented in another
           way. (Example: MedianOp)
           @param size size to ajust
           @return the given size in this base implementation
           **/
      virtual utils::Size adaptSize(const utils::Size &size){ return size; }

      protected:
       ///TODO: later private with getter and setter functions
      utils::Size  m_oMaskSize;  ///< size of filter mask
      utils::Point m_oAnchor;    ///< anchor of filter mask
      utils::Point m_oROIOffset; ///< to-be-used ROI offset for source image
    };
  } // namespace filter
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/OpROIHandler.h                 **
** Module : ICLFilter                                              **
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
#include <ICLCore/ImgBase.h>

namespace icl {
  namespace filter{
    /// Utility base class for Image Operators
    /** The OpROIHandler class builds a utility base class for ICL unary and binary
        operator classes. Each Operator is performed on the ROI of the source image(s)
        only. The destination image is <em> always </em> adapted in its parameters to the
        necessary values. We distinguish the following modes:

        - Adapt the destination image in its size and ROI to the source
          image. Hence the destination will have the same size and ROI as the
          source image. Nevertheless the operator is applied on the ROI only,
          leaving the previous content of the destination image unchanged
          in this border region. So this mode requires subsequent handling
          of the border.(bClipToROI = false)

          <pre>
         source image    destination-image
          xxxxxxxxx        xxxxxxxxx
          xxxx....x        xxxx....x
          xxxx....x   -->  xxxx....x
          xxxx....x        xxxx....x
          </pre>

        - Adapt the destination images size, such that it exactly comprises
          the ROI of the source image. (bClipToROI = true)
          <pre>
         source image    destination-image
          xxxxxxxxx
          xxxx....x        ....
          xxxx....x   -->  ....
          xxxx....x        ....
          </pre>

        - Only check the destination images parameters and issue a warning
          if not set correctly already. In this case the filter operation itself
          should not be performed. (bCheckOnly = true)

        To this end the Filter class provides variables m_bCheck and m_bClipToROI
        accessible by setter and getter functions as well as several version of
        the prepare() methods which checks and adapts the destination image if
        neccessary.
    */
    class ICLFilter_API OpROIHandler {
      public:
      /// Destructor
      virtual ~OpROIHandler(){}


      private:
        friend class UnaryOp;
        friend class BinaryOp;
      /// change adaption of destination image (see class description)
      void setClipToROI (bool bClipToROI) {this->m_bClipToROI = bClipToROI;}
      void setCheckOnly (bool bCheckOnly) {this->m_bCheckOnly = bCheckOnly;}

      bool getClipToROI() const { return m_bClipToROI; }
      bool getCheckOnly() const { return m_bCheckOnly; }

      /// Filter is a base class for other classes and should be instantiated
      OpROIHandler() : m_bClipToROI (true), m_bCheckOnly (false) {}

      /// check+adapt destination images parameters against given values
      /// bCheckOnly mode ignores the given imgSize
      bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize,
                    core::format eFormat, int nChannels, const utils::Rect& roi,
                    utils::Time timestamp=utils::Time::null);

      /// check+adapt destination image to properties of given source image
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
        // ensure that the checkOnly flag is set to TRUE, if source and destination images are the same pointers,
        // this can not lead to problems, because *ppoDst is reallocated only if its depth differs from
        // the source images depth (which is not true if poSrc==*ppoDst
        //ICLASSERT_RETURN_VAL( !(ppoDst && (*ppoDst==poSrc) && !getCheckOnly() ), false);
        return prepare (ppoDst, poSrc->getDepth(), chooseSize (poSrc),
                        poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                        poSrc->getTime());
      }

      /// check+adapt destination image to properties of given source image
      /// but use explicitly given depth
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepth) {
        // ensure that the checkOnly flag is set to TRUE, if source and destination images are the same pointers,
        // this can not lead to problems, because *ppoDst is reallocated only if its depth differs from
        // the source images depth (which is not true if poSrc==*ppoDst
        // ICLASSERT_RETURN_VAL( !(ppoDst && (*ppoDst==poSrc) && !getCheckOnly() ), false);
        return prepare (ppoDst, eDepth, chooseSize (poSrc),
                        poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                        poSrc->getTime());
      }
      /// return to-be-used image size depending on bClipToROI
      const utils::Size chooseSize (const core::ImgBase *poSrc) {
        return m_bClipToROI ? poSrc->getROISize () : poSrc->getSize ();
      }
      /// return to-be-used ROI depending on bClipToROI
      const utils::Rect chooseROI (const core::ImgBase *poSrc) {
        return m_bClipToROI ? utils::Rect (utils::Point::null, poSrc->getROISize ())
          : poSrc->getROI();
      }

      bool m_bClipToROI, m_bCheckOnly;
    };
  } // namespace filter
}

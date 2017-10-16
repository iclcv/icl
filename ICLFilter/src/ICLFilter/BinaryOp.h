/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryOp.h                     **
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
#include <ICLFilter/OpROIHandler.h>

namespace icl{
  namespace filter{
    /// Abstract base class for binary image operations \ingroup BINARY
    /** A list of all binary operators can be found here: \n
        \ref BINARY
    **/
    class ICLFilter_API BinaryOp{
      public:

      /// default constructor
      BinaryOp();

      /// copy constructor
      BinaryOp(const BinaryOp &other);

      /// assignment operator
      BinaryOp &operator=(const BinaryOp &other);

      /// virtual destructor
      virtual ~BinaryOp();


      /// pure virtual apply function
      virtual void apply(const core::ImgBase *operand1,const core::ImgBase *operand2, core::ImgBase **result)=0;

      /// applyfunction without explicit destination image
      /** Usually this function must not be reimplemented, because it's default operation does simply use
          an internal buffer to call apply(const ImgBase*,const ImgBase*,ImgBase**). */
      virtual const core::ImgBase *apply(const core::ImgBase *operand1, const core::ImgBase *operand2);

      /// function operator (alternative for apply(src1,src2,dst)
      inline void operator()(const core::ImgBase *src1,const core::ImgBase *src2, core::ImgBase **dst){
        apply(src1,src2,dst);
      }

      /// function operator for the implicit destination apply(a,b) call
      inline const core::ImgBase *operator()(const core::ImgBase *src1,const core::ImgBase *src2){
        return apply(src1,src2);
      }

      /// reference based function operator
      inline const core::ImgBase &operator()(const core::ImgBase &sr1,const core::ImgBase &src2){
        return *apply(&sr1,&src2);
      }

      /// sets if the image should be clip to ROI or not
      /**
        @param bClipToROI true=yes, false=no
      */
      void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }

      /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
      /**
        @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
      */
      void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }

      /// returns the ClipToROI status
      /**
        @return true=ClipToROI is enable, false=ClipToROI is disabled
      */
      bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }

      /// returns the CheckOnly status
      /**
        @return true=CheckOnly is enable, false=CheckOnly is disabled
      */
      bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }

      protected:
      bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize,
                    core::format eFormat, int nChannels, const utils::Rect& roi,
                    utils::Time timestamp=utils::Time::null){
        return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
      }

      /// check+adapt destination image to properties of given source image
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
        return m_oROIHandler.prepare(ppoDst, poSrc);
      }

      /// check+adapt destination image to properties of given source image
      /// but use explicitly given depth
      virtual bool prepare (core::ImgBase **ppoDst,
                            const core::ImgBase *poSrc,
                            core::depth eDepth) {
        return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
      }

      static inline bool check(const core::ImgBase *operand1,
                               const core::ImgBase *operand2 ,
                               bool checkDepths = true) {
        if(!checkDepths) {
          return operand1->getChannels() == operand2->getChannels() &&
            operand1->getROISize() == operand2->getROISize() ;
        } else {
          return operand1->getChannels() == operand2->getChannels() &&
            operand1->getROISize() == operand2->getROISize() &&
            operand1->getDepth() == operand2->getDepth() ;
        }
      }

    private:
      OpROIHandler m_oROIHandler;

      /// internal image buffer which is used for the apply function without destination image argument
      core::ImgBase *m_buf;
    };
  } // namespace filter
}


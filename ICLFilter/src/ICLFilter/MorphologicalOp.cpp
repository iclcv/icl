/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MorphologicalOp.cpp            **
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

#include <ICLFilter/MorphologicalOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Range.h>
#include <ICLCore/ImgBorder.h>
#include <functional>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    namespace {

      // ================================================================
      // C++ morphological kernel
      // ================================================================

      template<class T, typename cmp_func>
      void morph_cpp(const Img<T> &src, Img<T> &dst, MorphologicalOp &op,
                     T init, cmp_func cmp, const icl8u *mask){
        const int srcW = src.getWidth(), dstW = dst.getWidth();
        const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
        const int maskW = op.getMaskSize().width, maskH = op.getMaskSize().height;
        const Point anchor = op.getAnchor();
        const Point roiOff = op.getROIOffset();
        const T *srcData;
        T *dstROI;

        for(int c = 0; c < src.getChannels(); ++c){
          srcData = src.getData(c);
          dstROI = dst.getROIData(c);
          for(int y = 0; y < roiH; ++y){
            T *dstRow = dstROI + y * dstW;
            for(int x = 0; x < roiW; ++x){
              const icl8u *m = mask;
              T best = init;
              for(int my = 0; my < maskH; ++my){
                const T *row = srcData + (roiOff.y - anchor.y + y + my) * srcW
                               + (roiOff.x - anchor.x + x);
                for(int mx = 0; mx < maskW; ++mx, ++m){
                  if(*m && cmp(row[mx], best)) best = row[mx];
                }
              }
              dstRow[x] = best;
            }
          }
        }
      }

      static Rect shrink_roi(Rect roi, const Size &maskSize){
        int dx = (maskSize.width-1)/2;
        int dy = (maskSize.height-1)/2;
        roi.x+=dx;
        roi.y+=dy;
        roi.width -= 2*dx;
        roi.height -= 2*dy;
        return roi;
      }

      template<class T>
      static void rep_border(Img<T> image, const Size &maskSize){
        Rect roi = shrink_roi(image.getImageRect(),maskSize);
        image.setROI(roi);
        ImgBorder::copy(&image);
      }

    } // anonymous namespace

    // ================================================================
    // C++ typed apply — basic ops + composite ops via nested instances
    // ================================================================

    template<class T>
    void MorphologicalOp::apply_t(const Image &srcImg, Image &dstImg){
      const Img<T> &src = srcImg.as<T>();
      Img<T> &dst = dstImg.as<T>();
      Range<T> limits = Range<T>::limits();
      Size sizeSave;
      std::vector<icl8u> maskSave;
      if(m_eType == dilate3x3 || m_eType == erode3x3){
        sizeSave = getMaskSize();
        std::copy(getMask(),getMask()+sizeSave.getDim(),back_inserter(maskSave));
        setMask(Size(3,3));
      }
      switch (m_eType){
        case dilate:
        case dilate3x3:
        case dilateBorderReplicate:
          morph_cpp(src,dst,*this,limits.minVal,std::greater<T>(),getMask());
          break;
        case erode:
        case erode3x3:
        case erodeBorderReplicate:
          morph_cpp(src,dst,*this,limits.maxVal,std::less<T>(),getMask());
          break;
        case tophatBorder:
        case blackhatBorder:{
          MorphologicalOp op(m_eType==tophatBorder ? openBorder : closeBorder,getMaskSize(),getMask());
          op.setClipToROI(getClipToROI());
          op.setCheckOnly(getCheckOnly());
          op.apply(srcImg, m_openingAndClosingBuffer);
          BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
          sub.setClipToROI(getClipToROI());
          sub.setCheckOnly(getCheckOnly());

          Rect roi = srcImg.getROI();
          roi = shrink_roi(roi,getMaskSize());
          roi = shrink_roi(roi,getMaskSize());
          Image srcROIAdapted = srcImg.deepCopy();
          srcROIAdapted.setROI(roi);

          if(m_eType == tophatBorder){
            sub.apply(srcROIAdapted, m_openingAndClosingBuffer, dstImg);
          }else{
            sub.apply(m_openingAndClosingBuffer, srcROIAdapted, dstImg);
          }
          break;
        }
        case gradientBorder:{
          MorphologicalOp op(closeBorder,getMaskSize(),getMask());
          op.setClipToROI(getClipToROI());
          op.setCheckOnly(getCheckOnly());
          op.apply(srcImg, m_gradientBorderBuffer_1);
          op.setOptype(openBorder);
          op.apply(srcImg, m_gradientBorderBuffer_2);
          BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
          sub.setClipToROI(getClipToROI());
          sub.setCheckOnly(getCheckOnly());

          sub.apply(m_gradientBorderBuffer_1, m_gradientBorderBuffer_2, dstImg);
          break;
        }
        case openBorder:
        case closeBorder:{
          MorphologicalOp op(m_eType==openBorder ? erode : dilate,getMaskSize(),getMask());
          op.setClipToROI(getClipToROI());
          op.setCheckOnly(getCheckOnly());
          op.apply(srcImg, m_openingAndClosingBuffer);
          op.setOptype(m_eType==openBorder ? dilate : erode);
          op.apply(m_openingAndClosingBuffer, dstImg);
          break;
        }
        default:
          ERROR_LOG("invalid optype: " << static_cast<int>(m_eType));
      }
      if(!getClipToROI() && (m_eType == erodeBorderReplicate || m_eType == dilateBorderReplicate) ){
        rep_border(dst,getMaskSize());
      }else if (m_eType == erodeBorderReplicate || m_eType == dilateBorderReplicate){
        ERROR_LOG("border replication does not work if clipToROI is set [operation was applied, border replication was skipped]");
      }
      if(m_eType == dilate3x3 || m_eType == erode3x3){
        setMask(sizeSave,maskSave.data());
      }
    }

    void MorphologicalOp::cpp_morph(const Image &src, Image &dst, MorphologicalOp &op) {
      switch(src.getDepth()){
        case depth8u:  op.apply_t<icl8u>(src, dst); break;
        case depth32f: op.apply_t<icl32f>(src, dst); break;
        default: ICL_INVALID_DEPTH; break;
      }
    }

    // ================================================================
    // Constructors / Destructor
    // ================================================================

    MorphologicalOp::MorphologicalOp(optype eOptype, const Size &maskSize, const icl8u *pcMask){
      ICLASSERT_RETURN(maskSize.getDim());
      m_eType = eOptype;
      m_pcMask = 0;
      setMask(maskSize, pcMask);

      initDispatching("MorphologicalOp");
      auto &sel = addSelector<MorphSig>("apply");
      sel.add(Backend::Cpp, cpp_morph);
    }

    MorphologicalOp::MorphologicalOp(const std::string &o, const Size &maskSize, const icl8u *pcMask){
      ICLASSERT_RETURN(maskSize.getDim());

#define CHECK_OPTYPE(X) else if(o == #X) { m_eType = X; }
      if(o == "dilate") { m_eType = dilate; }
      CHECK_OPTYPE(erode)
      CHECK_OPTYPE(dilate3x3)
      CHECK_OPTYPE(erode3x3)
      CHECK_OPTYPE(dilateBorderReplicate)
      CHECK_OPTYPE(erodeBorderReplicate)
      CHECK_OPTYPE(openBorder)
      CHECK_OPTYPE(closeBorder)
      CHECK_OPTYPE(tophatBorder)
      CHECK_OPTYPE(blackhatBorder)
      CHECK_OPTYPE(gradientBorder)
#undef CHECK_OPTYPE
      else{
        throw ICLException("MorphologicalOp::MorphologicalOp: invalid optype string!");
      }
      m_pcMask = 0;
      setMask(maskSize, pcMask);

      initDispatching("MorphologicalOp");
      auto &sel = addSelector<MorphSig>("apply");
      sel.add(Backend::Cpp, cpp_morph);
    }

    MorphologicalOp::~MorphologicalOp(){
      ICL_DELETE_ARRAY(m_pcMask);
    }

    // ================================================================
    // Shared methods
    // ================================================================

    void MorphologicalOp::setMask(Size maskSize, const icl8u* pcMask) {
      //make maskSize odd:
      maskSize = ((maskSize/2)*2)+Size(1,1);

      if(m_eType >= 6){
        NeighborhoodOp::setMask(Size(1,1));
      }else{
        NeighborhoodOp::setMask(maskSize);
      }

      ICL_DELETE_ARRAY(m_pcMask);
      m_pcMask = new icl8u[maskSize.getDim()];
      if(pcMask){
        std::copy(pcMask,pcMask+maskSize.getDim(),m_pcMask);
      }else{
        std::fill(m_pcMask,m_pcMask+maskSize.getDim(),255);
      }

      m_oMaskSizeMorphOp = maskSize;
      ++m_maskVersion;
    }

    const icl8u* MorphologicalOp::getMask() const{
      return m_pcMask;
    }
    Size MorphologicalOp::getMaskSize() const{
      return m_oMaskSizeMorphOp;
    }
    void MorphologicalOp::setOptype(optype type){
      m_eType = type;
      setMask(m_oMaskSizeMorphOp, m_pcMask);
    }
    MorphologicalOp::optype MorphologicalOp::getOptype() const{
      return m_eType;
    }

    void MorphologicalOp::apply(const core::Image &src, core::Image &dst) {
      if(!prepare(dst, src)) return;
      getSelector<MorphSig>("apply").resolve(src)->apply(src, dst, *this);
    }

  } // namespace filter
}

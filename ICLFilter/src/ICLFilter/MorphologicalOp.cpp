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

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
  #ifndef ICL_HAVE_IPP
    template<class T, typename cmp_func>
    void morph_cpp(const Img<T> &src, Img<T> &dst, MorphologicalOp &op,
                   T init, cmp_func cmp, const icl8u *mask){

      Point an = op.getAnchor();
      Size si = op.getMaskSize();
      for(int c=0;c<src.getChannels();++c){
        const ImgIterator<T> s(const_cast<T*>(src.getData(c)), src.getWidth(),Rect(op.getROIOffset(), dst.getROISize()));
        const ImgIterator<T> sEnd = ImgIterator<T>::create_end_roi_iterator(src.getData(c),src.getWidth(), Rect(op.getROIOffset(), dst.getROISize()));
        ImgIterator<T> d = dst.beginROI(c);
        for(; s != sEnd; ++s){
          const icl8u *m = mask;
          T best = init;
          for(const ImgIterator<T> sR (s,si,an);sR.inRegionSubROI(); ++sR, ++m){
            if(*m && cmp(*sR,best)) best = *sR;
          }
          *d++ = best;
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


    template<class T>
    void MorphologicalOp::apply_t(const ImgBase *poSrc, ImgBase **ppoDst){
      const Img<T> &src = *poSrc->asImg<T>();
      Img<T> &dst = *(*ppoDst)->asImg<T>();
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
          op.apply(poSrc,&m_openingAndClosingBuffer);
          BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
          sub.setClipToROI(getClipToROI());
          sub.setCheckOnly(getCheckOnly());

          Rect roi = poSrc->getROI();
          roi = shrink_roi(roi,getMaskSize());
          roi = shrink_roi(roi,getMaskSize());
          const ImgBase *srcROIAdapted = poSrc->shallowCopy(roi);

          if(m_eType == tophatBorder){
            sub.apply(srcROIAdapted,m_openingAndClosingBuffer,ppoDst);
          }else{
            sub.apply(m_openingAndClosingBuffer,srcROIAdapted,ppoDst);
          }

          delete srcROIAdapted;
          break;
        }
        case gradientBorder:{
          MorphologicalOp op(closeBorder,getMaskSize(),getMask());
          op.setClipToROI(getClipToROI());
          op.setCheckOnly(getCheckOnly());
          op.apply(poSrc,&m_gradientBorderBuffer_1);
          op.setOptype(openBorder);
          op.apply(poSrc,&m_gradientBorderBuffer_2);
          BinaryArithmeticalOp sub(BinaryArithmeticalOp::subOp);
          sub.setClipToROI(getClipToROI());
          sub.setCheckOnly(getCheckOnly());

          sub.apply(m_gradientBorderBuffer_1,m_gradientBorderBuffer_2,ppoDst);
          break;
        }
        case openBorder:
        case closeBorder:{
          MorphologicalOp op(m_eType==openBorder ? erode : dilate,getMaskSize(),getMask());
          op.setClipToROI(getClipToROI());
          op.setCheckOnly(getCheckOnly());
          op.apply(poSrc,&m_openingAndClosingBuffer);
          op.setOptype(m_eType==openBorder ? erode : dilate);
          op.apply(m_openingAndClosingBuffer,ppoDst);
          break;
        }
        default:
          ERROR_LOG("invalid optype: " << (int) m_eType);
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


    void MorphologicalOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
      FUNCTION_LOG("");
      if (!prepare (ppoDst, poSrc)) return;

      switch (poSrc->getDepth()){
        case depth8u:
          apply_t<icl8u>(poSrc,ppoDst);
          break;
        case depth32f:
          apply_t<icl32f>(poSrc,ppoDst);
          break;
        default:
          ICL_INVALID_DEPTH;
          break;
      }
    }

    MorphologicalOp::MorphologicalOp (optype eOptype, const Size &maskSize,const icl8u *pcMask):
      m_openingAndClosingBuffer(0),m_gradientBorderBuffer_1(0),m_gradientBorderBuffer_2(0)
    {
      ICLASSERT_RETURN(maskSize.getDim());
      m_pcMask = 0;
      setMask (maskSize,pcMask);
      m_eType = eOptype;
    }

    MorphologicalOp::MorphologicalOp (const std::string &o, const Size &maskSize,const icl8u *pcMask):
      m_openingAndClosingBuffer(0),m_gradientBorderBuffer_1(0),m_gradientBorderBuffer_2(0)
    {
      ICLASSERT_RETURN(maskSize.getDim());
      m_pcMask = 0;
      setMask (maskSize,pcMask);

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
    }


    MorphologicalOp::~MorphologicalOp(){
      ICL_DELETE_ARRAY(m_pcMask);
      ICL_DELETE(m_openingAndClosingBuffer);
      ICL_DELETE(m_gradientBorderBuffer_1);
      ICL_DELETE(m_gradientBorderBuffer_2);
    }



  #else //  ICL_HAVE_IPP is defined !
    MorphologicalOp::MorphologicalOp (optype eOptype, const Size &maskSize,const icl8u *pcMask){
      ICLASSERT_RETURN(maskSize.getDim());

      m_eType=eOptype;
      m_pcMask = 0;
      setMask (maskSize,pcMask);

      m_bMorphState8u=false;
      m_bMorphState32f=false;
      m_bMorphAdvState8u=false;
      m_bMorphAdvState32f=false;
      m_bHas_changed=true;
      m_bHas_changedAdv=true;

      m_pState8u = 0;
      m_pState32f = 0;
      m_pAdvState8u = 0;
      m_pAdvState32f = 0;
    }

    MorphologicalOp::MorphologicalOp (const std::string &o, const Size &maskSize,const icl8u *pcMask){
      ICLASSERT_RETURN(maskSize.getDim());
      m_pcMask = 0;
      setMask (maskSize,pcMask);

    m_bMorphState8u=false;
      m_bMorphState32f=false;
      m_bMorphAdvState8u=false;
      m_bMorphAdvState32f=false;
      m_bHas_changed=true;
      m_bHas_changedAdv=true;

      m_pState8u = 0;
      m_pState32f = 0;
      m_pAdvState8u = 0;
      m_pAdvState32f = 0;

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
    }



    MorphologicalOp::~MorphologicalOp(){
      deleteMorphStates();
      ICL_DELETE(m_pcMask);
    }

    void MorphologicalOp::deleteMorphStates(){
      if (m_bMorphState8u){
        //ippiMorphologyFree(m_pState8u);
        ippsFree(m_pState8u);
        ippsFree(m_pBuf);
        m_bMorphState8u=false;
      }
      if (m_bMorphAdvState8u){
        //ippiMorphAdvFree(m_pAdvState8u);
        ippsFree(m_pAdvState8u);
        ippsFree(m_pAdvBuf);

        m_bMorphAdvState8u=false;
      }
      if (m_bMorphState32f){
        //ippiMorphologyFree(m_pState32f);
        ippsFree(m_pState32f);
        ippsFree(m_pBuf);
        m_bMorphState32f=false;
      }
      if (m_bMorphAdvState32f){
        //ippiMorphAdvFree(m_pAdvState32f);
        ippsFree(m_pAdvState32f);
        ippsFree(m_pAdvBuf);
        m_bMorphAdvState32f=false;
      }
    }

    void MorphologicalOp::checkMorphAdvState8u(const Size roiSize){
      if (m_bHas_changedAdv){
        deleteMorphStates();
        IppStatus status = ippStsNoErr;
	      int specSize = 0, bufferSize = 0;     /*  working buffers size */
	      status = ippiMorphAdvGetSize_8u_C1R(roiSize, m_oMaskSizeMorphOp, &specSize, &bufferSize);
	      m_pAdvState8u = (IppiMorphAdvState*)ippsMalloc_8u(specSize);
	      m_pAdvBuf = (Ipp8u*)ippsMalloc_8u(bufferSize);
	      status = ippiMorphAdvInit_8u_C1R(roiSize, m_pcMask, m_oMaskSizeMorphOp, m_pAdvState8u, m_pAdvBuf);
	      if(status!=ippStsNoErr){
          WARNING_LOG("IPP Error");
        }
        //ippiMorphAdvInitAlloc_8u_C1R(&m_pAdvState8u, roiSize, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor);
        m_bMorphAdvState8u=true;
        m_bHas_changedAdv=false;
      }
    }

      void MorphologicalOp::checkMorphState8u(const Size roiSize){
      if (m_bHas_changed){
        deleteMorphStates();
        IppStatus status = ippStsNoErr;
	      int specSize = 0, bufferSize = 0;     /*  working buffers size */
	      status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, m_oMaskSizeMorphOp, &specSize, &bufferSize);
	      m_pState8u = (IppiMorphState*)ippsMalloc_8u(specSize);
	      m_pBuf = (Ipp8u*)ippsMalloc_8u(bufferSize);
	      status = ippiMorphologyBorderInit_8u_C1R(roiSize, m_pcMask, m_oMaskSizeMorphOp, m_pState8u, m_pBuf);
	      if(status!=ippStsNoErr){
          WARNING_LOG("IPP Error");
        }
        //ippiMorphologyInitAlloc_8u_C1R(roiSize.width, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor,&m_pState8u);

        m_bMorphState8u=true;
        m_bHas_changed=false;
      }
    }
    void MorphologicalOp::checkMorphAdvState32f(const Size roiSize){
      if (m_bHas_changedAdv){
        deleteMorphStates();
        IppStatus status = ippStsNoErr;
	      int specSize = 0, bufferSize = 0;     /*  working buffers size */
	      status = ippiMorphAdvGetSize_32f_C1R(roiSize, m_oMaskSizeMorphOp, &specSize, &bufferSize);
	      m_pAdvState32f = (IppiMorphAdvState*)ippsMalloc_8u(specSize);
	      m_pAdvBuf = (Ipp8u*)ippsMalloc_8u(bufferSize);
	      status = ippiMorphAdvInit_32f_C1R(roiSize, m_pcMask, m_oMaskSizeMorphOp, m_pAdvState32f, m_pAdvBuf);
	      if(status!=ippStsNoErr){
          WARNING_LOG("IPP Error");
        }
        //ippiMorphAdvInitAlloc_32f_C1R(&m_pAdvState32f, roiSize, m_pcMask, m_oMaskSize, m_oAnchor);
        m_bMorphAdvState32f=true;
        m_bHas_changedAdv=false;
      }
    }

      void MorphologicalOp::checkMorphState32f(const Size roiSize){
      if (m_bHas_changed){
        deleteMorphStates();
        IppStatus status = ippStsNoErr;
	      int specSize = 0, bufferSize = 0;     /*  working buffers size */
	      status = ippiMorphologyBorderGetSize_32f_C1R(roiSize, m_oMaskSizeMorphOp, &specSize, &bufferSize);
	      m_pState32f = (IppiMorphState*)ippsMalloc_8u(specSize);
	      m_pBuf = (Ipp8u*)ippsMalloc_8u(bufferSize);
	      status = ippiMorphologyBorderInit_32f_C1R(roiSize, m_pcMask, m_oMaskSizeMorphOp, m_pState32f, m_pBuf);
	      if(status!=ippStsNoErr){
          WARNING_LOG("IPP Error");
        }
        //ippiMorphologyInitAlloc_32f_C1R(roiSize.width, m_pcMask, m_oMaskSizeMorphOp, m_oAnchor,&m_pState32f);

        m_bMorphState32f=true;
        m_bHas_changed=false;
      }
    }

    void MorphologicalOp::apply (const ImgBase *poSrc, ImgBase **ppoDst){
      FUNCTION_LOG("");
      if (!prepare (ppoDst, poSrc)) return;

      IppStatus s = ippStsNoErr;
      switch (poSrc->getDepth()){
        case depth8u:{
          const Img8u *src = poSrc->asImg<icl8u>();
          Img8u *dst = (*ppoDst)->asImg<icl8u>();
          switch (m_eType){
            case dilate:
              //s=ippiMorphologicalCall<icl8u,ippiDilateBorder_8u_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case erode:
              //s=ippiMorphologicalCall<icl8u,ippiErodeBorder_8u_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiErodeBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case dilate3x3:
              //s=ippiMorphologicalCall3x3<icl8u,ippiDilateBorder_8u_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case erode3x3:
              //s=ippiMorphologicalCall3x3<icl8u,ippiErodeBorder_8u_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) { //call checkMorphState8u(Size(3,3)) first?
                IppStatus s = ippiErodeBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case dilateBorderReplicate:
              checkMorphState8u(src->getROISize());
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              //s=ippiMorphologicalBorderReplicateCall<icl8u,ippiDilateBorder_8u_C1R> (src,dst,m_pState8u);
              break;
            case erodeBorderReplicate:
              checkMorphState8u(src->getROISize());
              //s=ippiMorphologicalBorderReplicateCall<icl8u,ippiErodeBorder_8u_C1R> (src,dst,m_pState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiErodeBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState8u, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case openBorder:
              checkMorphAdvState8u(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl8u,ippiMorphOpenBorder_8u_C1R> (src,dst,m_pAdvState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphOpenBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState8u, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case closeBorder:
              checkMorphAdvState8u(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl8u,ippiMorphCloseBorder_8u_C1R> (src,dst,m_pAdvState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphCloseBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState8u, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case tophatBorder:
              checkMorphAdvState8u(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl8u,ippiMorphTophatBorder_8u_C1R> (src,dst,m_pAdvState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphTophatBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState8u, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case blackhatBorder:
              checkMorphAdvState8u(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl8u,ippiMorphBlackhatBorder_8u_C1R> (src,dst,m_pAdvState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphBlackhatBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState8u, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case gradientBorder:
              checkMorphAdvState8u(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl8u,ippiMorphGradientBorder_8u_C1R> (src,dst,m_pAdvState8u);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphGradientBorder_8u_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState8u, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
          }
        break;
        }
        case depth32f:{
          const Img32f *src = poSrc->asImg<icl32f>();
          Img32f *dst = (*ppoDst)->asImg<icl32f>();
          switch (m_eType){
            case dilate:
              //s=ippiMorphologicalCall<icl32f,ippiDilate_32f_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case erode:
              //s=ippiMorphologicalCall<icl32f,ippiErode_32f_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiErodeBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case dilate3x3:
              //s=ippiMorphologicalCall3x3<icl32f,ippiDilate3x3_32f_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case erode3x3:
              //s=ippiMorphologicalCall3x3<icl32f,ippiErode3x3_32f_C1R> (src,dst);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiErodeBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case dilateBorderReplicate:
              checkMorphState32f(src->getROISize());
              //s=ippiMorphologicalBorderReplicateCall<icl32f,ippiDilateBorderReplicate_32f_C1R> (src,dst,m_pState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiDilateBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case erodeBorderReplicate:
              checkMorphState32f(src->getROISize());
              //s=ippiMorphologicalBorderReplicateCall<icl32f,ippiErodeBorderReplicate_32f_C1R> (src,dst,m_pState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiErodeBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pState32f, m_pBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case openBorder:
              checkMorphAdvState32f(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl32f,ippiMorphOpenBorder_32f_C1R> (src,dst,m_pAdvState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphOpenBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState32f, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case closeBorder:
              checkMorphAdvState32f(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl32f,ippiMorphCloseBorder_32f_C1R> (src,dst,m_pAdvState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphCloseBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState32f, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case tophatBorder:
              checkMorphAdvState32f(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl32f,ippiMorphTophatBorder_32f_C1R> (src,dst,m_pAdvState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphTophatBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState32f, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case blackhatBorder:
              checkMorphAdvState32f(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl32f,ippiMorphBlackhatBorder_32f_C1R> (src,dst,m_pAdvState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphBlackhatBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState32f, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
            case gradientBorder:
              checkMorphAdvState32f(src->getROISize());
              //s=ippiMorphologicalBorderCall<icl32f,ippiMorphGradientBorder_32f_C1R> (src,dst,m_pAdvState32f);
              for(int c=0; c < src->getChannels(); c++) {
                IppStatus s = ippiMorphGradientBorder_32f_C1R(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl, 0,
                               m_pAdvState32f, m_pAdvBuf);
                if(s != ippStsNoErr) WARNING_LOG("IPP Error");
              }
              break;
          }
          break;
        }
        default: ICL_INVALID_DEPTH; break;
      }
      if(s != ippStsNoErr){
        ERROR_LOG("IPP-Error: \"" << ippGetStatusString(s) << "\"");
      }


    }


/*
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
    IppStatus MorphologicalOp::ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState* advState) {
      for(int c=0; c < src->getChannels(); c++) {
        IppStatus s = ippiFunc(src->getROIData (c, this->m_oROIOffset),
                               src->getLineStep(),
                               dst->getROIData (c),
                               dst->getLineStep(),
                               dst->getROISize(),
                               ippBorderRepl,
                               advState
                               );
        if(s != ippStsNoErr) return s;
      }
      return ippStsNoErr;
    }
*/
    // }}}
  #endif

    void MorphologicalOp::setMask (Size maskSize,const icl8u* pcMask) {
      //make maskSize odd:
      maskSize = ((maskSize/2)*2)+Size(1,1);

      if(m_eType >= 6){
        NeighborhoodOp::setMask (Size(1,1));
      }else{
        NeighborhoodOp::setMask (maskSize);
      }

      ICL_DELETE_ARRAY(m_pcMask);
      m_pcMask = new icl8u[maskSize.getDim()];
      if(pcMask){
        std::copy(pcMask,pcMask+maskSize.getDim(),m_pcMask);
      }else{
        std::fill(m_pcMask,m_pcMask+maskSize.getDim(),255);
      }

      m_oMaskSizeMorphOp=maskSize;
      m_bHas_changed=true;
      m_bHas_changedAdv=true;
    }

    const icl8u* MorphologicalOp::getMask() const{
      return m_pcMask;
    }
    Size MorphologicalOp::getMaskSize() const{
      return m_oMaskSizeMorphOp;
    }
    void MorphologicalOp::setOptype(optype type){
      m_eType=type;
      setMask(m_oMaskSizeMorphOp,m_pcMask);
    }
    MorphologicalOp::optype MorphologicalOp::getOptype() const{
      return m_eType;
    }


  } // namespace filter
}

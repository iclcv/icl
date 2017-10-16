/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CCLUT.cpp                          **
** Module : ICLCore                                                **
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

#include <ICLCore/CCLUT.h>
#include <ICLCore/CCFunctions.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace {
    static const int C1 = 256;
    static const int C2 = 256*256;
    static const int C3 = 256*256*256;

    static unsigned char ROW[256];
    struct ROW_INIT { ROW_INIT(){ for(int i=0;i<256;i++) ROW[i] = (unsigned char)(i); } } __RI;;


    template<class T>
    inline std::vector<T*> get_ptrs(Img<T> &image, int offs=0){
      std::vector<T*> v(image.getChannels());
      for(int i=0;i<image.getChannels();i++){
        v[i] = image.getData(i)+offs;
      }
      return v;
    }

    template<class T>
    inline std::vector<const T*> get_ptrs(const Img<T> &image, int offs=0){
      std::vector<const T*> v(image.getChannels());
      for(int i=0;i<image.getChannels();i++){
        v[i] = image.getData(i)+offs;
      }
      return v;
    }

    template<class T>
    inline void get_ptrs_3(Img<T> &image, T *&a, T *&b, T *&c){
      a = image.getData(0);
      b = image.getData(1);
      c = image.getData(2);
    }

    template<class T>
    inline void get_ptrs_3(const Img<T> &image,const T *&a,const T *&b,const T *&c){
      a = image.getData(0);
      b = image.getData(1);
      c = image.getData(2);
    }

    template<class T>
    inline void get_ptrs_2(Img<T> &image,T *&a,T *&b){
      a = image.getData(0);
      b = image.getData(1);
    }

    template<class T>
    inline void get_ptrs_2(const Img<T> &image,const T *&a,const T *&b){
      a = image.getData(0);
      b = image.getData(1);
    }

    template<class S, class D>
    void cc_3x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
      ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2, *s3;
        D *d1, *d2, *d3;
        icl8u *l1, *l2, *l3;

        get_ptrs_3(src,s1,s2,s3);
        get_ptrs_3(dst,d1,d2,d3);
        get_ptrs_3(lut,l1,l2,l3);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = C2*clipped_cast<S,icl8u>(s1[i]) + C1*clipped_cast<S,icl8u>(s2[i]) + clipped_cast<S,icl8u>(s3[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
          d2[i] = clipped_cast<icl8u,D>(l2[idx]);
          d3[i] = clipped_cast<icl8u,D>(l3[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_3x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2, *s3;
        D *d1, *d2;
        icl8u *l1, *l2;

        get_ptrs_3(src,s1,s2,s3);
        get_ptrs_2(dst,d1,d2);
        get_ptrs_2(lut,l1,l2);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = C2*clipped_cast<S,icl8u>(s1[i]) + C1*clipped_cast<S,icl8u>(s2[i]) + clipped_cast<S,icl8u>(s3[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
          d2[i] = clipped_cast<icl8u,D>(l2[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_3x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2, *s3;
        D *d1 = dst.getData(0);
        icl8u *l1 = lut.getData(0);

        get_ptrs_3(src,s1,s2,s3);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = C2*clipped_cast<S,icl8u>(s1[i]) + C1*clipped_cast<S,icl8u>(s2[i]) + clipped_cast<S,icl8u>(s3[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_2x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2;
        D *d1, *d2, *d3;
        icl8u *l1, *l2, *l3;

        get_ptrs_2(src,s1,s2);
        get_ptrs_3(dst,d1,d2,d3);
        get_ptrs_3(lut,l1,l2,l3);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
        idx = C1*clipped_cast<S,icl8u>(s1[i]) + clipped_cast<S,icl8u>(s2[i]);
        d1[i] = clipped_cast<icl8u,D>(l1[idx]);
        d2[i] = clipped_cast<icl8u,D>(l2[idx]);
        d3[i] = clipped_cast<icl8u,D>(l3[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_2x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2;
        D *d1, *d2;
        icl8u *l1, *l2;

        get_ptrs_2(src,s1,s2);
        get_ptrs_2(dst,d1,d2);
        get_ptrs_2(lut,l1,l2);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = C1*clipped_cast<S,icl8u>(s1[i]) + clipped_cast<S,icl8u>(s2[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
          d2[i] = clipped_cast<icl8u,D>(l2[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_2x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1,*s2;
        D *d1 = dst.getData(0);
        icl8u *l1 = lut.getData(0);

        get_ptrs_2(src,s1,s2);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = C1*clipped_cast<S,icl8u>(s1[i]) + clipped_cast<S,icl8u>(s2[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_1x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1 = src.getData(0);
        D *d1, *d2, *d3;
        icl8u *l1, *l2, *l3;

        get_ptrs_3(dst,d1,d2,d3);
        get_ptrs_3(lut,l1,l2,l3);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = clipped_cast<S,icl8u>(s1[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
          d2[i] = clipped_cast<icl8u,D>(l2[idx]);
          d3[i] = clipped_cast<icl8u,D>(l3[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_1x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1 = src.getData(0);
        D *d1, *d2;
        icl8u *l1, *l2;

        get_ptrs_2(dst,d1,d2);
        get_ptrs_2(lut,l1,l2);

        const int DIM = src.getDim();
        register int idx(0);
        for(int i=0;i<DIM;i++){
          idx = clipped_cast<S,icl8u>(s1[i]);
          d1[i] = clipped_cast<icl8u,D>(l1[idx]);
          d2[i] = clipped_cast<icl8u,D>(l2[idx]);
        }
      }
    }

    template<class S, class D>
    void cc_1x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
      if(roiOnly){
        ERROR_LOG("not yet implemented with roi support!");
      }else{
        const S *s1 = src.getData(0);
        D *d1 = dst.getData(0);
        icl8u *l1 = lut.getData(0);

        const int DIM = src.getDim();
        for(int i=0;i<DIM;i++){
          d1[i] = clipped_cast<icl8u,D>(l1[clipped_cast<S,icl8u>(s1[i])]);
        }
      }
    }

    inline Img8u create_lut_3X(format srcFmt, format dstFmt){
      /// this must be much bigger
      Img8u bufSrc(Size(C2,1),srcFmt);
      Img8u bufDst(Size(C3,1),dstFmt);

      progress_init();
      for(int r=0;r<256;r++){
        std::fill(bufSrc.getData(0), bufSrc.getData(0)+C2, icl8u(r));
        icl8u *pg = bufSrc.getData(1);
        for(int g=0;g<256;g++){
          std::fill(pg+g*C1,pg+(g+1)*C1, icl8u(g));
          copy(ROW, ROW+C1,bufSrc.getData(2) + g*C1);
        }
        Img8u bufDstTmp(bufSrc.getSize(),dstFmt,get_ptrs(bufDst,r*C2));
        cc(&bufSrc,&bufDstTmp);
        progress(r,256);
      }
      progress_finish();
      return bufDst;
    }

    inline Img8u create_lut_2X(format srcFmt, format dstFmt){
      Img8u bufSrc(Size(C2,1),srcFmt);
      Img8u bufDst(Size(C2,1),dstFmt);

      progress_init();
      for(int r=0;r<256;r++){
        std::fill(bufSrc.getData(0)+r*C1, bufSrc.getData(0)+(r+1)*C1, icl8u(r));
        copy(ROW, ROW+C1,bufSrc.getData(1)+r*C1);
        progress(r,512);
      }
      cc(&bufSrc,&bufDst);
      progress_finish();
      return bufDst;
    }

    inline Img8u create_lut_1X(format srcFmt, format dstFmt){
      Img8u bufSrc(Size(C1,1),srcFmt);
      Img8u bufDst(Size(C1,1),dstFmt);

      copy(ROW, ROW+C1,bufSrc.getData(0));

      icl::core::cc(&bufSrc,&bufDst);
      return bufDst;
    }

    template<class S, class D>
    inline void cc_sd(const Img<S> *src, Img<D> *dst, Img8u &lut, bool roiOnly){
      switch(src->getChannels()){
        case 1:
          switch(dst->getChannels()){
            case 1: cc_1x1(*src, *dst, lut, roiOnly); break;
            case 2: cc_1x2(*src, *dst, lut, roiOnly); break;
            case 3: cc_1x3(*src, *dst, lut, roiOnly); break;
            default: throw ICLException("CCLUT internal error (code 1)");
          }
          break;
        case 2:
          switch(dst->getChannels()){
            case 1: cc_2x1(*src, *dst, lut, roiOnly); break;
            case 2: cc_2x2(*src, *dst, lut, roiOnly); break;
            case 3: cc_2x3(*src, *dst, lut, roiOnly); break;
            default: throw ICLException("CCLUT internal error (code 2)");
          }
          break;
        case 3:
          switch(dst->getChannels()){
            case 1: cc_3x1(*src, *dst, lut, roiOnly); break;
            case 2: cc_3x2(*src, *dst, lut, roiOnly); break;
            case 3: cc_3x3(*src, *dst, lut, roiOnly); break;
            default: throw ICLException("CCLUT internal error (code 3)");
          }
          break;

        default: throw ICLException("CCLUT internal error (code 4)");
      }
    }

    template<class S>
    inline void cc_s(const Img<S> *src, ImgBase *dst, Img8u &lut, bool roiOnly){
      switch(dst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: cc_sd(src,dst->asImg<icl##D>(),lut,roiOnly); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }
  namespace core{

    CCLUT::CCLUT(format srcFmt, format dstFmt){
      switch(getChannelsOfFormat(srcFmt)){
        case 1: m_oLUT = create_lut_1X(srcFmt, dstFmt); break;
        case 2: m_oLUT = create_lut_2X(srcFmt, dstFmt); break;
        case 3: m_oLUT = create_lut_3X(srcFmt, dstFmt); break;
        default: ICL_INVALID_FORMAT;
      }
    }
    void CCLUT::cc(const ImgBase *src, ImgBase *dst, bool roiOnly){
      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: cc_s(src->asImg<icl##D>(),dst, m_oLUT,roiOnly); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  } // namespace core
}

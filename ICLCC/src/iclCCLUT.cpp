#include <iclCCLUT.h>
#include <iclCC.h>
using namespace std;
using namespace icl;

namespace{
  const int C1 = 256;
  const int C2 = 256*256;
  const int C3 = 256*256*256;
  icl8u ROW[256];
  char BACK_LINE[] = { char(27), char(91), char(65), char(0)};// "\E[A\0"
  struct ROW_INIT { ROW_INIT(){ for(int i=0;i<256;i++) ROW[i] = icl8u(i); } };
  ROW_INIT __RI;

  int BAR_LEN = 30;
  void progress_init(){
    // {{{ open

    printf("\n");
    printf("%screating lookup table:\n",BACK_LINE);
  }

  // }}}
  
  void progress_finish(){
    // {{{ open

    printf("%screating lookup table:100%s[##############################]\n\n",BACK_LINE,"%");
    
  }

  // }}}
  
  void progress(int curr, int max){
    // {{{ open

    printf("%screating lookup table:",BACK_LINE);
    float frac = (float)curr/(float)max;
    printf("%3d%s",(int)(frac*100),"%");
    
    int N1 = (int)(frac*BAR_LEN);
    int N2 = BAR_LEN - N1;
    
    printf("[");
    for(int i=0;i<N1;i++){
      printf("#");
    }
    for(int i=0;i<N2;i++){
      printf("=");
    }
    printf("]\n");
    
  }

  // }}}
  
  template<class T>
  inline vector<T*> get_ptrs(Img<T> &image, int offs=0){
    // {{{ open

    vector<T*> v(image.getChannels());
    for(int i=0;i<image.getChannels();i++){
      v[i] = image.getData(i)+offs;
    }
    return v;
  }

  // }}}

  template<class T>
  inline vector<const T*> get_ptrs(const Img<T> &image, int offs=0){
    // {{{ open

    vector<const T*> v(image.getChannels());
    for(int i=0;i<image.getChannels();i++){
      v[i] = image.getData(i)+offs;
    }
    return v;
  }

  // }}}


  template<class T>
  inline void get_ptrs_3(Img<T> &image, T *&a, T *&b, T *&c){
    // {{{ open

    a = image.getData(0);
    b = image.getData(1);
    c = image.getData(2);
  }

  // }}}

  template<class T>
  inline void get_ptrs_3(const Img<T> &image,const T *&a,const T *&b,const T *&c){
    // {{{ open

    a = image.getData(0);
    b = image.getData(1);
    c = image.getData(2);
  }

  // }}}

  template<class T>
  inline void get_ptrs_2(Img<T> &image,T *&a,T *&b){
    // {{{ open
    a = image.getData(0);
    b = image.getData(1);
  }

  // }}}

  template<class T>
  inline void get_ptrs_2(const Img<T> &image,const T *&a,const T *&b){
    // {{{ open
    a = image.getData(0);
    b = image.getData(1);
  }

  // }}}

  template<class S, class D>
  void cc_3x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C2*Cast<S,icl8u>::cast(s1[i]) + C1*Cast<S,icl8u>::cast(s2[i]) + Cast<S,icl8u>::cast(s3[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
        d3[i] = Cast<icl8u,D>::cast(l3[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_3x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C2*Cast<S,icl8u>::cast(s1[i]) + C1*Cast<S,icl8u>::cast(s2[i]) + Cast<S,icl8u>::cast(s3[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_3x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C2*Cast<S,icl8u>::cast(s1[i]) + C1*Cast<S,icl8u>::cast(s2[i]) + Cast<S,icl8u>::cast(s3[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_2x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C1*Cast<S,icl8u>::cast(s1[i]) + Cast<S,icl8u>::cast(s2[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
        d3[i] = Cast<icl8u,D>::cast(l3[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_2x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C1*Cast<S,icl8u>::cast(s1[i]) + Cast<S,icl8u>::cast(s2[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_2x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = C1*Cast<S,icl8u>::cast(s1[i]) + Cast<S,icl8u>::cast(s2[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_1x3(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = Cast<S,icl8u>::cast(s1[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
        d3[i] = Cast<icl8u,D>::cast(l3[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_1x2(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open
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
        idx = Cast<S,icl8u>::cast(s1[i]);
        d1[i] = Cast<icl8u,D>::cast(l1[idx]);
        d2[i] = Cast<icl8u,D>::cast(l2[idx]);
      }
    }
  }

  // }}}

  template<class S, class D>
  void cc_1x1(const Img<S> &src, Img<D> &dst, Img8u &lut, bool roiOnly){
    // {{{ open

    if(roiOnly){
      ERROR_LOG("not yet implemented with roi support!");
    }else{
      const S *s1 = src.getData(0);
      D *d1 = dst.getData(0);
      icl8u *l1 = lut.getData(0);
      
      const int DIM = src.getDim();
      for(int i=0;i<DIM;i++){
        d1[i] = Cast<icl8u,D>::cast(l1[Cast<S,icl8u>::cast(s1[i])]);
      }
    }
  }

  // }}}

  
  inline Img8u create_lut_3X(format srcFmt, format dstFmt){
    // {{{ open
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

  // }}}

  inline Img8u create_lut_2X(format srcFmt, format dstFmt){
    // {{{ open

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

  // }}}

  inline Img8u create_lut_1X(format srcFmt, format dstFmt){
    // {{{ open
    Img8u bufSrc(Size(C1,1),srcFmt);
    Img8u bufDst(Size(C1,1),dstFmt);
    
    copy(ROW, ROW+C1,bufSrc.getData(0));
    
    icl::cc(&bufSrc,&bufDst);
    return bufDst;
  }  

  // }}}
  

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

namespace icl{
  CCLUT::CCLUT(format srcFmt, format dstFmt):
    m_eSrcFmt(srcFmt),m_eDstFmt(dstFmt){
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
}

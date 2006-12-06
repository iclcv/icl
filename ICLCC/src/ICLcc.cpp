#ifndef ICLIMAGE_COLOR_CONVERSION_CPP
#define ICLIMAGE_COLOR_CONVERSION_CPP

#include <ICLcc.h>
#include <math.h>

namespace icl{

#ifdef WITH_IPP_OPTIMIZATION
// {{{ convert,set, and copy channels using IPP#define CONVERT_8U32F(S,SC,D,DC) ippiConvert_8u32f_C1R(S->asImg<icl8u>()->getData(SC),S->getLineStep(),D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize())#define CONVERT_32F8U(S,SC,D,DC) ippiConvert_32f8u_C1R(S->asImg<icl32f>()->getData(SC),S->getLineStep(),D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize(),ippRndZero)   #define SET_32F(V,D,DC) ippiSet_32f_C1R(V,D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize());#define SET_8U(V,D,DC) ippiSet_8u_C1R(V,D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize());#define COPY_8U(S,SC,D,DC) ippiCopy_8u_C1R(S->asImg<icl8u>()->getData(SC),S->getLineStep(),D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize())  #define COPY_32F(S,SC,D,DC) ippiCopy_32f_C1R(S->asImg<icl32f>()->getData(SC),S->getLineStep(),D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize())  // }}}
#else
// {{{ convert,set, and copy channels using C++#define CONVERT_8U32F(S,SC,D,DC)                                                        \{                                                                                       \  icl8u *pucSrc = S->asImg<icl8u>()->getData(SC);                                       \  icl32f *pfDst = D->asImg<icl32f>()->getData(DC);                                      \  icl32f *pfDstEnd = pfDst+D->getDim();                                                 \  while(pfDst < pfDstEnd) *pfDst++ = static_cast<icl32f>(*pucSrc++);                    \}#define CONVERT_32F8U(S,SC,D,DC)                                                        \{                                                                                       \  icl32f *fSrc = S->asImg<icl32f>()->getData(SC);                                       \  icl8u *pucDst = D->asImg<icl8u>()->getData(DC);                                       \  icl8u *pucDstEnd = pucDst+D->getDim();                                                \  while(pucDst < pucDstEnd) *pucDst++ = static_cast<icl8u>(*fSrc++);                    \}#define SET_32F(V,D,DC) D->asImg<icl32f>()->clear(DC,V)#define SET_8U(V,D,DC) D->asImg<icl8u>()->clear(DC,V)#define COPY_8U(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getDim()*sizeof(icl8u))#define COPY_32F(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getDim()*sizeof(icl32f))// }}}
#endif

static const icl8u ucDefaultHue = 0;
static const icl8u ucDefaultSaturation = 0;
static const icl32f fDefaultHue = ucDefaultHue;
static const icl32f fDefaultSaturation = ucDefaultSaturation;
  
void iclcc(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  ICLASSERT_RETURN (poDst->getSize() == poSrc->getSize());  if(poSrc->getFormat()==formatMatrix || poDst->getFormat()==formatMatrix){    printf("error in iclcc: formatMatrix is not allowed here!\n");    return;  }  if(poSrc->getDepth()==depth8u){    switch(poSrc->getFormat()){      case formatGray: convertFromGray8(poDst,poSrc); break;      case formatRGB: convertFromRGB8(poDst,poSrc); break;      case formatHLS: convertFromHLS8(poDst,poSrc); break;      case formatLAB: convertFromLAB8(poDst,poSrc); break;      case formatYUV: convertFromYUV8(poDst,poSrc); break;      default: return;    }  }else{    switch(poSrc->getFormat()){      case formatGray: convertFromGray32(poDst,poSrc); break;      case formatRGB: convertFromRGB32(poDst,poSrc); break;      case formatHLS: convertFromHLS32(poDst,poSrc); break;      case formatLAB: convertFromLAB32(poDst,poSrc); break;      case formatYUV: convertFromYUV32(poDst,poSrc); break;      default: return;    }  }  poDst->setROI (poSrc->getROI());}// }}}

void convertFromGray8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        COPY_8U(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_8U(poSrc,0,poDst,0);        COPY_8U(poSrc,0,poDst,1);        COPY_8U(poSrc,0,poDst,2);        break;      case formatHLS:        SET_8U(ucDefaultHue,poDst,0);        COPY_8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:        CONVERT_8U32F(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_8U32F(poSrc,0,poDst,0);        CONVERT_8U32F(poSrc,0,poDst,1);        CONVERT_8U32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        CONVERT_8U32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}  // }}}
  
void convertFromGray32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        CONVERT_32F8U(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_32F8U(poSrc,0,poDst,0);        CONVERT_32F8U(poSrc,0,poDst,1);        CONVERT_32F8U(poSrc,0,poDst,2);        break;      case formatHLS:         SET_8U(ucDefaultHue,poDst,0);        CONVERT_32F8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:       COPY_32F(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_32F(poSrc,0,poDst,0);        COPY_32F(poSrc,0,poDst,1);        COPY_32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        COPY_32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}   // }}}
 
void convertFromRGB8(ImgBase *poDstImage, ImgBase *poSrcImage){
  // {{{ open    icl8u *pR = poSrcImage->asImg<icl8u>()->getData(0);    icl8u *pG = poSrcImage->asImg<icl8u>()->getData(1);    icl8u *pB = poSrcImage->asImg<icl8u>()->getData(2);        if(poDstImage->getDepth() == depth8u){      switch(poDstImage->getFormat()){        case formatGray:{ //no ROI          register icl8u *poDst = poDstImage->asImg<icl8u>()->getData(0);          register icl8u *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=((*pR++)+(*pG++)+(*pB++))/3;           }          break;        }        case formatRGB:          COPY_8U(poSrcImage,0,poDstImage,0);          COPY_8U(poSrcImage,1,poDstImage,1);          COPY_8U(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);            register icl8u *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }          break;        case formatChroma:        {          register int sum = 0;          register icl8u *pChromaR = poDstImage->asImg<icl8u>()->getData(0);          register icl8u *pChromaG = poDstImage->asImg<icl8u>()->getData(1);          register icl8u *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=((*pR * 255) / sum);            *pChromaG++=((*pG * 255) / sum);                        pR++; pG++; pB++;            }                  break;        }        case formatYUV:        case formatLAB:        default:          ERROR_LOG("unsupported format!");      }    }else{//depth32f      switch(poDstImage->getFormat()){        case formatGray:{          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=(icl32f)((*pR++)+(*pG++)+(*pB++))/3.0;          }          break;        }        case formatRGB:          CONVERT_8U32F(poSrcImage,0,poDstImage,0);          CONVERT_8U32F(poSrcImage,1,poDstImage,1);          CONVERT_8U32F(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);            register icl32f *pHEnd = pH+poDstImage->getDim();                        while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pS++,*pL++);            }          }          break;        case formatChroma:        {          register int sum = 0;          register icl32f *pChromaR = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *pChromaG = poDstImage->asImg<icl32f>()->getData(1);          register icl32f *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=((*pR * 255) / sum);            *pChromaG++=((*pG * 255) / sum);                        pR++; pG++; pB++;          }          break;        }        case formatYUV:        case formatLAB:        default:          ERROR_LOG("unsupported format!");      }    }}  // }}}

void convertFromRGB32(ImgBase *poDstImage, ImgBase *poSrcImage){
  // {{{ open    icl32f *pR = poSrcImage->asImg<icl32f>()->getData(0);    icl32f *pG = poSrcImage->asImg<icl32f>()->getData(1);    icl32f *pB = poSrcImage->asImg<icl32f>()->getData(2);    if(poDstImage->getDepth() == depth8u){          switch(poDstImage->getFormat()){        case formatGray:{ //no ROI          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=(icl8u)(((*pR++)+(*pG++)+(*pB++))/3);          }          break;        }        case formatRGB:          CONVERT_32F8U(poSrcImage,0,poDstImage,0);          CONVERT_32F8U(poSrcImage,1,poDstImage,1);          CONVERT_32F8U(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);            register icl8u *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatChroma:        {          register float sum = 0;          register icl8u *pChromaR = poDstImage->asImg<icl8u>()->getData(0);          register icl8u *pChromaG = poDstImage->asImg<icl8u>()->getData(1);          register icl8u *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=Cast<icl32f,icl8u>::cast((*pR * 255) / sum);            *pChromaG++=Cast<icl32f,icl8u>::cast((*pG * 255) / sum);            pR++; pG++; pB++;          }                    break;        }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format !");      }    }else{      switch(poDstImage->getFormat()){        case formatGray:          {            register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *poDstEnd = poDst+poDstImage->getDim();            while(poDst!=poDstEnd){              *poDst++=((*pR++)+(*pG++)+(*pB++))/3.0;            }            break;          }        case formatRGB:          COPY_32F(poSrcImage,0,poDstImage,0);          COPY_32F(poSrcImage,1,poDstImage,1);          COPY_32F(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);            register icl32f *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatChroma:        {          register float sum = 0;          register icl32f *pChromaR = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *pChromaG = poDstImage->asImg<icl32f>()->getData(1);          register icl32f *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=((*pR * 255) / sum);            *pChromaG++=((*pG * 255) / sum);                        pR++; pG++; pB++;          }          break;        }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format!");          break;      }    }}  // }}}

void convertFromRGBA8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  convertFromRGB8(poDst,poSrc);  }  // }}}

void convertFromRGBA32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open    convertFromRGB32(poDst,poSrc);  }  // }}}

void convertFromLAB8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromLAB32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromYUV8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromYUV32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromHLS8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open
#ifdef WITH_IPP_OPTIMIZATION

  if(poDst->getFormat() == formatRGB){
    icl8u *buffer = new icl8u[poDst->getDim()*3];
    icl8u *buffer2 = new icl8u[poDst->getDim()*3];
    planarToInterleaved(poSrc->asImg<icl8u>(),buffer,Point::zero);
    ippiHLSToRGB_8u_C3R(buffer,poSrc->getLineStep()*3, buffer2,poSrc->getLineStep()*3,poSrc->getSize());
    interleavedToPlanar(buffer2,poDst->getSize(), 3, poDst->asImg<icl8u>());
    delete [] buffer;
    delete [] buffer2;
  }else{
    ERROR_LOG("converting from hls is not yet supported!");
  }
#else
  (void)poDst;
  (void)poSrc;
  ERROR_LOG("converting from hls is not yet supported!");
#endif
}

  // }}}

void convertFromHLS32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open
#ifdef WITH_IPP_OPTIMIZATION

  if(poDst->getFormat() == formatRGB){
    icl32f *buffer = new icl32f[poDst->getDim()*3];
    icl32f *buffer2 = new icl32f[poDst->getDim()*3];
    planarToInterleaved(poSrc->asImg<icl32f>(),buffer,Point::zero);
    ippiHLSToRGB_32f_C3R(buffer,poSrc->getLineStep()*3, buffer2,poSrc->getLineStep()*3,poSrc->getSize());
    interleavedToPlanar(buffer2,poDst->getSize(), 3, poDst->asImg<icl32f>());
    delete [] buffer;
    delete [] buffer2;
  }else{
    ERROR_LOG("converting from hls is not yet supported!");
  }
#else
  (void)poDst;
  (void)poSrc;
  ERROR_LOG("converting from hls is not yet supported!");
#endif
}

  // }}}
    
void convertYUV420ToRGB8(Img8u *poDst, unsigned char *pucSrc,const Size &s){
  // {{{ open#ifdef WITH_IPP_OPTIMIZATION  icl8u *apucSrc[] = {pucSrc,pucSrc+s.getDim(), pucSrc+s.getDim()+s.getDim()/4};  icl8u *apucDst[] = {poDst->getData(0),poDst->getData(1),poDst->getData(2)};  ippiYUV420ToRGB_8u_P3(apucSrc,apucDst,s); #else  // allocate memory for lookup tables  static float fy_lut[256];  static float fu_lut[256];  static float fv_lut[256];  static int r_lut[65536];  static int b_lut[65536];  static float g_lut1[65536];  static float g_lut2[256];  static int iInitedFlag=0;  // initialize to lookup tables  if(!iInitedFlag){    float fy,fu,fv;    for(int i=0;i<256;i++){      fy_lut[i] = (255* (i - 16)) / 219;      fu_lut[i] = (127 * (i - 128)) / 112;      fv_lut[i] = (127 * (i - 128)) / 112;    }        for(int v=0;v<256;v++){      g_lut2[v] = 0.714 * fv_lut[v];    }        for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int u=0;u<256;u++){	g_lut1[y+256*u] = fy - 0.344 * fu_lut[u];      }    }          for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int v=0;v<256;v++){	fv = fv_lut[v];	r_lut[y+256*v]= (int)( fy + (1.402 * fv) );	fu = fu_lut[v];	b_lut[y+256*v]= (int)( fy + (1.772 * fu) );       }    }        iInitedFlag = 1;  }  // creating temporary pointer for fast data access  int iW = s.width;  int iH = s.height;      icl8u *pucR = poDst->getData(0);  icl8u *pucG = poDst->getData(1);  icl8u *pucB = poDst->getData(2);  icl8u *ptY = pucSrc;  icl8u *ptU = ptY+iW*iH;  icl8u *ptV = ptU+(iW*iH)/4;    register int r,g,b,y,u,v;    register int Xflag=0;  register int Yflag=1;  register int w2 = iW/2;    // converting the image (ptY,ptU,ptV)----->(pucR,pucG,pucB)  for(int yy=iH-1; yy >=0 ; yy--){    for(int xx=0; xx < iW; xx++){      u=*ptU;      v=*ptV;      y=*ptY;            r = r_lut[y+256*v];      g = (int) ( g_lut1[y+256*u] - g_lut2[v]);      b = b_lut[y+256*u];            #define LIMIT(x) (x)>255?255:(x)<0?0:(x);      *pucR++=LIMIT(r);      *pucG++=LIMIT(g);      *pucB++=LIMIT(b);      #undef LIMIT            if(Xflag++){        ptV++;        ptU++;        Xflag=0;      }      ptY++;    }    if(Yflag++){      ptU -= w2;      ptV -= w2;      Yflag = 0;    }  }#endif}    // }}}
  
void convertToARGB32Interleaved(unsigned char *pucDst, Img8u *poSrc){
  // {{{ open  if(!poSrc || poSrc->getChannels() != 4 ) return;#ifdef WITH_IPP_OPTIMIZATION  icl8u* apucChannels[4]={poSrc->getData(0),poSrc->getData(1),poSrc->getData(2),poSrc->getData(3)};  ippiCopy_8u_P4C4R(apucChannels,poSrc->getLineStep(),pucDst,poSrc->getLineStep()*4,poSrc->getSize());#else  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst, Img8u *poSrc) not yet implemented \n");#endif}   // }}}

void convertToARGB32Interleaved(unsigned char *pucDst, Img32f *poSrc, Img8u *poBuffer){
  // {{{ open  if(!poSrc || poSrc->getChannels() != 4 ) return;#ifdef WITH_IPP_OPTIMIZATION  poSrc->convertTo<icl8u>(poBuffer);  icl8u* apucChannels[4]={poBuffer->getData(0),poBuffer->getData(1),poBuffer->getData(2),poBuffer->getData(3)};  ippiCopy_8u_P4C4R(apucChannels,poBuffer->getLineStep(),pucDst,poBuffer->getLineStep()*4,poBuffer->getSize());#else  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst,"         " Img32f *poSrc, Img8u* poBuffer) not yet implemented \n");#endif}   // }}}
//template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, icl8u*, int, IppiSize, IppCmpOp)>


template<typename T>
inline void fallbackInterleavedToPlanar(const T *src, const Size &srcSize, int c,  Img<T> *dst){
  printf("fallbackInterleavedToPlanar\n");
	ICLASSERT_RETURN(src);
	ICLASSERT_RETURN(dst);
	dst->setChannels(c);
	dst->setSize(srcSize);
	T** pp=new T* [c];
	T** ppEnd=pp+c;
	for (int i=0;i<c;i++){
		pp[i]=dst->getData(i);
	}
	const T* srcEnd=src+srcSize.getDim()*c;
	while (src<srcEnd){
		for (T** p=pp;p<ppEnd;++p,++src){
			*(*p)=*src;
		}
	}
  delete [] pp;
}


template<typename T>
inline void fallbackPlanarToInterleaved(const Img<T> *src, T *dst){
  printf("fallbackPlanarToInterleaved\n");
	ICLASSERT_RETURN(src);
	ICLASSERT_RETURN(dst);
	int c=src->getChannels();
	int dim=src->getSize().getDim();
  printf("a\n");
	T** pp=new T* [c];
	T** ppEnd=pp+c;
  printf("b\n");
	for (int i=0;i<c;i++){
		pp[i]=src->getData(i);
	}
  printf("c\n");
	T* dstEnd=dst+c*dim;
	while (dst<dstEnd){
		for (T** p=pp;p<ppEnd;++p,++dst){
      printf(",\n");
			*dst=*(*p);
		}
    printf(".\n");
	}
  printf("d\n");
	delete [] pp;
}



void interleavedToPlanar(const icl8u *src, const Size &srcSize, int srcChannels,  Img8u *dst){  
#ifdef WITH_IPP_OPTIMIZATION
ICLASSERT_RETURN(srcChannels>0);
	dst->setChannels(srcChannels);
switch(srcChannels){
	case 1:
		ippiCopy_8u_C1R(src,srcSize.width,dst->getData(0),srcSize.width,srcSize);
		break;
	case 3: {
		icl8u* apucChannels[3]={dst->getData(0),dst->getData(1),dst->getData(2)};
		ippiCopy_8u_C3P3R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);
		break;
	}
	case 4: {
		icl8u* apucChannels[4]={dst->getData(0),dst->getData(1),dst->getData(2),dst->getData(3)};
		ippiCopy_8u_C4P4R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);
		break;
	}
	default:
		fallbackInterleavedToPlanar(src,srcSize,srcChannels,dst);
		break;
}
#else
  fallbackInterleavedToPlanar(src,srcSize,srcChannels,dst);
#endif

}
// 		fallbackInterleavedToPlanar(src,srcSize,srcChannels,dst);  macht noch ein seg fault;
//		fallbackPlanarToInterleaved(src,dst); auch!

void interleavedToPlanar(const icl32f *src, const Size &srcSize, int srcChannels,  Img32f *dst){
#ifdef WITH_IPP_OPTIMIZATION
ICLASSERT_RETURN(srcChannels>0);
	dst->setChannels(srcChannels);
switch(srcChannels){
	case 1:
		ippiCopy_32f_C1R(src,srcSize.width,dst->getData(0),srcSize.width,srcSize);
		break;
	case 3: {
		icl32f* apucChannels[3]={dst->getData(0),dst->getData(1),dst->getData(2)};
		ippiCopy_32f_C3P3R(src,srcSize.width*srcChannels*sizeof(icl32f),apucChannels,srcSize.width*sizeof(icl32f),srcSize);
		break;
	}
	case 4: {
		icl32f* apucChannels[4]={dst->getData(0),dst->getData(1),dst->getData(2),dst->getData(3)};
		ippiCopy_32f_C4P4R(src,srcSize.width*srcChannels*sizeof(icl32f),apucChannels,srcSize.width*sizeof(icl32f),srcSize);
		break;
	}
	default:
		fallbackInterleavedToPlanar(src,srcSize,srcChannels,dst);
		break;
}
#else
  fallbackInterleavedToPlanar(src,srcSize,srcChannels,dst);
#endif

}

void planarToInterleaved(const Img8u *src, icl8u *dst,const Point ROIoffset){

#ifdef WITH_IPP_OPTIMIZATION
    ICLASSERT_RETURN(src->getChannels()>0);
switch(src->getChannels()){
	case 1:
		ippiCopy_8u_C1R(src->getData(0),src->getSize().width,dst,src->getSize().width,src->getSize());
	case 3: { 
		icl8u* apucChannels[3]={src->getROIData(0,ROIoffset),src->getROIData(1,ROIoffset),src->getROIData(2,ROIoffset)};
		ippiCopy_8u_P3C3R(apucChannels,src->getLineStep(),dst,src->getLineStep()*3,src->getSize());
		break;
	}
	case 4: {
		icl8u* apucChannels[4]={src->getROIData(0,ROIoffset),src->getROIData(1,ROIoffset),src->getROIData(2,ROIoffset),src->getROIData(3,ROIoffset)};
		ippiCopy_8u_P4C4R(apucChannels,src->getLineStep(),dst,src->getLineStep()*4,src->getSize());
		break;
	}
	default:
		fallbackPlanarToInterleaved(src,dst);
		break;
}
#else
  fallbackPlanarToInterleaved(src,dst);
#endif
}

void planarToInterleaved(const Img32f *src, icl32f *dst,const Point ROIoffset){

#ifdef WITH_IPP_OPTIMIZATION
    ICLASSERT_RETURN(src->getChannels()>0);
switch(src->getChannels()){
	case 1:
		ippiCopy_32f_C1R(src->getData(0),src->getSize().width,dst,src->getSize().width,src->getSize());
	case 3: { 
		icl32f* apucChannels[3]={src->getROIData(0,ROIoffset),src->getROIData(1,ROIoffset),src->getROIData(2,ROIoffset)};
		ippiCopy_32f_P3C3R(apucChannels,src->getLineStep(),dst,src->getLineStep()*3,src->getROISize());
		break;
	}
	case 4: {
		icl32f* apucChannels[4]={src->getData(0),src->getData(1),src->getData(2),src->getData(3)};
		ippiCopy_32f_P4C4R(apucChannels,src->getLineStep(),dst,src->getLineStep()*4,src->getSize());
		break;
	}
	default:
		fallbackPlanarToInterleaved(src,dst);
		break;
}
#else
  fallbackPlanarToInterleaved(src,dst);
#endif
}




static unsigned char aucHlsTable[257];
static float afHlsTable[257];
  
/// intern utility function
void get_min_and_max(const unsigned char &r,const unsigned char &g,const unsigned char&b,unsigned char &min,unsigned char &max){
  // {{{ open    if(r<g) {      min=r;      max=g;    }    else {      min=g;      max=r;	    }    if(b<min)      min=b;    else {      max = b>max ? b : max;    }  }    // }}}



void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, unsigned char &h, unsigned char &l, unsigned char &s){
  // {{{ open    static int RG,RB,nom,denom, cos2Hx256;    static unsigned char min;    get_min_and_max(r, g, b,min,l);    s = l>0 ? 255-((255*min)/l) : 0;      //Hue    RG = r - g;    RB = r - b;    nom = (RG+RB)*(RG+RB);    denom = (RG*RG+RB*(g-b))<<2;      cos2Hx256 = denom>0 ? (nom<<8)/denom : 0;    h=aucHlsTable[cos2Hx256];    if ((RG+RB)<0) h=127-h;    if (b>g) h=255-h;  }  // }}}

void rgb_to_hls(const float &r,const float &g,const float &b, unsigned char &h, unsigned char &l, unsigned char &s){
  // {{{ open    rgb_to_hls((unsigned char)(r),(unsigned char)(g),(unsigned char)(b),h,l,s);    }  // }}}
void rgb_to_hls(const float &r,const float &g,const float &b, float &h, float &l, float &s){
  // {{{ open    static unsigned char H,L,S;    rgb_to_hls((unsigned char)(r),(unsigned char)(g),(unsigned char)(b),H,L,S);    h=(float)H;    l=(float)L;    s=(float)S;  }  // }}}
void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, float &h, float &l, float &s){
  // {{{ open    static unsigned char H,L,S;    rgb_to_hls(r,g,b,H,L,S);    h=(float)H;    l=(float)L;    s=(float)S;  }  // }}}

void init_table() {
  // {{{ open    static int inited = 0;    if(inited) return;    for(int i=0 ; i<256 ; i++){      aucHlsTable[i] = static_cast<unsigned char>(acos(sqrt((float)i/256.0)) * 2 * 63 / M_PI);      afHlsTable[i] = (float)aucHlsTable[i];//(255.0*360.0); or scale to 360° ?    }    aucHlsTable[256]=aucHlsTable[255];    afHlsTable[256]=afHlsTable[255];    inited = 1;  }  // }}}


}//namespace icl

#endif


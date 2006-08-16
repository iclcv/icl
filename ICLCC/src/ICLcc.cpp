#ifndef ICLIMAGE_COLOR_CONVERSION_CPP
#define ICLIMAGE_COLOR_CONVERSION_CPP

#include "ICLcc.h"
#include <math.h>

namespace icl{

#ifdef WITH_IPP_OPTIMIZATION
// {{{ convert,set, and copy channels using IPP#define CONVERT_8U32F(S,SC,D,DC) ippiConvert_8u32f_C1R(S->asImg<icl8u>()->getROIData(SC),S->getLineStep(),D->asImg<icl32f>()->getROIData(DC),D->getLineStep(),D->getROISize())#define CONVERT_32F8U(S,SC,D,DC) ippiConvert_32f8u_C1R(S->asImg<icl32f>()->getROIData(SC),S->getLineStep(),D->asImg<icl8u>()->getROIData(DC),D->getLineStep(),D->getROISize(),ippRndZero)   #define SET_32F(V,D,DC) ippiSet_32f_C1R(V,D->asImg<icl32f>()->getROIData(DC),D->getLineStep(),D->getROISize());#define SET_8U(V,D,DC) ippiSet_8u_C1R(V,D->asImg<icl8u>()->getROIData(DC),D->getLineStep(),D->getROISize());#define COPY_8U(S,SC,D,DC) ippiCopy_8u_C1R(S->asImg<icl8u>()->getROIData(SC),S->getLineStep(),D->asImg<icl8u>()->getROIData(DC),D->getLineStep(),D->getROISize())  #define COPY_32F(S,SC,D,DC) ippiCopy_32f_C1R(S->asImg<icl32f>()->getROIData(SC),S->getLineStep(),D->asImg<icl32f>()->getROIData(DC),D->getLineStep(),D->getROISize())  // }}}
#else
// {{{ convert,set, and copy channels using C++#define CONVERT_8U32F(S,SC,D,DC)                                                        \    for(int x=0;x<S->getWidth();x++){                                                   \       for(int y=0;y<S->getHeight();y++){                                               \         (*(S->asImg<icl8u>()))(x,y,SC)=static_cast<icl8u>((*(D->asImg<icl32f>()))(x,y,DC));      \       }                                                                                \    }#define CONVERT_32F8U(S,SC,D,DC)                                                        \    for(int x=0;x<S->getWidth();x++){                                                   \       for(int y=0;y<S->getHeight();y++){                                               \         (*(S->asImg<icl32f>()))(x,y,SC)=static_cast<icl32f>((*(D->asImg<icl8u>()))(x,y,DC));     \       }                                                                                \    }#define SET_32F(V,D,DC) D->asImg<icl32f>()->clear(DC,V)#define SET_8U(V,D,DC) D->asImg<icl8u>()->clear(DC,V)#define COPY_8U(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getWidth()*D->getHeight()*sizeof(icl8u))#define COPY_32F(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getWidth()*D->getHeight()*sizeof(icl32f))// }}}
#endif

static const icl8u ucDefaultHue = 0;
static const icl8u ucDefaultSaturation = 0;
static const icl32f fDefaultHue = ucDefaultHue;
static const icl32f fDefaultSaturation = ucDefaultSaturation;
  
void iclcc(ImgI *poDst, ImgI *poSrc){
  // {{{ open  if(poSrc->getFormat()==formatMatrix || poDst->getFormat()==formatMatrix){    printf("error in iclcc: formatMatrix is not allowed here!\n");    return;  }  if(poSrc->getDepth()==depth8u){    switch(poSrc->getFormat()){      case formatGray: convertFromGray8(poDst,poSrc); break;      case formatRGB: convertFromRGB8(poDst,poSrc); break;      case formatHLS: convertFromHLS8(poDst,poSrc); break;      case formatLAB: convertFromLAB8(poDst,poSrc); break;      case formatYUV: convertFromYUV8(poDst,poSrc); break;      default: return;    }  }else{    switch(poSrc->getFormat()){      case formatGray: convertFromGray32(poDst,poSrc); break;      case formatRGB: convertFromRGB32(poDst,poSrc); break;      case formatHLS: convertFromHLS32(poDst,poSrc); break;      case formatLAB: convertFromLAB32(poDst,poSrc); break;      case formatYUV: convertFromYUV32(poDst,poSrc); break;      default: return;    }  }}// }}}

void convertFromGray8(ImgI *poDst, ImgI *poSrc){
  // {{{ open  if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        COPY_8U(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_8U(poSrc,0,poDst,0);        COPY_8U(poSrc,0,poDst,1);        COPY_8U(poSrc,0,poDst,2);        break;      case formatHLS:        SET_8U(ucDefaultHue,poDst,0);        COPY_8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:        CONVERT_8U32F(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_8U32F(poSrc,0,poDst,0);        CONVERT_8U32F(poSrc,0,poDst,1);        CONVERT_8U32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        CONVERT_8U32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}  // }}}
  
void convertFromGray32(ImgI *poDst, ImgI *poSrc){
  // {{{ open if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        CONVERT_32F8U(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_32F8U(poSrc,0,poDst,0);        CONVERT_32F8U(poSrc,0,poDst,1);        CONVERT_32F8U(poSrc,0,poDst,2);        break;      case formatHLS:         SET_8U(ucDefaultHue,poDst,0);        CONVERT_32F8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:       COPY_32F(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_32F(poSrc,0,poDst,0);        COPY_32F(poSrc,0,poDst,1);        COPY_32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        COPY_32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}   // }}}
 
void convertFromRGB8(ImgI *poDstImage, ImgI *poSrcImage){
  // {{{ open    icl8u *pR = poSrcImage->asImg<icl8u>()->getData(0);    icl8u *pG = poSrcImage->asImg<icl8u>()->getData(1);    icl8u *pB = poSrcImage->asImg<icl8u>()->getData(2);        if(poDstImage->getDepth() == depth8u){      switch(poDstImage->getFormat()){        case formatGray:{ //no ROI          register icl8u *poDst = poDstImage->asImg<icl8u>()->getData(0);          register icl8u *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=((*pR++)+(*pG++)+(*pB++))/3;           }          break;        }        case formatRGB:          COPY_8U(poSrcImage,0,poDstImage,0);          COPY_8U(poSrcImage,1,poDstImage,1);          COPY_8U(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);            register icl8u *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }          break;        case formatYUV:        case formatLAB:        default:          ERROR_LOG("unsupported format!");      }    }else{//depth32f      switch(poDstImage->getFormat()){        case formatGray:{          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=(icl32f)((*pR++)+(*pG++)+(*pB++))/3.0;          }          break;        }        case formatRGB:          CONVERT_8U32F(poSrcImage,0,poDstImage,0);          CONVERT_8U32F(poSrcImage,1,poDstImage,1);          CONVERT_8U32F(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);            register icl32f *pHEnd = pH+poDstImage->getDim();                        while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pS++,*pL++);            }          }          break;        case formatYUV:        case formatLAB:        default:          ERROR_LOG("unsupported format!");      }    }}  // }}}

void convertFromRGB32(ImgI *poDstImage, ImgI *poSrcImage){
  // {{{ open    icl32f *pR = poSrcImage->asImg<icl32f>()->getData(0);    icl32f *pG = poSrcImage->asImg<icl32f>()->getData(1);    icl32f *pB = poSrcImage->asImg<icl32f>()->getData(2);    if(poDstImage->getDepth() == depth8u){          switch(poDstImage->getFormat()){        case formatGray:{ //no ROI          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=(icl8u)(((*pR++)+(*pG++)+(*pB++))/3);          }          break;        }        case formatRGB:          CONVERT_32F8U(poSrcImage,0,poDstImage,0);          CONVERT_32F8U(poSrcImage,1,poDstImage,1);          CONVERT_32F8U(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);            register icl8u *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format !");      }    }else{      switch(poDstImage->getFormat()){        case formatGray:          {            register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *poDstEnd = poDst+poDstImage->getDim();            while(poDst!=poDstEnd){              *poDst++=((*pR++)+(*pG++)+(*pB++))/3.0;            }            break;          }        case formatRGB:          COPY_32F(poSrcImage,0,poDstImage,0);          COPY_32F(poSrcImage,1,poDstImage,1);          COPY_32F(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);            register icl32f *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format!");          break;      }    }}  // }}}

void convertFromRGBA8(ImgI *poDst, ImgI *poSrc){
  // {{{ open  convertFromRGB8(poDst,poSrc);  }  // }}}

void convertFromRGBA32(ImgI *poDst, ImgI *poSrc){
  // {{{ open    convertFromRGB32(poDst,poSrc);  }  // }}}

void convertFromLAB8(ImgI *poDst, ImgI *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromLAB32(ImgI *poDst, ImgI *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromYUV8(ImgI *poDst, ImgI *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromYUV32(ImgI *poDst, ImgI *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromHLS8(ImgI *poDst, ImgI *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from hls is not yet supported!");}  // }}}

void convertFromHLS32(ImgI *poDst, ImgI *poSrc){
  // {{{ open    (void)poDst;    (void)poSrc;    ERROR_LOG("converting from hls is not yet supported!");}  // }}}
  

  
void convertYUV420ToRGB8(Img8u *poDst, unsigned char *pucSrc,const Size &s){
  // {{{ open#ifdef WITH_IPP_OPTIMIZATION  icl8u *apucSrc[] = {pucSrc,pucSrc+s.getDim(), pucSrc+s.getDim()+s.getDim()/4};  icl8u *apucDst[] = {poDst->getData(0),poDst->getData(1),poDst->getData(2)};  ippiYUV420ToRGB_8u_P3(apucSrc,apucDst,s); #else  // allocate memory for lookup tables  static float fy_lut[256];  static float fu_lut[256];  static float fv_lut[256];  static int r_lut[65536];  static int b_lut[65536];  static float g_lut1[65536];  static float g_lut2[256];  static int iInitedFlag=0;  // initialize to lookup tables  if(!iInitedFlag){    float fy,fu,fv;    for(int i=0;i<256;i++){      fy_lut[i] = (255* (i - 16)) / 219;      fu_lut[i] = (127 * (i - 128)) / 112;      fv_lut[i] = (127 * (i - 128)) / 112;    }        for(int v=0;v<256;v++){      g_lut2[v] = 0.714 * fv_lut[v];    }        for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int u=0;u<256;u++){	g_lut1[y+256*u] = fy - 0.344 * fu_lut[u];      }    }          for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int v=0;v<256;v++){	fv = fv_lut[v];	r_lut[y+256*v]= (int)( fy + (1.402 * fv) );	fu = fu_lut[v];	b_lut[y+256*v]= (int)( fy + (1.772 * fu) );       }    }        iInitedFlag = 1;  }  // creating temporary pointer for fast data access  int iW = s.width;  int iH = s.height;      icl8u *pucR = poDst->getData(0);  icl8u *pucG = poDst->getData(1);  icl8u *pucB = poDst->getData(2);  icl8u *ptY = pucSrc;  icl8u *ptU = ptY+iW*iH;  icl8u *ptV = ptU+(iW*iH)/4;    register int r,g,b,y,u,v;    register int Xflag=0;  register int Yflag=1;  register int w2 = iW/2;    // converting the image (ptY,ptU,ptV)----->(pucR,pucG,pucB)  for(int yy=iH-1; yy >=0 ; yy--){    for(int xx=0; xx < iW; xx++){      u=*ptU;      v=*ptV;      y=*ptY;            r = r_lut[y+256*v];      g = (int) ( g_lut1[y+256*u] - g_lut2[v]);      b = b_lut[y+256*u];            #define LIMIT(x) (x)>255?255:(x)<0?0:(x);      *pucR++=LIMIT(r);      *pucG++=LIMIT(g);      *pucB++=LIMIT(b);      #undef LIMIT            if(Xflag++){        ptV++;        ptU++;        Xflag=0;      }      ptY++;    }    if(Yflag++){      ptU -= w2;      ptV -= w2;      Yflag = 0;    }  }#endif}    // }}}
  
  
  
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

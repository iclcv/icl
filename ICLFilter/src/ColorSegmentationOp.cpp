/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ColorSegmentationOp.cpp                  **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLCC/CCFunctions.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLIO/FileWriter.h>
#include <ICLIO/FileGrabber.h>
#include <ICLFilter/ColorSegmentationOp.h>

namespace icl{

  struct ColorSegmentationOp::LUT3D : public Uncopyable{
    int dim, w, h,t, wh;
    icl8u *data;
    mutable Img8u image;
    LUT3D(int w, int h, int t):
      dim(w*h*t),w(w),h(h),t(t),wh(w*h),data(dim ? new icl8u[dim] : 0){
    }
    ~LUT3D(){
      ICL_DELETE_ARRAY(data);
    }
    inline icl8u &operator()(int x, int y, int z){
      return data[x+w*y+wh*z];
    }
    inline const icl8u &operator()(int x, int y, int z) const{
      return data[x+w*y+wh*z];
    }

    void resize(int w, int h, int t){
      if(w*h*t != dim){
        ICL_DELETE_ARRAY(data);
        dim = w*h*t;
        wh = w*h;
        this->w = w;
        this->h = h;
        this->t = t;
        data = dim ? new icl8u[dim] : 0;
      }
    }
    
    void clear(icl8u value) {
      std::fill(data,data+dim,value);
    }
    
    void save(const std::string &filename){
      std::vector<icl8u*> datas(t);
      for(int i=0;i<t;++i){
        datas[i] = data + wh*i;
      }
      Img8u tmp(Size(w,h),t,formatMatrix,datas,false);
      FileWriter(filename).write(&tmp);
    }
    
    void load(const std::string &filename){
      FileGrabber g(filename);
      g.setIgnoreDesiredParams(true);
      const Img8u &image = *g.grab()->asImg<icl8u>();
      ICLASSERT_THROW(image.getWidth() == w &&
                      image.getHeight() == h &&
                      image.getChannels() == t, ICLException("invalid lut dimensions"));
      for(int i=0;i<t;++i){
        std::copy(image.begin(i),image.end(i),data+wh*i);
      }
    }
    
    const Img8u &getImage(int xDim, int yDim, int zSlice) const{
      ICLASSERT_THROW(zSlice >= 0 && zSlice < t,ICLException("invalid zSlice value"));
      const int dims[3]={w,h,t};
      int w = dims[xDim];
      int h = dims[yDim];
      image.setSize(Size(w,h));
      image.setChannels(1);
      Channel8u cim = image[0];

      if(xDim == 0){
        if(yDim == 0){
          std::copy(data+wh*zSlice,data+(wh+1)*zSlice,image.begin(0));
        }else{
#define IMPL_XY_LOOP(X,Y,Z)                     \
          for(int x=0;x<w;++x){                 \
            for(int y=0;y<h;++y){               \
              cim(x,y) = operator()(X,Y,Z);     \
            }                                   \
          }
          IMPL_XY_LOOP(x,zSlice,y);
        }
      }else if(xDim == 1){
        if(yDim == 0){
          IMPL_XY_LOOP(y,x,zSlice);
        }else if(yDim == 2){
          IMPL_XY_LOOP(zSlice,x,y);
        }
      }else if(xDim == 2){
        if(yDim == 0){
          IMPL_XY_LOOP(y,zSlice,x);
        }else{
          IMPL_XY_LOOP(zSlice,y,x);
        }
#undef IMPL_XY_LOOP
      }
      return image;
    }
  };

  template<icl8u xShift, icl8u yShift, icl8u zShift>
  struct ShiftedLUT3D_T{
    const ColorSegmentationOp::LUT3D &lut;
    ShiftedLUT3D_T(const ColorSegmentationOp::LUT3D &lut):lut(lut){}
    
    inline const icl8u &operator()(int x, int y, int z) const{
      return lut(x>>xShift,y>>yShift,z>>zShift);
    }

  };

  struct ShiftedLUT3D{
    int xShift,yShift,zShift;
    ColorSegmentationOp::LUT3D &lut;

    ShiftedLUT3D(int xShift, int yShift, int zShift, ColorSegmentationOp::LUT3D &lut):
      xShift(xShift),yShift(yShift),zShift(zShift),lut(lut){}
    
    ShiftedLUT3D(const icl8u *shifts, ColorSegmentationOp::LUT3D &lut):
      xShift(shifts[0]),yShift(shifts[1]),zShift(shifts[2]),lut(lut){}
    
    void resize(){
      lut.resize((0xFF>>xShift)+1,(0xFF>>yShift)+1,(0xFF>>zShift)+1);
    }
    
    inline icl8u &operator()(int x, int y, int z){
      return lut(x>>xShift,y>>yShift,z>>zShift);
    }

    inline const icl8u &operator()(int x, int y, int z) const{
      return lut(x>>xShift,y>>yShift,z>>zShift);
    }

  };

 template<class SrcIterator, class DstIterator, icl8u xShift, icl8u yShift, icl8u zShift>
  static void apply_lookup(SrcIterator a, SrcIterator b, SrcIterator c, DstIterator d, DstIterator dEnd, const ShiftedLUT3D_T<xShift,yShift,zShift> &lut){
    while(d!=dEnd){
      *d = lut(*a,*b,*c);
      ++a; ++b; ++c; ++d;
    }
  }

  ColorSegmentationOp::ColorSegmentationOp(icl8u c0shift, icl8u c1shift, icl8u c2shift, format fmt)  throw (ICLException):
    m_segFormat(fmt),m_lut(new LUT3D(0,0,0)){
    ICLASSERT_THROW(getChannelsOfFormat(fmt) == 3,ICLException("Construktor ColorSegmentationOp: format must be a 3-channel format"));
    setSegmentationShifts(c0shift,c1shift,c2shift);
  }

  ColorSegmentationOp::~ColorSegmentationOp(){
    ICL_DELETE(m_lut);
  }
  

  void ColorSegmentationOp::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_THROW(src,ICLException("ColorSegmentationOp::apply: source must not be null"));
    ICLASSERT_THROW(src->hasFullROI(), ICLException("ColorSegmentationOp::apply: source image has a ROI which is not supported yet!"));
    
    // preparing destination image
    if(!dst) dst = bpp(m_outputBuffer);
    if(getClipToROI()){
      bool ok = prepare(dst,depth8u,src->getROISize(),formatMatrix,1,Rect(Point::null,src->getROISize()),src->getTime());
      ICLASSERT_THROW(ok,ICLException("ColorSegmentationOp::apply: unable to prepare destination image"));
    }else{
      bool ok = prepare(dst,depth8u,src->getSize(),formatMatrix,1,src->getROI(),src->getTime());
      ICLASSERT_THROW(ok,ICLException("ColorSegmentationOp::apply: unable to prepare destination image"));
    }
    Img8u &dstRef = *(*dst)->asImg<icl8u>();
    
    // preparing source image
    if(src->getFormat() != m_segFormat || src->getDepth() != depth8u){
      m_inputBuffer.setFormat(m_segFormat);
      cc(src,&m_inputBuffer);
      src = &m_inputBuffer;
    }
    const Img8u &srcRef = *src->asImg<icl8u>();

    // we use cross-instantiated templates for better performance
#define SHIFT_2_CASE(SH0,SH1,SH2)                                       \
    case SH2: apply_lookup(srcRef.begin(0),srcRef.begin(1),srcRef.begin(2), \
                           dstRef.begin(0),dstRef.end(0),ShiftedLUT3D_T<SH0,SH1,SH2>(*m_lut)); break
                                                                            

    
#define SHIFT_2(SH0,SH1)                                                \
    switch(m_bitShifts[2]){                                             \
      SHIFT_2_CASE(SH0,SH1,0); SHIFT_2_CASE(SH0,SH1,1);                 \
      SHIFT_2_CASE(SH0,SH1,2); SHIFT_2_CASE(SH0,SH1,3);                 \
      SHIFT_2_CASE(SH0,SH1,4); SHIFT_2_CASE(SH0,SH1,5);                 \
      SHIFT_2_CASE(SH0,SH1,6); SHIFT_2_CASE(SH0,SH1,7);                 \
      default: apply_lookup(srcRef.begin(0),                            \
                            srcRef.begin(1),                            \
                            srcRef.begin(2),                            \
                            dstRef.begin(0),                            \
                            dstRef.end(0),                              \
                            ShiftedLUT3D_T<SH0,SH1,8>(*m_lut));  break; \
    }

#define SHIFT_1_CASE(SH0,SH1)                   \
    case SH1: SHIFT_2(SH0,SH1) break;
  

#define SHIFT_1(SH0)                                    \
      switch(m_bitShifts[1]){                           \
        SHIFT_1_CASE(SH0,0);SHIFT_1_CASE(SH0,1);        \
        SHIFT_1_CASE(SH0,2);SHIFT_1_CASE(SH0,3);        \
        SHIFT_1_CASE(SH0,4);SHIFT_1_CASE(SH0,5);        \
        SHIFT_1_CASE(SH0,6);SHIFT_1_CASE(SH0,7);        \
        default: SHIFT_2(SH0,8); break;                 \
      }


    switch(m_bitShifts[0]){
#define SHIFT_0_CASE(SH0) case SH0: SHIFT_1(SH0); break
      SHIFT_0_CASE(0); SHIFT_0_CASE(1); SHIFT_0_CASE(2); SHIFT_0_CASE(3);
      SHIFT_0_CASE(4); SHIFT_0_CASE(5); SHIFT_0_CASE(6); SHIFT_0_CASE(7);
      default: SHIFT_1(8); break;
    }
  }


  const Img8u &ColorSegmentationOp::getSegmentationPreview(){
    TODO_LOG("implemenent ColorSegmentationOp::getSegmentationPreview");
    return m_segPreview;
  }
    
  void ColorSegmentationOp::setSegmentationFormat(format fmt) throw (ICLException){
    ICLASSERT_THROW(getChannelsOfFormat(fmt) == 3, ICLException("ColorSegmentationOp::setSegmentationFormat: Segmentation format must have 3 channels"));
    m_segFormat = fmt;
  }  

  void ColorSegmentationOp::setSegmentationShifts(icl8u c0shift, icl8u c1shift, icl8u c2shift){
    m_bitShifts[0] = c0shift;
    m_bitShifts[1] = c1shift;
    m_bitShifts[2] = c2shift;
    ShiftedLUT3D lut(m_bitShifts,*m_lut);
    lut.resize();
  }
  
  void ColorSegmentationOp::lutEntry(icl8u a, icl8u b, icl8u c, icl8u rA, icl8u rB, icl8u rC, icl8u value){
    const int sa = pow(2, m_bitShifts[0]);
    const int sb = pow(2, m_bitShifts[1]);
    const int sc = pow(2, m_bitShifts[2]);
    ShiftedLUT3D lut(m_bitShifts,*m_lut);
   
    for(int ia=a-rA; ia<=a+rA; ia+=sa){
      //if(ia < 0) continue;
      //else if(ia > 255) break;
      for(int ib=b-rB; ib<=b+rB; ib+=sb){
        //if(ib < 0) continue;
        //else if(ib > 255); break;
        for(int ic=c-rC; ic<=c+rC; ic+=sc){
          //if(ic < 0) continue;
          //else if(ic > 255) break;
          if(ia >= 0 && ia < 256 && 
             ib >= 0 && ib < 256 && 
             ic >= 0 && ic < 256){
               lut(ia,ib,ic) = value;
          }
        }
      }
    }
  }

  void ColorSegmentationOp::lutEntry(format fmt, int a, int b, int c, int rA, int rB, int rC, icl8u value) throw (ICLException){
    if(fmt == m_segFormat) lutEntry(a,b,c,rA,rB,rC,value);
    ICLASSERT_THROW(getChannelsOfFormat(fmt) == 3, ICLException("ColorSegmentationOp::lutEntry format must have 3 channels"));
    Img8u src(Size(1,1),fmt),dst(Size(1,1),m_segFormat);
    src(0,0).set(a,b,c);
    cc(&src,&dst);
    
    //DEBUG_LOG("adding color: Y:" << (int)dst(0,0,0) << " U:" << (int)dst(0,0,1) << " V:" << (int)dst(0,0,2));
    lutEntry(dst(0,0,0),dst(0,0,1),dst(0,0,2),rA,rB,rC,value);
  }

  void ColorSegmentationOp::clearLUT(icl8u value){
    m_lut->clear(value);
  }

  const Img8u &ColorSegmentationOp::getLUTPreview(int xDim, int yDim, icl8u zValue){
    return m_lut->getImage(xDim,yDim,zValue>>m_bitShifts[2]);
  }

  void ColorSegmentationOp::load(const std::string &filename){
    try{
      m_lut->load(filename);
    }catch(ICLException &ex){
      ERROR_LOG(ex.what());
    }
  }
  void  ColorSegmentationOp::save(const std::string &filename) {
    m_lut->save(filename);
  }
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ColorSegmentationOp.cpp        **
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

#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/File.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLCore/Color.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
  
    class ColorSegmentationOp::LUT3D : public Uncopyable{
      public:
      int dim, w, h,t, wh;
      icl8u *data;
      mutable Img8u image;
      mutable Img8u colorImage;
      mutable std::vector<Color> classmeans;
      LUT3D(int w, int h, int t):
        dim(w*h*t),w(w),h(h),t(t),wh(w*h),data(dim ? new icl8u[dim] : 0){
        clear(0);
        classmeans.resize(256);
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
        wh = w*h;
        this->w = w;
        this->h = h;
        this->t = t;     
        
        if(w*h*t != dim){
          ICL_DELETE_ARRAY(data);
          dim = w*h*t;
          data = dim ? new icl8u[dim] : 0;
        }
        clear(0);
      }
      
      void clear(icl8u value) {
        std::fill(data,data+dim,value);
      }
      
      void save(const std::string &filename, format fmt){
        ICLASSERT_THROW(filename.length(),ICLException("ColorSegmentationOp::save: emtpy filename"));
        std::string fn = filename;
        int len = (int)fn.length();
  
        bool isPGM = (len >= 4) && (fn.substr(len-4,4)==".pgm");
        bool isGZ = (len >= 7) && (fn.substr(len-7,7)==".pgm.gz");
        
        if( (!isPGM) && (!isGZ) ){
          WARNING_LOG("invalid file type attaching suffix '.pgm.gz'");
          fn += ".pgm.gz";
        }
        
        File f(fn,File::writeText);
        f << str("P5") << "\n" << w << str(" ") << str(h*t) << "\n";
        f << str("255") << "\n";
        f << str("# ColorSegmentationOp dims ") << w << str(" ") << h << str(" ") << t << "\n";
        f << str("# ColorSegmentationOp format ") << str(fmt) << "\n";
  
        f.write(data,dim);
        f.close();
      }
      
      format load(const std::string &filename){
        static const std::string err="ColorSegmentationOp:load invalid file format ";
        File f(filename,File::readBinary);
        if(f.readLine() != "P5") throw ICLException(err+"('P5' expected)");
        std::vector<int> whpgm = parseVecStr<int>(f.readLine()," ");
        if(parse<int>(f.readLine()) != 255) throw ICLException(err+"('255' expected)");
        std::vector<std::string> opt1 = tok(f.readLine()," ");
        std::vector<std::string> opt2 = tok(f.readLine()," ");
        if(opt1.size() != 6) throw ICLException(err+"('# ColorSegmentationOp dims w h t' expected)");
        if(opt1[0] != "#" || opt1[1] != "ColorSegmentationOp" || opt1[2] != "dims"){
          throw ICLException(err+"('# ColorSegmentationOp dims w h t' expected)");
        }
        int w = parse<int>(opt1[3]);
        int h = parse<int>(opt1[4]);
        int t = parse<int>(opt1[5]);
        
        if(opt2.size() != 4) throw ICLException(err+"('# ColorSegmentationOp format f' expected)");
        if(opt2[0] != "#" || opt2[1] != "ColorSegmentationOp" || opt2[2] != "format"){
          throw ICLException(err+"('# ColorSegmentationOp format f' expected)");
        }
        format fmt = parse<format>(opt2[3]);
        int n = f.bytesAvailable();
        if(n < w*h*t) throw ICLException(err+"('not enough remaining bytes available', "
                                         "bytes left:" + str(n) +" needed:" + str(w*h*t) + ")");
  
        resize(w,h,t);
  
        std::copy(f.getCurrentDataPointer(),f.getCurrentDataPointer()+dim,data);
        
        return fmt;
      }
      
      const Img8u &getImage(int xDim, int yDim, int zSlice) const{
        zSlice = clip(zSlice,0,t-1);
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
  
      icl8u &getEntry(int xDim, int yDim, int x, int y, int z){
        const int dims[3]={xDim,yDim,3-(xDim+yDim)};
        const int xyz[3] = {x,y,z};
        return operator()(xyz[dims[0]],xyz[dims[1]],xyz[dims[2]]);
      }
  
      const std::vector<Color> &getClassMeanColors(format srcfmt){
        WARNING_LOG("this method is still buggy");
        std::fill(classmeans.begin(),classmeans.end(),Color(0,0,0));
        int nums[256]={0};
        for(int z=0;z<t;++z){
          for(int y=0;y<h;++y){
            for(int x=0;x<w;++x){
              int c = operator()(x,y,z);
              if(!c) continue;
              int xs = x*(256/w), ys = y*(256/h), zs = z*(256/t);
  
              int r,g,b;
              float fr, fg, fb;
              switch(srcfmt){
                case formatYUV:
                  cc_util_yuv_to_rgb(xs,ys,zs,r,g,b); break;
                case formatHLS:{
                  cc_util_hls_to_rgb(xs,ys,zs,fr,fg,fb);
                  r=fr; g=fg; b=fb;
                  break;
                }case formatRGB:
                  r=xs; g=ys; b=zs;
                  break;
                default:
                  throw ICLException("ColorSegmetationOp::getColoredLUTPreview: supported formats are yuv,rgb and hls");
              }
              if(c == 3){
                SHOW( Color(r,g,b).transp() );
              }
              classmeans[c] += Color(r,g,b);
              nums[c]++;
            }
          }
        }
        for(int i=1;i<256;++i){
          if(!nums[i]) continue;
          for(int o=0;o<3;++o){
            classmeans[i][o] = float(classmeans[i][o])/nums[i];
          }
        }
        return classmeans;
      }
      
  
      const Img8u &getColoredImage(int xDim, int yDim, int zSlice,
                                   int xShift,int yShift,int zShift, 
                                   format srcfmt){
  #if 0
        getClassMeanColors(srcfmt);
        colorImage.fill(0);
  
        const Channel8u c = getImage(xDim,yDim,zSlice)[0];
        colorImage.setFormat(formatRGB);
        colorImage.setSize(c.getSize());
        for(int y=0;y<c.getHeight();++y){
          for(int x=0;x<c.getWidth();++x){
            if(!c(x,y)) continue;
            colorImage(x,y) = classmeans[c(x,y)];
          }
        }
        return colorImage;
  #else
        colorImage.fill(0);
  
        const Channel8u c = getImage(xDim,yDim,zSlice)[0];
        colorImage.setFormat(formatRGB);
        colorImage.setSize(c.getSize());
        Channel8u r=colorImage[0],g=colorImage[1],b=colorImage[2];
        for(int y=0;y<c.getHeight();++y){
          for(int x=0;x<c.getWidth();++x){
            if(!c(x,y)) continue;
            int sfmt[3];
            int xs = x<<xShift, ys = y<<yShift, zs = zSlice<<zShift;
            switch(xDim){
              case 0:
                sfmt[0] = xs; sfmt[1] = ys; sfmt[2] = zs;
                if(yDim==2) std::swap(sfmt[1],sfmt[2]);
                break;
              case 1: // x=1, y=2, z=0
                sfmt[1] = xs; sfmt[0] = ys; sfmt[2] = zs;
                if(yDim==2) std::swap(sfmt[0],sfmt[2]);
                break;
              case 2:
                sfmt[2] = xs; sfmt[0] = ys; sfmt[1] = zs;
                if(yDim==1) std::swap(sfmt[0],sfmt[1]);
                break;
            }
            int rgb[3];
            float frgb[3];
            switch(srcfmt){
              case formatYUV:
                cc_util_yuv_to_rgb(sfmt[0],sfmt[1],sfmt[2],rgb[0],rgb[1],rgb[2]); break;
              case formatHLS:{
                cc_util_hls_to_rgb(sfmt[0],sfmt[1],sfmt[2],frgb[0],frgb[1],frgb[2]); break;
                std::copy(frgb,frgb+3,rgb);
                break;
              }case formatRGB:
                std::copy(sfmt,sfmt+3,rgb);
                break;
              default:
                throw ICLException("ColorSegmetationOp::getColoredLUTPreview: supported formats are yuv,rgb and hls");
            }
            r(x,y) = rgb[0];
            g(x,y) = rgb[1];
            b(x,y) = rgb[2];
          }
        }
        return colorImage;
  #endif
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
      lutEntry(dst(0,0,0),dst(0,0,1),dst(0,0,2),rA,rB,rC,value);
    }
  
    void ColorSegmentationOp::clearLUT(icl8u value){
      m_lut->clear(value);
    }
  
    const Img8u &ColorSegmentationOp::getLUTPreview(int xDim, int yDim, icl8u zValue){
      ICLASSERT_THROW(xDim>=0 && yDim>=0 && xDim<3 && yDim<3 && xDim!=yDim,
                      ICLException("ColorSegmentationOp::getLUTPreview invalid for x-dim or y-dim"));
      return m_lut->getImage(xDim,yDim,zValue>>m_bitShifts[3-(xDim+yDim)]);
    }
  
    const Img8u &ColorSegmentationOp::getColoredLUTPreview(int xDim, int yDim, icl8u zValue){
      ICLASSERT_THROW(xDim>=0 && yDim>=0 && xDim<3 && yDim<3 && xDim!=yDim,
                      ICLException("ColorSegmentationOp::getColoredLUTPreview invalid for x-dim or y-dim"));
      return m_lut->getColoredImage(xDim,yDim,zValue>>m_bitShifts[3-(xDim+yDim)],
                                    m_bitShifts[xDim],m_bitShifts[yDim],m_bitShifts[3-(xDim+yDim)],
                                    m_segFormat);
    }
    
    static int compute_shift(int len){
      switch(len){
        case 256: return 0;
        case 128: return 1;
        case 64: return 2;
        case 32: return 3;
        case 16: return 4;
        case 8: return 5;
        case 4: return 6;
        case 2: return 7;
        default: return 8;
      }
    }
    
    void ColorSegmentationOp::load(const std::string &filename){
      try{
        m_segFormat = m_lut->load(filename);
  
        m_bitShifts[0] = compute_shift(m_lut->w);
        m_bitShifts[1] = compute_shift(m_lut->h);
        m_bitShifts[2] = compute_shift(m_lut->t);
      }catch(ICLException &ex){
        ERROR_LOG(ex.what());
      }
    }
    void  ColorSegmentationOp::save(const std::string &filename) {
      m_lut->save(filename,m_segFormat);
    }
  
  
    const std::vector<Color> &ColorSegmentationOp::getClassMeanColors(){
      return m_lut->getClassMeanColors(m_segFormat);
    }
  
    icl8u ColorSegmentationOp::classifyPixel(icl8u R, icl8u G, icl8u B){
      int a=-1,b=-1,c=-1;
      switch(m_segFormat){
        case formatYUV:
          cc_util_rgb_to_yuv(R,G,B,a,b,c); 
          break;
        case formatHLS:
          float H,L,S;
          cc_util_rgb_to_hls(R,G,B,H,L,S); 
          a = H; b = L; c = S;
          break;
        case formatRGB:
          a = R; b = G; c = B; 
          break;
        default:
          throw ICLException("ColorSegmentationOp::classifyPixel invalid seg-format (allowed is yuv, hls and rgb)");
      }
      ShiftedLUT3D lut(m_bitShifts,*m_lut);
      return lut(a,b,c);
    }
    
    const icl8u *ColorSegmentationOp::getLUT() const{
      return m_lut->data;
    }
    
    icl8u *ColorSegmentationOp::getLUT(){
      return m_lut->data;
    }
    
    void ColorSegmentationOp::getLUTDims(int &w, int &h, int &t) const{
      w = m_lut->w;
      h = m_lut->h;    
      t = m_lut->t;
    }
  } // namespace filter
}

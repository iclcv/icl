#include <ICLQuick.h>
#include <FileReader.h>
#include <TestImages.h>
#include <Converter.h>
#include <PWCGrabber.h>
#include <ICLCC.h>
#include <map>

#include <ConvolutionOp.h>
#include <MedianOp.h>
#include <MorphologicalOp.h>
#include <BinaryArithmeticalOp.h>
#include <UnaryArithmeticalOp.h>
#include <FileWriter.h>
#include <UnaryCompareOp.h>
#include <LUTOp.h>

namespace icl{

  using namespace std;

  ImgQ zeros(int width, int height, int channels){
    // {{{ open

    return ImgQ(Size(width,height),channels);
  }

  // }}}
  ImgQ ones(int width, int height, int channels){
    // {{{ open

    ImgQ a(Size(width,height),channels);
    a.clear(-1,1.0,false);
    return a;
  }

  // }}}
  ImgQ load(const string &filename, format fmt){
    // {{{ open

    FileReader g(filename);
    ImgQ *image = g.grab()->convert<ICL_QUICK_TYPE>();
    if(!image){
      return ImgQ();
    }
    
    ImgQ im(image->getSize(),fmt);
    im.setTime(image->getTime());
    cc(image,&im);
    delete image;
    return im;

  }

  // }}}
  ImgQ create(const string &name, format fmt){
    // {{{ open

    ImgQ *image = TestImages::create(name,fmt,depth32f)->asImg<ICL_QUICK_TYPE>();
    if(!image){
      return ImgQ();
    }
    ImgQ im = *image;
    delete image;
    return im;
  }

  // }}}

  namespace{
    static PWCGrabber *G[4] = {0,0,0,0};
    struct PWCReleaser{
      // {{{ open

      ~PWCReleaser(){
        for(int i=0;i<4;i++){
          if(G[i]) delete G[i];
        }
      }
    };

    // }}}
    static PWCReleaser __r;
  }
  
  ImgQ pwc(int device, const Size &size, format fmt, bool releaseGrabber){
    // {{{ open

    if(device > 4){
      ERROR_LOG("device must be in 1,2,3 or 4");
      return ImgQ();
    }
    
    if(!G[device]){
      G[device] = new PWCGrabber(Size(640,480),device);
    }
    ImgQ *image = G[device]->grab()->convert<ICL_QUICK_TYPE>();
    ImgQ im(size,fmt);
    im.setTime(image->getTime());
    cc(image,&im);
    delete image;
    if(releaseGrabber){
      delete G[device];
      G[device] = 0;
    }
    return im;    
  }

  // }}}
  ImgQ ieee(int device,const Size &size, format fmt, bool releaseGrabber){
    // {{{ open

    WARNING_LOG("this function is not yet implemented");
    return ImgQ();
  }

  // }}}
  
  ImgQ filter(const ImgQ &image,const string &filter){
    // {{{ open

    static map<string,UnaryOp*> M;
    bool inited = false;
    if(!inited){
      static icl8u mask[9] = {1,1,1,1,1,1,1,1,1};
      static Size s3x3(3,3);
      M["sobelx"] = new ConvolutionOp(ConvolutionOp::kernelSobelX3x3);
      M["sobely"] = new ConvolutionOp(ConvolutionOp::kernelSobelY3x3);
      M["gauss"] = new ConvolutionOp(ConvolutionOp::kernelGauss3x3);
      M["laplace"] = new ConvolutionOp(ConvolutionOp::kernelLaplace3x3);
      M["median"] = new MedianOp(Size(3,3));
      M["dilation"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::dilate);
      M["erosion"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::erode);
      M["opening"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::openBorder);
      M["closing"] = new MorphologicalOp(s3x3,(char*)mask, MorphologicalOp::closeBorder);
    }    
    UnaryOp* u = M[filter];
    if(!u){
      WARNING_LOG("nothing known about filter type:" << filter);
      return ImgQ();
    }
    ImgBase *dst = 0;
    u->apply(&image,&dst);
    ImgQ *dstQ = dst->convert<ICL_QUICK_TYPE>();
    delete dst;
    ImgQ im = *dstQ;
    delete dstQ;
    return im;    
  } 

  // }}}
  ImgQ copy(const ImgQ &image){
    // {{{ open

    ImgQ *cpy = image.deepCopy()->asImg<ICL_QUICK_TYPE>();
    ImgQ im = *cpy;
    delete cpy;
    return im;
  }

  // }}}
  ImgQ copyroi(const ImgQ &image){
    // {{{ open

    ImgQ *cpy = image.deepCopyROI()->asImg<ICL_QUICK_TYPE>();
    ImgQ im = *cpy;
    delete cpy;
    return im;
  }

  // }}}
  ImgQ norm(const ImgQ &image){
    // {{{ open

    ImgQ *cpy = image.deepCopyROI();
    cpy->normalizeAllChannels(Range<ICL_QUICK_TYPE>(0,255));
    ImgQ x = *cpy;
    delete cpy;
    return x;
  }

  // }}}
  
  void save(const ImgQ &image,const string &filename){
    // {{{ open

    Img<float> bla = Img<float>(Size(5,5),5);
    //    Img<float> blub = bla;
    ImgQ roi = copyroi(image);
    //    roi = copyroi(image);
    FileWriter(filename).write(&roi);
  }

  // }}}
  void show(const ImgQ &image){
    // {{{ open

    TestImages::xv(&image);
  }

  // }}}
  void print(const ImgQ &image){
    // {{{ open

    image.print("image");
  }

  // }}}

  namespace{
    inline ImgQ *prepare_for_binary_op(const ImgQ &a, const ImgQ &b, ImgQ &na, ImgQ &nb){
      // {{{ open

      if(a.getROISize() == b.getROISize() && a.getChannels() == b.getChannels()){
        na = copy(a);
        nb = copy(b);
        return new ImgQ(a.getParams());
      }

    Size sa = a.getROISize(), sb = b.getROISize();
    Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
    int cr = max(a.getChannels(),b.getChannels());
      
    na = ImgQ(sr,a.getChannels());
    na.setROI(a.getROI());
    a.deepCopyROIToROI(&na);
    na.setFullROI();
    na.setChannels(cr);


    nb = ImgQ(sr,b.getChannels());
    nb.setROI(b.getROI());
    b.deepCopyROIToROI(&nb);
    nb.setFullROI();
    nb.setChannels(cr);
      
    return new ImgQ(na.getParams());
  }

  // }}}
    
    inline ImgQ apply_binary_arithmetical_op(const ImgQ &a, const ImgQ &b,  BinaryArithmeticalOp::optype ot){
      // {{{ open

      BinaryArithmeticalOp binop(ot);
      binop.setCheckOnly(true);
      binop.setClipToROI(true);

      if(a.getROISize() == b.getROISize() && a.getChannels() == b.getChannels()){
          ImgBase *res = new ImgQ(a.getParams());
          binop.apply(&a,&b,&res);
          ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
          delete res;
          return x;
      }
      
      Size sa = a.getROISize(), sb = b.getROISize();
      Size sr(max(sa.width,sb.width), max(sa.height,sb.height) );
      int cr = max(a.getChannels(),b.getChannels());
      
      ImgQ na(sr,a.getChannels());
      na.setROI(a.getROI());
      a.deepCopyROIToROI(&na);
      na.setFullROI();
      na.setChannels(cr);
      
      
      ImgQ nb(sr,b.getChannels());
      nb.setROI(b.getROI());
      b.deepCopyROIToROI(&nb);
      nb.setFullROI();
      nb.setChannels(cr);
      
      ImgBase * res = new ImgQ(na.getParams());
      
      binop.apply(&na,&nb,&res);
      ImgQ x = *(res->asImg<ICL_QUICK_TYPE>());
      delete res;
      return x;
    }

    // }}}
    inline ImgQ apply_unary_arithmetical_op(const ImgQ &image,ICL_QUICK_TYPE val, UnaryArithmeticalOp::optype ot){
      // {{{ open

      ImgBase *dst = 0;
      UnaryArithmeticalOp(ot,val).apply(&image,&dst);
      ImgQ r = *(dst->asImg<ICL_QUICK_TYPE>());
      delete dst;
      return r;
    }

    // }}}
  
  } // end of anonymous namespace
  
  ImgQ operator+(const ImgQ &a,const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::addOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::subOp);
  }

  // }}}
  ImgQ operator*(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::mulOp);
  }

  // }}}
  ImgQ operator/(const ImgQ &a, const ImgQ &b){
    // {{{ open

    return apply_binary_arithmetical_op(a,b,BinaryArithmeticalOp::divOp);
  }

  // }}}

  ImgQ operator+(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::addOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::subOp);
  }

  // }}}
  ImgQ operator*(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::mulOp);
  }

  // }}}
  ImgQ operator/(const ImgQ &image, float val){
    // {{{ open

    return apply_unary_arithmetical_op(image,val,UnaryArithmeticalOp::divOp);
  }

  // }}}

  ImgQ operator+(float val, const ImgQ &image){
    // {{{ open

    return image+val;
  }

  // }}}
  ImgQ operator-(float val, const ImgQ &image){
    // {{{ open
    ImgQ res(image.getROISize(),image.getChannels());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itI = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.getROIIterator(c);
      for(;itI.inRegion();++itI,++itRes){
        *itRes = val - (*itI);
      }
    }
    return res;
  }

  // }}}
  ImgQ operator*(float val, const ImgQ &image){
    // {{{ open

    return image*val;
  }

  // }}}
  ImgQ operator/(float val, const ImgQ &image){
    // {{{ open
    ImgQ res(image.getROISize(),image.getChannels());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itI = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itRes = res.getROIIterator(c);
      for(;itI.inRegion();++itI,++itRes){
        *itRes = ((*itI) == 0) ? 0 :  val / (*itI);
      }
    }
    return res;
  }

  // }}}


  ImgQ cc(const ImgQ& image, format fmt){
    // {{{ open

    ImgQ dst(image.getSize(),fmt);
    ImgQ src = copyroi(image);
    cc(&src,&dst);
    return dst;
  }

  // }}}
  ImgQ rgb(const ImgQ &image){
    // {{{ open

    return cc(image,formatRGB);
  }

  // }}}
  ImgQ hls(const ImgQ &image){
    // {{{ open

    return cc(image,formatHLS);
  }

  // }}}
  ImgQ lab(const ImgQ &image){
    // {{{ open

    return cc(image,formatLAB);
  }

  // }}}
  ImgQ gray(const ImgQ &image){
    // {{{ open

    return cc(image,formatGray);
  }

  // }}}


  ImgQ scale(const ImgQ& image, float factor){
    // {{{ open

    return scale(image,(int)(factor*image.getWidth()), (int)(factor*image.getHeight()));
  }

  // }}}
  ImgQ scale(const ImgQ& image, int width, int height){
    // {{{ open

    ImgQ *n = image.scaledCopyROI(Size(width,height));
    ImgQ a = *n;
    delete n;
    return a;    
  }

  // }}}
  ImgQ channel(const ImgQ &image, int channel){
    // {{{ open
    const ImgQ *c = image.selectChannel(channel)->asImg<ICL_QUICK_TYPE>();
    const ImgQ a = copy(*c);
    delete c;
    return a;
  }

  // }}}
  ImgQ levels(const ImgQ &image, icl8u levels){
    // {{{ open

    ImgBase *src = image.convertROI(depth8u);
    ImgBase *dst = 0;
    LUTOp(levels).apply(src,&dst);
    ImgQ *a = dst->convert<ICL_QUICK_TYPE>();
    ImgQ b = *a;
    delete a;
    delete src;
    delete dst;
    return b;
  }

  // }}}
  ImgQ thresh(const ImgQ &image, float threshold){
    // {{{ open

    ImgQ r(image.getROISize(),image.getChannels(),image.getFormat());
    r.setTime(image.getTime());
    for(int c=0;c<image.getChannels();++c){
      ConstImgIterator<ICL_QUICK_TYPE> itSrc = image.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itDst = r.getIterator(c);
      for(;itSrc.inRegion();++itSrc,++itDst){
        *itDst = *itSrc < threshold ? 0 : 255;
      }
    }    
    return r;
  }

  // }}}
  ImgQ flipx(const ImgQ& image){
    // {{{ open

    ImgQ *r = image.flippedCopyROI(axisHorz);
    ImgQ r2 = *r;
    delete r;
    return r2;
  }

  // }}}
  ImgQ flipy(const ImgQ& image){
    // {{{ open

    ImgQ *r = image.flippedCopyROI(axisVert);
    ImgQ r2 = *r;
    delete r;
    return r2;
  }

  // }}}

  ImgQ exp(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::expOp);
  }

  // }}}
  ImgQ ln(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::lnOp);
  }

  // }}}
  ImgQ sqr(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::sqrOp);
  }

  // }}}
  ImgQ sqrt(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::sqrtOp);
  }

  // }}}
  ImgQ abs(const ImgQ &image){
    // {{{ open

    return apply_unary_arithmetical_op(image,0,UnaryArithmeticalOp::absOp);
  }

  // }}}
  ImgQ operator-(const ImgQ &image){
    // {{{ open

    return image*(-1);
  }

  // }}}

  ImgQ operator||(const ImgQ &a, const ImgQ &b){
    // {{{ open

    ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
  
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.getROIIterator(c);
      for(;itA.inRegion();++itA,++itB, ++itR){
        *itR = *itA || *itB;
      }
    }
    return r;
  }

  // }}}
  ImgQ operator&&(const ImgQ &a, const ImgQ &b){
    // {{{ open

   ImgQ na,nb;
    ImgQ *res = prepare_for_binary_op(a,b,na,nb);
    ImgQ r = *res;
    delete res;
    for(int c=0;c<a.getChannels();c++){
      ImgIterator<ICL_QUICK_TYPE> itA = na.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itB = nb.getROIIterator(c);
      ImgIterator<ICL_QUICK_TYPE> itR = r.getROIIterator(c);
      for(;itA.inRegion();++itA,++itB, ++itR){
        *itR = *itA && *itB;
      }
    }
    return r;
  }

  // }}}
  ImgQ operator,(const ImgQ &a, const ImgQ &b){
    // {{{ open
    if(a.getSize() == Size::null) return copy(b);
    if(b.getSize() == Size::null) return copy(a);
    ImgQ r = zeros(a.getWidth()+b.getWidth(),max(a.getHeight(),b.getHeight()),max(a.getChannels(),b.getChannels()));
    r.setROI(a.getROI());
    roi(r) = a;

    Rect newroi(Point(r.getROIXOffset()+r.getROIWidth(), r.getROIYOffset()),b.getROISize());
    r.setROI(newroi);

    roi(r) = b;
    r.setFullROI();
    return r;
  }

  // }}}
  ImgQ operator%(const ImgQ &a, const ImgQ &b){
    // {{{ open
    if(a.getSize() == Size::null) return copy(b);
    if(b.getSize() == Size::null) return copy(a);
    ImgQ r = zeros(max(a.getWidth(),b.getWidth()),a.getHeight()+b.getHeight(),max(a.getChannels(),b.getChannels()));
    r.setROI(a.getROI());
    roi(r) = a;
    Rect newroi(Point(r.getROIXOffset(),r.getROIYOffset()+r.getROIHeight()), b.getROISize());
    r.setROI(newroi);
    roi(r) = b;
    r.setFullROI();
    return r;
  }

  // }}}

  ImgROI &ImgROI::operator=(float val){
    // {{{ open

    image.clear(-1,val,true);
    return *this;
  }

  // }}}
  ImgROI &ImgROI::operator=(const ImgQ &i){
    // {{{ open
    ICLASSERT_RETURN_VAL(image.getROISize() == i.getROISize(),*this);
    for(int c=0;c<min(image.getChannels(),i.getChannels());++c){
      deepCopyChannelROI(&i,c,i.getROIOffset(),i.getROISize(),
                         &image,c,image.getROIOffset(),image.getROISize());
    }
    return *this;
  }

  // }}}
  ImgROI &ImgROI::operator=(const ImgROI &r){
    // {{{ open
    ICLASSERT_RETURN_VAL(image.getROISize() == r.image.getROISize(),*this);
    for(int c=0;c<min(image.getChannels(),r.image.getChannels());++c){
      deepCopyChannelROI(&(r.image),c,r.image.getROIOffset(),r.image.getROISize(),
                         &image,c,image.getROIOffset(),image.getROISize());
    }
    return *this;
  }

  // }}}

  ImgROI::operator ImgQ(){
    // {{{ open

    return image;
  }

  // }}}
  ImgROI roi(ImgQ &r){
    // {{{ open

    ImgROI roi = { r };
    return roi;
  }

  // }}}
  ImgROI data(ImgQ &r){
    // {{{ open

    ImgROI roi = { r };
    roi.image.setFullROI();
    return roi;
  }

  // }}}

}

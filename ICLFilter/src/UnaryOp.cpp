#include <ICLFilter/UnaryOp.h>
#include <ICLFilter/UnaryOpWork.h>
#include <ICLUtils/Macros.h>
#include <ICLFilter/ImageSplitter.h>

#include <map>
#include <ICLUtils/StringUtils.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/RotateOp.h>
#include <ICLFilter/ScaleOp.h>

#ifdef HAVE_IPP
#include <ICLFilter/CannyOp.h>
#endif

#include <ICLFilter/ChamferOp.h>
#include <ICLFilter/GaborOp.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLFilter/LocalThresholdOp.h>

namespace icl{

  UnaryOp::UnaryOp():m_poMT(0),m_buf(0){};
  
  UnaryOp::UnaryOp(const UnaryOp &other):
    m_poMT(0),m_oROIHandler(other.m_oROIHandler),m_buf(0){}
  
  UnaryOp &UnaryOp::operator=(const UnaryOp &other){
    m_oROIHandler = other.m_oROIHandler;
    ICL_DELETE( m_poMT );
    return *this;
  }
  UnaryOp::~UnaryOp(){
    ICL_DELETE( m_poMT );
    ICL_DELETE( m_buf );
  }
  
  const ImgBase *UnaryOp::apply(const ImgBase *src){
    apply(src,&m_buf);
    return m_buf;
  }

  
  void UnaryOp::applyMT(const ImgBase *poSrc, ImgBase **ppoDst, unsigned int nThreads){
    ICLASSERT_RETURN( nThreads > 0 );
    ICLASSERT_RETURN( poSrc );
    if(nThreads == 1){
      apply(poSrc,ppoDst);
      return;
    }

    if(!prepare (ppoDst, poSrc)) return;
  
    bool ctr = getClipToROI();
    bool co = getCheckOnly();
    
    setClipToROI(false);
    setCheckOnly(true);
    
    const std::vector<ImgBase*> srcs = ImageSplitter::split(poSrc,nThreads);
    std::vector<ImgBase*> dsts = ImageSplitter::split(*ppoDst,nThreads);
    
    MultiThreader::WorkSet works(nThreads);
    
    for(unsigned int i=0;i<nThreads;i++){
      works[i] = new UnaryOpWork(this,srcs[i],const_cast<ImgBase*>(dsts[i]));
    }
    
    if(!m_poMT){
      m_poMT = new MultiThreader(nThreads);
    }else{
      if(m_poMT->getNumThreads() != (int)nThreads){
        delete m_poMT;
        m_poMT = new MultiThreader(nThreads);
      }
    }
    
    (*m_poMT)( works );
    
    for(unsigned int i=0;i<nThreads;i++){
      delete works[i];
    }

    setClipToROI(ctr);
    setCheckOnly(co);

    ImageSplitter::release(srcs);
    ImageSplitter::release(dsts);
  }


  namespace unary_op_from_string{
    typedef std::vector<std::string> paramlist;
    typedef UnaryOp* (*creator)(const paramlist &params);

    struct Creator{
      Creator(creator c=0,
            const std::string &n="",
            const std::string &s=""):c(c),s(s),n(n){}
      creator c;
      std::string s; // syntax
      std::string n; // name
      operator const std::string&() const{ return n; }
      UnaryOp *operator()(const paramlist &params) const {
        return c(params);
      }
    };
    static std::map<std::string,Creator> CREATORS;
    
    UnaryOp *create_generic_conv(const paramlist &paramsIn){
      paramlist params = paramsIn;
      ICLASSERT_THROW(params.size() >= 2,ICLException(str(__FUNCTION__)+":param list must have at least two elements"));
      Size kernelSize = parse<Size>(params[0]);
      ICLASSERT_THROW(kernelSize.getDim()>=1,ICLException(str(__FUNCTION__)+":kernel dimension is <=0"))
      params.erase(params.begin());
      std::vector<float> kernelValues = parseVec<float>(params);
      ICLASSERT_THROW(kernelSize.getDim() == (int)kernelValues.size(),ICLException(str(__FUNCTION__)+":kernel dimension is "+
                                                                                   str(kernelSize.getDim())+" kernel value list's size is "+
                                                                                   str(kernelValues.size())));
      return new ConvolutionOp(ConvolutionKernel(kernelValues.data(),kernelSize));
    }
    
    template<ConvolutionKernel::fixedType t>
    UnaryOp *create_fixed_conv(const paramlist &params){
      ICLASSERT_THROW(!params.size(),ICLException(str(__FUNCTION__)+": no parameters allowed here"));
      return new ConvolutionOp(ConvolutionKernel(t));
    }

    template<MorphologicalOp::optype t>
    UnaryOp *create_fixed_morph(const paramlist &params){
      Size kernelSize = params.size()?parse<Size>(params.front()):Size(3,3);
      ICLASSERT_THROW(kernelSize.getDim()>=1,ICLException(str(__FUNCTION__)+": kernel dimension is <=0"));
      return new MorphologicalOp(t,kernelSize);
    }
    
    UnaryOp *create_median(const paramlist &params){
      Size kernelSize = params.size()?parse<Size>(params.front()):Size(3,3);
      ICLASSERT_THROW(kernelSize.getDim()>=1,ICLException(str(__FUNCTION__)+": kernel dimension is <=0"));
      return new MedianOp(kernelSize);
    }

    struct PassOp:public UnaryOp{
      void apply(const ImgBase *src, ImgBase **dst){
        ICLASSERT(dst);
        const_cast<ImgBase*>(src)->deepCopy(dst);
        (*dst)->setFullROI();
      }
      static UnaryOp *create(const paramlist &params){
        ICLASSERT_THROW(!params.size(),ICLException(str(__FUNCTION__)+": no parameters allowed here"));
        return new PassOp;
      }
    };

    UnaryOp *create_rotate(const paramlist &params){
      ICLASSERT_THROW(params.size() == 1,ICLException(str(__FUNCTION__)+": params list size must be 1 here"));
      float angle = parse<float>(params.front());
      return new RotateOp(angle);
    }

    UnaryOp *create_scale(const paramlist &params){
      ICLASSERT_THROW(params.size() == 1 ||params.size() == 2 || params.size() == 3,ICLException(str(__FUNCTION__)+": params list size must be 1,2 or 3 here"));
      float fx = parse<float>(params[0]);
      if(fx > 5) throw ICLException(str(__FUNCTION__)+": scale factor fx must not be higher than 5");
      float fy = params.size()>1 ? parse<float>(params[1]) : fx;
      if(fy > 5) throw ICLException(str(__FUNCTION__)+": scale factor fy must not be higher than 5");
      scalemode sm = interpolateNN;
      if(params.size()==3 && params[2] == "NN"){}
      else if(params.size()==3 && params[2] == "LIN"){sm = interpolateLIN;}
      else if(params.size()==3 && params[2] == "RA"){sm = interpolateRA;}
      else if(params.size()==3) throw ICLException(str(__FUNCTION__)+": 3rd param must be one of NN, LIN or RA");
      
      return new ScaleOp(fx,fy,sm);
    }
    
#ifdef HAVE_IPP
     UnaryOp *create_canny(const paramlist &params){
       ICLASSERT_THROW(params.size() == 2 ||params.size() == 3,ICLException(str(__FUNCTION__)+": params list size must be 2 or 3 here"));
       float low = parse<float>(params[0]);
       float hi = parse<float>(params[1]);
       bool preblur = params.size()==3?parse<bool>(params[2]):false;
       return new CannyOp(low,hi,preblur);
     }
#endif
    
    template<class T>
    UnaryOp *create_any(const paramlist &params){
      ICLASSERT_THROW(!params.size(),ICLException(str(__FUNCTION__)+": no params allowed here"));
      return new T;
    }

    
    UnaryOp *create_gabor(const paramlist &params){
      ICLASSERT_THROW(params.size()<=6,ICLException(str(__FUNCTION__)+": max 6. params allowed"));
      Size kernelSize(5,5);
      float lambda = 1;
      float theta = 1;
      float psi = 1;
      float sigma = 1;
      float gamma = 1;
      
      if(params.size() > 0){
        kernelSize = parse<Size>(params[0]);
        if(kernelSize.getDim() < 1) throw ICLException(str(__FUNCTION__)+": kernel dimension must be > 0");
      }
      if(params.size() > 1)lambda = parse<float>(params[1]);
      if(params.size() > 2)theta = parse<float>(params[2]);
      if(params.size() > 3)psi = parse<float>(params[3]);
      if(params.size() > 4)sigma = parse<float>(params[4]);
      if(params.size() > 5)gamma = parse<float>(params[5]);
      return new GaborOp(kernelSize,std::vector<icl32f>(1,lambda),std::vector<icl32f>(1,theta),
                         std::vector<icl32f>(1,psi),std::vector<icl32f>(1,sigma),std::vector<icl32f>(1,gamma));
      
    }
    UnaryOp *create_compare(const paramlist &params){
      ICLASSERT_THROW(params.size()<=4,ICLException(str(__FUNCTION__)+": max 3. params allowed"));
      UnaryCompareOp::optype ot = UnaryCompareOp::lteq;
      icl64f value = 127;
      icl64f toll = 0;
      if(params.size()>0){
        if(params[0] == "<") ot = UnaryCompareOp::lt;
        else if(params[0] == "<") ot = UnaryCompareOp::lt;
        else if(params[0] == ">") ot = UnaryCompareOp::gt;
        else if(params[0] == "<=") ot = UnaryCompareOp::lteq;
        else if(params[0] == ">=") ot = UnaryCompareOp::gteq;
        else if(params[0] == "==") ot = UnaryCompareOp::eq;
      }
      if(params.size()>1){
        value = parse<float>(params[1]);
      }
      if(params.size()>2){
        toll = parse<float>(params[2]);
      }
      return new UnaryCompareOp(ot,value,toll);
    }

    UnaryOp *create_localThresh(const paramlist &params){
      ICLASSERT_THROW(params.size()<=4,ICLException(str(__FUNCTION__)+": max 3. params allowed"));
      unsigned int maskSize=10;
      int globalThreshold=0;
      float gammaSlope=0;
      if(params.size()>0) maskSize = parse<unsigned int>(params[0]);
      if(!maskSize) throw ICLException(str(__FUNCTION__)+": masksize is 0");
      if(params.size()>1) globalThreshold = parse<int>(params[1]);
      if(params.size()>2) gammaSlope = parse<float>(params[2]);
      return new LocalThresholdOp(maskSize,globalThreshold,gammaSlope);
    }
    
    void static_init(){
      static bool first = true;
      if(!first)return;
      first = false;
      
      CREATORS["copy"] = Creator(PassOp::create,"copy","");
      
      // convolution op      
#define FIXED_CONV_CREATOR_ENTRY(X)                                     \
      CREATORS[#X] = Creator(create_fixed_conv<ConvolutionKernel::X>,#X,"")
      FIXED_CONV_CREATOR_ENTRY(gauss3x3);
      FIXED_CONV_CREATOR_ENTRY(laplace3x3);
      FIXED_CONV_CREATOR_ENTRY(sobelX3x3);
      FIXED_CONV_CREATOR_ENTRY(sobelY3x3);
      FIXED_CONV_CREATOR_ENTRY(gauss5x5);
      FIXED_CONV_CREATOR_ENTRY(laplace5x5);
      FIXED_CONV_CREATOR_ENTRY(sobelX5x5);
      FIXED_CONV_CREATOR_ENTRY(sobelY5x5);

      CREATORS["conv"] = Creator(create_generic_conv,"conv",
                                 "size,comma sep. value list");
#undef FIXED_CONV_CREATOR_ENTRY
      
      // morphological op
#define FIXED_MORPH_CREATOR_ENTRY(X) \
      CREATORS[#X] = Creator(create_fixed_morph<MorphologicalOp::X>,#X,"kernel-size WxH=3x3");
      
      FIXED_MORPH_CREATOR_ENTRY(dilate);
      FIXED_MORPH_CREATOR_ENTRY(erode);
      FIXED_MORPH_CREATOR_ENTRY(dilate3x3);
      FIXED_MORPH_CREATOR_ENTRY(erode3x3);

      FIXED_MORPH_CREATOR_ENTRY(dilateBorderReplicate);
      FIXED_MORPH_CREATOR_ENTRY(erodeBorderReplicate);
      
      FIXED_MORPH_CREATOR_ENTRY(openBorder);
      FIXED_MORPH_CREATOR_ENTRY(closeBorder);

      FIXED_MORPH_CREATOR_ENTRY(tophatBorder);
      FIXED_MORPH_CREATOR_ENTRY(blackhatBorder);
      FIXED_MORPH_CREATOR_ENTRY(gradientBorder);
#undef FIXED_MORPH_CREATOR_ENTRY

      // median
      CREATORS["median"] = Creator(create_median,"median","kernel-size WxH=3x3");


      CREATORS["rotate"] = Creator(create_rotate,"rotate","angle in degree");

      CREATORS["scale"] = Creator(create_scale,"scale","fx (<5),fy (<5)=fx,interplation=NN (one of RA,LIN or NN)");

#ifdef HAVE_IPP
      CREATORS["canny"] = Creator(create_canny,"canny","lowThresh,highThresh,preblur=false (true or false)");
#endif

      CREATORS["chamfer"] = Creator(create_any<ChamferOp>,"chamfer","");

      CREATORS["gabor"] = Creator(create_gabor,"gabor","kernelSize=(5,5),lambda=1,theta=1,psi=1,sigma=1,gamma=1");

      CREATORS["compare"] = Creator(create_compare,"compare","op=>= (one of <,<=,>,>= or ==), value=127, tollerance=0");

      CREATORS["localThresh"] = Creator(create_localThresh,"localThresh","maskSize=10,globalThreshold=0,gammaSlope=0");
    }

  }

  UnaryOp *UnaryOp::fromString(const std::string &definition) throw (ICLException){
    unary_op_from_string::static_init();
    bool hasParams = true;
    if(!definition.size()) throw ICLException(str(__FUNCTION__)+": empty defintion string");
    std::string::size_type a = definition.find('(');
    std::string name;
    std::vector<std::string> plist;
    if(a == std::string::npos){
      hasParams = false;
      name = definition;
    }else{
      if(definition[definition.size()-1] != ')') throw ICLException(str(__FUNCTION__)+": missing closing ')' in definition ["+definition+"]");
      name = definition.substr(0,a);
      std::string ps = definition.substr(a+1);
      plist = tok(ps.substr(0,ps.size()-1),",");
    }
    
    std::map<std::string,unary_op_from_string::Creator>::iterator it=unary_op_from_string::CREATORS.find(name);
    if(it == unary_op_from_string::CREATORS.end()) throw ICLException(str(__FUNCTION__)+": no op found for given specifier ["+name+"]");
    UnaryOp * op = it->second(plist);
    if(!op) throw ICLException("wrong parameter list syntax");
    return op;
  }

  std::string UnaryOp::getFromStringSyntax(const std::string &opSpecifier) throw (ICLException){
    unary_op_from_string::static_init();
    std::map<std::string,unary_op_from_string::Creator>::iterator it=unary_op_from_string::CREATORS.find(opSpecifier);
    if(it == unary_op_from_string::CREATORS.end()) throw ICLException(str(__FUNCTION__)+": no op found for given specifier ["+opSpecifier+"]");
    return it->second.s;
  }
  
  std::vector<std::string> UnaryOp::listFromStringOps(){
    unary_op_from_string::static_init();
    std::vector<std::string> v(unary_op_from_string::CREATORS.size());
    int i=0;
    for(std::map<std::string,unary_op_from_string::Creator>::iterator it=unary_op_from_string::CREATORS.begin();
        it!=unary_op_from_string::CREATORS.end();++it){
      v[i++] = it->second;
    }
    return v;
  }
    
  void UnaryOp::applyFromString(const std::string &definition, const ImgBase *src, ImgBase **dst) throw (ICLException){
    unary_op_from_string::static_init();
    UnaryOp *op = fromString(definition);
    if(!op) throw ICLException(str(__FUNCTION__)+": no op found for given definition string ["+definition+"]");
    op->apply(src,dst);
    delete op;
  }
}

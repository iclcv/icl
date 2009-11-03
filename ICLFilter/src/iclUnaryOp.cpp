#include <iclUnaryOp.h>
#include <iclUnaryOpWork.h>
#include <iclMacros.h>
#include <iclImageSplitter.h>

#include <map>
#include <iclStringUtils.h>
#include <iclConvolutionOp.h>
#include <iclMorphologicalOp.h>
#include <iclMedianOp.h>

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
      ICLASSERT_THROW(kernelSize.getDim(),ICLException(str(__FUNCTION__)+":kernel dimension is 0"))
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
      ICLASSERT_THROW(kernelSize.getDim(),ICLException(str(__FUNCTION__)+": kernel dimension is 0"));
      return new MorphologicalOp(t,kernelSize);
    }
    
    UnaryOp *create_median(const paramlist &params){
      Size kernelSize = params.size()?parse<Size>(params.front()):Size(3,3);
      ICLASSERT_THROW(kernelSize.getDim(),ICLException(str(__FUNCTION__)+": kernel dimension is 0"));
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

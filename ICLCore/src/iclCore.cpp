#include <iclCore.h>
#include <iclException.h>
#include <iclImg.h>
#include <iclStringUtils.h>

using namespace std;

namespace icl{
  
  ImgBase *imgNew(depth d, const ImgParams &params){
    // {{{ open
  
    switch (d){
      case depth8u:  return new Img8u(params); break;
      case depth16s: return new Img16s(params); break;
      case depth32s: return new Img32s(params); break;
      case depth32f: return new Img32f(params); break;
      case depth64f: return new Img64f(params); break;
      default: ICL_INVALID_DEPTH; break;
    }
  }

  // }}}

  int getChannelsOfFormat(format eFormat){
    // {{{ open

    switch (eFormat){
      case formatRGB:
      case formatHLS:
      case formatLAB:
      case formatYUV:
        return 3;
        
      case formatChroma:
        return 2;
        
      case formatGray:
      case formatMatrix:
      default:
        return 1;
    }
  }

  // }}}


  /// puts a string representation of format into the given stream
  std::ostream &operator<<(std::ostream &s,const format &f){
    if( ((int)f<0) || ((int)f)>=7) return s << "formatUnknown";
    static const char *fmts[7] = { 
      "formatGray",
      "formatRGB",
      "formatHLS",
      "formatYUV",
      "formatLAB",
      "formatChroma",
      "formatMatrix"
    };
    return s << fmts[f];
  }
  
  /// puts a string representation of depth into the given stream
  std::ostream &operator<<(std::ostream &s,const depth &d){
    if( ((int)d<0) || ((int)d)>=5) return s << "depthUnknown";
    static const char *depths[7] = { 
      "depth8u",
      "depth16s",
      "depth32s",
      "depth32f",
      "depth64f"
    };
    return s << depths[d];
    
  }

  /// puts a string representation of format into the given stream
  std::istream &operator>>(std::istream &s, format &f){
    char cs[7]={0};
    
    s >> cs[0]; 
    s.unget();
    if(cs[0] == 'f'){
      for(int i=0;i<6;++i)s>>cs[i];
      ICLASSERT(str(cs) == "format");
    }else{
      // nothing, this is just a compability mode for 
      // someone forgetting the format prefix!
    }
    std::fill(cs,cs+7,'\0');
    s >> cs[0];
    cs[0] = tolower(cs[0]);
    int rest = 2;
    std::string expect;
    switch(cs[0]){
      case 'g':
        rest = 3;
        expect="gray";
        f = formatGray;
        break;
      case 'r':
        expect="rgb";
        f = formatRGB;
        break;
      case 'h':
        expect="hls";
        f = formatHLS;
        break;
      case 'y':
        expect="yuv";
        f = formatYUV;
        break;
      case 'L':
        expect="LAB";
        f = formatLAB;
        break;
      case 'c':
        rest = 5;
        f = formatChroma;
        expect="chroma";
        break;
      case 'm':
        rest = 5;
        f = formatMatrix;
        expect="matrix";
      default:
        ERROR_LOG("unable to parse format-type");
        return s;
    }
    for(int i=0;i<rest;++i){
      s >> cs[i+1];
      cs[i+1] = tolower(cs[i+1]);
    }
    if(expect != cs){
      ERROR_LOG("unabled t parse format: found: " << cs << " expected:" << expect);
    }
    return s;
  }
  
  /// puts a string representation of depth into the given stream
  std::istream &operator>>(std::istream &s, depth &d){
    char cs[6]={0};
    s >> cs[0];
    s.unget();
    if(cs[0] == 'd'){
      for(int i=0;i<5;++i) s>>cs[i];
      ICLASSERT(str(cs) == "depth");
    }else{
      // compability mode for someone forgetting the depth-prefix
    }
    std::fill(cs,cs+6,'\0');
    s >> cs[0];
    switch(cs[0]){
      case '8': 
        s >> cs[1];
        ICLASSERT(str(cs) == "8u");
        d = depth8u;
        return s;
      case '1':
        s >> cs[1] >> cs[2];
        ICLASSERT(str(cs) == "16s");
        d = depth16s;
        return s;
      case '3':
        s >> cs[1] >> cs[2];
        if(cs[2] == 's'){
          ICLASSERT(str(cs) == "32s");
          d = depth32s;
        }else{
          ICLASSERT(str(cs) == "32f");
          d = depth32f;
        }
        return s;
      case '6':    
        s >> cs[1] >> cs[2];
        ICLASSERT(str(cs) == "64f");
        d = depth64f;
        return s;
      default:
        ERROR_LOG("error parsing depth-type");
        return s;
    }    
  }
  



  ImgBase *ensureDepth(ImgBase **ppoImage, depth d){
    // {{{ open
    if(!ppoImage){
      return imgNew(d);
    }
    if(!*ppoImage){
      *ppoImage = imgNew(d);
    }
    else if((*ppoImage)->getDepth() != d){
      ImgBase *poNew = imgNew(d,(*ppoImage)->getParams());
      delete *ppoImage;
      *ppoImage = poNew;     
    }
    return *ppoImage;
  }

  // }}}
  
  ImgBase *ensureCompatible(ImgBase **ppoDst, depth d, const ImgParams &params){
    // {{{ open
    if(!ppoDst){
      return imgNew(d,params);
    }
    if(!*ppoDst){
      *ppoDst = imgNew(d,params);
    }else{
      ensureDepth(ppoDst,d);
      (*ppoDst)->setParams(params);
    }
    return *ppoDst;
  }

  // }}}
  
  ImgBase* ensureCompatible(ImgBase **dst, depth d, const Size &size, int channels, format fmt, const Rect &roi){
    // {{{ open

    FUNCTION_LOG("");
    if(fmt != formatMatrix && getChannelsOfFormat(fmt) != channels){
      ensureCompatible(dst,d,size,channels,roi);
      throw InvalidImgParamException("channels and format");
      return 0;
    }else{
      ImgBase *ret = ensureCompatible(dst,d,size,channels,roi);
      (*dst)->setFormat(fmt);
      return ret;
    }
  }

  // }}}

  ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src){
    // {{{ open
    return ensureCompatible(dst,src->getDepth(),src->getParams());
  }
  // }}}

  unsigned int getSizeOf(depth eDepth){
    // {{{ open
    static unsigned int s_aiSizeTable[]= { sizeof(icl8u),
                                           sizeof(icl16s),
                                           sizeof(icl32s),
                                           sizeof(icl32f),                                           
                                           sizeof(icl64f) };
    return s_aiSizeTable[eDepth];
  }

  // }}}

} //namespace

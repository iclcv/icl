#include <iclCore.h>
#include <iclException.h>
#include <iclImg.h>

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

  string translateFormat(format eFormat){
    // {{{ open

    switch(eFormat){
      case formatRGB: return "rgb";
      case formatHLS: return "hls";
      case formatLAB: return "lab";
      case formatYUV: return "yuv";
      case formatGray: return "gray";
      case formatMatrix: return "matrix";
      case formatChroma: return "chroma";
      default: ICL_INVALID_FORMAT; return "undefined format";        
    }
  }

  // }}}
 
  format translateFormat(const string& sFormat){
    // {{{ open

    if(sFormat.length()<=0){
      ICL_INVALID_FORMAT;
      return formatMatrix;
    }
    switch(sFormat[0]){
      case 'r': return formatRGB;
      case 'h': return formatHLS;
      case 'l': return formatLAB;
      case 'y': return formatYUV;
      case 'g': return formatGray;
      case 'm': return formatMatrix;
      case 'c': return formatChroma;
      default: ICL_INVALID_FORMAT; return formatMatrix;
    }
  }

  // }}}
  
  static string asDepthTypes[] = {"8u","16s","32s","32f","64f"};
  string translateDepth(depth eDepth){
    // {{{ open
    return string("depth") + asDepthTypes[eDepth];
  }

// }}}

  depth translateDepth(const std::string& sDepth){
    // {{{ open
    if (sDepth.substr(0,5) == "depth") {
       string t(sDepth.substr(5));
       for (int i=0; i <= depthLast; i++)
          if (t == asDepthTypes[i]) return static_cast<depth>(i);
    }
    ICL_INVALID_DEPTH;
  }

// }}}

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

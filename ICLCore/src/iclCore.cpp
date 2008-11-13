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

  /// creates a size string like "640x480"
  std::string translateSize(const Size &size){
    // {{{ open

    char buf[100];
    sprintf(buf,"%dx%d",size.width,size.height);
    return buf;
  }

  // }}}
  
  /// translates a size string into a size variable
  /** if the string could not be parsed, the returned size is "(-1,-1)" */
  Size translateSize(const std::string &size){
    // {{{ open
    // extended:
    Size s = Size::fromString(size);
    if(s!=Size(-1,-1)) return s;

    unsigned int pos = size.find('x',0);
    if(pos == string::npos || pos == 0 || pos == size.length()-1 ) return Size::null;
    int w = atoi(size.c_str());
    int h = atoi(size.c_str()+pos+1);
    return Size(w,h);
  }

  // }}}

  /// creates a rect string like "640x480@(5,10)"
  std::string translateRect(const Rect &r){
    // {{{ open

    return translateSize(r.size())+string("@")+translatePoint(r.ul());
  }

  // }}}
  
  /// translates a rect string into a Rect variable
  /** if the string could not be parsed, the returned Rect is "(-1,-1)@-1x-1" */
  Rect translateRect(const std::string &r){
    // {{{ open

    int pos = r.find('@',0);
    return Rect(translatePoint(r.c_str()+pos+1),translateSize(r));
  }

  // }}}
  
  std::string translatePoint(const Point &p){
    // {{{ open

    char buf[100];
    sprintf(buf,"(%d,%d)",p.x,p.y);
    return buf;
  }

  // }}}


  Point translatePoint(const std::string &p){
    // {{{ open

    unsigned int pos = p.find(',',0);
    if(pos == string::npos || pos == 0 || pos == p.length()-1 ||
       p.find('(',0) != 0 || p.find(')',0) != p.length()-1  )   return Point::null;
    int x = atoi(p.c_str()+1);
    int y = atoi(p.c_str()+pos+1);
    return Point(x,y);
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

#include "ImgParams.h"
#include "ICLCore.h"


namespace icl{
  const ImgParams ImgParams::null(0,0,0,0,0,0,0);

  ImgParams::ImgParams(const ImgParams &params){
    // {{{ open
    FUNCTION_LOG("");
    setup(params.getSize(), params.getFormat(), params.getChannels(), params.getROI());
  }
  
  // }}}

  ImgParams::ImgParams(const Size &size, format fmt, const Rect &roi){
    // {{{ open

    FUNCTION_LOG("");
    setup(size,fmt,getChannelsOfFormat(fmt),roi);
  }

  // }}}

  ImgParams::ImgParams(const Size &size,int channels, const Rect &roi ){
    // {{{ open

    FUNCTION_LOG("");
    setup(size,formatMatrix,channels,roi);
  }

  // }}}

  ImgParams::ImgParams(int width, int height, format fmt, int roiX, int roiY, int roiWidth, int roiHeight){
    // {{{ open

    FUNCTION_LOG("");
    setup(Size(width,height),fmt,getChannelsOfFormat(fmt),Rect(roiX,roiY,roiWidth, roiHeight));
  }

  // }}}
  
  ImgParams::ImgParams(int width, int height, int channels, int roiX, int roiY, int roiWidth, int roiHeight){
    // {{{ open

    FUNCTION_LOG("");
    setup(Size(width,height),formatMatrix, channels, Rect(roiX,roiY,roiWidth, roiHeight));
  }

  // }}}

  bool ImgParams::operator==(const ImgParams &other) const{
    // {{{ open

    FUNCTION_LOG("");
    return getSize()==other.getSize() &&
      getROI() == other.getROI() &&
      getChannels() == other.getChannels() && 
      getFormat() == other.getFormat();
  }

  // }}}
  
  void ImgParams::setup(const Size &size, format fmt, int channels,const Rect &roi){
    // {{{ open

    FUNCTION_LOG("setup(width="<<size.width<<",height="<<size.height<<",format="<<translateFormat(fmt)<<
                 ",channels="<<channels<<",ROI=("<<roi.x<<","<<roi.y<<","<<roi.width<<","<<roi.height<<"))");


    ICLASSERT_THROW(fmt == formatMatrix || channels == getChannelsOfFormat(fmt),
                    InvalidImgParamException("format and channels"))
    m_oSize = size;
    m_eFormat = fmt;
    m_iChannels = channels;
    setROI(roi);
  }

  // }}}

  void ImgParams::setSize(const Size &size){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_THROW( size.width>=0 && size.height>=0 ,InvalidImgParamException("size"));
    m_oSize = size;
    setROI(Rect::null);
  }

  // }}}
  
  void ImgParams::setFormat(format fmt){
    // {{{ open

    FUNCTION_LOG("");
    m_eFormat = fmt;
    if(fmt != formatMatrix) m_iChannels = getChannelsOfFormat(fmt);
  }

  // }}}

  void ImgParams::setChannels(int channels){
    // {{{ open

    FUNCTION_LOG("");
    if(m_iChannels != channels){
      m_eFormat = formatMatrix;
    }
    m_iChannels = channels;

  }

  // }}}
  
  void ImgParams::setROIOffset(const Point &offset) {
    // {{{ open

      FUNCTION_LOG("");
      ICLASSERT_THROW(offset.x >= 0 && offset.x + getROIWidth() <= getWidth() &&
                      offset.y >= 0 && offset.y + getROIHeight() <= getHeight(),
                      InvalidImgParamException("roi-offset"));
      m_oROI.x = offset.x;
      m_oROI.y = offset.y;
    }

  // }}}
      
  void ImgParams::setROISize(const Size &roisize) {
    // {{{ open

    FUNCTION_LOG("");
    Size size(roisize);
    if(!(size)){
      size = m_oSize;
    }
    ICLASSERT_THROW(size.width >= 1 && getROIXOffset() + size.width  <= getWidth() &&
                    size.width >= 1 && getROIYOffset() + size.height <= getHeight(),
                    InvalidImgParamException("roi-size"));
    m_oROI.width = size.width;
    m_oROI.height = size.height;
  }

  // }}}
  
  void ImgParams::setROI(const Point &offset, const Size &roisize){
    // {{{ open

    FUNCTION_LOG("");
    Size size(roisize);
    if( !(size) ){
      size = m_oSize;
    }
    ICLASSERT_THROW(offset.x >= 0 && size.width >= 1 && offset.x + size.width  <= m_oSize.width &&
                    offset.y >= 0 && size.width >= 1 && offset.y + size.height <= m_oSize.height,
                    InvalidImgParamException("roi"));
    m_oROI.x = offset.x;
    m_oROI.y = offset.y;
    m_oROI.width = size.width;
    m_oROI.height = size.height;
  }

  // }}}

  void ImgParams::setROI(const Rect &roi) {
    // {{{ open

    FUNCTION_LOG("");
    if(roi){
      setROI (roi.ul(),roi.size());
    }else{
      m_oROI.x = 0;
      m_oROI.y = 0;
      m_oROI.width = m_oSize.width;
      m_oROI.height = m_oSize.height;
    }
  }

  // }}}
  
  void ImgParams::setROISizeAdaptive(const Size &s){
    // {{{ open

    FUNCTION_LOG("setROISizeAdaptive("<< s.width << "," << s.height << ")");
    
    int iW = s.width <=  0 ? getSize().width  + s.width  : s.width;
    int iH = s.height <= 0 ? getSize().height + s.height : s.height;
    
    m_oROI.width  =  clip(iW, 1, getWidth()-getROIXOffset());
    m_oROI.height =  clip(iH, 1, getHeight()-getROIYOffset());
  }

  // }}}
  
  void ImgParams::setROIOffsetAdaptive(const Point &p){
    // {{{ open

    FUNCTION_LOG("setROIOffsetAdaptive("<< p.x << "," << p.y << ")");
    
    int x = p.x < 0 ? getWidth()  - getROIWidth() + p.x : p.x;
    int y = p.y < 0 ? getHeight() - getROIHeight() + p.y : p.y;
    
    m_oROI.x =  clip(x,0,getWidth()-getROIWidth());
    m_oROI.y =  clip(y,0,getHeight()-getROIHeight());
  }

  // }}}

  void ImgParams::setROIAdaptive(const Rect &roi){
    // {{{ open

    FUNCTION_LOG("");
    m_oROI.x = 0;
    m_oROI.y = 0;
    setROISizeAdaptive (roi.size());
    setROIOffsetAdaptive (roi.ul());
  }

  // }}}
  


}

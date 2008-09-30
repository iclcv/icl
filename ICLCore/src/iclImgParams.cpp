#include <iclImgParams.h>
#include <iclCore.h>
#include <sstream>

namespace icl{
  const ImgParams ImgParams::null(0,0,0,formatMatrix,0,0,0,0);

  ImgParams::ImgParams(const Size &size, format fmt, const Rect &roi) {
    // {{{ open
    setup(size,fmt,getChannelsOfFormat(fmt),roi); 
  }
  // }}}

  ImgParams::ImgParams(int width, int height, format fmt, int roiX, int roiY, int roiWidth, int roiHeight){
    // {{{ open
    setup(Size(width,height),fmt,getChannelsOfFormat(fmt),Rect(roiX,roiY,roiWidth,roiHeight));
  }
  // }}}
  
  ImgParams::ImgParams(int width, int height, int channels, int roiX, int roiY, int roiWidth, int roiHeight){
    // {{{ open
    setup(Size(width,height),formatMatrix, channels, Rect(roiX,roiY,roiWidth,roiHeight));
  }
  // }}}

  ImgParams::ImgParams(int width, int height, int channels, format fmt, int roiX, int roiY, int roiWidth, int roiHeight){
    // {{{ open
    //    ERROR_LOG("creating ImgParams: (" << width << "," << height << "," << channels << "," << fmt << "...)");
    setup(Size(width,height),fmt, channels, Rect(roiX,roiY,roiWidth,roiHeight));
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

    /*
    ICLASSERT_THROW(fmt == formatMatrix || channels == getChannelsOfFormat(fmt),
                    InvalidImgParamException("format and channels"))
   */

    std::ostringstream str;
    str << "incompatible format (" << translateFormat(fmt) << ") and channelcount (" << channels << ")";
    ICLASSERT_THROW(fmt == formatMatrix || channels == getChannelsOfFormat(fmt),
                    InvalidImgParamException(str.str()));

    m_oSize = size;
    m_eFormat = fmt;
    m_iChannels = channels;
    if (!roi.isNull()) setROI(roi); // set given roi if non-empty
    else setROI (Point::null, size); // or set full ROI
  }

  // }}}

  void ImgParams::setSize(const Size &size){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_THROW( size.width>=0 && size.height>=0 ,InvalidImgParamException("size"));
    m_oSize = size;
    setFullROI();
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
      
  void ImgParams::setROISize(const Size &size) {
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_THROW(size.width >= 0 && getROIXOffset() + size.width  <= getWidth() &&
                    size.width >= 0 && getROIYOffset() + size.height <= getHeight(),
                    InvalidImgParamException("roi-size"));
    m_oROI.width = size.width;
    m_oROI.height = size.height;
  }

  // }}}
  
  void ImgParams::setROI(const Point &offset, const Size &size){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_THROW(offset.x >= 0 && size.width >= 0 && offset.x + size.width  <= m_oSize.width &&
                    offset.y >= 0 && size.height >= 0 && offset.y + size.height <= m_oSize.height,
                    InvalidImgParamException("roi"));
    m_oROI.x = offset.x;
    m_oROI.y = offset.y;
    m_oROI.width = size.width;
    m_oROI.height = size.height;
  }

  // }}}

  void ImgParams::setROISizeAdaptive(const Size &s){
    // {{{ open

    FUNCTION_LOG("setROISizeAdaptive("<< s.width << "," << s.height << ")");
    
    int iW = s.width <=  0 ? getSize().width  + s.width  : s.width;
    int iH = s.height <= 0 ? getSize().height + s.height : s.height;
    
    m_oROI.width  =  clip(iW, 0, getWidth()-getROIXOffset());
    m_oROI.height =  clip(iH, 0, getHeight()-getROIYOffset());
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
  
  Rect& ImgParams::adaptROI(Rect &roi) const {
    // {{{ open
     ImgParams curParams(*this);
     curParams.setROIAdaptive(roi);
     roi = curParams.getROI();
     return roi;
  }
  // }}}

}

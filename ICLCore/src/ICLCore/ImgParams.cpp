/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgParams.cpp                      **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLCore/ImgParams.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;

namespace icl{
  namespace core{
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

      ICLASSERT_THROW(fmt == formatMatrix || channels == getChannelsOfFormat(fmt),
                      InvalidImgParamException("incompatible format (" + str(fmt) + ") and channel count (" + str(channels) + ")"));

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
      setROISizeAdaptive (roi.getSize());
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

    /// ostream operator SIZExCHANNELS@FORMAT.ROI
    std::ostream &operator<<(std::ostream &s, const ImgParams &p){
      return s << p.getSize() << 'x' << p.getChannels() << '@' << p.getFormat() << '.' << p.getROI();
    }

    /// istream operator
    std::istream &operator>>(std::istream &s, ImgParams &p){
      char c;
      Size size;
      int channels;
      format fmt;
      Rect roi;
      s >> size >> c >> channels >> c >> fmt >> c >> roi;

      p = ImgParams(size,channels,fmt,roi);

      return s;
    }


  } // namespace core
}

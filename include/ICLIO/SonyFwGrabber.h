/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SonyFwGrabber.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Felix Reinhard                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once

#ifdef WIN32
#ifdef WITH_SONYIIDC

#include <ICLIO/Grabber.h>
#include <sony/iidcapi.h>
#include <iostream>
#include <vector>
#include <string>

#define SONY_GAIN 470
#define SONY_SHUTTER 562
#define SONY_WHITEBALANCE_U 45
#define SONY_WHITEBALANCE_V 12
#define SONY_HUE 15

#define GET_FORMAT(formatindex) ((formatindex>>8)&0x07)
#define GET_MODE(formatindex) ((formatindex>>12)&0x07)
#define GET_COLORCODING(formatindex) (formatindex & 0x0f)

// TODO THIS MUST NOT BE LOCATED IN A .h FILE !!
using namespace std;

namespace icl {
  namespace io{
  
    /// Grabber implementation for grabbing on our Camera-Head \ingroup GRABBER_G
    class SonyFwGrabber : public Grabber {
  
      public:
      SonyFwGrabber();
      ~SonyFwGrabber();
  
      bool init();
      bool initTrigger();
  
      /// grabbing function  
      /** \copydoc icl::Grabber::grab(icl::core::ImgBase**)  **/    
      virtual const core::ImgBase* grabUD(core::ImgBase **poDst=0);
  
      void grabStereo (core::ImgBase*& ppoDstLeft, core::ImgBase*& ppoDstRight);
      void grabStereoTrigger (core::ImgBase*& leftImage, core::ImgBase*& rightImage);
  
      /** @{ @name properties and parameters */
      
      /// interface for the setter function for video device properties 
      /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
      virtual void setProperty(const std::string &property, const std::string &value);
  
      /// returns a list of properties, that can be set using setProperty
      /** @return list of supported property names **/
      virtual std::vector<std::string> getPropertyList();
  
      /// returns the current value of a property or a parameter
      virtual std::string getValue(const std::string &name);
      /** @} */
  
      void setDevice(int dev) {
        if ((dev <= m_lNumCameras) && (dev >=0))
          m_iDevice = dev; 
      }
      inline icl::utils::Size getSize() { return icl::utils::Size(m_iWidth, m_iHeight); }
  
      private:
      /// current grabbing width
      int m_iWidth;
      /// current grabbing height
      int m_iHeight;
      /// current grabbing device
      int m_iDevice;
      /// current grabbing fps value
      float m_fFps;
      /// current number of devices
      long m_lNumCameras;
      /// image data buffers for grabbing
      BYTE ***m_pppImgBuffer;
      /// camera handels
      HIIDC m_hCamera[10];
  
      void GetCamAllString(long camIndex, char *strCamera);
      void copyGrabbingBuffer (int iDevice, core::ImgBase *poDst);
  
    };
  
  } // namespace io
}

#endif //SONYIIDC
#endif //WIN32

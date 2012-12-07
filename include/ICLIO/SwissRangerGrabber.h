/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SwissRangerGrabber.h                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace io{
    
    
    /// Grabber-Implementation for the SwissRanger time-of-flight camera
    class SwissRangerGrabberImpl : public Grabber{
      public:
        /// Internally used data-class
        class SwissRanger;

        /// Create interface to device with given serial number:
        /** @param serialNumber if 0 -> automatic select\n
                              if < 0 open selection dialog (windows: gui, linux: shell input)
                              if > 0 specify serial number of device
         @param bufferDepth
         @param pickChannel
      */
        SwissRangerGrabberImpl(int serialNumber=0,
                               core::depth bufferDepth=core::depth32f,
                               int pickChannel=-1)
        throw (utils::ICLException);

        /// Destructor
        ~SwissRangerGrabberImpl();

        /// returns a list of all found devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        /// grab an undistorted image
        const core::ImgBase *acquireImage();
        
        /// Internally used utility function, that might be interesting elsewhere
        static float getMaxRangeMM(const std::string &modulationFreq) throw (utils::ICLException);

        /// adds properties to Configurable
        void addProperties();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

      private:
        /// utility function
        float getMaxRangeVal() const;

        /// Internal data
        SwissRanger *m_sr;

        /// Internally used mutex locks grabbing and setting of properties
        utils::Mutex m_mutex;
    };

    
    /// SwissRanger grabber using the libMesaSR library \ingroup GRABBER_G
    /** for more details: @see SwissRangerGrabberImpl */
    class SwissRangerGrabber : public GrabberHandle<SwissRangerGrabberImpl>{

      public:
        /// ID-creation function for the GrabberHandles internal unique ID
        static inline std::string create_id(int dev){
          return std::string("device-")+utils::str(dev);
        }

        /// Constructor
        /** see SwissRangerGrabberImpl::SwissRangerGrabberImpl for details */
        SwissRangerGrabber(int serialNumber=0, core::depth bufferDepth=core::depth32f, int pickChannel=-1) throw (utils::ICLException){
          std::string id = create_id(serialNumber);
          if(isNew(id)){
            initialize(new SwissRangerGrabberImpl(serialNumber,bufferDepth,pickChannel),id);
          }else{
            initialize(id);
          }
        }

        /// returns a list of all found devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
          return SwissRangerGrabberImpl::getDeviceList(rescan);
        }

        /// Utility function
        static inline float getMaxRangeMM(const std::string &modulationFreq) throw (utils::ICLException){
          return SwissRangerGrabberImpl::getMaxRangeMM(modulationFreq);
        }
    };
    
  } // namespace io
}


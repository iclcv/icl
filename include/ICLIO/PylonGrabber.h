/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/PylonGrabber.h                           **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#ifndef ICL_PYLON_GRABBER_H
#define ICL_PYLON_GRABBER_H

#include <pylon/PylonIncludes.h>

#include <ICLIO/GrabberHandle.h>
#include <ICLIO/PylonUtils.h>
#include <ICLIO/PylonCameraOptions.h>
#include <ICLIO/PylonGrabberThread.h>
#include <ICLIO/PylonColorConverter.h>

namespace icl {
  namespace pylon {

    /// Actual implementation of the Basler Pylon based GIG-E Grabber \ingroup GIGE_G
    class PylonGrabberImpl : public Grabber, public Interruptable {
      public:
        friend class PylonGrabber;

        /// interface for the setter function for video device properties
        virtual void setProperty(const std::string &property, const std::string &value);
        /// returns a list of properties, that can be set using setProperty
        virtual std::vector<std::string> getPropertyList();
        /// checks if property is returned, implemented, available and of processable GenApi::EInterfaceType
        virtual bool supportsProperty(const std::string &property);
        /// get type of property
        virtual std::string getType(const std::string &name);
        /// get information of a properties valid values
        virtual std::string getInfo(const std::string &name);
        /// returns the current value of a property or a parameter
        virtual std::string getValue(const std::string &name);
        /// Returns whether this property may be changed internally.
        virtual int isVolatile(const std::string &propertyName);

        /// Destructor
        ~PylonGrabberImpl();
         
        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::Grabber::grab(ImgBase**) **/
        virtual const ImgBase* acquireImage();

        /// Prints information about the startup argument options
        static void printHelp();
        /// Uses args to choose a pylon device
        /**
        * @param args The arguments provided to this grabber.
        * @throw ICLException when no suitable device exists.
        */
        static Pylon::CDeviceInfo getDeviceFromArgs(std::string args) throw(ICLException);

      private:
        /// The constructor is private so only the friend class can create instances
        /**
        * @param dev The PylonDevice that should be used for image acquisition.
        * @param args The arguments provided to this grabber.
        */
        PylonGrabberImpl(const Pylon::CDeviceInfo &dev, const std::string args);

        /// Used to determine wether (and to what) to convert an image.
        enum convert_to {
          yes_rgba,
          yes_mono8u,
          no_mono8u,
          no_mono16,
        };

        /// A mutex lock to synchronize buffer and color converter access.
        Mutex m_ImgMutex;
        /// The PylonEnvironment automation.
        PylonAutoEnv m_PylonEnv;
        /// Count of buffers for grabbing
        static const int m_NumBuffers = 4;
        /// The camera interface.
        Pylon::IPylonDevice* m_Camera;
        /// The streamGrabber of the camera.
        Pylon::IStreamGrabber* m_Grabber;
        /// PylonCameraOptions used to get and set camera settings.
        PylonCameraOptions* m_CameraOptions;
        /// PylonColorConverter used for color conversion.
        PylonColorConverter* m_ColorConverter;
        /// PylonGrabberThread used for continous image acquisition.
        PylonGrabberThread* m_GrabberThread;
        /// A list of used buffers.
        std::vector<PylonGrabberBuffer<uint16_t>*> m_BufferList;
        /// A pointer to the last used buffer.
        TsBuffer<int16_t>* m_LastBuffer;
    
        /// starts the acquisition of pictures by the camera
        void acquisitionStart();
        /// stops the acquisition of pictures by the camera
        void acquisitionStop();
        /// creates buffers and registers them at the grabber
        void grabbingStart();
        /// deregisters buffers from grabber and deletes them
        void grabbingStop();

        /// helper function that makes default settings for the camera.
        void cameraDefaultSettings();
        /// Converts pImageBuffer to correct type and writes it into m_Image
        void convert(const void *pImageBuffer);
    };

    /// Grabber implementation for a Basler Pylon-based GIG-E Grabber \ingroup GIGE_G
    /** This is just a wrapper class of the underlying PylonGrabberImpl class */
    struct PylonGrabber : public GrabberHandle<PylonGrabberImpl>{

      /// create a new PylonGrabber
      /** @see PylonGrabberImpl for more details*/
      inline PylonGrabber(const std::string args="") throw(ICLException) {
      /// looking for Pylon device compatible to args
      Pylon::CDeviceInfo dev = getDeviceFromArgs(args);
        if(isNew(dev.GetFullName().c_str())){
          initialize(new PylonGrabberImpl(dev, args), dev.GetFullName().c_str());
        }else{
          initialize(dev.GetFullName().c_str());
        }
      }

      /// Returns a list of detected pylon devices
      static Pylon::DeviceInfoList_t
      getPylonDeviceList(Pylon::DeviceInfoList_t* filter=NULL){
        return icl::pylon::getPylonDeviceList();
      }
    
      /// Prints information about the startup argument options of PylonGrabberImpl
      static void printHelp(){
        return pylon::printHelp();
      }

      static std::vector<GrabberDeviceDescription> getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
        if(rescan){
          deviceList.clear();
          Pylon::DeviceInfoList_t devs = getPylonDeviceList();
          for(unsigned int i = 0 ; i < devs.size() ; ++i){
            deviceList.push_back(
              GrabberDeviceDescription(
                "pylon",
                str(i) + "|||" + devs.at(i).GetFullName().c_str(),
                devs.at(i).GetFullName().c_str()
              )
            );
          }
        }
      return deviceList;
      }
    };

  } //namespace pylon
} //namespace icl

#endif


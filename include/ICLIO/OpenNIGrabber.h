/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/OpenNIGrabber.h                          **
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

#ifndef ICL_OPENNI_GRABBER_H
#define ICL_OPENNI_GRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Time.h>
#include <ICLIO/OpenNIUtils.h>
#include <ICLUtils/Thread.h>

#include <XnOS.h>
#include <XnCppWrapper.h>

namespace icl {

  // Forward declaration of OpenNIGrabberImpl
  class OpenNIGrabberImpl;

  /// Internally spawned thread class for continuous grabbing
  class OpenNIGrabberThread : public Thread {
    public:
      /// Constructor sets used grabber
      OpenNIGrabberThread(OpenNIGrabberImpl* grabber);

      /// constantly calls grabNextImage.
      void run();
    private:
      OpenNIGrabberImpl* m_Grabber;
  };

  /// Actual implementation of the OpenNI based Grabber
  class OpenNIGrabberImpl : public Grabber {
    public:
      friend class OpenNIGrabber;

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
      ~OpenNIGrabberImpl();
         
      /// grab function grabs an image (destination image is adapted on demand)
      /** @copydoc icl::Grabber::grab(ImgBase**) **/
      virtual const ImgBase* acquireImage();

      /// grabs an image from Imagegenerator
      void grabNextImage();

      /// switches the current generator to desired
      void setGeneratorTo(icl_openni::OpenNIMapGenerator::Generators desired);

    private:
      /// The constructor is private so only the friend class can create instances
      /**
      * @param device NodeInfo of the device to use.
      */
      OpenNIGrabberImpl(std::string args);

      /// Returns the string representation of the currently used device.
      std::string getName();

      /// Mutex used for concurrency issues.
      icl::Mutex m_Mutex;
      /// a device number and grabber id
      int m_Id;
      /// the OpenNI context
      xn::Context m_Context;
      /// holds a pointer to the currently used image generator
      icl_openni::OpenNIMapGenerator* m_Generator;
      /// Internally used ReadWriteBuffer
      icl_openni::ReadWriteBuffer<ImgBase>* m_Buffer;
      /// A Thread continuously grabbing images
      OpenNIGrabberThread* m_GrabberThread;
      /// whether double frames should be omited
      bool m_OmitDoubleFrames;
  };

  /// Grabber implementation for OpenNI based camera access.
  /**
      This is just a wrapper class of the underlying OpenNIGrabberImpl class
  **/
  struct OpenNIGrabber : public GrabberHandle<OpenNIGrabberImpl>{

  /// create a new OpenNIGrabber
  /** @see OpenNIGrabber for more details*/
  inline OpenNIGrabber(const std::string args="") throw(ICLException) {
    /// looking for OpenNI device compatible to args
    if(isNew(args)){
      DEBUG_LOG("isnew")
      OpenNIGrabberImpl* tmp = new OpenNIGrabberImpl(args);
      initialize(tmp, args);
    }else{
      DEBUG_LOG("isold Grabber")
      initialize(args);
    }
      DEBUG_LOG("end")
    }
    
  static std::vector<GrabberDeviceDescription> getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> deviceList;
    if(rescan){
      deviceList.clear();
      xn::Context context;
      context.Init();
      xn::NodeInfoList nodes;
      context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , nodes, NULL);
      int i = 0;
      for (xn::NodeInfoList::Iterator it = nodes.Begin(); it != nodes.End(); ++it, ++i){
        deviceList.push_back(
          GrabberDeviceDescription("oni", str(i) + "|||" + str(i), str(i))
        );
      }
      context.Release();
    }
  return deviceList;
  }
 };

} //namespace icl

#endif


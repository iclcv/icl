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

#pragma once

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Time.h>
#include <ICLIO/OpenNIUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/OpenNIIncludes.h>

namespace icl {
  namespace io{

    // Forward declaration of OpenNIGrabberImpl
    class OpenNIGrabberImpl;

    /// Internally spawned thread class for continuous grabbing
    class OpenNIGrabberThread : public utils::Thread {
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
        friend class OpenNIGrabberThread;

        /// Destructor
        ~OpenNIGrabberImpl();

        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        virtual const core::ImgBase* acquireImage();

        /**
            returns the underlying handle of the grabber.
            In this case the corresponding MapGenerator.
        **/
        virtual void* getHandle();

      private:
        /// The constructor is private so only the friend class can create instances
        /**
        * @param args NodeInfo of the device to use.
        */
        OpenNIGrabberImpl(std::string args);

        /// makes the MapGenerator grab a new image. called repeatedly in thread.
        void grabNextImage();

        /**
            switches the current generator to desired. this function works but
            after changing to another Generator the camcfg-properties will not
            be refreshed.
        **/
        void setGeneratorTo(icl_openni::OpenNIMapGenerator::Generators desired);

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

        /// Returns the string representation of the currently used device.
        std::string getName();

        /// Mutex used for concurrency issues.
        utils::Mutex m_Mutex;
        /// a grabber id
        std::string m_Id;
        /// the OpenNI context
        xn::Context m_Context;
        /// pointer to the currently used image generator
        icl_openni::OpenNIMapGenerator* m_Generator;
        /// internally used ReadWriteBuffer
        icl_openni::ReadWriteBuffer<core::ImgBase>* m_Buffer;
        /// a thread continuously grabbing images
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
        inline OpenNIGrabber(const std::string args="") throw(utils::ICLException) {
          /// looking for OpenNI device compatible to args
          if(isNew(args)){
            OpenNIGrabberImpl* tmp = new OpenNIGrabberImpl(args);
            initialize(tmp, args);
          }else{
            initialize(args);
          }
        }

        /// returns the underlying handle of the grabber. In this case the corresponding MapGenerator.
        virtual void* getHandle(){
          return m_instance -> ptr -> getHandle();
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
                    GrabberDeviceDescription("oni", utils::str(i) + "|||" + utils::str(i), utils::str(i))
                    );
            }
            context.Release();
          }
          return deviceList;
        }
    };

  } // namespace io
} //namespace icl


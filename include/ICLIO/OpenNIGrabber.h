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

#include <XnOS.h>
#include <XnCppWrapper.h>

namespace icl {

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

      private:
        /// The constructor is private so only the friend class can create instances
        /**
        * @param device NodeInfo of the device to use.
        */
        OpenNIGrabberImpl(std::string name, std::string args);

        /// Returns a string rep((DepthGenerator*) m_Generator)resentation of NodeInfo.
        static std::string getTreeStringRepresentation(xn::NodeInfo info);

        /// gets progargs and finds the corresponding device
        static std::string getDeviceNodeNameFromArgs(std::string args);

        /// creates a NodeInfo corresponding to passed TreeStringRepresentation.
        xn::NodeInfo* createDeviceFromName(std::string name);

        /// Returns the string representation of the currently used device.
        std::string getName();

        /// switches generator.
        void setGeneratorTo(std::string value);

        /// the OpenNI context
        OpenNIAutoContext m_AutoContext;
        xn::Context* m_Context;
        /// A NodeInfo describing the underlying device.
        xn::NodeInfo* m_Device;
        /// A mutex lock to synchronize buffer and color converter access.
        Mutex m_GeneratorLock;
        /// This pointer holds an OpenNIImageGenerator
        OpenNIImageGenerator* m_Generator;
        /// Which generator should be used on next acqusition.
        OpenNIImageGenerator::Generators m_SetToGenerator;
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
      DEBUG_LOG("get device name")
      std::string name = OpenNIGrabberImpl::getDeviceNodeNameFromArgs(args);
      DEBUG_LOG(name)
        if(isNew(name)){
            DEBUG_LOG("isnew")
            OpenNIGrabberImpl* tmp = new OpenNIGrabberImpl(name, args);
            initialize(tmp, name);
        }else{
          DEBUG_LOG("isold Grabber")
          initialize(name);
        }
      DEBUG_LOG("end")
      }
    
      static std::vector<GrabberDeviceDescription> getDeviceList(bool rescan){
        static std::vector<GrabberDeviceDescription> deviceList;
        if(rescan){
          deviceList.clear();
          xn::Context context;
          xn::NodeInfoList nodes;
          context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL , nodes, NULL);
          int i = 0;
          for (xn::NodeInfoList::Iterator it = nodes.Begin(); it != nodes.End(); ++it){
            std::string name = OpenNIGrabberImpl::getTreeStringRepresentation(*it);
            deviceList.push_back(
              GrabberDeviceDescription(
                "oni",
                str(i) + "|||" + name,
                name
                )
              );
            i++;
          }
        }
      return deviceList;
      }
    };

} //namespace icl

#endif


/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#ifndef ICL_OPENNI_GRABBER_H
#define ICL_OPENNI_GRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Time.h>

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
        * @param args The arguments provided to this grabber.
        */
        OpenNIGrabberImpl(const std::string args);

        /// Returna a unique qualifier for the currently used camera.
        std::string getName();

        /// A mutex lock to synchronize buffer and color converter access.
        Mutex m_ImgMutex;
    };

    /// Grabber implementation for OpenNI based camera access.
    /**
        This is just a wrapper class of the underlying OpenNIGrabberImpl class
    **/
    struct OpenNIGrabber : public GrabberHandle<OpenNIGrabberImpl>{

      /// create a new OpenNIGrabber
      /** @see OpenNIGrabber for more details*/
      inline OpenNIGrabber(const std::string args="") throw(ICLException) {
      /// looking for Pylon device compatible to args
        OpenNIGrabberImpl* tmp = new OpenNIGrabberImpl(args);
        if(isNew(tmp -> getName())){
            initialize(tmp, tmp -> getName());
        }else{
          initialize(tmp -> getName());
        }
      }
    
      static std::vector<GrabberDeviceDescription> getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
        if(rescan){
          deviceList.clear();
          deviceList.push_back(
            GrabberDeviceDescription(
              "oni",
              "0|||someNiDevice",
                "someNiDevice"
              )
            );
        }
      return deviceList;
      }
    };

} //namespace icl

#endif


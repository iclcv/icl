/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/RSBGrabber.h                             **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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


#ifndef ICL_RSB_GRABBER_H
#define ICL_RSB_GRABBER_H

#include <ICLIO/GrabberHandle.h>

#if !defined(HAVE_RSB) || !defined(HAVE_PROTOBUF)
#warning "This header should only be included if HAVE_RSB and HAVE_PROTOBUF are defined and available in ICL"
#endif

namespace icl{
  namespace io{
    
    /// Grabber implementation for RSB based image transfer
    class RSBGrabberImpl : public Grabber{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
      
      public:
      
      /// empty constructor (creates a null instance)
      RSBGrabberImpl();
  
      /// Destructor
      ~RSBGrabberImpl();
      
      /// main constructor with given scope and comma separated transportList
      /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
      RSBGrabberImpl(const std::string &scope, const std::string &transportList="spread");
      
      /// deferred intialization with given scope and comma separated transportList
      /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
      void init(const std::string &scope, const std::string &transportList="spread");
      
      /// grabber-interface 
      virtual const ImgBase *acquireImage();
      
      /// returns whether this grabber has not jet been initialized
      inline bool isNull() const { return !m_data; }
  
      /// returns a list of all available rsb streams
      static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
  
      /// interface for the setter function for video device properties 
      /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
      virtual void setProperty(const std::string &property, const std::string &value);
      
      /// returns a list of properties, that can be set usingsetProperty
      /** @return list of supported property names **/
      virtual std::vector<std::string> getPropertyList();
      
      /// get type of property
      /** \copydoc icl::Grabber::getType(const std::string &)*/
      virtual std::string getType(const std::string &name);
  
      /// get information of a property valid values
      /** \copydoc icl::Grabber::getInfo(const std::string &)*/
      virtual std::string getInfo(const std::string &name);
  
      /// returns the current value of a property or a parameter
      virtual std::string getValue(const std::string &name);
      /** @} */
  
      /// volatileness
      virtual int isVolatile(const std::string &name);
  
    };
  
    /// Grabber class that grabs images from RSB scope
    /** for more details: @see RSBGrabberImpl */
    class RSBGrabber : public GrabberHandle<RSBGrabberImpl>{
      public:
      
      /// returns a list of available pwc devices 
      /** @see RSBGrabberImpl for more details*/
      static inline const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
        return RSBGrabberImpl::getDeviceList(rescan);
      }
      
      /// creates a new RSBGrabber instance
      /** @see RSBGrabberImpl for more details */
      inline RSBGrabber(const std::string &scope, const std::string &transportList="spread"){
        std::string uid = transportList+":"+scope;
        if(isNew(uid)){
          initialize(new RSBGrabberImpl(scope, transportList),uid);
        }else{
          initialize(uid);
        }
      }
      /// empty constructor (initialize late using init())
      /** @see RSBGrabberImpl for more details */
      inline RSBGrabber(){}
      
      /// for deferred connection to (other) shared memory segment
      /** @see RSBGrabberImpl for more details */
      inline void init(const std::string &scope, const std::string &transportList="spread") throw (ICLException){
        std::string uid = transportList+":"+scope;
        if(isNew(uid)){
          initialize(new RSBGrabberImpl(scope, transportList),uid);
        }else{
          initialize(uid);
        }
      }
      
      /// not necessary for this type
      static void resetBus(){}
    };  
  
  } // namespace io
}

#endif

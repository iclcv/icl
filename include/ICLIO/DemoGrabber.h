/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/DemoGrabber.h                            **
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

#pragma once

#include <ICLCore/Color.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/GrabberHandle.h>

namespace icl{
  namespace io{
  
    
    /// Implementation class for the DemoGrabber
    class DemoGrabberImpl : public Grabber{
      public:
      friend class DemoGrabber;
  
      /// default grab function
      virtual const core::ImgBase* acquireImage();
  
      /// Destructor
      ~DemoGrabberImpl();
      
      private:
      /// Create a DemoGrabber with given max. fps count
      DemoGrabberImpl(float maxFPS=30);
      
      /// Current rel. location of the rect
      utils::Point32f m_x; 
      
      /// Current rel. velocity of the rect
      utils::Point32f m_v;
  
      /// maximum velocity of the rect
      utils::Point32f m_maxV;
  
      /// relative size of the rect
      utils::Size32f m_size;
  
      /// Color of the rect (light red)
      core::Color m_color;
      
      /// max. fpsCount for this grabber instance
      float m_maxFPS;
      
      /// time variable to ensure max. fpsCount
      utils::Time m_lastTime;
  
      /// extra buffer for the output image
      core::ImgBase *m_drawBuffer;
      
      /// current output format
      core::format m_drawFormat;
  
      /// current output depth
      core::depth m_drawDepth;
      
      /// current output size
      utils::Size m_drawSize;
      
      /// mutex for locking properties and grabbing
      utils::Mutex m_mutex;
      
      public:
      /// the demo-grabber provides some demo properties
      virtual std::vector<std::string> getPropertyListC();
      
      /// the demo-grabber provides some demo properties
      virtual void setProperty(const std::string &property, const std::string &value);
  
      /// the demo-grabber provides some demo properties
      virtual std::string getType(const std::string &name);
  
      /// the demo-grabber provides some demo properties
      virtual std::string getInfo(const std::string &name);
  
      /// the demo-grabber provides some demo properties
      virtual std::string getValue(const std::string &name);
  
      /// the demo-grabber provides some demo properties
      virtual int isVolatile(const std::string &propertyName);

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);
    };  
  
    /// Demo Grabber class providing am image with a moving rect
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It can be set up to work at a certain fps to avoid
        some real unexpected behaviour */
    class DemoGrabber : public GrabberHandle<DemoGrabberImpl>{
      public:
      inline DemoGrabber(float maxFPS=30){
        std::string id = utils::str(maxFPS);
        if(isNew(id)){
          initialize(new DemoGrabberImpl(maxFPS),id);
        }else{
          initialize(id);
        }
      }
    };
  } // namespace io
}


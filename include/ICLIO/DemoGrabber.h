/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#ifndef ICL_DEMO_GRABBER_H
#define ICL_DEMO_GRABBER_H

#include <ICLCC/Color.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/GrabberHandle.h>

namespace icl{

  
  /// Implementation class for the DemoGrabber
  class DemoGrabberImpl : public Grabber{
    public:
    friend class DemoGrabber;

    /// default grab function
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);
    
    private:
    /// Create a DemoGrabber with given max. fps count
    DemoGrabberImpl(float maxFPS=30);

    /// Current rel. location of the rect
    Point32f m_x; 
    
    /// Current rel. velocity of the rect
    Point32f m_v;

    /// maximum velocity of the rect
    Point32f m_maxV;

    /// relative size of the rect
    Size32f m_size;

    /// Color of the rect (light red)
    Color m_color;
    
    /// max. fpsCount for this grabber instance
    float m_maxFPS;
    
    /// time variable to ensure max. fpsCount
    Time m_lastTime;
    
  };  
  /** \endcond */

  /// Demo Grabber class providing am image with a moving rect
  /** This grabber can be used as placeholder whenever no senseful Grabber
      is available. It can be set up to work at a certain fps to avoid
      some real unexpected behaviour */
  class DemoGrabber : public GrabberHandle<DemoGrabberImpl>{
    public:
    inline DemoGrabber(float maxFPS=30){
      std::string id = str(maxFPS);
      if(isNew(id)){
        initialize(new DemoGrabberImpl(maxFPS),id);
      }else{
        initialize(id);
      }
    }
  };
}

#endif

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/RSBRemoteGUI.h                         **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <string>

namespace icl{
  namespace qt{ 
    
    /** \cond */
    class GUI; 
    /** \endcond */ 
    
    /// Creates an RSB-interface to externally get and set parameters of the wrapped GUI
    /** For each GUI-component that has a given 'handle', the
        RSBRemoteGUI will create an RSB-informer that informs if the
        component's content changes or was pressed and an RSB-listener
        that can be used to externally set the component state. 
        As informer scopes, the given base-scope is used and the component's handle is
        appended as sub-scope. E.g. if the base-scope is /foo/bar, and the following GUI
        layout is used (@see qt::DynamicGUI):
        <pre>
        <vbox>         
          <image handle="vis" label="Current image"/>
          <hbox>
             <slider args="0,255,128" handle="threshold" label="The Threshold"/>
             <button args="Click Me" handle="the button"/>
          </hbox>
        </vbox>
        </pre>
        , the following RSB-informers are created
        * /foo/bar/vis Which sends stringified mouse-events that occur on the image component
        * /foo/bar/threshold Which sends slider valus as strings
        * /foo/bar/the_button Which sends a "1" string when the button was pressed
        
        Please note, that spaces in the handle-names are internally
        translated to underscore characters as RSB-scopes do not
        support spaces. All informers uses std::strings as type.

        In addition to the informers, each component is also endowed with an RSB-listener:
        * /foo/bar/set/vis Allows for setting the visualized image (*this is a more complex listener)
        * /foo/bar/set/threshold Allows for setting the current threshold slider value
        * /foo/bar/set/the_button Allows for externally 'clicking' the button

        Please note, that for externally setting values of the
        RemoteGUI, the "/set/" subscope is inserted. A special
        treatment was needed for image, draw and draw3D componenents,
        as setting images using a stringified transport syntax would
        be very inefficient. Instead, for each component of that type,
        a standard io::RSBGrabber instance is instantiated and
        internally automatically synchronized with the corresponding
        GUI component, so that other applications can simply send
        images using an io::RSBImageOutput (we strongly recommend to
        use the io::GenericImageSender instead) allows for setting
    */
    class RSBRemoteGUI : public utils::Uncopyable{
      struct Data;   //!< internal data class
      Data *m_data;  //!< internal data pointer

      public:
      /// Constructor that wraps around an exisiting GUI instance
      /** The existing GUI instance must have been 'created' before.
          An empty rsbBaseScope can only be used in the null-construction case, i.e.
          when the gui-argument is also null.
          
          The createSetterGUI argument is used for debugging purpose only. It allows to 
          create the counter-part of an RSBRemoteGUI for testing the GUI synchronization
      */
      RSBRemoteGUI(GUI *gui=0, const std::string &rsbBaseScope="", bool createSetterGUI=false, bool verboseMode=false);
      
      /// sets up the verbose mode, that writes some debug info to std::out
      void setVerboseMode(bool on);
      
      /// initializes this instance by wrapping around the given GUI
      /** @see RSBRemoteGUI::RSBRemoteGUI*/
      void init(GUI *gui, const std::string &rsbBaseScope, bool createSetterGUI=false);
      
      /// Destructor
      ~RSBRemoteGUI();
    };
  }
}

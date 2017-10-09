/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/CamCfgWidget.h                         **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/ImgBase.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <QSplitter>

namespace icl{
  namespace qt{

    /// Special QWidget implementation for configuring grabber properties
    /** \section GEN General Information
        The CamCfgWidget can be used if an application needs to configure
        the properties of an ICL Grabber instance. It automatically connects
        to the corresponding Grabber instance (it uses an instance of type
        ICLIO/GenericGrabber) and provides an interface for it's properties.
        The CamCfgWidget can be created in two modes:

        \section MODES Widget Modes
        The CamCfgWidget can be used in two different modes:
        - <b>A Simple Mode:</b>\n
          Here, the widget contains only components for
          - setting up grabber device properties
          - saving all properties to an xml-file
          - loading an xml-property file
          - If the Widget was created without parameters (i.e. 2nd constructor
            gets "" and "" as it's first two arguments, then the widget will
            also create a combo-box for selecting <em>available</em> devices.
            Devices are <em>available</em> a grabber for this device is already
            instantiated, or if The GenericGrabber::getDeviceList(..) function
            found this device.\n
            This mode is used if a icl::GUI-compoment "camcfg()" or e.g.
            "camcfg(dc,0)" is created.
            \image html camcfg.png "The CamCfgWiget in it's simple mode. Here as popup from the icl-xcf-publisher application"

        - <b>A Complex Mode:</b>\n
          This mode is used for the application icl-camcfg. Here, the Widget
          does also contain these components:
          - The device selection combo-box is always enabled here
          - A preview-component, that shows the currently grabbed image
          - A core::format and a size combo-box. These two allow to adjust the
            special grabber properties 'core::format' and 'size', which change the actual
            image acquisition from the camera
          - a start/stop grabbing button. As long as this button is toggled,
            the grabber is used in order to visualize preview-images in the image
            preview field
          - An FPS limiter that can be used to limit the maximum number of
            frames per second captured.
          - Some components, that allow to ajust the grabbers desired parameters
            (core::depth, core::format and size) and wheter these are used or ignored.
          - a 'rescan' button that allows for rescanning devices at run-time. However,
            it's worse mention that this feature heavily depends on the used devices'
            underlying libraries. I.e. this might sometimes not show new devices and
            even sometimes mess up already existent devices.
        \image html camcfg.png "The CamCfgWiget used for the icl-camcfg application"
    */
    class ICLQt_API CamCfgWidget : public QWidget{
      class Data;  //!< internal data class
      Data* data; //!< internal data storage

      public:
      /// Creates a full Configuration Widget with device and preview widget
      CamCfgWidget(const std::string &deviceFilter="",QWidget *parent=0);

      // TODO: where is the definition for this?
      /// Creates a Configuration Widget for a single device
      //CamCfgWidget(const std::string &devType="", const std::string &devID="", QWidget *parent=0);

      /// Destructor
      ~CamCfgWidget();

      /// returns the current image
      virtual const core::ImgBase *getCurrentImage();

      /// reimplemented
      virtual void setVisible (bool visible);

      /// used as GUI::Callback for processing GUI events
      void callback(const std::string &handle);

      /// updates CamCfgGui
      virtual void update();


      private:
      void scan();
    };

  } // namespace qt
}


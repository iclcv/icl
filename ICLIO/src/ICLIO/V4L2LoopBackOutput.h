/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/V4L2LoopBackOutput.h                   **
** Module : ICLIO                                                  **
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
#include <ICLIO/SharedMemorySegment.h>
#include <ICLIO/ImageOutput.h>

namespace icl{
  namespace io{

    /// ImageOutput implementation for V4L2-Looback devices
    /** The V4L2LoopBackOutput allows you to send images through a V4L2 loopback device.
        Please don't mix these up with common web-cam devices.
        You must install the v4l2looback kernel module fisrt. Once this is installed,
        you can load the kernel module and instantiate a number of loopback devices using
        <pre>sudo modprobe v4l2loopback devices=1</pre>
        Please note that the next free devices are used. I.e. if you alread have a webcam
        or built-in can in your PC/Laptop, which uses /dev/video0, the command above will
        automatically create /dev/video1 as a loopback-device

        Once the device is created it must be initalized using
        <pre>v4l2loopback-ctl set-caps 'any' /dev/video1</pre>

        If the device is somehow broken or it can't be accessed anymore, you can re-initialize it
        using
        <pre>
        sudo modprobe -r v4l2loopback && sleep 1 && sudo modprobe v4l2loopback devices=1 && sleep 1 && v4l2loopback-ctl set-caps 'any' /dev/video1
        </pre>

        Please don't use the V4L2LoopBackOutput directly, but the GenericImageOutput with
        the output-specifier 'v4l'

        More information can be found at https://github.com/umlaeute/v4l2loopback
    */

    class ICLIO_API V4L2LoopBackOutput : public ImageOutput{
      struct Data;   //!< internal data structure
      Data *m_data;  //!< internal data pointer

      public:
      /// creates an unintialized instance
      V4L2LoopBackOutput();

      /// Destructor ...
      ~V4L2LoopBackOutput();

      /// creates V4L2LoopBackImageOutput from given device string
      /** The device string can either be the full device-name, such as /dev/video1 or
          just an integer that specifies the device id (1 for /dev/video1 etc.) */
      V4L2LoopBackOutput(const std::string &device) throw (utils::ICLException);

      /// initializes a the deivce
      void init(const std::string &deviceString) throw (utils::ICLException);

      /// actual publishing function
      virtual void send(const core::ImgBase *image);
    };
  }
}

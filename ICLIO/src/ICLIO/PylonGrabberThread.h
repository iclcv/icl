/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonGrabberThread.h                   **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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
#include <ICLUtils/Thread.h>
#include <ICLIO/PylonUtils.h>
#include <ICLIO/PylonColorConverter.h>
#include <ICLIO/PylonCameraOptions.h>

#include <queue>

namespace icl {
  namespace io{
    namespace pylon {
  
      /// Internally spawned thread class for continuous grabbing \ingroup GIGE_G
      class PylonGrabberThread : public utils::Thread {
        public:
          /// Constructor sets all internal fields and allocates memory
          /**
          * @param grabber The IStreamGrabber providing the images.
          * @param converter 
          * @param options 
          */
          ICLIO_API PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                                  PylonColorConverter* converter,
                                  PylonCameraOptions* options);
          /// Destructor frees all allocated memory
          ICLIO_API ~PylonGrabberThread();
          /// acquires images and writes them into an internal queue
          ICLIO_API void run();
          /// reinitializes buffer
          ICLIO_API void resetBuffer();
          /// getter for the most current image
          /**
          * @return a pointer to an internally used TsBuffer the buffer
          *         can safely be used until the next call to
          *         getCurrentImage() or resetBuffer().
          */
          ICLIO_API core::ImgBase* getCurrentImage();
        private:
          /// A pointer to the image-providing StreamGrabber.
          Pylon::IStreamGrabber* m_Grabber;
          /// A pointer to the ColorConverter.
          PylonColorConverter* m_Converter;
          /// A pointer to the CameraOptions.
          PylonCameraOptions* m_Options;
          /// A buffer holding read and write buffers
          ConcGrabberBuffer m_Buffers;
          /// A counter for acquisition errors.
          int m_Error;
          /// A counter for acquisition timeouts.
          int m_Timeout;
          /// A counter for correct acquisitions.
          int m_Acquired;
  
          /// grabs a single image into m_BufferQueue.
          void grab();
    };
  
    }//namespace pylon
  } // namespace io
}//namespace icl

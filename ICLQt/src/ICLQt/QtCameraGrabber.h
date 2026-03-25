/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2015 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QtCameraGrabber.h                      **
** Module : ICLQt                                                  **
** Authors: Matthias Esau                                          **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#pragma once

#include <QtMultimedia/QCamera>
#include <QtMultimedia/QMediaCaptureSession>
#include <ICLQt/Common.h>
#include <ICLQt/ICLVideoSurface.h>

namespace icl{
  namespace qt{
    class ICLQt_API QtCameraGrabber: public icl::io::Grabber{
      public:

        /// Create Camera grabber with given device id or name
        QtCameraGrabber(const std::string &device="0");

        /// Destructor
        ~QtCameraGrabber();

        /// grab function
        virtual const core::ImgBase *acquireDisplay();

      protected:
        QCamera* cam;
        QMediaCaptureSession* captureSession;
        ICLVideoSurface* surface;
    };
  }
}

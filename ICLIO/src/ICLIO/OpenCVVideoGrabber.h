/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenCVVideoGrabber.h                   **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski, Viktor Richter                   **
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
#include <ICLCore/OpenCV.h>
#include <ICLIO/Grabber.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Exception.h>
#ifdef ICL_HAVE_OPENCV
#include <opencv/highgui.h>
#endif

#include <string>
#include <ICLUtils/File.h>
#include <ICLUtils/Mutex.h>
namespace icl{
  namespace io{

    /// opencv base grabber implementation for movie files \ingroup MOVIE_FILE_G
    class ICLIO_API OpenCVVideoGrabber : public Grabber{
        struct Data; //!< pimpl type
        Data *data; //!< pimpl pointer
        utils::Mutex mutex; //! locking
        bool updating; //! used while updating configurable

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

      public:
        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        virtual const core::ImgBase *acquireImage();

        /// Constructor creates a new OpenCVVideoGrabber instance
        /** @param fileName name of file to use */
        OpenCVVideoGrabber(const std::string &fileName) throw (utils::FileNotFoundException);

        /// Destructor
        ~OpenCVVideoGrabber();
    };

  } // namespace io
}

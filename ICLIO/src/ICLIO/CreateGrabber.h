/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/CreateGrabber.h                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <ICLCore/Color.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/Grabber.h>

namespace icl{
  namespace io{

    
    /// Create Grabber class that provides an image from ICL's create function
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It provides an instance of an image that is created with
        the icl::io::TestImages::create function */
    class CreateGrabber : public Grabber{
      public:

        /// default grab function
        virtual const core::ImgBase* acquireImage();

        /// Create a CreateGrabber with given max. fps count
        CreateGrabber(const std::string &what);

        /// Destructor
        ~CreateGrabber();

      private:

        /// internal image
        core::ImgBase *m_image;
        /// tells whether timestamp is actualized on each grab
        bool m_updateTimeStamp;

        /// callback function for property changes.
        void processPropertyChange(const utils::Configurable::Property &p);
    };

  } // namespace io
}


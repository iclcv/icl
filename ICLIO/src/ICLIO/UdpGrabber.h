/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/UdpGrabber.h                           **
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

#include <ICLIO/Grabber.h>
#include <QtCore/QObject>

namespace icl{
  namespace io{
    
    /// Grabber class that grabs images from Udp-Network sockets
    /** This class is only available if Qt is supported, because 
        QUdpSocket is used internally
    */
    class UdpGrabber : public QObject, public Grabber {
      Q_OBJECT;
      /// Internal Data storage class
      struct Data;
      
      /// Hidden Data container
      Data *m_data;
      
      /// Connects an unconnected grabber to given shared memory segment
      void init(int port) throw (utils::ICLException);
      
      private slots:
      
      /// hand incomming data
      void processData();

      public:
      
      /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
      UdpGrabber(int port=-1) throw(utils::ICLException);
      
      /// Destructor
      ~UdpGrabber();
      
      /// returns a list of all available shared-memory image-streams
      static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
      
      /// grabbing function
      /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
      virtual const core::ImgBase* acquireImage();
    };
    
  } // namespace io
}


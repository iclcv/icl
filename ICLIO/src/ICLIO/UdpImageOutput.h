/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/UdpImageOutput.h                       **
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

#include <ICLIO/GenericImageOutput.h>

namespace icl{
  namespace io{
    
    /// image output implementation for Udp-based network transfer
    class UdpImageOutput : public ImageOutput{
      public:
      struct Data;  //!< pimpl type

      private:
      Data *m_data; //!< pimpl pointer
      
      public:
  
      
      /// Create UdpImageOutput with given targetPC and port
      /** Of targetPC is "", a null output is created, that must be initialized
          with init before send can be called */
      UdpImageOutput(const std::string &targetPC="", int port=44444);

      /// Destructor
      ~UdpImageOutput();

      
      /// deferred initialization 
      void init(const std::string &targetPC, int port=44444);
      
      /// sender method
      virtual void send(const core::ImgBase *image);
      
      /// returns whether this is a null instance
      inline bool isNull() const { return !m_data; }
      
      /// returns whether this is not a null instance
      inline operator bool() const { return static_cast<bool>(m_data); }
    };
  } // namespace io
}


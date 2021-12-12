/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/RSBImageOutput.h                       **
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
#include <ICLIO/GenericImageOutput.h>

#if !defined(ICL_HAVE_RSB) || !defined(ICL_HAVE_PROTOBUF)
  #if WIN32
    #pragma WARNING("This header should only be included if ICL_HAVE_RSB and ICL_HAVE_PROTOBUF are defined and available in ICL")
  #else
    #warning "This header should only be included if ICL_HAVE_RSB and ICL_HAVE_PROTOBUF are defined and available in ICL"
  #endif
#endif
namespace icl{
  namespace io{

    /// image output implementation using the "Robotics Service Bus (RSB)"
    class RSBImageOutput : public ImageOutput{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer

      public:
      /// creates a null instance
      ICLIO_API RSBImageOutput();

      /// Destructor
      ICLIO_API ~RSBImageOutput();

      /// create RSBImageOutput with given scope and comma separated transport list
      /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
      ICLIO_API RSBImageOutput(const std::string &scope, const std::string &transportList = "spread");

      /// deferred initialization (see RSBImageOutput(const string&,const string&)
      /** supported transports are socket, spread and inprocess. Please note, that
          the spread-transport requires spread running. */
      ICLIO_API void init(const std::string &scope, const std::string &transportList = "spread");

      /// sender method
      ICLIO_API virtual void send(const core::ImgBase *image);

      /// returns whether this is a null instance
      inline bool isNull() const { return !m_data; }

      /// returns whether this is not a null instance
      inline operator bool() const { return static_cast<bool>(m_data); }
    };
  } // namespace io
}

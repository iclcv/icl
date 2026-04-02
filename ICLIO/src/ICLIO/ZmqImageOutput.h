// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLIO/GenericImageOutput.h>

namespace icl::io {
    /// image output implementation using the ZeroMQ (ZMQ) framwork
    class ZmqImageOutput : public ImageOutput{
      public:
      struct Data;  //!< pimpl type

      private:
      Data *m_data; //!< pimpl pointer

      public:


      /// Create UdpImageOutput with given targetPC and port
      /** Of targetPC is "", a null output is created, that must be initialized
          with init before send can be called */
      ICLIO_API ZmqImageOutput(int port=44444);

      /// Destructor
      ICLIO_API ~ZmqImageOutput();


      /// deferred initialization
      ICLIO_API void init(int port=44444);

      /// sender method
      ICLIO_API virtual void send(const core::Image &image);

      /// returns whether this is a null instance
      inline bool isNull() const { return !m_data; }

      /// returns whether this is not a null instance
      inline operator bool() const { return static_cast<bool>(m_data); }
    };
  } // namespace icl::io
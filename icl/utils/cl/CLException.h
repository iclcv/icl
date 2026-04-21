// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Losing, Christof Elbrechter

#ifdef ICL_HAVE_OPENCL
#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>
#include <string>
#include <sstream>
#include <iostream>

namespace icl::utils {
	    /// Base class for an OpenCL Exception
  class CLException: public ICLException {
		public:
			static std::string getMessage(const int errorCode, const std::string& message){
				std::stringstream sstr;
				sstr << message << " clErrorCode " << errorCode << "\n";
				return sstr.str();
			}
			CLException(const std::string &msg) noexcept : ICLException(msg) {}
		};
		/// Class for an OpenCL Exception during initialization
  class CLInitException: public CLException {
		public:
			CLInitException(const std::string &msg) noexcept : CLException(msg) {}
		};
		/// Class for an OpenCL Exception during kernel compiling
  class CLBuildException: public CLException {
		public:
			CLBuildException(const std::string &msg) noexcept : CLException(msg) {}
		};
		/// Class for an OpenCL Exception associated with buffers
  class CLBufferException: public CLException {
		public:
			CLBufferException(const std::string &msg) noexcept : CLException(msg) {}
		};

		/// Class for an OpenCL Exception associated with kernels
  class CLKernelException: public CLException {
		public:
			CLKernelException(const std::string &msg) noexcept : CLException(msg) {}
		};

	}
#endif

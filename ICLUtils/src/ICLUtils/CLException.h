/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLException.cpp                  **
** Module : ICLUtils                                               **
** Authors: Viktor Losing                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#ifdef HAVE_OPENCL
#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <ICLUtils/Exception.h>
using namespace std;
namespace icl {
	namespace utils {
	    /// Base class for an OpenCL Exception
		class CLException: public ICLException {
		public:
			static string getMessage(const int errorCode, const string message){
				std::stringstream sstr;
				sstr << message << " clErrorCode " << errorCode << endl;
				return sstr.str();
			}
			CLException(const std::string &msg) throw() : ICLException(msg) {}
		};
		/// Class for an OpenCL Exception during initialization
		class CLInitException: public CLException {
		public:
			CLInitException(const std::string &msg) throw() : CLException(msg) {}
		};
		/// Class for an OpenCL Exception during kernel compiling
		class CLBuildException: public CLException {
		public:
			CLBuildException(const std::string &msg) throw() : CLException(msg) {}
		};
		/// Class for an OpenCL Exception associated with buffers
		class CLBufferException: public CLException {
		public:
			CLBufferException(const std::string &msg) throw() : CLException(msg) {}
		};

		/// Class for an OpenCL Exception associated with kernels
		class CLKernelException: public CLException {
		public:
			CLKernelException(const std::string &msg) throw() : CLException(msg) {}
		};

	}

}
#endif


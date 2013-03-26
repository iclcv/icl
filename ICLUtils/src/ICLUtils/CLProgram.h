/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLProgram.h                      **
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
//XXVL Freigabe bei ImageUndistortion von Impl scheint zu fehlen...

#ifdef HAVE_OPENCL
#pragma once
#include <string.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#include <fstream>
#include <ICLUtils/CLException.h>

using namespace std;
namespace icl {
	namespace utils {

	/// Wrapper for an OpenCL Program
	    /**
	      This class provides a simple initialization of OpenCL including platform
	      and device selection. The kernel sources can be initialized by filestream
	      or string. Platform and device various device informations may be listed.

	      example:
          static char KernelText[] = "	__kernel void convolve(...)";

	      CLProgram program("gpu", KernelText);
		  program.listSelectedDevice();
		  CLBuffer inputBuffer = program.createBuffer("r",
				sizeof(sizeof(unsigned int)) * inputSignalHeight * inputSignalWidth,
				inputSignal);
		  CLBuffer outputSignalBuffer = program.createBuffer("w",
				sizeof(unsigned int) * outputSignalHeight * outputSignalWidth);
		  CLKernel kernel = program.createKernel("convolve");
     	  kernel.setArgs(inputSignalBuffer, outputSignalBuffer, inputSignalWidth, maskWidth);
		  kernel.apply(outputSignalWidth * outputSignalHeight, 0, 0);
		  outputSignalBuffer.read(outputSignal,
				outputSignalWidth * outputSignalHeight * sizeof(unsigned int));
	     *
	    **/
		class CLProgram {
		public:
			struct Impl;

		private:
			Impl *impl;
		public:
			//deviceType = "gpu" only gpu devices
			//deviceType = "cpu" only cpu devices
			CLProgram(const string deviceType, const string &sourceCode) throw(CLInitException, CLBuildException);
			CLProgram(const string deviceType, ifstream &fileStream) throw(CLInitException, CLBuildException);
			~CLProgram();
			CLProgram();
			CLProgram(const CLProgram& other);
			CLProgram const& operator=(CLProgram const& other);

			//accessMode = "r" only read
			//accessMode = "w" only write
			//accessMode = "rw" read and write
			/// creates a buffer object
			CLBuffer createBuffer(const string &accessMode, size_t size, void *src=0) throw(CLBufferException);
			/// creates a kernel
			CLKernel createKernel(const string &id) throw (CLKernelException);

			/// lists various properties of the selected platform
			void listSelectedPlatform();
			/// lists various properties of the selected device
			void listSelectedDevice();
			/// lists various properties of all platforms and their devices
			static void listAllPlatformsAndDevices();
			/// lists various properties of all platforms
			static void listAllPlatforms();
		}
		;

	}
}
#endif

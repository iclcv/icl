/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLProgram.cpp                    **
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
#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/StringUtils.h>

#include <ICLUtils/CLIncludes.h>

#include <iostream>
#include <sstream>
#include <exception>
#include <set>
#include <map>
#include <vector>

namespace icl {
	namespace utils {

	// /////////////////////////////////////////////////////////////////////////////////////////////

	struct CLProgram::Impl {

		Impl() : is_valid(false), program(nullptr) {}

		~Impl() {
			if (program) {
				clReleaseProgram(program);
				program = nullptr;
			}
		}

		bool is_valid;
		CLDeviceContext deviceContext;
		cl_program program;

		void initProgram(const std::string &sourceText) {
			const char *src = sourceText.c_str();
			size_t len = sourceText.size();
			cl_int err = CL_SUCCESS;

			cl_context context = deviceContext.getContext();
			program = clCreateProgramWithSource(context, 1, &src, &len, &err);
			if (err != CL_SUCCESS) {
				throw CLBuildException(CLException::getMessage(err, "clCreateProgramWithSource"));
			}

			cl_device_id device = deviceContext.getDevice();
			err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
			if (err != CL_SUCCESS) {
				// Retrieve build log
				size_t logSize = 0;
				clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
				std::vector<char> buildLog(logSize + 1, '\0');
				clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog.data(), nullptr);
				throw CLBuildException(std::string(buildLog.data()));
			}

			is_valid = true;
		}

		void initDevice(const std::string device_type_str) {
			deviceContext = CLDeviceContext(device_type_str);
		}

		void initDevice(CLDeviceContext const &device_context) {
			deviceContext = device_context;
		}

		CLBuffer createBuffer(const std::string &accessMode, size_t size,
							  const void *src = 0) {
			return deviceContext.createBuffer(accessMode, size, src);
		}

		CLImage2D createImage2D(const std::string &accessMode, const size_t width, const size_t height,
								int depth, int num_channel, const void *src=0){
			return deviceContext.createImage2D(accessMode, width, height, depth, num_channel, src);
		}

		CLKernel createKernel(const std::string &id) {
			return CLKernel(id, program, deviceContext.getCommandQueue());
		}

		// Release the current program (if any) without destroying the Impl
		void releaseProgram() {
			if (program) {
				clReleaseProgram(program);
				program = nullptr;
			}
		}

		// Copy program handle from another Impl, retaining the reference
		void copyFrom(const Impl &other) {
			releaseProgram();
			deviceContext = other.deviceContext;
			program = other.program;
			is_valid = other.is_valid;
			if (program) {
				clRetainProgram(program);
			}
		}
	};

	// /////////////////////////////////////////////////////////////////////////////////////////////

	CLProgram::CLProgram(const std::string &deviceType, const std::string &sourceCode) {
		impl = new Impl();
		try {
			impl->initDevice(deviceType);
			impl->initProgram(sourceCode);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const std::string &deviceType, std::ifstream &fileStream) {
		impl = new Impl();
		try {
			impl->initDevice(deviceType);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const std::string &sourceCode, const CLProgram &parent) {
		impl = new Impl();
		try {
			impl->initDevice(parent.impl->deviceContext);
			impl->initProgram(sourceCode);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(std::ifstream &fileStream, const CLProgram &parent) {
		impl = new Impl();
		try {
			impl->initDevice(parent.impl->deviceContext);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const CLProgram& other){
		impl = new Impl();
		impl->copyFrom(*other.impl);
	}

	CLProgram const& CLProgram::operator=(CLProgram const& other){
		impl->copyFrom(*other.impl);
		return *this;
	}

	CLProgram::CLProgram(){
		impl = new Impl();
	}

	CLProgram::~CLProgram() {
		delete impl;
	}

	CLProgram::CLProgram(const CLDeviceContext &device_context, const std::string &sourceCode) {
		impl = new Impl();
		try {
			impl->initDevice(device_context);
			impl->initProgram(sourceCode);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const CLDeviceContext &device_context, std::ifstream &fileStream) {
		impl = new Impl();
		try {
			impl->initDevice(device_context);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLBuffer CLProgram::createBuffer(const std::string &accessMode, size_t size,
									 const void *src) {
		return impl->deviceContext.createBuffer(accessMode, size, src);
	}

	CLImage2D CLProgram::createImage2D(const std::string &accessMode, const size_t width, const size_t height, int depth, const void *src){
		return impl->deviceContext.createImage2D(accessMode, width, height, depth, 1, src);
	}

	CLImage2D CLProgram::createImage2D(const std::string &accessMode, const size_t width, const size_t height, int depth, int num_channel, const void *src){
		return impl->deviceContext.createImage2D(accessMode, width, height, depth, num_channel, src);
	}

	CLKernel CLProgram::createKernel(const std::string &id) {
		return impl->createKernel(id);
	}

	bool CLProgram::isValid() {
		return (static_cast<bool>(impl) && impl->is_valid);
	}

	bool CLProgram::isValid() const {
		return (static_cast<bool>(impl) && impl->is_valid);
	}

	void CLProgram::listSelectedPlatform() {
		impl->deviceContext.listSelectedPlatform();
	}
	void CLProgram::listSelectedDevice() {
		impl->deviceContext.listSelectedDevice();
	}

	void CLProgram::listAllPlatformsAndDevices() {
		CLDeviceContext::listAllPlatformsAndDevices();
	}

	void CLProgram::listAllPlatforms() {
		CLDeviceContext::listAllPlatforms();
	}

	CLDeviceContext CLProgram::getDeviceContext() {
		return impl->deviceContext;
	}

  }
}
#endif

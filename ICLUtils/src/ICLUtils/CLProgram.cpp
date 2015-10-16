/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLProgramm.cpp                   **
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
#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching

#include <ICLUtils/CLIncludes.h>

#include <iostream>
#include <sstream>
#include <exception>
#include <set>
#include <map>

using namespace std;
namespace icl {
	namespace utils {

	// /////////////////////////////////////////////////////////////////////////////////////////////

	struct CLProgram::Impl {

		Impl() : is_valid(false) {}

		bool is_valid;
		CLDeviceContext deviceContext;
		cl::Program program;
		void initProgram(const string &sourceText) throw (CLBuildException) {
			cl::Program::Sources sources(1, std::make_pair(sourceText.c_str(), 0)); //kernel source
			program = cl::Program(deviceContext.getContext(), sources);//program (bind context and source)
			std::vector<cl::Device> deviceList;
			deviceList.push_back(deviceContext.getDevice());
			try {
				program.build(deviceList);
			} catch (cl::Error& error) {
				throw CLBuildException(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(deviceContext.getDevice()));
			}
			is_valid = true;
		}
		void initDevice(const std::string device_type_str) throw (CLInitException) {
			try {
				deviceContext = CLDeviceContext(device_type_str);
			} catch (cl::Error& error) { //catch openCL errors
			throw CLInitException(CLException::getMessage(error.err(), error.what()));
			}
		}

		void initDevice(CLDeviceContext const &device_context) throw (CLInitException) {
			try {
				deviceContext = device_context;
			} catch (cl::Error& error) { //catch openCL errors
			throw CLInitException(CLException::getMessage(error.err(), error.what()));
			}
		}

		CLBuffer createBuffer(const string &accessMode, size_t size,
							  const void *src = 0) throw (CLBufferException) {
			return deviceContext.createBuffer(accessMode, size, src);
		}

		CLImage2D createImage2D(const string &accessMode,  const size_t width, const size_t height,
								int depth, int num_channel, const void *src=0) throw(CLBufferException){
			return deviceContext.createImage2D(accessMode, width, height, depth, num_channel, src);
		}

		CLKernel createKernel(const string &id) throw (CLKernelException) {
			return CLKernel(id, program, deviceContext.getCommandQueue());
		}

	};

	// /////////////////////////////////////////////////////////////////////////////////////////////

	CLProgram::CLProgram(const string deviceType, const string &sourceCode)
	throw (CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(deviceType);
			impl->initProgram(sourceCode);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const string deviceType, ifstream &fileStream)
		throw (CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(deviceType);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const string &sourceCode, const CLProgram &parent)
	throw(CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(parent.impl->deviceContext);
			impl->initProgram(sourceCode);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(ifstream &fileStream, const CLProgram &parent)
	throw(CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(parent.impl->deviceContext);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const CLProgram& other){
		impl = new Impl();
		impl->deviceContext = other.impl->deviceContext;
		impl->program = other.impl->program;
		impl->is_valid = other.impl->is_valid;
	}

	CLProgram const& CLProgram::operator=(CLProgram const& other){
		impl->deviceContext = other.impl->deviceContext;
		impl->program = other.impl->program;
		impl->is_valid = other.impl->is_valid;
		return *this;
	}

	CLProgram::CLProgram(){
		impl = new Impl();
	}

	CLProgram::~CLProgram() {
		delete impl;
	}

	CLProgram::CLProgram(const CLDeviceContext &device_context, const string &sourceCode) throw(CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(device_context);
			impl->initProgram(sourceCode);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLProgram::CLProgram(const CLDeviceContext &device_context, ifstream &fileStream) throw(CLInitException, CLBuildException) {
		impl = new Impl();
		try {
			impl->initDevice(device_context);
			std::string srcProg(std::istreambuf_iterator<char>(fileStream),
								(std::istreambuf_iterator<char>()));
			impl->initProgram(srcProg);
		} catch (const CLException &e) {
			delete impl; // ensure impl is released
			throw;
		}
	}

	CLBuffer CLProgram::createBuffer(const string &accessMode, size_t size,
									 const void *src) throw(CLBufferException) {
		return impl->deviceContext.createBuffer(accessMode, size, src);
	}

	CLImage2D CLProgram::createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, const void *src) throw(CLBufferException){
		return impl->deviceContext.createImage2D(accessMode, width, height, depth, 1, src);
	}

	CLImage2D CLProgram::createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, int num_channel, const void *src) throw(CLBufferException){
		return impl->deviceContext.createImage2D(accessMode, width, height, depth, num_channel, src);
	}

	CLKernel CLProgram::createKernel(const string &id) throw (CLKernelException) {
		return impl->createKernel(id);
	}

	bool CLProgram::isValid() {
		return ((bool)impl && impl->is_valid);
	}

	bool CLProgram::isValid() const {
		return ((bool)impl && impl->is_valid);
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

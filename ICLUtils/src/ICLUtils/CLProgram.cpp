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

#include <CL/cl.h>
#undef CL_VERSION_1_2
#include <CL/cl.hpp>
#include <iostream>
#include <sstream>
#include <exception>

using namespace std;
namespace icl {
  namespace utils {

    struct CLProgram::Impl {

      cl::Platform platform;
      cl::Program program;
      cl::Context context;
      cl::Device device;
      cl::CommandQueue cmdQueue;
      void initProgram(const string &sourceText) throw (CLBuildException) {
        cl::Program::Sources sources(1, std::make_pair(sourceText.c_str(), 0)); //kernel source
        program = cl::Program(context, sources);//program (bind context and source)
        std::vector<cl::Device> deviceList;
        deviceList.push_back(device);
        try {
          program.build(deviceList);
        } catch (cl::Error& error) {
          throw CLBuildException(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
        }

      }
      void initDevice(const cl_device_type deviceType) throw (CLInitException) {
        try {
          std::vector<cl::Platform> platformList; //get number of available openCL platforms
          cl::Platform::get(&platformList);
          selectFirstDevice(deviceType, platformList);
          std::cout << "desired openCL " << deviceTypeToString(deviceType) << " device selected" << std::endl;
          std::vector<cl::Device> deviceList;
          deviceList.push_back(device);
          context = cl::Context(deviceList);//select GPU device
          cmdQueue = cl::CommandQueue(context, device, 0);//create command queue
        } catch (cl::Error& error) { //catch openCL errors
          throw CLInitException(CLException::getMessage(error.err(), error.what()));
        }
      }

      void selectFirstDevice(const cl_device_type deviceType,
                             const std::vector<cl::Platform>& platformList) throw (CLInitException) {
        for (unsigned int i = 0; i < platformList.size(); i++) { //check all platforms
          std::vector<cl::Device> deviceList;
          try {
            platformList.at(i).getDevices(deviceType, &deviceList);
            device = deviceList[0];
            platform = platformList[i];
            return;
          } catch(cl::Error &error) {
            /*explicit catching of exception!
                getDevices throws an exception when a platform doesn't provide the desired device,
                in this case the remaining platform should be asked.
                */
          }
        }
        string errorMsg = "No appropriate OpenCL device found for type ";
        errorMsg = errorMsg.append(deviceTypeToString(deviceType));
        throw CLInitException(errorMsg);
      }

      static void listPlatformInfos(cl::Platform & platform) {
        std::string value;
        platform.getInfo(CL_PLATFORM_PROFILE, &value);
        listInfo("CL_PLATFORM_PROFILE", value);
        platform.getInfo(CL_PLATFORM_VERSION, &value);
        listInfo("CL_PLATFORM_VERSION", value);

      }
      static void listDeviceInfos(cl::Device & device) {
        std::string strValue;
        cl_uint uintValue;
        cl_ulong ulongValue;
        std::size_t sizeValue;

        device.getInfo(CL_DEVICE_NAME, &strValue);
        listInfo("CL_DEVICE_NAME", strValue);
        cl_device_type deviceType;
        device.getInfo(CL_DEVICE_TYPE, &deviceType);
        strValue = deviceTypeToString(deviceType);
        listInfo("CL_DEVICE_TYPE", strValue);

        device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &uintValue);
        listInfo("CL_DEVICE_MAX_COMPUTE_UNITS", str(uintValue));

        device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &sizeValue);
        listInfo("CL_DEVICE_MAX_WORK_GROUP_SIZE", str(sizeValue));
        device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &ulongValue);
        listInfo("CL_DEVICE_MAX_MEM_ALLOC_SIZE", str(ulongValue));

        device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &ulongValue);
        listInfo("CL_DEVICE_GLOBAL_MEM_SIZE", str(ulongValue));

        device.getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &ulongValue);
        listInfo("CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE", str(ulongValue));

        device.getInfo(CL_DEVICE_MAX_CONSTANT_ARGS, &uintValue);
        listInfo("CL_DEVICE_MAX_CONSTANT_ARGS", str(uintValue));
        cl_device_local_mem_type memType;
        device.getInfo(CL_DEVICE_LOCAL_MEM_TYPE, &memType);
        listInfo("CL_DEVICE_LOCAL_MEM_TYPE", memTypeToString(memType));

        device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &ulongValue);
        listInfo("CL_DEVICE_LOCAL_MEM_SIZE", str(ulongValue));
        std::cout << std::endl << std::endl;

      }

      static cl_device_type stringToDeviceType(string deviceType) throw (CLInitException) {
        if (deviceType.compare("gpu") == 0) {
          return CL_DEVICE_TYPE_GPU;
        } else if (deviceType.compare("cpu") == 0) {
          return CL_DEVICE_TYPE_CPU;
        }
        string msg = "unknown device type ";
        msg = msg.append(deviceType);
        throw CLInitException(msg);
      }

      static string deviceTypeToString(cl_device_type deviceType) {
        std::string value;
        switch (deviceType) {
          case CL_DEVICE_TYPE_CPU:
            return "cpu";
          case CL_DEVICE_TYPE_GPU:
            return "gpu";
          case CL_DEVICE_TYPE_ACCELERATOR:
            return "accelerator";
          case CL_DEVICE_TYPE_DEFAULT:
            return "default";
          default:
            return "unknown";
        }

      }
      static void listInfo(std::string infoName, std::string infoValue) {
        std::cout << "\t" << infoName << ":\t" << infoValue << std::endl;

      }
      static std::string memTypeToString(cl_device_local_mem_type memType) {
        std::string value;
        switch (memType) {
          case CL_LOCAL:
            return "LOCAL";
          case CL_GLOBAL:
            return "GLOBAL";
          default:
            return "UNKNOWN";
        }
      }

      CLBuffer createBuffer(const string &accessMode, size_t size,
                            const void *src = 0) throw (CLBufferException) {
        return CLBuffer(context, cmdQueue, accessMode, size, src);
      }

      CLImage2D createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, const void *src=0) throw(CLBufferException){
          return CLImage2D(context, cmdQueue, accessMode, width, height, depth, src);
      }
      CLKernel createKernel(const string &id) throw (CLKernelException) {
        return CLKernel(id, program, cmdQueue);
      }

    };

    CLProgram::CLProgram(const string deviceType, const string &sourceCode)
      throw (CLInitException, CLBuildException) {
      impl = new Impl();
      cl_device_type devType = Impl::stringToDeviceType(deviceType);
      try {
        impl->initDevice(devType);
        impl->initProgram(sourceCode);
      } catch (const CLException &e) {
        delete impl; // ensure impl is released
        throw;
      }
    }

    CLProgram::CLProgram(const string deviceType, ifstream &fileStream)
      throw (CLInitException, CLBuildException) {
      impl = new Impl();
      cl_device_type devType = Impl::stringToDeviceType(deviceType);
      try {
        impl->initDevice(devType);
        std::string contents;
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
      impl->cmdQueue = other.impl->cmdQueue;
      impl->platform = other.impl->platform;
      impl->program = other.impl->program;
      impl->context = other.impl->context;
      impl->device = other.impl->device;
    }

    CLProgram const& CLProgram::operator=(CLProgram const& other){
      impl->cmdQueue = other.impl->cmdQueue;
      impl->platform = other.impl->platform;
      impl->program = other.impl->program;
      impl->context = other.impl->context;
      impl->device = other.impl->device;
      return *this;
    }
    CLProgram::CLProgram(){
      impl = new Impl();
    }


    CLProgram::~CLProgram() {
      delete impl;
    }

    CLBuffer CLProgram::createBuffer(const string &accessMode, size_t size,
                                     const void *src) throw(CLBufferException) {
      return impl->createBuffer(accessMode, size, src);
    }

    CLImage2D CLProgram::createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, const void *src) throw(CLBufferException){
        return impl->createImage2D(accessMode, width, height, depth, src);
    }



    CLKernel CLProgram::createKernel(const string &id) throw (CLKernelException) {
      return impl->createKernel(id);
    }

    void CLProgram::listSelectedPlatform() {
      cout << "selected Platform: " << endl;
      Impl::listPlatformInfos(impl->platform);
    }
    void CLProgram::listSelectedDevice() {
      cout << "selected Device: " << endl;
      Impl::listDeviceInfos(impl->device);
    }

    void CLProgram::listAllPlatformsAndDevices() {
      std::vector<cl::Platform> platformList;
      cl::Platform::get(&platformList);
      for (unsigned int i = 0; i < platformList.size(); i++) { //check all platforms
        Impl::listPlatformInfos(platformList[i]);
        std::vector<cl::Device> deviceList;
        if (platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList)
            == CL_SUCCESS) {
          cout << "GPU devices" << endl;
          for (unsigned j = 0; j < deviceList.size(); j++) {
            Impl::listDeviceInfos(deviceList[j]);
          }
        }
        if (platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList)
            == CL_SUCCESS) {
          cout << "CPU devices" << endl;
          for (unsigned j = 0; j < deviceList.size(); j++) {
            Impl::listDeviceInfos(deviceList[j]);
          }
        }
      }

    }

    void CLProgram::listAllPlatforms() {
      std::vector<cl::Platform> platformList;
      cl::Platform::get(&platformList);
      for (unsigned i = 0; i < platformList.size(); i++) {
        Impl::listPlatformInfos(platformList[i]);
      }
    }

  }
}
#endif

/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLDeviceContext.cpp              **
 ** Module : ICLUtils                                               **
 ** Authors: Tobias Roehlig                                         **
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
#include <ICLUtils/CLDeviceContext.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/CLIncludes.h>
#include <ICLUtils/CLException.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cstdlib>
#include <cstring>

namespace icl::utils {
    // ///////////////////////////////////////////////////////////////////////////
    // Helper: query a string-valued info from a platform
    static std::string getPlatformInfoString(cl_platform_id platform, cl_platform_info param) {
      size_t len = 0;
      cl_int err = clGetPlatformInfo(platform, param, 0, nullptr, &len);
      if (err != CL_SUCCESS || len == 0) return std::string();
      std::vector<char> buf(len);
      err = clGetPlatformInfo(platform, param, len, buf.data(), nullptr);
      if (err != CL_SUCCESS) return std::string();
      return std::string(buf.data());
    }

    // Helper: query a string-valued info from a device
    static std::string getDeviceInfoString(cl_device_id device, cl_device_info param) {
      size_t len = 0;
      cl_int err = clGetDeviceInfo(device, param, 0, nullptr, &len);
      if (err != CL_SUCCESS || len == 0) return std::string();
      std::vector<char> buf(len);
      err = clGetDeviceInfo(device, param, len, buf.data(), nullptr);
      if (err != CL_SUCCESS) return std::string();
      return std::string(buf.data());
    }

    // Helper: query a scalar-valued info from a device
    template<typename T>
    static T getDeviceInfoScalar(cl_device_id device, cl_device_info param) {
      T val{};
      clGetDeviceInfo(device, param, sizeof(T), &val, nullptr);
      return val;
    }

    // ///////////////////////////////////////////////////////////////////////////

    struct CLDeviceContext::Impl {
      std::string m_device_type_str;
      cl_platform_id platform;
      cl_context context;
      cl_device_id device;
      cl_command_queue cmdQueue;
      std::map<cl_channel_order, std::set<cl_channel_type>> supported_channel_orders;

      Impl()
        : m_device_type_str("")
        , platform(nullptr)
        , context(nullptr)
        , device(nullptr)
        , cmdQueue(nullptr) {}

      ~Impl() {
        if (cmdQueue) {
          clReleaseCommandQueue(cmdQueue);
          cmdQueue = nullptr;
        }
        if (context) {
          clReleaseContext(context);
          context = nullptr;
        }
      }

      void init(std::string const &device_type_str) {
        m_device_type_str = device_type_str;
        cl_device_type device_type = stringToDeviceType(m_device_type_str);

        // Get platforms
        cl_uint numPlatforms = 0;
        cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms);
        if (err != CL_SUCCESS || numPlatforms == 0) {
          m_device_type_str = "";
          throw CLInitException(CLException::getMessage(err, "clGetPlatformIDs"));
        }
        std::vector<cl_platform_id> platforms(numPlatforms);
        err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
        if (err != CL_SUCCESS) {
          m_device_type_str = "";
          throw CLInitException(CLException::getMessage(err, "clGetPlatformIDs"));
        }

        // Select first matching device
        selectFirstDevice(device_type, platforms);

        std::cout << "[CLDeviceContext] desired OpenCL "
                  << deviceTypeToString(device_type)
                  << " device selected" << std::endl;

        // Create context
        cl_context_properties props[] = {
          CL_CONTEXT_PLATFORM,
          reinterpret_cast<cl_context_properties>(platform),
          0
        };
        context = clCreateContext(props, 1, &device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS) {
          m_device_type_str = "";
          throw CLInitException(CLException::getMessage(err, "clCreateContext"));
        }

        // Create command queue
        // Use the non-deprecated API when available (OpenCL 2.0+), fall back otherwise
#ifdef CL_VERSION_2_0
        cmdQueue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
#else
        cmdQueue = clCreateCommandQueue(context, device, 0, &err);
#endif
        if (err != CL_SUCCESS) {
          m_device_type_str = "";
          throw CLInitException(CLException::getMessage(err, "clCreateCommandQueue"));
        }

        prepareSupportedImageFormats();
      }

      void selectFirstDevice(cl_device_type deviceType,
                             std::vector<cl_platform_id> const &platformList) {
        for (size_t i = 0; i < platformList.size(); ++i) {
          cl_uint numDevices = 0;
          cl_int err = clGetDeviceIDs(platformList[i], deviceType, 0, nullptr, &numDevices);
          if (err != CL_SUCCESS || numDevices == 0) {
            DEBUG_LOG("error selecting a device on platform " << i);
            continue;
          }
          std::vector<cl_device_id> devices(numDevices);
          err = clGetDeviceIDs(platformList[i], deviceType, numDevices, devices.data(), nullptr);
          if (err != CL_SUCCESS) {
            DEBUG_LOG("error selecting a device on platform " << i);
            continue;
          }
          device = devices[0];
          platform = platformList[i];
          return;
        }
        std::string errorMsg = "No appropriate OpenCL device found for type ";
        errorMsg.append(deviceTypeToString(deviceType));
        throw CLInitException(errorMsg);
      }

      void prepareSupportedImageFormats() {
        cl_uint numFormats = 0;
        cl_int err = clGetSupportedImageFormats(context, CL_MEM_READ_WRITE,
                                                CL_MEM_OBJECT_IMAGE2D,
                                                0, nullptr, &numFormats);
        if (err != CL_SUCCESS || numFormats == 0) {
          supported_channel_orders.clear();
          return;
        }
        std::vector<cl_image_format> formats(numFormats);
        err = clGetSupportedImageFormats(context, CL_MEM_READ_WRITE,
                                         CL_MEM_OBJECT_IMAGE2D,
                                         numFormats, formats.data(), nullptr);
        if (err != CL_SUCCESS) {
          supported_channel_orders.clear();
          return;
        }
        supported_channel_orders.clear();
        cl_channel_order current_order = 0;
        for (cl_uint i = 0; i < numFormats; ++i) {
          cl_image_format const &fmt = formats[i];
          if (current_order != fmt.image_channel_order) {
            current_order = fmt.image_channel_order;
            supported_channel_orders[current_order] = std::set<cl_channel_type>();
          }
          supported_channel_orders[current_order].insert(fmt.image_channel_data_type);
        }
      }

      static void listPlatformInfos(cl_platform_id platform) {
        std::string value;
        value = getPlatformInfoString(platform, CL_PLATFORM_PROFILE);
        listInfo("CL_PLATFORM_PROFILE", value);
        value = getPlatformInfoString(platform, CL_PLATFORM_VERSION);
        listInfo("CL_PLATFORM_VERSION", value);
      }

      static void listDeviceInfos(cl_device_id device) {
        std::string strValue;
        strValue = getDeviceInfoString(device, CL_DEVICE_NAME);
        listInfo("CL_DEVICE_NAME", strValue);

        cl_device_type deviceType = getDeviceInfoScalar<cl_device_type>(device, CL_DEVICE_TYPE);
        strValue = deviceTypeToString(deviceType);
        listInfo("CL_DEVICE_TYPE", strValue);

        cl_uint uintValue = getDeviceInfoScalar<cl_uint>(device, CL_DEVICE_MAX_COMPUTE_UNITS);
        listInfo("CL_DEVICE_MAX_COMPUTE_UNITS", str(uintValue));

        size_t sizeValue = getDeviceInfoScalar<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
        listInfo("CL_DEVICE_MAX_WORK_GROUP_SIZE", str(sizeValue));

        cl_ulong ulongValue = getDeviceInfoScalar<cl_ulong>(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE);
        listInfo("CL_DEVICE_MAX_MEM_ALLOC_SIZE", str(ulongValue));

        ulongValue = getDeviceInfoScalar<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_SIZE);
        listInfo("CL_DEVICE_GLOBAL_MEM_SIZE", str(ulongValue));

        ulongValue = getDeviceInfoScalar<cl_ulong>(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
        listInfo("CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE", str(ulongValue));

        uintValue = getDeviceInfoScalar<cl_uint>(device, CL_DEVICE_MAX_CONSTANT_ARGS);
        listInfo("CL_DEVICE_MAX_CONSTANT_ARGS", str(uintValue));

        cl_device_local_mem_type memType =
          getDeviceInfoScalar<cl_device_local_mem_type>(device, CL_DEVICE_LOCAL_MEM_TYPE);
        listInfo("CL_DEVICE_LOCAL_MEM_TYPE", memTypeToString(memType));

        ulongValue = getDeviceInfoScalar<cl_ulong>(device, CL_DEVICE_LOCAL_MEM_SIZE);
        listInfo("CL_DEVICE_LOCAL_MEM_SIZE", str(ulongValue));

        std::cout << std::endl << std::endl;
      }

      static cl_device_type stringToDeviceType(std::string const &deviceType) {
        if (deviceType == "gpu") {
          return CL_DEVICE_TYPE_GPU;
        } else if (deviceType == "cpu") {
          return CL_DEVICE_TYPE_CPU;
        }
        std::string msg = "unknown device type ";
        msg.append(deviceType);
        throw CLInitException(msg);
      }

      static std::string deviceTypeToString(cl_device_type deviceType) {
        switch (deviceType) {
          case CL_DEVICE_TYPE_CPU:         return "cpu";
          case CL_DEVICE_TYPE_GPU:         return "gpu";
          case CL_DEVICE_TYPE_ACCELERATOR: return "accelerator";
          case CL_DEVICE_TYPE_DEFAULT:     return "default";
          default:                         return "unknown";
        }
      }

      static void listInfo(std::string const &infoName, std::string const &infoValue) {
        std::cout << "\t" << infoName << ":\t" << infoValue << std::endl;
      }

      static std::string memTypeToString(cl_device_local_mem_type memType) {
        switch (memType) {
          case CL_LOCAL:  return "LOCAL";
          case CL_GLOBAL: return "GLOBAL";
          default:        return "UNKNOWN";
        }
      }

      CLBuffer createBuffer(std::string const &accessMode, size_t size,
                            const void *src = nullptr) {
        return CLBuffer(context, cmdQueue, accessMode, size, src);
      }

      CLBuffer createBuffer(std::string const &accessMode, size_t length,
                            size_t byteDepth, const void *src = nullptr) {
        return CLBuffer(context, cmdQueue, accessMode, length, byteDepth, src);
      }

      CLImage2D createImage2D(std::string const &accessMode, size_t width,
                              size_t height, int depth, int num_channel,
                              const void *src = nullptr) {
        return CLImage2D(context, cmdQueue, accessMode, width, height, depth,
                         num_channel, src, supported_channel_orders);
      }

      CLBuffer *createBufferHeap(std::string const &accessMode, size_t length,
                                 size_t byteDepth, const void *src = nullptr) {
        CLBuffer *res = nullptr;
        try {
          res = new CLBuffer(context, cmdQueue, accessMode, length, byteDepth, src);
        } catch (CLException const &) {
          ICL_DELETE(res);
          throw;
        }
        return res;
      }

      CLImage2D *createImage2DHeap(std::string const &accessMode, size_t width,
                                   size_t height, int depth, int num_channel,
                                   const void *src = nullptr) {
        CLImage2D *res = nullptr;
        try {
          res = new CLImage2D(context, cmdQueue, accessMode, width, height, depth,
                              num_channel, src, supported_channel_orders);
        } catch (CLException const &) {
          ICL_DELETE(res);
          throw;
        }
        return res;
      }
    };

    // ///////////////////////////////////////////////////////////////////////////

    CLDeviceContext::CLDeviceContext()
      : impl(nullptr) {
      impl = new Impl();
    }

    CLDeviceContext::CLDeviceContext(std::string const &device)
      : impl(nullptr) {
      impl = new Impl();
      try {
        impl->init(device);
      } catch (CLException const &) {
        ICL_DELETE(impl);
        throw;
      }
    }

    CLDeviceContext::CLDeviceContext(const CLDeviceContext &other) : impl(nullptr) {
      impl = new Impl();
      impl->m_device_type_str = other.impl->m_device_type_str;
      impl->platform = other.impl->platform;
      impl->device = other.impl->device;
      impl->supported_channel_orders = other.impl->supported_channel_orders;

      // Retain the shared OpenCL objects so both instances can release independently
      impl->context = other.impl->context;
      if (impl->context) {
        clRetainContext(impl->context);
      }
      impl->cmdQueue = other.impl->cmdQueue;
      if (impl->cmdQueue) {
        clRetainCommandQueue(impl->cmdQueue);
      }
    }

    CLDeviceContext const &CLDeviceContext::operator=(CLDeviceContext const &other) {
      if (this == &other) return *this;

      // Release our current OpenCL objects
      if (impl->cmdQueue) {
        clReleaseCommandQueue(impl->cmdQueue);
      }
      if (impl->context) {
        clReleaseContext(impl->context);
      }

      impl->m_device_type_str = other.impl->m_device_type_str;
      impl->platform = other.impl->platform;
      impl->device = other.impl->device;
      impl->supported_channel_orders = other.impl->supported_channel_orders;

      // Retain the new shared OpenCL objects
      impl->context = other.impl->context;
      if (impl->context) {
        clRetainContext(impl->context);
      }
      impl->cmdQueue = other.impl->cmdQueue;
      if (impl->cmdQueue) {
        clRetainCommandQueue(impl->cmdQueue);
      }

      return *this;
    }

    CLBuffer CLDeviceContext::createBuffer(const std::string &accessMode, size_t size,
                                          const void *src) {
      return impl->createBuffer(accessMode, size, src);
    }

    CLBuffer CLDeviceContext::createBuffer(const std::string &accessMode, size_t length,
                                          size_t byteDepth, const void *src) {
      return impl->createBuffer(accessMode, length, byteDepth, src);
    }

    CLImage2D CLDeviceContext::createImage2D(const std::string &accessMode, size_t width,
                                            size_t height, int depth, const void *src) {
      return impl->createImage2D(accessMode, width, height, depth, 1, src);
    }

    CLImage2D CLDeviceContext::createImage2D(const std::string &accessMode, size_t width,
                                            size_t height, int depth, int num_channel,
                                            const void *src) {
      return impl->createImage2D(accessMode, width, height, depth, num_channel, src);
    }

    CLBuffer *CLDeviceContext::createBufferHeap(const std::string &accessMode, size_t length,
                                               size_t byteDepth, const void *src) {
      return impl->createBufferHeap(accessMode, length, byteDepth, src);
    }

    CLImage2D *CLDeviceContext::createImage2DHeap(const std::string &accessMode, size_t width,
                                                  size_t height, int depth, int num_channel,
                                                  const void *src) {
      return impl->createImage2DHeap(accessMode, width, height, depth, num_channel, src);
    }

    CLDeviceContext::~CLDeviceContext() {
      ICL_DELETE(impl);
    }

    std::string const CLDeviceContext::getDeviceTypeString() {
      return impl->m_device_type_str;
    }

    std::string const CLDeviceContext::getDeviceTypeString() const {
      return impl->m_device_type_str;
    }

    cl_device_id CLDeviceContext::getDevice() {
      return impl->device;
    }

    cl_command_queue CLDeviceContext::getCommandQueue() {
      return impl->cmdQueue;
    }

    cl_context CLDeviceContext::getContext() {
      return impl->context;
    }

    void CLDeviceContext::listSelectedPlatform() {
      std::cout << "selected Platform: " << std::endl;
      Impl::listPlatformInfos(impl->platform);
    }

    void CLDeviceContext::listSelectedDevice() {
      std::cout << "selected Device: " << std::endl;
      Impl::listDeviceInfos(impl->device);
    }

    void CLDeviceContext::listAllPlatformsAndDevices() {
      cl_uint numPlatforms = 0;
      cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms);
      if (err != CL_SUCCESS || numPlatforms == 0) return;
      std::vector<cl_platform_id> platforms(numPlatforms);
      clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

      for (cl_uint i = 0; i < numPlatforms; ++i) {
        Impl::listPlatformInfos(platforms[i]);

        cl_uint numDevices = 0;
        if (clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices) == CL_SUCCESS
            && numDevices > 0) {
          std::vector<cl_device_id> devices(numDevices);
          clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
          std::cout << "GPU devices" << std::endl;
          for (cl_uint j = 0; j < numDevices; ++j) {
            Impl::listDeviceInfos(devices[j]);
          }
        }

        numDevices = 0;
        if (clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices) == CL_SUCCESS
            && numDevices > 0) {
          std::vector<cl_device_id> devices(numDevices);
          clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, numDevices, devices.data(), nullptr);
          std::cout << "CPU devices" << std::endl;
          for (cl_uint j = 0; j < numDevices; ++j) {
            Impl::listDeviceInfos(devices[j]);
          }
        }
      }
    }

    void CLDeviceContext::listAllPlatforms() {
      cl_uint numPlatforms = 0;
      cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms);
      if (err != CL_SUCCESS || numPlatforms == 0) return;
      std::vector<cl_platform_id> platforms(numPlatforms);
      clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
      for (cl_uint i = 0; i < numPlatforms; ++i) {
        Impl::listPlatformInfos(platforms[i]);
      }
    }

  }
#endif

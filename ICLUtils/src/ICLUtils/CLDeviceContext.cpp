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
#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching

#include <ICLUtils/CLIncludes.h>

#include <ICLUtils/CLException.h>
#include <iostream>
#include <sstream>
#include <exception>
#include <set>
#include <map>

namespace icl {
	namespace utils {

		// /////////////////////////////////////////////////////////////////////////////////////////

		struct CLDeviceContext::Impl {
			std::string m_device_type_str;
			cl::Platform platform;
			cl::Context context;
			cl::Device device;
			cl::CommandQueue cmdQueue;
			std::map< cl_channel_order,set<cl_channel_type> > supported_channel_orders;

			Impl() : m_device_type_str("") {}

                  void init(std::string const &device_type_str) throw(CLInitException) {
                    m_device_type_str = device_type_str;
                    cl_device_type device_type = stringToDeviceType(m_device_type_str);
                    try {
                      std::vector<cl::Platform> platformList; //get number of available openCL platforms
                      cl::Platform::get(&platformList);
                      selectFirstDevice(device_type, platformList);

                      std::cout << "[CLDeviceContext] desired OpenCL " << deviceTypeToString(device_type) << " device selected" << std::endl;
                      std::vector<cl::Device> deviceList;
                      deviceList.push_back(device);
                      context = cl::Context(deviceList);//select GPU device
                      cmdQueue = cl::CommandQueue(context, device);//create command queue
                      prepareSupportedImageFormats();
                    } catch (cl::Error& error) { //catch openCL errors
                      m_device_type_str = "";
                      throw CLInitException(CLException::getMessage(error.err(), error.what()));
                    }
                  }

			void initDeviceFromExisting(cl::Context &other_context, cl::Device &other_device, cl::CommandQueue &other_cmd_queue) {
				m_device_type_str = "";
				try {
					device = other_device;
					context = other_context;
					cmdQueue = other_cmd_queue;
					prepareSupportedImageFormats();
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
                            } catch(cl::Error &e) {
                              DEBUG_LOG("error selecting a device: " << e.what());
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
                  
			void prepareSupportedImageFormats() {
				  std::vector<cl::ImageFormat> formats;
				  cl_mem_flags memFlags = CL_MEM_READ_WRITE;
				  context.getSupportedImageFormats(memFlags,CL_MEM_OBJECT_IMAGE2D,&formats);
				  supported_channel_orders.clear();
				  cl_channel_order current_order = 0;
				  for(uint32_t i = 0; i < formats.size(); ++i) {
					  cl::ImageFormat &format = formats[i];
					  if (current_order != format.image_channel_order) {
						  current_order = format.image_channel_order;
						  supported_channel_orders[current_order] = std::set<cl_channel_type>();
					  }
					  supported_channel_orders[current_order].insert(format.image_channel_data_type);
				  }
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

			CLBuffer createBuffer(const string &accessMode, size_t length, size_t byteDepth,
								  const void *src = 0) throw (CLBufferException) {
				return CLBuffer(context, cmdQueue, accessMode, length, byteDepth, src);
			}

			CLImage2D createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, int num_channel, const void *src=0) throw(CLBufferException){
				return CLImage2D(context, cmdQueue, accessMode, width, height, depth, num_channel, src, supported_channel_orders);
			}

			CLBuffer *createBufferHeap(const string &accessMode, size_t length, size_t byteDepth,
								  const void *src = 0) throw (CLBufferException) {
				CLBuffer *res = 0;
				try {
					res = new CLBuffer(context, cmdQueue, accessMode, length, byteDepth, src);
				} catch(CLException const &e) {
					(void)e;
					ICL_DELETE(res);
					throw;
				}
				return res;
			}

			CLImage2D *createImage2DHeap(const string &accessMode,  const size_t width, const size_t height, int depth, int num_channel, const void *src=0) throw(CLBufferException){
				CLImage2D *res = 0;
				try {
					res = new CLImage2D(context, cmdQueue, accessMode, width, height, depth, num_channel, src, supported_channel_orders);
				} catch(CLException const &e) {
					(void)e;
					ICL_DELETE(res);
					throw;
				}
				return res;
			}

		};

		// /////////////////////////////////////////////////////////////////////////////////////////

		CLDeviceContext::CLDeviceContext() throw(CLInitException, CLBuildException)
			: impl(0) {
			impl = new Impl();
		}

		CLDeviceContext::CLDeviceContext(std::string const &device) throw(CLInitException, CLBuildException)
			: impl(0) {
			impl = new Impl();
			try {
				impl->init(device);
			} catch (CLException const &e) {
				(void)e;
				ICL_DELETE(impl);
				throw;
			}
		}

		CLDeviceContext::CLDeviceContext(const CLDeviceContext &other) : impl(0) {
			impl = new Impl();
			this->impl->m_device_type_str = other.impl->m_device_type_str;
			this->impl->platform = other.impl->platform;
			this->impl->cmdQueue = other.impl->cmdQueue;
			this->impl->context = other.impl->context;
			this->impl->device = other.impl->device;
			this->impl->supported_channel_orders = other.impl->supported_channel_orders;
		}

		CLDeviceContext const &CLDeviceContext::operator=(CLDeviceContext const& other) {
			this->impl->m_device_type_str = other.impl->m_device_type_str;
			this->impl->platform = other.impl->platform;
			this->impl->cmdQueue = other.impl->cmdQueue;
			this->impl->context = other.impl->context;
			this->impl->device = other.impl->device;
			this->impl->supported_channel_orders = other.impl->supported_channel_orders;
			return *this;
		}

		CLBuffer CLDeviceContext::createBuffer(const string &accessMode, size_t size, const void *src) throw(CLBufferException) {
		  return impl->createBuffer(accessMode, size, src);
		}

		CLBuffer CLDeviceContext::createBuffer(const string &accessMode, size_t length, size_t byteDepth, const void *src) throw(CLBufferException) {
			return impl->createBuffer(accessMode,length,byteDepth,src);
		}

		CLImage2D CLDeviceContext::createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, const void *src) throw(CLBufferException){
			return impl->createImage2D(accessMode, width, height, depth, 1, src);
		}

		CLImage2D CLDeviceContext::createImage2D(const string &accessMode,  const size_t width, const size_t height, int depth, int num_channel, const void *src) throw(CLBufferException){
			return impl->createImage2D(accessMode, width, height, depth, num_channel, src);
		}

		CLBuffer *CLDeviceContext::createBufferHeap(const string &accessMode, size_t length, size_t byteDepth, const void *src) throw(CLBufferException) {
		  return impl->createBufferHeap(accessMode, length, byteDepth, src);
		}

		CLImage2D *CLDeviceContext::createImage2DHeap(const string &accessMode,  const size_t width, const size_t height, int depth, int num_channel, const void *src) throw(CLBufferException){
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

		cl::Device &CLDeviceContext::getDevice() {
			return impl->device;
		}

		cl::CommandQueue &CLDeviceContext::getCommandQueue() {
			return impl->cmdQueue;
		}

		cl::Context &CLDeviceContext::getContext() {
			return impl->context;
		}

		void CLDeviceContext::listSelectedPlatform() {
		  cout << "selected Platform: " << endl;
		  Impl::listPlatformInfos(impl->platform);
		}
		void CLDeviceContext::listSelectedDevice() {
		  cout << "selected Device: " << endl;
		  Impl::listDeviceInfos(impl->device);
		}

		void CLDeviceContext::listAllPlatformsAndDevices() {
		  std::vector<cl::Platform> platformList;
		  cl::Platform::get(&platformList);
		  for (unsigned int i = 0; i < platformList.size(); i++) { //check all platforms
			Impl::listPlatformInfos(platformList[i]);
			std::vector<cl::Device> deviceList;
			if (platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList) == CL_SUCCESS) {
			  cout << "GPU devices" << endl;
			  for (unsigned j = 0; j < deviceList.size(); j++) {
				Impl::listDeviceInfos(deviceList[j]);
			  }
			}
			if (platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList) == CL_SUCCESS) {
			  cout << "CPU devices" << endl;
			  for (unsigned j = 0; j < deviceList.size(); j++) {
				Impl::listDeviceInfos(deviceList[j]);
			  }
			}
		  }
		}

		void CLDeviceContext::listAllPlatforms() {
		  std::vector<cl::Platform> platformList;
		  cl::Platform::get(&platformList);
		  for (unsigned i = 0; i < platformList.size(); i++) {
			Impl::listPlatformInfos(platformList[i]);
		  }
		}
	}
}

#endif

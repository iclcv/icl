/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLDeviceContext.h                **
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

#pragma once

#ifdef ICL_HAVE_OPENCL

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/CLException.h>
#include <string>


namespace cl{
  class Device;
  class CommandQueue;
  class Context;
}

namespace icl {
	namespace utils {

		/**
		 * @brief The CLDeviceContext class allows preparation of the device used for CLPrograms.
		 *
		 * The CLDeviceContext class is a wrapper for the opencl c++ device, command queue and context classes.
		 * This class is used in CLProgram to instanciate a device and allocate memory. It also is used
		 * in CLMemoryAssistant to manage the buffer and image allocation. A CLDeviceContext can be shared over
		 * different CLPrograms to allow memory exchanges, e.g. for processing pipelines
		 *
		 */
		class ICLUtils_API CLDeviceContext {
			/**
			 * Wrapper struct for internal implementation
			 */
			struct Impl;
			///
			Impl *impl;

			/// returns the internal cl device structure
			cl::Device &getDevice();
			/// returns the internal cl command queue structure
			cl::CommandQueue &getCommandQueue();
			/// returns the internal cl context structure
			cl::Context &getContext();

		public:

			friend class CLProgram; //!< for tight integration with CLProgram instances
			friend class CLMemoryAssistant; //!< for tight integration with CLMemoryAssistant instances

			/**
			 * @brief CLDeviceContext Creates a dummy CLDeviceContext
			 */
			CLDeviceContext();
			/**
			 * @brief CLDeviceContext Creates a CLDeviceContext on the given device type
			 * @param device type of device as string. Current supported device types: "gpu","cpu".
			 */
			CLDeviceContext(std::string const &device);
			/**
			 * @brief CLDeviceContext Creates a shallow copy of the given CLDeviceContext
			 * @param other another CLDeviceContext to copy from
			 */
			CLDeviceContext(const CLDeviceContext &other);
			/**
			 * @brief operator = shallowly copies from other CLDeviceContext
			 * @param other
			 * @return
			 */
			CLDeviceContext const &operator=(CLDeviceContext const& other);
			/**
			 * @brief ~CLDeviceContext Destructor
			 */
			~CLDeviceContext();

			//accessMode = "r" only read
			//accessMode = "w" only write
			//accessMode = "rw" read and write
			/**
			 * @brief createBuffer creates a buffer object for memory exchange with graphics card memory
			 * @param accessMode can either be "r", "w" or "rw", which refers to the readibility of the data
			 * by the OpenCL source code
			 * @param size Fixed size for this buffer in bytes
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return The CLBuffer instance.
			 *
			 * Throws a CLBufferException in case of allocation problems.\n
			 */
			CLBuffer createBuffer(const string &accessMode, size_t size, const void *src=0);

			/**
			 * @brief createBuffer createBuffer creates a buffer object for memory exchange with graphics card memory
			 * @param accessMode accessMode can either be "r", "w" or "rw", which refers to the readibility of the data
			 * by the OpenCL source code
			 * @param length Fixed length of the buffer (in original type size e.g. float,int, ..., etc.)
			 * @param byteDepth Byte depth of the data type (e.g. float = 4, int32 = 4, uchar = 1 ... etc.)
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return
			 */
			CLBuffer createBuffer(const string &accessMode, size_t length, size_t byteDepth, const void *src = 0);

			/**
			 * @brief createImage2D Creates a image2D object for memory exchange with graphics card memory
			 * @param accessMode accessMode accessMode can either be "r", "w" or "rw", which refers to the readibility of the data
			 * by the OpenCL source code
			 * @param width The width of the image passed
			 * @param height The height of the image passed
			 * @param depth various image depths can be used \n
				  depth8u  = 0, < 8Bit unsigned integer values range {0,1,...255} \n
				  depth16s = 1, < 16Bit signed integer values \n
				  depth32s = 2, < 32Bit signed integer values \n
				  depth32f = 3, < 32Bit floating point values \n
				  depth64f = 4, < 64Bit floating point values (WARNING! float64 is not always supported by opencl and thus is not handled in ICL so far) \n
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return CLImage2D instance
			 *
			 * In this case, the number of channels are assumed to be equal to one.\n
			 * Throws a CLBufferException in case of allocation problems.\n
			 */
			CLImage2D createImage2D(const string &accessMode, const size_t width, const size_t height, int depth, const void *src=0);

			/**
			 * @brief createImage2D
			 * @param accessMode
			 * @param width
			 * @param height
			 * @param depth various image depths can be used \n
				  depth8u  = 0, < 8Bit unsigned integer values range {0,1,...255} \n
				  depth16s = 1, < 16Bit signed integer values \n
				  depth32s = 2, < 32Bit signed integer values \n
				  depth32f = 3, < 32Bit floating point values \n
				  depth64f = 4, < 64Bit floating point values (WARNING! float64 is not always supported by opencl and thus is not handled in ICL so far) \n
			 * @param num_channel The number of image channels
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return CLImage2D instance
			 *
			 * Throws a CLBufferException in case of allocation problems.\n
			 */
			CLImage2D createImage2D(const string &accessMode, const size_t width, const size_t height, int depth, int num_channel, const void *src=0);

			/**
			 * @brief getDeviceTypeString Returns the device type as string
			 * @return The device type as string ("gpu" or "cpu")
			 */
			std::string const getDeviceTypeString();
			/**
			 * @brief getDeviceTypeString Returns the device type as string (const)
			 * @return The device type as string ("gpu" or "cpu")
			 */
			std::string const getDeviceTypeString() const;

			/// lists various properties of the selected platform
			void listSelectedPlatform();

			/// lists various properties of the selected device
			void listSelectedDevice();

			/// lists various properties of all platforms and their devices
			static void listAllPlatformsAndDevices();

			/// lists various properties of all platforms
			static void listAllPlatforms();

		private:

			/**
			 * @brief createBufferHeap creates a buffer object pointer for memory exchange with graphics card memory. The pointer has to be handles by the user
			 * @param accessMode accessMode can either be "r", "w" or "rw", which refers to the readibility of the data
			 * by the OpenCL source code
			 * @param length Fixed length of the buffer (in original type size e.g. float,int, ..., etc.)
			 * @param byteDepth Byte depth of the data type (e.g. float = 4, int32 = 4, uchar = 1 ... etc.)
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return Pointer to CLBuffer instance
			 *
			 * Throws a CLBufferException in case of allocation problems.\n
			 */
			CLBuffer *createBufferHeap(const string &accessMode, size_t length, size_t byteDepth, const void *src=0);

			/**
			 * @brief createImage2DHeap Creates a image2D object pointer for memory exchange with graphics card memory. The pointer has to be handles by the user
			 * @param accessMode accessMode accessMode can either be "r", "w" or "rw", which refers to the readibility of the data
			 * by the OpenCL source code
			 * @param width The width of the image passed
			 * @param height The height of the image passed
			 * @param depth various image depths can be used \n
				  depth8u  = 0, < 8Bit unsigned integer values range {0,1,...255} \n
				  depth16s = 1, < 16Bit signed integer values \n
				  depth32s = 2, < 32Bit signed integer values \n
				  depth32f = 3, < 32Bit floating point values \n
				  depth64f = 4, < 64Bit floating point values (WARNING! float64 is not always supported by opencl and thus is not handled in ICL so far) \n
			 * @param num_channel The number of image channels
			 * @param src Optional source pointer. If not NULL the content of this pointer will automatically be uploaded to the device.
			 * @return pointer to CLImage2D instance
			 *
			 * Throws a CLBufferException in case of allocation problems.\n
			 */
			CLImage2D *createImage2DHeap(const string &accessMode, const size_t width, const size_t height, int depth, int num_channel, const void *src=0);
		};

	}
}

#endif

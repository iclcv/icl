/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLMemoryAssistant.h              **
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

#include <ICLUtils/CLDeviceContext.h>
#include <ICLUtils/CLMemory.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/Exception.h>
#include <map>

namespace icl {
	namespace utils {

		/**
		 * @brief The CLMemoryAssistant class is a helper class to maintain buffers and images for opencl-program pipelines
		 *
		 * In case of a pipeline of operators that use a CLProgram internally one can use the CLMemoryAssistant to instanciate a CLDeviceContext and
		 * create shared buffers and images for the device. So far, all implementations of an opencl program need to allow to pass CLBuffer or CLImage2D instances as
		 * in and output.
		 *
		 */
		class ICLUtils_API CLMemoryAssistant {

		public:
			/**
			 * @brief CLMemoryAssistant Default constructor, creates a dummy instance
			 */
			CLMemoryAssistant();

			/**
			 * @brief CLMemoryAssistant Creates an instance for the given device type (see CLDeviceContext for details)
			 * @param deviceType Device to use (see CLDeviceContext for details)
			 */
			CLMemoryAssistant(std::string const &deviceType);

			/// Destructor
			~CLMemoryAssistant();

			typedef std::string MemKeyType;
			typedef std::map<MemKeyType,CLMemory*> CLMemoryMap;

			/**
			 * @brief createNamedBuffer creates a CLBuffer object (internally handled as a CLMemory-pointer)
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 *
			 * Please see CLDeviceContext::createBuffer() for more details
			 */
			CLBuffer createNamedBuffer(MemKeyType const &key, const std::string &accessMode,
									   const size_t length, const size_t byteDepth, const void *src=0);

			/**
			 * @brief createNamedImage2D creates a Image2D object (internally handled as a CLMemory-pointer)
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 *
			 * Please see CLDeviceContext::createImage2D() for more details
			 */
			CLImage2D createNamedImage2D(MemKeyType const &key, const std::string &accessMode,
										 const size_t width, const size_t height, const int depth,
										 const int num_channel, const void *src=0);

			/**
			 * @brief operator [] Direct access operator to the internal CLMemory
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 * @return CLMemory pointer
			 */
			CLMemory *operator[](MemKeyType const &key);

			/**
			 * @brief asBuf shortcut to access and cast the internal CLMemory instance (CLBuffer)
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 * @return CLImage2D instance
			 */
			CLBuffer &asBuf(MemKeyType const &key);

			/**
			 * @brief asBuf shortcut to access and cast the internal CLMemory instance (CLBuffer)
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 * @return CLImage2D instance
			 */
			CLImage2D &asImg(MemKeyType const &key);

			/**
			 * @brief keyExist Returns true if the given key exists in this instance
			 * @param key The name (std::string) and identifier for this CLMemory instance
			 * @return true if the key exists, false otherwise
			 */
			bool keyExist(MemKeyType const &key) { return (bool)m_memory_map.count(key); }

			/**
			 * @brief clearAll clears and deletes all internal buffers. Use with care!
			 */
			void clearAll();

			/**
			 * @brief remove removes and deletes the CLMemory instance for key if available
			 * @param key Identifier of the memory object to remove from this instance
			 */
			void remove(MemKeyType const &key);

			/**
			 * @brief getDeviceContext returns the used device context that can be shared with CLProgram instances
			 * @return the internal CLDeviceContext instance
			 */
			CLDeviceContext const &getDeviceContext() { return m_context; }

		protected:

			/// device context used
			CLDeviceContext m_context;
			/// the map for internal storage
			CLMemoryMap m_memory_map;
		};

	}
}

#endif

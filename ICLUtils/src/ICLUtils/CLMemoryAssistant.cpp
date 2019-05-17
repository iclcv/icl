/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLMemoryAssistant.cpp            **
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

#include <ICLUtils/CLMemoryAssistant.h>

#ifdef ICL_HAVE_OPENCL

namespace icl {
	namespace utils {

		CLMemoryAssistant::CLMemoryAssistant() {}

		CLMemoryAssistant::CLMemoryAssistant(std::string const &deviceType)  {
			m_context = CLDeviceContext(deviceType);
		}

		CLMemoryAssistant::~CLMemoryAssistant() {
			clearAll();
		}

		void CLMemoryAssistant::clearAll() {
			for(CLMemoryMap::iterator it = m_memory_map.begin(); it != m_memory_map.end(); ++it) {
				ICL_DELETE(it->second);
			}
			m_memory_map.clear();
		}

		void CLMemoryAssistant::remove(MemKeyType const &key) {
			CLMemoryMap::iterator it = m_memory_map.find(key);
			if (it != m_memory_map.end()) {
				ICL_DELETE(it->second);
				m_memory_map.erase(key);
			}
		}

		CLBuffer CLMemoryAssistant::createNamedBuffer(MemKeyType const &key, const string &accessMode,
													  const size_t length, size_t byteDepth, const void *src)
		 {
			if (m_memory_map.find(key) != m_memory_map.end())
				throw ICLException("CLMemoryAssistant::createNamedBuffer(): Key already in use: " + key);
			CLBuffer *mem_ptr = 0;
			try {
				mem_ptr = m_context.createBufferHeap(accessMode,length,byteDepth,src);
				m_memory_map[key] = (CLMemory*)mem_ptr;
			} catch (CLException const &e) {
				(void)e;
				ICL_DELETE(mem_ptr);
				m_memory_map.erase(key);
				throw;
			}
			return *mem_ptr;
		}

		CLImage2D CLMemoryAssistant::createNamedImage2D(MemKeyType const &key, const string &accessMode,
														const size_t width, const size_t height, const int depth,
														const int num_channel, const void *src)
		 {
			if (m_memory_map.find(key) != m_memory_map.end())
				throw ICLException("CLMemoryAssistant::createNamedImage2D(): Key already in use: " + key);
			CLImage2D *mem_ptr = 0;
			try {
				mem_ptr = m_context.createImage2DHeap(accessMode,width,height,depth,num_channel,src);
				m_memory_map[key] = (CLMemory*)mem_ptr;
			} catch (CLException const &e) {
				(void)e;
				ICL_DELETE(mem_ptr);
				m_memory_map.erase(key);
				throw;
			}
			return *mem_ptr;
		}

		CLMemory *CLMemoryAssistant::operator[](MemKeyType const &key) {
			return m_memory_map.at(key);
		}

		CLBuffer &CLMemoryAssistant::asBuf(MemKeyType const &key) {
			return *(m_memory_map.at(key)->asCLBuffer());
		}

		CLImage2D &CLMemoryAssistant::asImg(MemKeyType const &key) {
			return *(m_memory_map.at(key)->asCLImage2D());
		}

	}
}

#endif

/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/ICLUtils/CLMemory.h                       **
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

#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/CLException.h>
#include <vector>

namespace icl {
	namespace utils {

		class CLBuffer;
		class CLImage2D;

		/**
		 * @brief The CLMemory class is a base class for CLBuffer and CLImage2D
		 */
		class ICLUtils_API CLMemory {

		public:

			/// smart pointer to this class type
			typedef SmartPtr<CLMemory> Ptr;

			/// Memorys type available
			enum MemoryType {
				Buffer,
				Image2D,
				Invalid
			};

		protected:

			/**
			 * @brief CLMemory Default constructor. Only inherit classes can use this constructor
			 * @param type
			 */
			CLMemory(MemoryType const type = Invalid) : m_dimensions(3,1), m_size(1), m_type(type) {}

			/// size of each dimension x,y,z
			std::vector<icl32s> m_dimensions;
			/// size in total x*y*z
			icl64s m_size;
			/// byte depth of the data type used (e.g. float32 = 4byte)
			icl32s m_byte_depth;

			/// sets the dimensions x,y,z and total size
			void setDimensions(icl32s const &x, icl32s const &y, icl32s const &z) {
				m_dimensions[0] = x; m_dimensions[1] = y; m_dimensions[2] = z;
				m_size = x*y*z;
			}

		private:

			/// The memory type of this instance (CLBuffer or CLImage2D)
			MemoryType m_type;

		public:

			/**
			 * @brief CLMemory default copy constructor
			 * @param other CLMemory instance to copy from
			 */
			CLMemory(CLMemory const &other) {
				this->m_type = other.m_type;
				this->m_dimensions = other.m_dimensions;
				this->m_size = other.m_size;
				this->m_byte_depth = other.m_byte_depth;
			}

			/**
			 * @brief operator = default assignment operator
			 * @param other CLMemory instance to copy from
			 * @return This CLMemory instance
			 */
			CLMemory& operator=(CLMemory const& other) {
				this->m_type = other.m_type;
				this->m_dimensions = other.m_dimensions;
				this->m_size = other.m_size;
				this->m_byte_depth = other.m_byte_depth;
				return *this;
			}

			/// Destructor
			virtual ~CLMemory() {}

			/**
			 * @brief asCLBuffer Casting function to cast to CLBuffer pointer (reinterpret_cast)
			 * @return Casted pointer to CLBuffer
			 */
			inline CLBuffer *asCLBuffer() throw(CLBufferException) {
				if (m_type != Buffer)
					throw CLBufferException("Invalid cast from CLMemory to CLBuffer pointer");
				return reinterpret_cast<CLBuffer*>(this);
			}

			/**
			 * @brief asCLImage2D Casting function to cast to CLImage2D pointer (reinterpret_cast)
			 * @return Casted pointer to CLImage2D
			 */
			inline CLImage2D *asCLImage2D() throw(CLBufferException) {
				if (m_type != Image2D)
					throw CLBufferException("Invalid cast from CLMemory to CLImage2D pointer");
				return reinterpret_cast<CLImage2D*>(this);
			}

			/// returns the dimensions (3Dim-std::vector for x,y,z dimensions)
			const std::vector<icl32s> getDimensions() const { return m_dimensions; }
			/// returns total size of the memory (x*y*z)
			const icl64s getSize() const { return m_size; }
			/// returns the bytedepth of the used data type (e.g. float32 = 4byte)
			const icl32s getByteDepth() const { return m_byte_depth; }

		};

	}
}

#endif

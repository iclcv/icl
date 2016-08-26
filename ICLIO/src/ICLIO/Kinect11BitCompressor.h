
#pragma once

#include <ICLCore/Core.h>

namespace icl {
	namespace io {

		class Kinect11BitCompressor {

		public:

			static size_t estimate_packed_size(size_t nSrc);

			static void unpack11to16(const uint16_t *src, uint16_t *dest, int n);

			static void pack16to11(const uint16_t *src, uint16_t *dest, int n);

			// ///////////////////////////////////////////////////////////////////////////////////////////

			static void unpack11to16_2(const uint16_t *src, uint16_t *dest, int n);

			static void pack16to11_2(const uint16_t *src, uint16_t *dest, int n);

			static const int Z0;
			static const int Zmin;
			static const int Zmax;
			static const int a;
			static const int b;

		};

	}// namespace io
}// namespace icl

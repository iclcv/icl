
#include <ICLIO/Kinect11BitCompressor.h>
#include <bitset>

namespace icl {
	namespace io {

		const int Kinect11BitCompressor::Z0 = 400;
		const int Kinect11BitCompressor::Zmin = 750;
		const int Kinect11BitCompressor::Zmax = 10000;
		const int Kinect11BitCompressor::a = Kinect11BitCompressor::Z0*(Kinect11BitCompressor::Z0+1);
		const int Kinect11BitCompressor::b = 1-(float(Kinect11BitCompressor::a)/Kinect11BitCompressor::Zmax);

		typedef std::bitset<16> Short;
		#define SHOWB(X) SHOW(Short((X)).to_string())

		size_t Kinect11BitCompressor::estimate_packed_size(size_t nSrc){
			size_t nBitsSrc = nSrc * 11;
			double nDst = double(nBitsSrc)/16.;
			return std::ceil(nDst);
		}

		void Kinect11BitCompressor::unpack11to16(const uint16_t *src, uint16_t *dest, int n){
			uint32_t mask = (1 << 11) - 1;
			uint32_t buffer = 0;
			int bitsIn = 0;
			while (n--) {
				while (bitsIn < 11) {
					buffer = (buffer << 16) | *(src++);
					bitsIn += 16;
				}
				bitsIn -= 11;
				*(dest++) = (buffer >> bitsIn) & mask;
			}
		}

		void Kinect11BitCompressor::pack16to11(const uint16_t *src, uint16_t *dest, int n){
			const uint32_t mask = (1 << 11) - 1;
			uint32_t buffer = 0;
			int bitsIn = 0;
			int n_p = estimate_packed_size(n);
			while (n_p--) {
				while (bitsIn < 16) {
					buffer = (buffer << 11) | (std::min(*src++,(uint16_t)mask) & mask);
					bitsIn += 11;
				}
				bitsIn -= 16;
				*(dest++) = (buffer >> bitsIn);
			}
		}

		// /////////////////////////////////////////////////////////////////////////////////////////////

		void Kinect11BitCompressor::unpack11to16_2(const uint16_t *src, uint16_t *dest, int n) {
			uint32_t mask = (1 << 11) - 1;
			uint32_t buffer = 0;
			int bitsIn = 0;
			while (n--) {
				while (bitsIn < 11) {
					buffer = (buffer << 16) | *(src++);
					bitsIn += 16;
				}
				bitsIn -= 11;
				uint32_t buf = (buffer >> bitsIn) & mask;
				*(dest++) = buf?float(a)/(buf-b):0;
			}
		}

		void Kinect11BitCompressor::pack16to11_2(const uint16_t *src, uint16_t *dest, int n) {
			const uint32_t mask = (1 << 11) - 1;
			uint32_t buffer = 0;
			uint32_t tmp = 0;
			int bitsIn = 0;
			int n_p = estimate_packed_size(n);
			while (n_p--) {
				while (bitsIn < 16) {
					tmp = (float(a)/(*src++))+b;
					//tmp = (tmp!=tmp)?0:tmp;
					buffer = (buffer << 11) | (std::min(tmp,mask) & mask);
					bitsIn += 11;
				}
				bitsIn -= 16;
				*(dest++) = (buffer >> bitsIn);
			}
		}

	}// namespace io
}//namespace icl


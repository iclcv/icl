// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// MKL DFTI backend for FFT operations.
// This file is excluded from the build when MKL is not found.

#include <ICLMath/FFTOps.h>
#include <ICLUtils/Exception.h>
#include <mkl_dfti.h>
#include <complex>
#include <vector>

using namespace icl::utils;

namespace icl::math {
  namespace {

    // ================================================================
    // Helper: MKL DFTI precision selector
    // ================================================================

    template<class T> DFTI_CONFIG_VALUE mklPrecision() { return DFTI_DOUBLE; }
    template<> DFTI_CONFIG_VALUE mklPrecision<float>() { return DFTI_SINGLE; }

    // ================================================================
    // Helper: unpack MKL PACK_FORMAT to full complex array
    // ================================================================

    template<class T>
    void unpackMklFft(T* src, std::complex<T>* dst, int cols, int rows) {
      int dim = cols * rows;
      dst[0] = std::complex<T>(src[0], 0);
      int j = 1;
      int offrow = cols * rows / 2;
      for(int i = 1; i < cols - 1; i += 2) {
        dst[j] = std::complex<T>(src[i], src[i + 1]);
        dst[cols - j] = std::complex<T>(src[i], -src[i + 1]);
        ++j;
      }
      if(rows % 2 == 0) dst[offrow] = std::complex<T>(src[dim - cols], 0);
      if(cols % 2 == 0) dst[cols / 2] = std::complex<T>(src[cols - 1], 0);
      if(cols % 2 == 0 && rows % 2 == 0) dst[offrow + cols / 2] = std::complex<T>(src[dim - 1], 0);
      j = cols;
      int offcol = cols / 2;
      for(int i = cols; i < dim - cols - 1; i += 2 * cols) {
        dst[j] = std::complex<T>(src[i], src[i + cols]);
        dst[dim - j] = std::complex<T>(src[i], -src[i + cols]);
        if(cols % 2 == 0) {
          dst[j + offcol] = std::complex<T>(src[i + cols - 1], src[i + 2 * cols - 1]);
          dst[dim - j + offcol] = std::complex<T>(src[i + cols - 1], -src[i + 2 * cols - 1]);
        }
        j += cols;
      }
      int a = cols / 2;
      if(cols % 2 == 1) a = cols / 2 + 1;
      int cindex = 1;
      for(int r = 1; r < a; ++r) {
        j = cols + r;
        for(int i = cols + cindex; i < dim; i += cols) {
          dst[j] = std::complex<T>(src[i], src[i + 1]);
          dst[dim - j + cols] = std::complex<T>(src[i], -src[i + 1]);
          j += cols;
        }
        cindex += 2;
      }
    }

    // ================================================================
    // Real-to-complex forward FFT via MKL DFTI
    // ================================================================

    template<class T>
    void mkl_fft_r2c(const T* src, int rows, int cols, std::complex<T>* dst) {
      int dim = rows * cols;

      // Copy input to working buffer (MKL may modify it)
      std::vector<T> srcbuf(src, src + dim);

      MKL_LONG l[2] = {(MKL_LONG)rows, (MKL_LONG)cols};
      DFTI_DESCRIPTOR_HANDLE desc = 0;
      MKL_LONG status = DftiCreateDescriptor(&desc, mklPrecision<T>(), DFTI_REAL, 2, l);

      MKL_LONG strides_in[3] = {0, (MKL_LONG)cols, 1};
      MKL_LONG strides_out[3] = {0, (MKL_LONG)cols, 1};
      DftiSetValue(desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
      DftiSetValue(desc, DFTI_PACKED_FORMAT, DFTI_PACK_FORMAT);
      DftiSetValue(desc, DFTI_INPUT_STRIDES, strides_in);
      DftiSetValue(desc, DFTI_OUTPUT_STRIDES, strides_out);
      status = DftiCommitDescriptor(desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR)) {
        DftiFreeDescriptor(&desc);
        throw ICLException("MKL FFT: DftiCommitDescriptor failed");
      }

      // Use dst as temp buffer for packed output, then unpack
      std::vector<T> packbuf(dim);
      status = DftiComputeForward(desc, srcbuf.data(), packbuf.data());
      DftiFreeDescriptor(&desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR))
        throw ICLException("MKL FFT: DftiComputeForward failed");

      unpackMklFft(packbuf.data(), dst, cols, rows);
    }

    // ================================================================
    // Complex-to-complex forward FFT via MKL DFTI
    // ================================================================

    template<class T>
    void mkl_fft_c2c(const std::complex<T>* src, int rows, int cols, std::complex<T>* dst) {
      int dim = rows * cols;

      MKL_LONG l[2] = {(MKL_LONG)rows, (MKL_LONG)cols};
      DFTI_DESCRIPTOR_HANDLE desc = 0;
      DftiCreateDescriptor(&desc, mklPrecision<T>(), DFTI_COMPLEX, 2, l);

      MKL_LONG strides[3] = {0, (MKL_LONG)cols, 1};
      DftiSetValue(desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
      DftiSetValue(desc, DFTI_INPUT_STRIDES, strides);
      DftiSetValue(desc, DFTI_OUTPUT_STRIDES, strides);
      MKL_LONG status = DftiCommitDescriptor(desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR)) {
        DftiFreeDescriptor(&desc);
        throw ICLException("MKL FFT: DftiCommitDescriptor failed");
      }

      // Copy input (MKL needs mutable input for some configurations)
      std::vector<std::complex<T>> srcbuf(src, src + dim);
      status = DftiComputeForward(desc, srcbuf.data(), dst);
      DftiFreeDescriptor(&desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR))
        throw ICLException("MKL FFT: DftiComputeForward failed");
    }

    // ================================================================
    // Complex-to-complex inverse FFT via MKL DFTI (normalized)
    // ================================================================

    template<class T>
    void mkl_ifft_c2c(const std::complex<T>* src, int rows, int cols, std::complex<T>* dst) {
      int dim = rows * cols;

      MKL_LONG l[2] = {(MKL_LONG)rows, (MKL_LONG)cols};
      DFTI_DESCRIPTOR_HANDLE desc = 0;
      DftiCreateDescriptor(&desc, mklPrecision<T>(), DFTI_COMPLEX, 2, l);

      MKL_LONG strides[3] = {0, (MKL_LONG)cols, 1};
      DftiSetValue(desc, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
      DftiSetValue(desc, DFTI_PACKED_FORMAT, DFTI_PACK_FORMAT);
      DftiSetValue(desc, DFTI_INPUT_STRIDES, strides);
      DftiSetValue(desc, DFTI_OUTPUT_STRIDES, strides);

      T scale = T(1) / T(dim);
      DftiSetValue(desc, DFTI_BACKWARD_SCALE, scale);

      MKL_LONG status = DftiCommitDescriptor(desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR)) {
        DftiFreeDescriptor(&desc);
        throw ICLException("MKL IFFT: DftiCommitDescriptor failed");
      }

      std::vector<std::complex<T>> srcbuf(src, src + dim);
      status = DftiComputeBackward(desc, srcbuf.data(), dst);
      DftiFreeDescriptor(&desc);
      if(!DftiErrorClass(status, DFTI_NO_ERROR))
        throw ICLException("MKL IFFT: DftiComputeBackward failed");
    }

  } // anonymous namespace

  static const int _mkl_fft_reg = []() {
    auto mkl_f = FFTOps<float>::instance().backends(Backend::Mkl);
    mkl_f.add<FFTOps<float>::R2CSig>(FFTOp::r2c, mkl_fft_r2c<float>, "MKL DFTI r2c");
    mkl_f.add<FFTOps<float>::C2CSig>(FFTOp::c2c, mkl_fft_c2c<float>, "MKL DFTI c2c");
    mkl_f.add<FFTOps<float>::InvC2CSig>(FFTOp::inv_c2c, mkl_ifft_c2c<float>, "MKL DFTI inv_c2c");

    auto mkl_d = FFTOps<double>::instance().backends(Backend::Mkl);
    mkl_d.add<FFTOps<double>::R2CSig>(FFTOp::r2c, mkl_fft_r2c<double>, "MKL DFTI r2c");
    mkl_d.add<FFTOps<double>::C2CSig>(FFTOp::c2c, mkl_fft_c2c<double>, "MKL DFTI c2c");
    mkl_d.add<FFTOps<double>::InvC2CSig>(FFTOp::inv_c2c, mkl_ifft_c2c<double>, "MKL DFTI inv_c2c");

    return 0;
  }();

  } // namespace icl::math
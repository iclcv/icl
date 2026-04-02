// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Apple Accelerate (vDSP) backend for FFT operations.
// Uses vDSP_DFT for 1D transforms, row-column decomposition for 2D.
// This file is excluded from the build when Accelerate is not found.

#include <ICLMath/FFTOps.h>
#include <ICLUtils/Exception.h>
#include <Accelerate/Accelerate.h>
#include <complex>
#include <vector>

using namespace icl::utils;

namespace icl::math {
    namespace {

      // ================================================================
      // Float (single precision) — vDSP_DFT
      // ================================================================

      void acc_fft_r2c_f(const float* src, int rows, int cols, std::complex<float>* dst) {
        int dim = rows * cols;

        // Convert real input to split complex (imag = 0)
        std::vector<float> real(dim), imag(dim, 0.0f);
        std::copy(src, src + dim, real.begin());

        // Row DFTs
        vDSP_DFT_Setup rowSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)cols, vDSP_DFT_FORWARD);
        if(!rowSetup) throw ICLException("Accelerate FFT: failed to create row DFT setup (cols=" + std::to_string(cols) + ")");

        std::vector<float> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_Execute(rowSetup,
                           real.data() + r * cols, imag.data() + r * cols,
                           tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetup(rowSetup);

        // Column DFTs
        vDSP_DFT_Setup colSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)rows, vDSP_DFT_FORWARD);
        if(!colSetup) throw ICLException("Accelerate FFT: failed to create col DFT setup (rows=" + std::to_string(rows) + ")");

        std::vector<float> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_Execute(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<float>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetup(colSetup);
      }

      void acc_fft_c2c_f(const std::complex<float>* src, int rows, int cols, std::complex<float>* dst) {
        int dim = rows * cols;

        // Deinterleave to split complex
        std::vector<float> real(dim), imag(dim);
        for(int i = 0; i < dim; ++i) { real[i] = src[i].real(); imag[i] = src[i].imag(); }

        // Row DFTs
        vDSP_DFT_Setup rowSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)cols, vDSP_DFT_FORWARD);
        if(!rowSetup) throw ICLException("Accelerate FFT: failed to create row DFT setup");

        std::vector<float> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_Execute(rowSetup,
                           real.data() + r * cols, imag.data() + r * cols,
                           tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetup(rowSetup);

        // Column DFTs
        vDSP_DFT_Setup colSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)rows, vDSP_DFT_FORWARD);
        if(!colSetup) throw ICLException("Accelerate FFT: failed to create col DFT setup");

        std::vector<float> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_Execute(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<float>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetup(colSetup);
      }

      void acc_ifft_c2c_f(const std::complex<float>* src, int rows, int cols, std::complex<float>* dst) {
        int dim = rows * cols;

        // Deinterleave to split complex
        std::vector<float> real(dim), imag(dim);
        for(int i = 0; i < dim; ++i) { real[i] = src[i].real(); imag[i] = src[i].imag(); }

        // Row inverse DFTs
        vDSP_DFT_Setup rowSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)cols, vDSP_DFT_INVERSE);
        if(!rowSetup) throw ICLException("Accelerate IFFT: failed to create row DFT setup");

        std::vector<float> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_Execute(rowSetup,
                           real.data() + r * cols, imag.data() + r * cols,
                           tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetup(rowSetup);

        // Column inverse DFTs
        vDSP_DFT_Setup colSetup = vDSP_DFT_zop_CreateSetup(NULL, (vDSP_Length)rows, vDSP_DFT_INVERSE);
        if(!colSetup) throw ICLException("Accelerate IFFT: failed to create col DFT setup");

        std::vector<float> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_Execute(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<float>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetup(colSetup);

        // Normalize by 1/(rows*cols) — vDSP inverse DFT does not normalize
        float scale = 1.0f / float(dim);
        for(int i = 0; i < dim; ++i) dst[i] *= scale;
      }

      // ================================================================
      // Double precision — vDSP_DFT_*D
      // ================================================================

      void acc_fft_r2c_d(const double* src, int rows, int cols, std::complex<double>* dst) {
        int dim = rows * cols;

        std::vector<double> real(dim), imag(dim, 0.0);
        std::copy(src, src + dim, real.begin());

        vDSP_DFT_SetupD rowSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)cols, vDSP_DFT_FORWARD);
        if(!rowSetup) throw ICLException("Accelerate FFT: failed to create row DFT setup (double)");

        std::vector<double> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_ExecuteD(rowSetup,
                            real.data() + r * cols, imag.data() + r * cols,
                            tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetupD(rowSetup);

        vDSP_DFT_SetupD colSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)rows, vDSP_DFT_FORWARD);
        if(!colSetup) throw ICLException("Accelerate FFT: failed to create col DFT setup (double)");

        std::vector<double> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_ExecuteD(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<double>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetupD(colSetup);
      }

      void acc_fft_c2c_d(const std::complex<double>* src, int rows, int cols, std::complex<double>* dst) {
        int dim = rows * cols;

        std::vector<double> real(dim), imag(dim);
        for(int i = 0; i < dim; ++i) { real[i] = src[i].real(); imag[i] = src[i].imag(); }

        vDSP_DFT_SetupD rowSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)cols, vDSP_DFT_FORWARD);
        if(!rowSetup) throw ICLException("Accelerate FFT: failed to create row DFT setup (double)");

        std::vector<double> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_ExecuteD(rowSetup,
                            real.data() + r * cols, imag.data() + r * cols,
                            tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetupD(rowSetup);

        vDSP_DFT_SetupD colSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)rows, vDSP_DFT_FORWARD);
        if(!colSetup) throw ICLException("Accelerate FFT: failed to create col DFT setup (double)");

        std::vector<double> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_ExecuteD(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<double>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetupD(colSetup);
      }

      void acc_ifft_c2c_d(const std::complex<double>* src, int rows, int cols, std::complex<double>* dst) {
        int dim = rows * cols;

        std::vector<double> real(dim), imag(dim);
        for(int i = 0; i < dim; ++i) { real[i] = src[i].real(); imag[i] = src[i].imag(); }

        vDSP_DFT_SetupD rowSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)cols, vDSP_DFT_INVERSE);
        if(!rowSetup) throw ICLException("Accelerate IFFT: failed to create row DFT setup (double)");

        std::vector<double> tmpR(dim), tmpI(dim);
        for(int r = 0; r < rows; ++r) {
          vDSP_DFT_ExecuteD(rowSetup,
                            real.data() + r * cols, imag.data() + r * cols,
                            tmpR.data() + r * cols, tmpI.data() + r * cols);
        }
        vDSP_DFT_DestroySetupD(rowSetup);

        vDSP_DFT_SetupD colSetup = vDSP_DFT_zop_CreateSetupD(NULL, (vDSP_Length)rows, vDSP_DFT_INVERSE);
        if(!colSetup) throw ICLException("Accelerate IFFT: failed to create col DFT setup (double)");

        std::vector<double> colIn(rows), colImIn(rows), colOut(rows), colImOut(rows);
        for(int c = 0; c < cols; ++c) {
          for(int r = 0; r < rows; ++r) { colIn[r] = tmpR[r * cols + c]; colImIn[r] = tmpI[r * cols + c]; }
          vDSP_DFT_ExecuteD(colSetup, colIn.data(), colImIn.data(), colOut.data(), colImOut.data());
          for(int r = 0; r < rows; ++r) dst[r * cols + c] = std::complex<double>(colOut[r], colImOut[r]);
        }
        vDSP_DFT_DestroySetupD(colSetup);

        double scale = 1.0 / double(dim);
        for(int i = 0; i < dim; ++i) dst[i] *= scale;
      }

    } // anonymous namespace

    static const int _acc_fft_reg = []() {
      auto acc_f = FFTOps<float>::instance().backends(Backend::Accelerate);
      acc_f.add<FFTOps<float>::R2CSig>(FFTOp::r2c, acc_fft_r2c_f, "Accelerate vDSP r2c");
      acc_f.add<FFTOps<float>::C2CSig>(FFTOp::c2c, acc_fft_c2c_f, "Accelerate vDSP c2c");
      acc_f.add<FFTOps<float>::InvC2CSig>(FFTOp::inv_c2c, acc_ifft_c2c_f, "Accelerate vDSP inv_c2c");

      auto acc_d = FFTOps<double>::instance().backends(Backend::Accelerate);
      acc_d.add<FFTOps<double>::R2CSig>(FFTOp::r2c, acc_fft_r2c_d, "Accelerate vDSP r2c");
      acc_d.add<FFTOps<double>::C2CSig>(FFTOp::c2c, acc_fft_c2c_d, "Accelerate vDSP c2c");
      acc_d.add<FFTOps<double>::InvC2CSig>(FFTOp::inv_c2c, acc_ifft_c2c_d, "Accelerate vDSP inv_c2c");

      return 0;
    }();

  } // namespace icl::math
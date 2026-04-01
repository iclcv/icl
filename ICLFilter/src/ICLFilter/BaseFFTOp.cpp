// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <ICLFilter/BaseFFTOp.h>
#include <ICLCore/CoreFunctions.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;
using namespace icl::math::fft;

namespace icl {
  namespace filter {

    // ================================================================
    // Data — internal state (buffers + params)
    // ================================================================

    struct BaseFFTOp::Data {
      bool m_inverse;
      ResultMode m_rm;
      SizeAdaptionMode m_sam;
      bool m_shift;
      bool m_forceDFT;
      bool m_join = false;
      Rect m_roi;
      ImgBase* m_adaptedSource = nullptr;
      DynMatrix<std::complex<float>>  m_buf32f;
      DynMatrix<std::complex<double>> m_buf64f;
      DynMatrix<std::complex<float>>  m_dstBuf32f;
      DynMatrix<std::complex<double>> m_dstBuf64f;

      Data(bool inverse, ResultMode rm, SizeAdaptionMode sam, bool shift, bool forceDFT)
        : m_inverse(inverse), m_rm(rm), m_sam(sam), m_shift(shift), m_forceDFT(forceDFT) {}

      ~Data() { ICL_DELETE(m_adaptedSource); }
    };

    // ================================================================
    // Constructor / Destructor
    // ================================================================

    BaseFFTOp::BaseFFTOp(bool inverse, ResultMode rm, SizeAdaptionMode sam,
                         bool shift, bool forceDFT)
      : m_data(new Data(inverse, rm, sam, shift, forceDFT)) {}

    BaseFFTOp::~BaseFFTOp() { delete m_data; }

    // ================================================================
    // Getters / Setters
    // ================================================================

    void BaseFFTOp::setResultMode(ResultMode rm) { m_data->m_rm = rm; }
    BaseFFTOp::ResultMode BaseFFTOp::getResultMode() const { return m_data->m_rm; }

    void BaseFFTOp::setSizeAdaptionMode(SizeAdaptionMode sam) { m_data->m_sam = sam; }
    BaseFFTOp::SizeAdaptionMode BaseFFTOp::getSizeAdaptionMode() const { return m_data->m_sam; }

    void BaseFFTOp::setForceDFT(bool f) { m_data->m_forceDFT = f; }
    bool BaseFFTOp::getForceDFT() const { return m_data->m_forceDFT; }

    void BaseFFTOp::setShift(bool s) { m_data->m_shift = s; }
    bool BaseFFTOp::getShift() const { return m_data->m_shift; }

    bool BaseFFTOp::isInverse() const { return m_data->m_inverse; }

    void BaseFFTOp::setJoinInput(bool join) { m_data->m_join = join; }
    bool BaseFFTOp::getJoinInput() const { return m_data->m_join; }

    void BaseFFTOp::setRemovePadROI(const Rect& roi) { m_data->m_roi = roi; }
    Rect BaseFFTOp::getRemovePadROI() const { return m_data->m_roi; }

    // ================================================================
    // adaptSource — size adaptation before FFT/IFFT
    // ================================================================

    template<class T>
    const Img<T>* BaseFFTOp::adaptSource(const Img<T>* src) {
      ICL_DELETE(m_data->m_adaptedSource);

      // Inverse: apply ifftshift before size adaptation
      if(m_data->m_inverse && m_data->m_shift) {
        for(int i = 0; i < src->getChannels(); ++i) {
          DynMatrix<T> t = src->extractDynMatrix(i);
          applyIfftShift(t);
        }
      }

      switch(m_data->m_sam) {
        case NO_SCALE:
          return src;

        case PAD_ZERO: {
          int h = nextPowerOf2(src->getHeight());
          int w = nextPowerOf2(src->getWidth());
          m_data->m_adaptedSource = new Img<T>(Size(w, h), src->getChannels(), formatMatrix);
          for(int i = 0; i < src->getChannels(); ++i) {
            DynMatrix<T> m = m_data->m_adaptedSource->asImg<T>()->extractDynMatrix(i);
            makeborder(src->extractDynMatrix(i), m, T(0));
          }
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        case PAD_COPY: {
          int h = nextPowerOf2(src->getHeight());
          int w = nextPowerOf2(src->getWidth());
          m_data->m_adaptedSource = new Img<T>(Size(w, h), src->getChannels(), formatMatrix);
          for(int i = 0; i < src->getChannels(); ++i) {
            DynMatrix<T> m = m_data->m_adaptedSource->asImg<T>()->extractDynMatrix(i);
            continueMatrixToPowerOf2(src->extractDynMatrix(i), m);
          }
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        case PAD_MIRROR: {
          int h = nextPowerOf2(src->getHeight());
          int w = nextPowerOf2(src->getWidth());
          m_data->m_adaptedSource = new Img<T>(Size(w, h), src->getChannels(), formatMatrix);
          for(int i = 0; i < src->getChannels(); ++i) {
            DynMatrix<T> m = m_data->m_adaptedSource->asImg<T>()->extractDynMatrix(i);
            mirrorOnCenter(src->extractDynMatrix(i), m);
          }
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        case SCALE_UP: {
          int h = nextPowerOf2(src->getHeight());
          int w = nextPowerOf2(src->getWidth());
          m_data->m_adaptedSource = src->scaledCopy(Size(w, h), interpolateLIN);
          m_data->m_adaptedSource->detach(-1);
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        case SCALE_DOWN: {
          int h = priorPowerOf2(src->getHeight());
          int w = priorPowerOf2(src->getWidth());
          m_data->m_adaptedSource = src->scaledCopy(Size(w, h), interpolateRA);
          m_data->m_adaptedSource->detach(-1);
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        case PAD_REMOVE: {
          const Rect& roi = m_data->m_roi;
          m_data->m_adaptedSource = new Img<T>(Size(roi.width, roi.height),
                                               src->getChannels(), formatMatrix);
          for(int i = 0; i < src->getChannels(); ++i) {
            DynMatrix<T> m = m_data->m_adaptedSource->asImg<T>()->extractDynMatrix(i);
            DynMatrix<T> m2 = src->extractDynMatrix(i);
            for(int y = 0; y < roi.height; ++y) {
              for(int x = 0; x < roi.width; ++x) {
                m(x, y) = m2(x + roi.x, y + roi.y);
              }
            }
          }
          return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
        }
        default:
          return src;
      }
    }

    // Explicit instantiations
    template const Img<icl8u>*  BaseFFTOp::adaptSource(const Img<icl8u>*);
    template const Img<icl16s>* BaseFFTOp::adaptSource(const Img<icl16s>*);
    template const Img<icl32s>* BaseFFTOp::adaptSource(const Img<icl32s>*);
    template const Img<icl32f>* BaseFFTOp::adaptSource(const Img<icl32f>*);
    template const Img<icl64f>* BaseFFTOp::adaptSource(const Img<icl64f>*);

    // ================================================================
    // applyInternal — per-channel FFT/IFFT + result mode extraction
    // ================================================================

    template<class SrcT, class DstT>
    void BaseFFTOp::applyInternal(const Img<SrcT>& src, Img<DstT>& dst,
                                  DynMatrix<std::complex<DstT>>& buf,
                                  DynMatrix<std::complex<DstT>>& dstBuf) {
      dstBuf.setBounds(dst.getWidth(), dst.getHeight());
      buf.setBounds(dst.getHeight(), dst.getWidth());

      const bool inverse = m_data->m_inverse;
      const bool join = inverse && m_data->m_join;
      DynMatrix<std::complex<DstT>> joinMat;
      if(join) joinMat.setBounds(src.getWidth(), src.getHeight());

      // For join mode, dst has half as many channels as src
      const int nIter = join ? dst.getChannels()
                       : src.getChannels();

      for(int ch = 0; ch < nIter; ++ch) {
        // Prepare input
        if(join) {
          joinComplex(src.extractDynMatrix(2 * ch),
                      src.extractDynMatrix(2 * ch + 1), joinMat);
        }

        // Compute FFT or IFFT
        if(m_data->m_forceDFT) {
          if(inverse) {
            if(join) idft2D(joinMat, dstBuf, buf);
            else { DynMatrix<SrcT> sm = src.extractDynMatrix(ch); idft2D(sm, dstBuf, buf); }
          } else {
            DynMatrix<SrcT> sm = src.extractDynMatrix(ch);
            dft2D(sm, dstBuf, buf);
          }
        } else {
          if(inverse) {
            if(join) ifft2D(joinMat, dstBuf, buf);
            else { DynMatrix<SrcT> sm = src.extractDynMatrix(ch); ifft2D(sm, dstBuf, buf); }
          } else {
            DynMatrix<SrcT> sm = src.extractDynMatrix(ch);
            fft2D(sm, dstBuf, buf);
          }
        }

        // Extract result mode
        switch(m_data->m_rm) {
          case TWO_CHANNEL_COMPLEX: {
            DynMatrix<DstT> re = dst.extractDynMatrix(2 * ch);
            DynMatrix<DstT> im = dst.extractDynMatrix(2 * ch + 1);
            split_complex(dstBuf, re, im);
            break;
          }
          case IMAG_ONLY: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            imagpart(dstBuf, mm);
            break;
          }
          case REAL_ONLY: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            realpart(dstBuf, mm);
            break;
          }
          case POWER_SPECTRUM: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            powerspectrum(dstBuf, mm);
            break;
          }
          case LOG_POWER_SPECTRUM: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            logpowerspectrum<DstT>(dstBuf, mm);
            break;
          }
          case MAGNITUDE_ONLY: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            magnitude(dstBuf, mm);
            break;
          }
          case PHASE_ONLY: {
            DynMatrix<DstT> mm = dst.extractDynMatrix(ch);
            phase(dstBuf, mm);
            break;
          }
          case TWO_CHANNEL_MAGNITUDE_PHASE: {
            DynMatrix<DstT> mag = dst.extractDynMatrix(2 * ch);
            DynMatrix<DstT> ph  = dst.extractDynMatrix(2 * ch + 1);
            split_magnitude_phase(dstBuf, mag, ph);
            break;
          }
        }
      }

      // Apply shift on output (forward: fftshift, inverse: not needed — was done in adaptSource)
      if(m_data->m_shift && !inverse) {
        for(int i = 0; i < dst.getChannels(); ++i) {
          DynMatrix<DstT> mm = dst.extractDynMatrix(i);
          applyFftShift(mm);
        }
      }
    }

    // Explicit instantiations (5 src types × 2 dst types = 10)
#define INST_APPLY_INTERNAL(S, D)                                       \
    template void BaseFFTOp::applyInternal(const Img<S>&, Img<D>&,      \
      DynMatrix<std::complex<D>>&, DynMatrix<std::complex<D>>&);

    INST_APPLY_INTERNAL(icl8u,  icl32f)
    INST_APPLY_INTERNAL(icl16s, icl32f)
    INST_APPLY_INTERNAL(icl32s, icl32f)
    INST_APPLY_INTERNAL(icl32f, icl32f)
    INST_APPLY_INTERNAL(icl64f, icl32f)
    INST_APPLY_INTERNAL(icl8u,  icl64f)
    INST_APPLY_INTERNAL(icl16s, icl64f)
    INST_APPLY_INTERNAL(icl32s, icl64f)
    INST_APPLY_INTERNAL(icl32f, icl64f)
    INST_APPLY_INTERNAL(icl64f, icl64f)
#undef INST_APPLY_INTERNAL

    // ================================================================
    // apply — main entry point (native Image API)
    // ================================================================

    void BaseFFTOp::apply(const Image& src, Image& dst) {
      ICLASSERT_RETURN(!src.isNull());

      // Determine output depth: use 64f if input is 64f, else 32f
      depth dstDepth = (src.getDepth() == depth64f) ? depth64f : depth32f;

      // Adapt source (size adaptation, ifftshift for inverse)
      const ImgBase* adapted = src.ptr();
      src.visit([&](const auto& typed) {
        adapted = adaptSource(&typed);
      });

      // Compute output channel count based on result mode and join setting
      int nChannels = adapted->getChannels();
      bool twoChannelMode = (m_data->m_rm == TWO_CHANNEL_COMPLEX
                          || m_data->m_rm == TWO_CHANNEL_MAGNITUDE_PHASE);
      if(m_data->m_inverse && m_data->m_join) {
        // join: 2 input channels → 1 output channel (or 2 for TWO_CHANNEL modes)
        nChannels = twoChannelMode ? adapted->getChannels() : adapted->getChannels() / 2;
      } else if(twoChannelMode) {
        nChannels = adapted->getChannels() * 2;
      }

      // Prepare destination
      Size adaptedSize = adapted->getSize();
      Rect dstROI(Point::null, adaptedSize);
      if(!prepare(dst, dstDepth, adaptedSize, formatMatrix, nChannels,
                  dstROI, adapted->getTime())) {
        throw ICLException("BaseFFTOp: preparation of destination image failed");
      }

      // Dispatch on src depth × dst depth
      auto dispatchDst = [&](const auto& typedSrc) {
        if(dstDepth == depth32f) {
          applyInternal(typedSrc, dst.as32f(), m_data->m_buf32f, m_data->m_dstBuf32f);
        } else {
          applyInternal(typedSrc, dst.as64f(), m_data->m_buf64f, m_data->m_dstBuf64f);
        }
      };

      switch(adapted->getDepth()) {
        case depth8u:  dispatchDst(*adapted->asImg<icl8u>());  break;
        case depth16s: dispatchDst(*adapted->asImg<icl16s>()); break;
        case depth32s: dispatchDst(*adapted->asImg<icl32s>()); break;
        case depth32f: dispatchDst(*adapted->asImg<icl32f>()); break;
        case depth64f: dispatchDst(*adapted->asImg<icl64f>()); break;
      }
    }

    // ================================================================
    // Shift implementations (from original FFTOp/IFFTOp)
    // ================================================================

    template<class T>
    void BaseFFTOp::applyFftShift(DynMatrix<T>& mat) {
      unsigned int cols = mat.cols();
      unsigned int cols2 = cols / 2;
      unsigned int rows = mat.rows();
      unsigned int rows2 = rows / 2;
      if(cols > 1 && rows > 1) {
        if(cols % 2 == 0 || rows % 2 == 0) {
          T temp = T(0);
          for(unsigned int y = 0; y < rows2; ++y) {
            for(unsigned int x = 0; x < cols2; ++x) {
              temp = mat(x, y);
              mat(x, y) = mat(x + cols2, y + rows2);
              mat(x + cols2, y + rows2) = temp;
              temp = mat(x + cols2, y);
              mat(x + cols2, y) = mat(x, y + rows2);
              mat(x, y + rows2) = temp;
            }
          }
          if(cols % 2 == 1 && rows % 2 == 0) {
            T t2, t3;
            for(unsigned int y = 0; y < rows2; ++y) {
              t2 = mat(cols - 1, y);
              t3 = mat(cols - 1, y + rows2);
              mat(cols - 1, y) = mat(0, y);
              mat(cols - 1, y + rows2) = mat(0, y + rows2);
              for(unsigned int x = 1; x < cols2; ++x) {
                mat(x - 1, y) = mat(x, y);
                mat(x - 1, y + rows2) = mat(x, y + rows2);
              }
              mat(cols2 - 1, y) = t3;
              mat(cols2 - 1, y + rows2) = t2;
            }
          } else if(cols % 2 == 0 && rows % 2 == 1) {
            T t2, t3;
            for(unsigned int x = 0; x < cols2; ++x) {
              t2 = mat(x, rows - 1);
              t3 = mat(x + cols2, rows - 1);
              mat(x, rows - 1) = mat(x, 0);
              mat(x + cols2, rows - 1) = mat(x + cols2, 0);
              for(unsigned int y = 1; y < rows2; ++y) {
                mat(x, y - 1) = mat(x, y);
                mat(x + cols2, y - 1) = mat(x + cols2, y);
              }
              mat(x, rows2 - 1) = t3;
              mat(x + cols2, rows2 - 1) = t2;
            }
          }
        } else {
          // Both odd
          unsigned int dim = cols, dim2 = dim / 2;
          for(unsigned int k = 0; k < rows; ++k) {
            T* dat = mat.row_begin(k);
            for(unsigned int i = 0; i < dim2; ++i) std::swap(dat[i], dat[i + dim2]);
            T temp = dat[dim - 1];
            dat[dim - 1] = dat[0];
            for(unsigned int i = 1; i < dim2; ++i) dat[i - 1] = dat[i];
            if(dim2 > 0) dat[dim2 - 1] = temp;
          }
          dim = rows; dim2 = dim / 2;
          for(unsigned int k = 0; k < cols; ++k) {
            for(unsigned int i = 0; i < dim2; ++i) std::swap(mat(k, i), mat(k, i + dim2));
            T temp = mat(k, dim - 1);
            mat(k, dim - 1) = mat(k, 0);
            for(unsigned int i = 1; i < dim2; ++i) mat(k, i - 1) = mat(k, i);
            if(dim2 > 0) mat(k, dim2 - 1) = temp;
          }
        }
      } else {
        // 1D case
        unsigned int dim = cols * rows, dim2 = dim / 2;
        for(unsigned int i = 0; i < dim2; ++i) std::swap(mat.data()[i], mat.data()[i + dim2]);
        if(cols % 2 == 1 && rows % 2 == 1) {
          T temp = mat.data()[dim - 1];
          mat.data()[dim - 1] = mat.data()[0];
          for(unsigned int i = 1; i < dim2; ++i) mat.data()[i - 1] = mat.data()[i];
          if(dim2 > 0) mat.data()[dim2 - 1] = temp;
        }
      }
    }

    template<class T>
    void BaseFFTOp::applyIfftShift(DynMatrix<T>& mat) {
      unsigned int cols = mat.cols();
      unsigned int cols2 = cols / 2;
      unsigned int rows = mat.rows();
      unsigned int rows2 = rows / 2;
      if(cols > 1 && rows > 1) {
        if(cols % 2 == 0 || rows % 2 == 0) {
          T temp = T(0);
          for(unsigned int y = 0; y < rows2; ++y) {
            for(unsigned int x = 0; x < cols2; ++x) {
              temp = mat(x, y);
              mat(x, y) = mat(x + cols2, y + rows2);
              mat(x + cols2, y + rows2) = temp;
              temp = mat(x + cols2, y);
              mat(x + cols2, y) = mat(x, y + rows2);
              mat(x, y + rows2) = temp;
            }
          }
          if(cols % 2 == 1 && rows % 2 == 0) {
            T t2, t3;
            for(unsigned int y = 0; y < rows2; ++y) {
              t2 = mat(cols - 1, y);
              t3 = mat(cols - 1, y + rows2);
              mat(cols - 1, y) = mat(0, y);
              mat(cols - 1, y + rows2) = mat(0, y + rows2);
              for(unsigned int x = 1; x < cols2; ++x) {
                mat(x - 1, y) = mat(x, y);
                mat(x - 1, y + rows2) = mat(x, y + rows2);
              }
              mat(cols2 - 1, y) = t3;
              mat(cols2 - 1, y + rows2) = t2;
            }
            for(unsigned int y = 0; y < rows; ++y) {
              for(unsigned int x = cols - 1; x > 0; --x) {
                std::swap(mat(x, y), mat(x - 1, y));
              }
            }
          } else if(cols % 2 == 0 && rows % 2 == 1) {
            T t2, t3;
            for(unsigned int x = 0; x < cols2; ++x) {
              t2 = mat(x, rows - 1);
              t3 = mat(x + cols2, rows - 1);
              mat(x, rows - 1) = mat(x, 0);
              mat(x + cols2, rows - 1) = mat(x + cols2, 0);
              for(unsigned int y = 1; y < rows2; ++y) {
                mat(x, y - 1) = mat(x, y);
                mat(x + cols2, y - 1) = mat(x + cols2, y);
              }
              mat(x, rows2 - 1) = t3;
              mat(x + cols2, rows2 - 1) = t2;
            }
            for(unsigned int x = 0; x < cols; ++x) {
              for(unsigned int y = rows - 1; y > 0; --y) {
                std::swap(mat(x, y), mat(x, y - 1));
              }
            }
          }
        } else {
          // Both odd
          unsigned int dim = cols, dim2 = dim / 2;
          for(unsigned int k = 0; k < rows; ++k) {
            T* dat = mat.row_begin(k);
            for(unsigned int i = 0; i < dim2; ++i) std::swap(dat[i], dat[i + dim2]);
            for(unsigned int i = dim - 1; i > dim2; --i) std::swap(dat[i], dat[i - 1]);
          }
          dim = rows; dim2 = dim / 2;
          for(unsigned int k = 0; k < cols; ++k) {
            for(unsigned int i = 0; i < dim2; ++i) std::swap(mat(k, i), mat(k, i + dim2));
            for(unsigned int i = dim - 1; i > dim2; --i) std::swap(mat(k, i), mat(k, i - 1));
          }
        }
      } else {
        // 1D case
        unsigned int dim = cols * rows, dim2 = dim / 2;
        for(unsigned int i = 0; i < dim2; ++i) std::swap(mat.data()[i], mat.data()[i + dim2]);
        if(cols % 2 == 1 && rows % 2 == 1) {
          for(unsigned int i = dim - 1; i > dim2; --i) std::swap(mat.data()[i], mat.data()[i - 1]);
        }
      }
    }

    // Shift instantiations
    template void BaseFFTOp::applyFftShift(DynMatrix<icl32f>&);
    template void BaseFFTOp::applyFftShift(DynMatrix<icl64f>&);
    template void BaseFFTOp::applyIfftShift(DynMatrix<icl8u>&);
    template void BaseFFTOp::applyIfftShift(DynMatrix<icl16s>&);
    template void BaseFFTOp::applyIfftShift(DynMatrix<icl32s>&);
    template void BaseFFTOp::applyIfftShift(DynMatrix<icl32f>&);
    template void BaseFFTOp::applyIfftShift(DynMatrix<icl64f>&);

  } // namespace filter
} // namespace icl

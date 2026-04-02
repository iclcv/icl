// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>

namespace icl::filter {
    /// Class that implements dithering mechanisms
    /** Dithering techniques approximate gray-scale values using black and white patterns.

        \section DATA Data Types
        Internally, the input image is converted to 8u format. The result will always be
        an Img8u image.


        \section LEVELS Levels
        Theoretically all numbers of levels (between 1 and 256) are supported, but logically
        the values that are given to the constructor or via DitheringOp::setLevels are clipped
        to the range [2,128]

        \section BENCH Benchmark
        As a very coarse estimate, dithering takes about 7.5ms per million pixels (single channel)
        on an Intel(R) Core(TM) i7-4700MQ CPU @ 2.40GHz with optized build.

        \section EX Examples

        <table>
        <tr>
        <td>\image html dither-base.png</td>
        <td>\image html dither-2.png</td>
        <td>\image html dither-3.png</td>
        <td>\image html dither-4.png</td>
        <td>\image html dither-8.png</td>
        </tr>
        <tr>
        <td>input image</td>
        <td>2 Levels</td>
        <td>3 Levels</td>
        <td>4 Levels</td>
        <td>8 Levels</td>
        </tr>

        </table>
        */
    class ICLFilter_API DitheringOp : public UnaryOp {
      public:
      DitheringOp(const DitheringOp&) = delete;
      DitheringOp& operator=(const DitheringOp&) = delete;


      enum Algorithm{
        FloydSteinberg //!< right not the only available algorithm (fast)
      };
      /// Constructor
      DitheringOp (Algorithm a=FloydSteinberg, int levels=2);

      /// Destructor
      virtual ~DitheringOp(){}

      /// Applies the mirror transform to the images
      void apply(const core::Image &src, core::Image &dst) override;

      /// returns the internal dithering algorithm used
      Algorithm getAlgorithm() const { return m_algorithm; }

      /// sets the internal dithering algorithm used
      void setAlgorithm(Algorithm a) { m_algorithm = a; }

      /// returns the number of quantisation levels
      int getLevels() const { return m_levels; }

      /// sets the number of quantisation levels
      void setLevels(int l) {
        if(l<2) l=2;
        if(l>128) l=128;
        m_levels = l;
      }

      /// import base-class symbols
      using UnaryOp::apply;

      private:
      Algorithm m_algorithm;
      int m_levels;
    };
  } // namespace icl::filter
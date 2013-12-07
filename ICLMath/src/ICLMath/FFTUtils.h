/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FFTUtils.h                         **
** Module : ICLMath                                                **
** Authors: Christian Groszewski, Christof Elbrechter              **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#include <complex>
#include <ICLUtils/BasicTypes.h>
#include <ICLMath/DynMatrix.h>
#include <ICLMath/FFTException.h>
#include <string.h>
#include <ICLUtils/CompatMacros.h>

using namespace icl::utils;

namespace icl{
  namespace math{
  
  namespace fft{
  
  /// 2*PI
  static const double FFT_2_PI = 2.0*3.1415926535897932384626433832795288419716939937510;
  
  /// PI
  static const double FFT_PI = 3.1415926535897932384626433832795288419716939937510;
  
  /// PI/2
  static const double FFT_PI_HALF = 1.5707963267948966192313216916397644209858469968755;
  
  ///1dfft computation (fallback)
  /**Computes the 1D Fast-Fourier-Transformation for given data.
     If the size of the dataarray is a power of 2, the fft is performed,
     else the dft is performed on the datapart, on which fft does not work.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     Possible outputdatatype are std::complex<icl32f> and std::complex<icl64f>
     @param n size of the dataarray
     @param data the dataarray
     @return array of fftvalues for data
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T std::complex<T2>* fft(unsigned int n, const T1* data);
  
  ///2dfft computation (fallback)
  /**Computes the 2D Fast-Fourier-Transformation for given data.
     Works even if datasize is not a power of 2.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     Possible outputdatatype are std::complex<icl32f> and std::complex<icl64f>
     @param src datamatrix of size MxN
     @param dst destinationmatrix of size MxN
     @param buf buffermatrix of size NxM !!!
     @return matrix of fftvalues for datamatrix
   */
  template<typename T1, typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> >&  fft2D_cpp(const DynMatrix<T1> &src,
  		DynMatrix<std::complex<T2> > &dst,DynMatrix<std::complex<T2> > &buf);
  
  ///2dfft computation
  /**Computes the 2D Fast-Fourier-Transformation for given data.
    Uses MKL or IPP if available, else fft2D_cpp (fallback).
    Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     Possible outputdatatype are std::complex<icl32f> and std::complex<icl64f>
     @param src datamatrix of size MxN
     @param dst destinationmatrix of size MxN
     @param buf buffermatrix of size NxM !!!
     @return matrix of fftvalues for datamatrix
   */
  template<typename T1, typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> >& fft2D(const DynMatrix<T1> &src, DynMatrix<std::complex<T2> > &dst,
  		DynMatrix<std::complex<T2> > &buf);
  
  ///1d dft computation
  /**Computes the 1D Diskrete-Fourier-Transformation for given data.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     Possible outputdatatype are std::complex<icl32f> and std::complex<icl64f>
     @param n size of the dataarray
     @param matrix the dataarray
     @return array of fftvalues for data
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T std::complex<T2>* dft(unsigned int n, T1 *matrix);
  
  ///2d dft computation
  /**Computes the 2D Diskrete-Fourier-Transformation for given data.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     Possible outputdatatype are std::complex<icl32f> and std::complex<icl64f>
     @param src datamatrix of size MxN
     @param dst destinationmatrix of size MxN
     @param buf buffermarix of size NxM
     @return matrix of fftvalues for datamatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> >&  dft2D(DynMatrix<T1> &src,
  		DynMatrix<std::complex<T2> >& dst,DynMatrix<std::complex<T2> >&buf);
  
  ///1d ifft computation
  /**Computes the 1D Inverse-Fast-Fourier-Transformation.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     @param n size of the dataarray
     @param matrix the dataarray
     @return array of ifftvalues for data
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T std::complex<T2>* ifft_cpp(unsigned int n, const T1* matrix);
  
  ///2d ifft computation (fallback)
  /**Computes the 2D Inverse-Fast-Fourier-Transformation.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     @param src datamatrix of size MxN
     @param dst destinationmatrix of size MxN
     @param buf buffermarix of size NxM
     @return matrix of ifftvalues for datamatrix
   */
  template<typename T1, typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> >&  ifft2D_cpp(const DynMatrix<T1>& src,
  		DynMatrix<std::complex<T2> > &dst,DynMatrix<std::complex<T2> > &buf);
  
  ///2d ifft computation
  /**Computes the 2D Inverse-Fast-Fourier-Transformation.
     Uses MKL or IPP if available, else ifft2D_cpp.
     Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
     icl64f, std::complex<icl32f>, std::complex<icl64f>.
     @param src datamatrix of size MxN
     @param dst destinationmatrix of size MxN
     @param buf buffermarix of size NxM
     @return matrix of ifftvalues for datamatrix
   */
  template<typename T1, typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> >&   ifft2D(const DynMatrix<T1> &src, DynMatrix<std::complex<T2> > &dst,
  		DynMatrix<std::complex<T2> > &buf);
  
  ///1d idft computation
  /**Computes the 1D Inverse-Diskrete-Fourier-Transformation for given data.
     @param n size of the dataarray
     @param matrix the dataarray
     @return array of fftvalues for data
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T std::complex<T2>* idft(unsigned int n, T1 *matrix);
  
  ///2d idft computation
  /**
   * Computes the 2D Inverse-Diskrete-Fourier-Transformation for given data.
   * @param src datamatrix of size MxN
   * @param dst destinationmatrix of size MxN
   * @param buf buffermarix of size NxM
   * @return matrix of ifftvalues for datamatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> > &idft2D(DynMatrix<T1> &src, DynMatrix<std::complex<T2> > &dst,
  		DynMatrix<std::complex<T2> > &buf);
  
  ///shifts upper left corner to center.
  /**Shifts the upper left corner of the matrix into the center of it.
     @param src datamatrix to be shifted
     @param dst destinationmatrix
     @return destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T>& fftshift(DynMatrix<T> &src, DynMatrix<T> &dst) throw (InvalidMatrixDimensionException);
  
  ///invers function to fftshift.
  /**Shifts the center of the matrix into the upper left corner .
     @param src datamatrix to be shifted
     @param dst destinationmatrix
     @return destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T>& ifftshift(DynMatrix<T> &src, DynMatrix<T> &dst) throw (InvalidMatrixDimensionException);
  
  ///computes the powerspectrum
  /**Computes and returns the powerspectrum of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T>& powerspectrum(const DynMatrix<std::complex<T> > &src, DynMatrix<T> &dst);
  
  ///computes the logpowerspectrum
  /**Computes and returns the log of the powerspectrum of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T>& logpowerspectrum(const DynMatrix<std::complex<T> > &src, DynMatrix<T> &dst);
  
  ///creates border with given value
  /**Resizes the sourcematrix to the next power of 2, centers the original matrix and fills the border
     with the given value.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @param borderFill Number for filling the border
     @return the destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T> &makeborder(const DynMatrix<T> &src, DynMatrix<T> &dst, T borderFill);
  
  ///mirrors the sourcematrix
  /**Resizes the sourcematrix to the next power of 2,
     centers the original matrix and mirrors it at its borders.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T> &mirrorOnCenter(const DynMatrix<T> &src, DynMatrix<T> &dst);
  
  ///appends copies of sourcematrix beside and under it
  /**Resizes the sourcematrix to the next power of 2
     and copies the sourcematrix as far as
     possible into the destinationmatrix, starting at 0,0.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T>
  ICL_MATH_API_T DynMatrix<T> &continueMatrixToPowerOf2(const DynMatrix<T> &src, DynMatrix<T> &dst);
  
  
  ///Returns the next value greater then n to power of 2
  ICL_MATH_API int nextPowerOf2(int n);
  
  ///Returns the prior value lower then n to power of 2
  ICL_MATH_API int priorPowerOf2(int n);
  
  ///split complexmatrix into realmatrix and imaginarymatrix
  /**Splits a matrix with complex values into two matrices with real and imaginary
     parts as values.
     @param src the sourcematrix with complex values
     @param real the destinationmatrix for the realparts
     @param img the destinationmatrix for the imaginaryparts
   */
  template<typename T>
  ICL_MATH_API_T void split_complex(const DynMatrix<std::complex<T> > &src, DynMatrix<T> &real, DynMatrix<T> &img);
  
  ///returns imaginary part complex matrix
  /**Computes and returns the imaginary part of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<T2> &imagpart(const DynMatrix<std::complex<T1> > &src, DynMatrix<T2> &dst);
  
  ///returns real part of complex matrix
  /**Computes and returns the real part of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<T2> &realpart(const DynMatrix<std::complex<T1> > &src, DynMatrix<T2> &dst);
  
  ///returns the magnitude of complex matrix
  /**Computes and returns the magnitude of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<T2>& magnitude(const DynMatrix<std::complex<T1> > &src, DynMatrix<T2> &dst);
  
  ///return phase of complexmatrix
  /**Computes and returns the phase of a matrix with complex values.
     @param src the sourcematrix
     @param dst the destinationmatrix
     @return the destinationmatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<T2>& phase(const DynMatrix<std::complex<T1> > &src, DynMatrix<T2> &dst);
  
  ///splits complexmatrix into magnitude and phase
  /**Splits a matrix with complex values into two matrices with magnitude and phase as values.
     @param src the sourcematrix with complex values
     @param mag the destinationmatrix for the magnitudevalues
     @param phase the destinationmatrix for the phasevalues
   */
  template<typename T>
  ICL_MATH_API_T void split_magnitude_phase(const DynMatrix<std::complex<T> > &src, DynMatrix<T> &mag, DynMatrix<T> &phase);
  
  ///joins to matrices to one complex
  /**@param real matrix for realpart
     @param im matrix for imaginary part
     @param dst destinationmatrix
   */
  template<typename T1,typename T2>
  ICL_MATH_API_T DynMatrix<std::complex<T2> > &joinComplex(const DynMatrix<T1> &real, const DynMatrix<T1> &im, DynMatrix<std::complex<T2> > &dst);
  
  } // namespace math
}
}

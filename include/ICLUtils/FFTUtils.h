#ifndef ICL_FFTUTILS_H_
#define ICL_FFTUTILS_H_

#include <complex>
#include <ICLCore/Types.h>
#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/FFTException.h>
#include <string.h>

namespace icl{

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
std::complex<T2>* fft(unsigned int n, const T1* data);

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
icl::DynMatrix<std::complex<T2> >&  fft2D_cpp(const icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buf);

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
icl::DynMatrix<std::complex<T2> >& fft2D(const icl::DynMatrix<T1> &src,icl::DynMatrix<std::complex<T2> > &dst,
		icl::DynMatrix<std::complex<T2> > &buf);

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
std::complex<T2>* dft(unsigned int n, T1 *matrix);

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
icl::DynMatrix<std::complex<T2> >&  dft2D(icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> >& dst,icl::DynMatrix<std::complex<T2> >&buf);

///1d ifft computation
/**Computes the 1D Inverse-Fast-Fourier-Transformation.
   Possible inputdatatypes are: icl8u, icl16u, icl32u, icl16s, icl32s, icl32f,
   icl64f, std::complex<icl32f>, std::complex<icl64f>.
   @param n size of the dataarray
   @param matrix the dataarray
   @return array of ifftvalues for data
 */
template<typename T1,typename T2>
std::complex<T2>* ifft_cpp(unsigned int n, const T1* matrix);

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
icl::DynMatrix<std::complex<T2> >&  ifft2D_cpp(const icl::DynMatrix<T1>& src,
		icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buf);

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
icl::DynMatrix<std::complex<T2> >&   ifft2D(const icl::DynMatrix<T1> &src,icl::DynMatrix<std::complex<T2> > &dst,
		icl::DynMatrix<std::complex<T2> > &buf);

///1d idft computation
/**Computes the 1D Inverse-Diskrete-Fourier-Transformation for given data.
   @param n size of the dataarray
   @param matrix the dataarray
   @return array of fftvalues for data
 */
template<typename T1,typename T2>
std::complex<T2>* idft(unsigned int n, T1 *matrix);

///2d idft computation
/**
 * Computes the 2D Inverse-Diskrete-Fourier-Transformation for given data.
 * @param src datamatrix of size MxN
 * @param dst destinationmatrix of size MxN
 * @param buf buffermarix of size NxM
 * @return matrix of ifftvalues for datamatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> > &idft2D(icl::DynMatrix<T1> &src, icl::DynMatrix<std::complex<T2> > &dst,
		icl::DynMatrix<std::complex<T2> > &buf);

///shifts upper left corner to center.
/**Shifts the upper left corner of the matrix into the center of it.
   @param src datamatrix to be shifted
   @param dst destinationmatrix
   @return destinationmatrix
 */
template<typename T>
icl::DynMatrix<T>& fftshift(icl::DynMatrix<T> &src,icl::DynMatrix<T> &dst) throw (InvalidMatrixDimensionException);

///invers function to fftshift.
/**Shifts the center of the matrix into the upper left corner .
   @param src datamatrix to be shifted
   @param dst destinationmatrix
   @return destinationmatrix
 */
template<typename T>
icl::DynMatrix<T>& ifftshift(icl::DynMatrix<T> &src,icl::DynMatrix<T> &dst) throw (InvalidMatrixDimensionException);

///computes the powerspectrum
/**Computes and returns the powerspectrum of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T>
icl::DynMatrix<T>& powerspectrum(const icl::DynMatrix<std::complex<T> > &src,icl::DynMatrix<T> &dst);

///computes the logpowerspectrum
/**Computes and returns the log of the powerspectrum of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T>
icl::DynMatrix<T>& logpowerspectrum(const icl::DynMatrix<std::complex<T> > &src,icl::DynMatrix<T> &dst);

///creates border with given value
/**Resizes the sourcematrix to the next power of 2, centers the original matrix and fills the border
   with the given value.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @param borderFill Number for filling the border
   @return the destinationmatrix
 */
template<typename T>
icl::DynMatrix<T> &makeborder(const icl::DynMatrix<T> &src,icl::DynMatrix<T> &dst, T borderFill);

///mirrors the sourcematrix
/**Resizes the sourcematrix to the next power of 2,
   centers the original matrix and mirrors it at its borders.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T>
icl::DynMatrix<T> &mirrorOnCenter(const icl::DynMatrix<T> &src, icl::DynMatrix<T> &dst);

///appends copies of sourcematrix beside and under it
/**Resizes the sourcematrix to the next power of 2
   and copies the sourcematrix as far as
   possible into the destinationmatrix, starting at 0,0.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T>
icl::DynMatrix<T> &continueMatrixToPowerOf2(const icl::DynMatrix<T> &src, icl::DynMatrix<T> &dst);


///Returns the next value greater then n to power of 2
int nextPowerOf2(int n);

///Returns the prior value lower then n to power of 2
int priorPowerOf2(int n);

///split complexmatrix into realmatrix and imaginarymatrix
/**Splits a matrix with complex values into two matrices with real and imaginary
   parts as values.
   @param src the sourcematrix with complex values
   @param real the destinationmatrix for the realparts
   @param img the destinationmatrix for the imaginaryparts
 */
template<typename T>
void split_complex(const icl::DynMatrix<std::complex<T> > &src, icl::DynMatrix<T> &real, icl::DynMatrix<T> &img);

///returns imaginary part complex matrix
/**Computes and returns the imaginary part of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<T2> &imagpart(const icl::DynMatrix<std::complex<T1> > &src,icl::DynMatrix<T2> &dst);

///returns real part of complex matrix
/**Computes and returns the real part of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<T2> &realpart(const icl::DynMatrix<std::complex<T1> > &src, icl::DynMatrix<T2> &dst);

///returns the magnitude of complex matrix
/**Computes and returns the magnitude of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<T2>& magnitude(const icl::DynMatrix<std::complex<T1> > &src, icl::DynMatrix<T2> &dst);

///return phase of complexmatrix
/**Computes and returns the phase of a matrix with complex values.
   @param src the sourcematrix
   @param dst the destinationmatrix
   @return the destinationmatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<T2>& phase(const icl::DynMatrix<std::complex<T1> > &src, icl::DynMatrix<T2> &dst);

///splits complexmatrix into magnitude and phase
/**Splits a matrix with complex values into two matrices with magnitude and phase as values.
   @param src the sourcematrix with complex values
   @param mag the destinationmatrix for the magnitudevalues
   @param phase the destinationmatrix for the phasevalues
 */
template<typename T>
void split_magnitude_phase(const icl::DynMatrix<std::complex<T> > &src, icl::DynMatrix<T> &mag, icl::DynMatrix<T> &phase);

///joins to matrices to one complex
/**@param real matrix for realpart
   @param im matrix for imaginary part
   @param dst destinationmatrix
 */
template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> > &joinComplex(const icl::DynMatrix<T1> &real, const icl::DynMatrix<T1> &im,icl::DynMatrix<std::complex<T2> > &dst);

}
}
#endif /* ICL_FFTUTILS_H_ */

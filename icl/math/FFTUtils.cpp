// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <icl/math/FFTUtils.h>
#include <icl/math/FFTOps.h>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <vector>

#ifdef ICL_SYSTEM_WINDOWS
#ifdef min
#undef min
#undef max
#endif
#endif

//#define FFT_DEBUG(X) std::cout << X << std::endl;
#define FFT_DEBUG(X)

using namespace icl::utils;

namespace icl::math {
  namespace fft{

    typedef std::complex<icl32f> icl32c;
    typedef std::complex<icl64f> icl64c;

    template<class T1, class T2>
    struct CreateComplex{
	static inline std::complex<T2> create_complex(const T1 &x){
        return std::complex<T2>(x);
	}
    };

#define ICL_INSTANTIATE_DEPTH(D)                                        \
    template<class T2>                                                \
    struct CreateComplex<icl##D,T2>{                                  \
	static inline std::complex<T2> create_complex(const icl##D &x){ \
        return std::complex<T2>((T2)x,0.0);                           \
      }                                                               \
    };                                                                \
    template<class T2>                                                \
    struct CreateComplex<std::complex<icl##D>,T2>{                    \
	static inline std::complex<T2> create_complex(const std::complex<icl##D> &x){ \
        return std::complex<T2>((T2)x.real(),(T2)x.imag());           \
      }                                                               \
    };
    ICL_INSTANTIATE_ALL_DEPTHS;
    ICL_INSTANTIATE_DEPTH(32u);
    ICL_INSTANTIATE_DEPTH(16u);
#undef ICL_INSTANTIATE_DEPTH

    template<typename T>
    DynMatrix<T>&  fftshift(DynMatrix<T> &src,DynMatrix<T> &dst){
	if(src.cols() != dst.cols() || src.rows() != dst.rows())
        throw InvalidMatrixDimensionException("number of columns(rows) of sourcematrix must be equal to number of colums(rows) of destinationmatrix");
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	unsigned int cols2 = cols/2;
	unsigned int rows2 = rows/2;
	if(cols >1 && rows > 1){
        for(unsigned int y=0;y<rows;++y){
          for(unsigned int x=0;x<cols;++x){
            dst.index_yx(((y+rows2))%rows, ((x+cols2))%cols) = src.index_yx(y, x);
          }
        }
	} else {
        int dim = cols*rows;
        int tmp = ceil(dim/2.0);
        for(int i=0;i<dim;++i){
          dst.data()[i] = src.data()[(tmp+i)%dim];
        }
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  fftshift(DynMatrix<icl8u> &src,DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  fftshift(DynMatrix<icl16u> &src,DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  fftshift(DynMatrix<icl16s> &src,DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  fftshift(DynMatrix<icl32u> &src,DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  fftshift(DynMatrix<icl32s> &src,DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  fftshift(DynMatrix<icl32f> &src,DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl32c >&  fftshift(DynMatrix<icl32c > &src,
                                                     DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  fftshift(DynMatrix<icl64f> &src,DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fftshift(DynMatrix<std::complex<icl64f> > &src,
                                                     DynMatrix<std::complex<icl64f> > &dst);

    template<typename T>
    DynMatrix<T>&  ifftshift(DynMatrix<T> &src,
                             DynMatrix<T> &dst){
	if(src.cols() != dst.cols() || src.rows() != dst.rows())
        throw InvalidMatrixDimensionException("number of columns(rows) of sourcematrix must "
                                              "be equal to number of colums(rows) "
                                              "of destinationmatrix");
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	unsigned int cols2 = cols/2;
	unsigned int rows2 = rows/2;
	if(cols >1 && rows > 1){
        for(unsigned int y=0;y<rows;++y){
          for(unsigned int x=0;x<cols;++x){
            dst.index_yx(((y+rows2))%rows, ((x+cols2))%cols) = src.index_yx(y, x);
          }
        }
        if(cols%2==0 && rows%2==0){
          //nothing to do
        } else  if(cols%2==1 && rows%2==0){
          for(unsigned int y=0;y<rows;++y){
            for(unsigned int x=cols-1;x>0;--x){
              std::swap(dst.index_yx(y, x),dst.index_yx(y, x-1));
            }
          }
        } else if(cols%2==0 && rows%2==1){
          for(unsigned int x=0;x<cols;++x){
            for(unsigned int y=rows-1;y>0;--y){
              std::swap(dst.index_yx(y, x),dst.index_yx(y-1, x));
            }
          }
        } else {
          for(unsigned int y=0;y<rows;++y){
            for(unsigned int x=cols-1;x>0;--x){
              std::swap(dst.index_yx(y, x),dst.index_yx(y, x-1));
            }
          }
          for(unsigned int x=0;x<cols;++x){
            for(unsigned int y=rows-1;y>0;--y){
              std::swap(dst.index_yx(y, x),dst.index_yx(y-1, x));
            }
          }
        }
	} else {
        int dim = cols*rows;
        int tmp = dim/2;
        for(int i=0;i<dim;++i){
          dst.data()[i] = src.data()[(tmp+i)%dim];
        }
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  ifftshift(DynMatrix<icl8u> &src,DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  ifftshift(DynMatrix<icl16u> &src,DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  ifftshift(DynMatrix<icl16s> &src,DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  ifftshift(DynMatrix<icl32u> &src,DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  ifftshift(DynMatrix<icl32s> &src,DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  ifftshift(DynMatrix<icl32f> &src,DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl32c >&  ifftshift(DynMatrix<icl32c > &src,
                                   DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  ifftshift(DynMatrix<icl64f> &src,DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifftshift(DynMatrix<std::complex<icl64f> > &src,
                                                 DynMatrix<std::complex<icl64f> > &dst);

    int priorPowerOf2(int n){
	int p = 1;
	while(p<n){
        p*=2;
	}
	p=p/2;
	return p;
    }

    int  nextPowerOf2(int n){
	int p = 1;
	while(p<n){
        p*=2;
	}
	return p;
    }



    template<typename T>
    DynMatrix<T>&  logpowerspectrum(const DynMatrix<std::complex<T> > &src,
                                    DynMatrix<T> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	std::complex<T> temp = static_cast<T>(0.0);
	const std::complex<T>* srcdata = src.data();
	T* dstdata = dst.data();
      T epsilon = std::numeric_limits<T>::min(); // 1.17549e-38 in order to avoid log(0)
	for(unsigned int i=0;i<src.dim();++i){
        temp = srcdata[i];
        dstdata[i] = log2(epsilon + temp.real()*temp.real()+temp.imag()*temp.imag());
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32f>&  logpowerspectrum(const DynMatrix<icl32c > &src,
                                         DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  logpowerspectrum(const DynMatrix<std::complex<icl64f> > &src,
                                         DynMatrix<icl64f> &dst);

    template<typename T>
    DynMatrix<T>&  powerspectrum(const DynMatrix<std::complex<T> > &src,
                                 DynMatrix<T> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	const std::complex<T>* srcdata = src.data();
	std::complex<T> temp = static_cast<T>(0.0);
	T* dstdata = dst.data();
	for(unsigned int i=0;i<src.dim();++i){
        temp = srcdata[i];
        dstdata[i] = temp.real()*temp.real()+temp.imag()*temp.imag();
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32f>&  powerspectrum(const DynMatrix<icl32c > &src,
                                      DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  powerspectrum(const DynMatrix<std::complex<icl64f> > &src,
                                      DynMatrix<icl64f> &dst);

    template<typename T>
    DynMatrix<T>&  continueMatrixToPowerOf2(const DynMatrix<T> &src,
                                            DynMatrix<T> &dst){
	unsigned int newCols = nextPowerOf2(src.cols());
	unsigned int newRows = nextPowerOf2(src.rows());
	if(dst.cols() != newCols || dst.rows() != newRows){
        dst.setBounds(newCols,newRows);
	}
	unsigned int nc = src.cols();
	unsigned int nr = src.rows();
	for(unsigned int i=0;i<newRows;++i){
        for(unsigned int j=0;j<newCols;++j){
          dst.index_yx(i, j) = src.index_yx(i%nr, j%nc);
        }
	}
	return dst;
    }

    template ICLMath_API
    DynMatrix<icl8u>&  continueMatrixToPowerOf2(const DynMatrix<icl8u> &src,
                                                DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  continueMatrixToPowerOf2(const DynMatrix<icl16u> &src,
                                                 DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  continueMatrixToPowerOf2(const DynMatrix<icl32u> &src,
                                                 DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  continueMatrixToPowerOf2(const DynMatrix<icl16s> &src,
                                                 DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  continueMatrixToPowerOf2(const DynMatrix<icl32s> &src,
                                                 DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  continueMatrixToPowerOf2(const DynMatrix<icl32f> &src,
                                                 DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl32c >&  continueMatrixToPowerOf2(
                                                  const DynMatrix<icl32c > &src,
                                                  DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  continueMatrixToPowerOf2(const DynMatrix<icl64f> &,
                                                 DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&
    continueMatrixToPowerOf2(const DynMatrix<std::complex<icl64f> > &src,
                             DynMatrix<std::complex<icl64f> > &dst);

    template<typename T>
    DynMatrix<T>  &mirrorOnCenter(const DynMatrix<T> &src,
                                  DynMatrix<T> &dst){
	unsigned int srcCols = src.cols();
	unsigned int srcRows = src.rows();
	unsigned int newCols = nextPowerOf2(srcCols);
	unsigned int newRows = nextPowerOf2(srcRows);
	if(newCols==srcCols && newRows==srcRows){
        dst = src;
        return dst;
	}
	if(dst.cols() != newCols || dst.rows() != newRows){
        dst.setBounds(newCols,newRows);
	}
	unsigned int nc = (newCols-srcCols)/2;
	unsigned int nr = (newRows-srcRows)/2;
	for(unsigned int i=0;i<srcRows;++i){
        for(unsigned int j=0;j<srcCols;++j){
          dst.index_yx(i+nr, j+nc) = src.index_yx(i, j);
        }
	}
	for(unsigned int i=0;i<newRows;++i){
        for(unsigned int j=0;j<newCols;++j){
          dst.index_yx(i, j) = src.index_yx((srcRows-(nr%srcRows)+i)%srcRows, (srcCols-(nc%srcCols)+j)%srcCols);
        }
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>  &mirrorOnCenter(const DynMatrix<icl8u> &src,
                                      DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u> &mirrorOnCenter(const DynMatrix<icl16u> &src,
                                      DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u> &mirrorOnCenter(const DynMatrix<icl32u> &src,
                                      DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s> &mirrorOnCenter(const DynMatrix<icl16s> &src,
                                      DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s> &mirrorOnCenter(const DynMatrix<icl32s> &src,
                                      DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f> &mirrorOnCenter(const DynMatrix<icl32f> &src,
                                      DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl32c > &mirrorOnCenter(
                                       const DynMatrix<icl32c > &src,
                                       DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl64f> &mirrorOnCenter(const DynMatrix<icl64f> &src,
                                      DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &mirrorOnCenter(
                                                     const DynMatrix<std::complex<icl64f> > &src,
                                                     DynMatrix<std::complex<icl64f> > &dst);

    template<typename T>
    DynMatrix<T> & makeborder(const DynMatrix<T> &src,
                              DynMatrix<T> &dst, T borderFill){
	unsigned int srcCols = src.cols();
	unsigned int srcRows = src.rows();
	unsigned int newCols = nextPowerOf2(srcCols);
	unsigned int newRows = nextPowerOf2(srcRows);
	if(dst.cols() != newCols || dst.rows() != newRows){
        dst.setBounds(newCols,newRows);
	}
	unsigned int nc = (newCols-srcCols)/2;
	unsigned int nr = (newRows-srcRows)/2;
	std::fill(dst.begin(),dst.end(),borderFill);
	for(unsigned int i=0;i<srcRows;++i){
        for(unsigned int j=0;j<srcCols;++j){
          dst.index_yx(i+nr, j+nc) = src.index_yx(i, j);
        }
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  makeborder(const DynMatrix<icl8u> &src,
                                  DynMatrix<icl8u> &dst, icl8u borderFill);
    template ICLMath_API
    DynMatrix<icl16u>&  makeborder(const DynMatrix<icl16u> &src,
                                   DynMatrix<icl16u> &dst, icl16u borderFill);
    template ICLMath_API
    DynMatrix<icl32u>&  makeborder(const DynMatrix<icl32u> &src,
                                   DynMatrix<icl32u> &dst, icl32u borderFill);
    template ICLMath_API
    DynMatrix<icl16s>&  makeborder(const DynMatrix<icl16s> &src,
                                   DynMatrix<icl16s> &dst, icl16s borderFill);
    template ICLMath_API
    DynMatrix<icl32s>&  makeborder(const DynMatrix<icl32s> &src,
                                   DynMatrix<icl32s> &dst, icl32s borderFill);
    template ICLMath_API
    DynMatrix<icl32f>&  makeborder(const DynMatrix<icl32f> &src,
                                   DynMatrix<icl32f> &dst,icl32f borderFill);
    template ICLMath_API
    DynMatrix<icl32c >&  makeborder(
                                    const DynMatrix<icl32c > &src,
                                    DynMatrix<icl32c > &dst,icl32c borderFill);
    template ICLMath_API
    DynMatrix<icl64f>&  makeborder(const DynMatrix<icl64f> &src,
                                   DynMatrix<icl64f> &dst,icl64f borderFill);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  makeborder(
                                                  const DynMatrix<std::complex<icl64f> > &src,
                                                  DynMatrix<std::complex<icl64f> > &dst,std::complex<icl64f> borderFill);

    template<typename T1,typename T2>
    DynMatrix<T2> &imagpart(const DynMatrix<std::complex<T1> > &src,
                            DynMatrix<T2> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	const std::complex<T1> *srcdata = src.data();
	T2 *dstdata = dst.data();
	for(unsigned int i=0;i<dst.dim();++i){
        dstdata[i] = (T2)srcdata[i].imag();
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u> &imagpart(const DynMatrix<icl32c > &src,
                               DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u> &imagpart(const DynMatrix<icl32c > &src,
                                DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u> &imagpart(const DynMatrix<icl32c > &src,
                                DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl32f> &imagpart(const DynMatrix<icl32c > &src,
                                DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f> &imagpart(const DynMatrix<icl32c > &src,
                                DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<icl8u> &imagpart(const DynMatrix<std::complex<icl64f> > &src,
                               DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u> &imagpart(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u> &imagpart(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl32f> &imagpart(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f> &imagpart(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl64f> &dst);

    template<typename T1,typename T2>
    DynMatrix<T2> &realpart(const DynMatrix<std::complex<T1> > &src,
                                 DynMatrix<T2> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	const std::complex<T1> *srcdata = src.data();
	T2 *dstdata = dst.data();
	for(unsigned int i=0;i<dst.dim();++i){
        dstdata[i] = (T2)srcdata[i].real();
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  realpart(const DynMatrix<icl32c > &src,
                                DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  realpart(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<icl8u>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  realpart(const DynMatrix<std::complex<icl64f> > &src,
                                 DynMatrix<icl64f> &dst);

    template<typename T>
    void split_complex(const DynMatrix<std::complex<T> > &src,
                       DynMatrix<T> &real,DynMatrix<T> &img){
	if(real.cols() != src.cols() || real.rows() != src.rows()){
        real.setBounds(src.cols(),src.rows());
	}
	if(img.cols() != src.cols() || img.rows() != src.rows()){
        img.setBounds(src.cols(),src.rows());
	}
	const std::complex<T> *srcdata = src.data();
	T *realdata = real.data();
	T *imagdata = img.data();
	for(unsigned int i=0;i<src.dim();++i){
        realdata[i] = static_cast<T>(srcdata[i].real());
        imagdata[i] = static_cast<T>(srcdata[i].imag());
	}
    }
    template ICLMath_API void
    split_complex(const DynMatrix<icl32c > &src,DynMatrix<icl32f> &real,
                  DynMatrix<icl32f> &img);
    template ICLMath_API void
    split_complex(const DynMatrix<std::complex<icl64f> > &src,DynMatrix<icl64f> &real,
                  DynMatrix<icl64f> &img);

    template<typename T1,typename T2>
    DynMatrix<T2>&  magnitude(const DynMatrix<std::complex<T1> > &src,
                              DynMatrix<T2> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	const std::complex<T1> *srcdata = src.data();
	T2 *dstdata = dst.data();
	for(unsigned int i=0;i<dst.dim();++i){
        dstdata[i] = (T2)(std::sqrt(srcdata[i].real()*srcdata[i].real()
                                    +srcdata[i].imag()*srcdata[i].imag()));
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  magnitude(const DynMatrix<icl32c > &src,
                                 DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  magnitude(const DynMatrix<icl32c > &src,
                                  DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  magnitude(const DynMatrix<icl32c > &src,
                                  DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  magnitude(const DynMatrix<icl32c > &src,
                                  DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  magnitude(const DynMatrix<icl32c > &src,
                                  DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  magnitude(const DynMatrix<icl32c > &src,
                                       DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  magnitude(const DynMatrix<icl32c > &src,
                                       DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<icl8u>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                      DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  magnitude(const DynMatrix<std::complex<icl64f> > &src,
                                       DynMatrix<icl64f> &dst);

    template<typename T1,typename T2>
    DynMatrix<T2>&  phase(const DynMatrix<std::complex<T1> > &src,
                               DynMatrix<T2> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
        dst.setBounds(src.cols(),src.rows());
	}
	T1 im = (T1)0;
	T1 re = (T1)0;
	const std::complex<T1> *srcdata = src.data();
	T2 *dstdata = dst.data();
	for(unsigned int i=0;i<src.rows();++i){
        im = srcdata[i].imag();
        re = srcdata[i].real();
        T1 zero = (T1) 0;
        if(re==zero && im==zero){
          //unbestimmt
          dstdata[i] = (T2) 0;
        } else if(re<zero && im>=zero){
          dstdata[i] = (T2) atan2(im,re)+FFT_PI;
        } else if(re<zero && im<zero){
          dstdata[i] = (T2) atan2(im,re)-FFT_PI;
        } else if(re==zero && im>zero){
          dstdata[i] = (T2) FFT_PI_HALF;
        } else if(re==zero && im<zero){
          dstdata[i] = (T2) -FFT_PI_HALF;
        } else {
          //real==0 && im whaterver else
          dstdata[i] = (T2) std::atan2(im,re);
        }
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl8u>&  phase(const DynMatrix<icl32c > &src,
                                  DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  phase(const DynMatrix<icl32c > &src,
                                   DynMatrix<icl64f> &dst);
    template ICLMath_API
    DynMatrix<icl8u>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                  DynMatrix<icl8u> &dst);
    template ICLMath_API
    DynMatrix<icl16u>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl16u> &dst);
    template ICLMath_API
    DynMatrix<icl32u>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl32u> &dst);
    template ICLMath_API
    DynMatrix<icl16s>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl16s> &dst);
    template ICLMath_API
    DynMatrix<icl32s>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl32s> &dst);
    template ICLMath_API
    DynMatrix<icl32f>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl32f> &dst);
    template ICLMath_API
    DynMatrix<icl64f>&  phase(const DynMatrix<std::complex<icl64f> > &src,
                                   DynMatrix<icl64f> &dst);

    template<typename T>
    void  split_magnitude_phase(const DynMatrix<std::complex<T> > &src,
                                DynMatrix<T> &magnitude,DynMatrix<T> &phase){
	if(magnitude.cols() != src.cols() || magnitude.rows() != src.rows()){
        magnitude.setBounds(src.cols(),src.rows());
	}
	if(phase.cols() != src.cols() || phase.rows() != src.rows()){
        phase.setBounds(src.cols(),src.rows());
	}
	T im;
	T re;
	T zero = static_cast<T>(0);
	const std::complex<T> *srcdata = src.data();
	T *magnitudedata = magnitude.data();
	T *phasedata = phase.data();
	for(unsigned int i=0;i<src.dim();++i){
        im = srcdata[i].imag();
        re = srcdata[i].real();
        magnitudedata[i] = static_cast<T>(std::sqrt(re*re+im*im));
        if(re==zero && im==zero){
          //unbestimmt
          phasedata[i] = zero;
        } else if(re<zero && im>=zero){
          phasedata[i] = static_cast<T>(atan2(im,re))+FFT_PI;
        } else if(re<zero && im<zero){
          phasedata[i] = static_cast<T>(atan2(im,re))-FFT_PI;
        } else if(re==zero && im>zero){
          phasedata[i] = static_cast<T>(FFT_PI_HALF);
        } else if(re==zero && im<zero){
          phasedata[i] = static_cast<T>(-FFT_PI_HALF);
        } else {
          //real==0 && im whaterver else
          phasedata[i] = static_cast<T>(std::atan2(im,re));
        }
	}
    }
    template ICLMath_API
    void  split_magnitude_phase(const DynMatrix<icl32c > &src,
                                DynMatrix<icl32f> &magnitude, DynMatrix<icl32f> &phase);
    template ICLMath_API
    void  split_magnitude_phase(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl64f> &magnitude, DynMatrix<icl64f> &phase);

    template<typename T1,typename T2>
    DynMatrix<std::complex<T2> > &joinComplex(const DynMatrix<T1> &real,
                                                   const DynMatrix<T1> &im, DynMatrix<std::complex<T2> > &dst){
	if(real.cols() != im.cols() || real.rows() != im.rows()){
        throw FFTException("param1 1 and param 2 in icl::fft::joinComplex have not the same size");
	}
	if(dst.cols() != real.cols() || dst.rows() != real.rows()){
        dst.setBounds(real.cols(),real.rows());
	}
	const T1 *realdata = real.data();
	const T1 *imdata = im.data();
	std::complex<T2> *dstdata = dst.data();
	for(unsigned int i=0;i<dst.dim();++i){
        dstdata[i] = std::complex<T2>((T2)realdata[i],(T2)imdata[i]);
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl8u> &real,
                                                       const DynMatrix<icl8u> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl16u> &real,
                                                       const DynMatrix<icl16u> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl32u> &real,
                                                       const DynMatrix<icl32u> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl16s> &real,
                                                       const DynMatrix<icl16s> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl32s> &real,
                                                       const DynMatrix<icl32s> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl32f> &real,
                                                       const DynMatrix<icl32f> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<icl32c > &joinComplex(const DynMatrix<icl64f> &real,
                                                       const DynMatrix<icl64f> &im,DynMatrix<icl32c > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl8u> &real,
                                                       const DynMatrix<icl8u> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl16u> &real,
                                                       const DynMatrix<icl16u> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl32u> &real,
                                                       const DynMatrix<icl32u> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl16s> &real,
                                                       const DynMatrix<icl16s> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl32s> &real,
                                                       const DynMatrix<icl32s> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl32f> &real,
                                                       const DynMatrix<icl32f> &im,DynMatrix<std::complex<icl64f> > &dst);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> > &joinComplex(const DynMatrix<icl64f> &real,
                                                       const DynMatrix<icl64f> &im,DynMatrix<std::complex<icl64f> > &dst);

    template<typename T1,typename T2>
    std::complex<T2>*  fft(unsigned int n, const T1* a){
	if(n==1){
        std::complex<T2> *cpya = new std::complex<T2>[1];
        cpya[0] = (std::complex<T2>)a[0];
        return cpya;
	} else if(n%2==0){
        unsigned int halfsize = n/2;
        T1 *even = new T1[halfsize];
        T1 *odd = new T1[halfsize];
        //unsigned int i=0;
        unsigned int j=0;
        //split array
        for(unsigned int i=0;i<halfsize;++i){
          j=2*i;
          even[i] = a[j];
          odd[i] = a[j+1];
        }
        std::complex<T2>* g = fft<T1,T2>(halfsize,even);
        std::complex<T2>* u = fft<T1,T2>(halfsize,odd);
        std::complex<T2>* c = new std::complex<T2>[n];
        double cp = -(FFT_2_PI)/n;
        std::complex<T2> fac(0.0,0.0);
        for(unsigned int k=0;k<halfsize;++k){
          double f=cp*k;
          fac = u[k]*std::complex<T2>(std::cos(f),std::sin(f));
          c[k]=g[k]+fac;
          c[k+halfsize]=g[k]-fac;
        }
        delete[] even;
        delete[] odd;
        delete[] g;
        delete[] u;
        return c;
	} else {
        T2 omega = -FFT_2_PI/n;
        std::complex<T2> *m3 = new std::complex<T2>[n];
        std::complex<T2> x((T2)0.0,(T2)0.0);
        T2 y = (T2) 0;
        for(unsigned int p =0;p<n;++p){
          x = std::complex<T2>(0.0,0.0);
          y=p*omega;
          for(unsigned int q = 0;q<n;++q){
            x=x+(std::complex<T2>(std::cos(y*q),std::sin(y*q))*((std::complex<T2>)a[q]));
          }
          m3[p]=x;
        }
        return m3;
	}
    }
    template ICLMath_API icl32c*  fft(unsigned int n, const icl8u* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl16u* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl32u* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl16s* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl32s* a);

    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl8u* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl16u* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl32u* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl16s* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl32s* a);

    template ICLMath_API icl32c*  fft(unsigned int n, const icl32f* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl32f* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl64f* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl64f* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const icl32c* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const icl32c* a);
    template ICLMath_API std::complex<icl64f>*  fft(unsigned int n, const std::complex<icl64f>* a);
    template ICLMath_API icl32c*  fft(unsigned int n, const std::complex<icl64f>* a);


    template<typename T1, typename T2>
    DynMatrix<std::complex<T2> >& fft2D_cpp(const DynMatrix<T1> &src,DynMatrix<std::complex<T2> > &dst,
                                                 DynMatrix<std::complex<T2> > &buf){
	FFT_DEBUG("fft2D_cpp");
	//check buffer
	if(buf.isNull() || buf.cols() != src.rows() || buf.rows() != src.cols()){
        buf.setBounds(src.rows(),src.cols());
	}
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	std::complex<T2> *temp=0;
	//already transposed
	for(unsigned int i=0;i<rows;++i){
        temp = fft<T1,T2>(cols,src.row_begin(i));
        for(unsigned int j=0;j<cols;++j){
          buf.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}

	for(unsigned int i=0;i<cols;++i){
        temp = fft<std::complex<T2>,T2 >(rows,buf.row_begin(i));
        for(unsigned int j=0;j<rows;++j){
          dst.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl8u> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl16u> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl16s> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl32u> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl32s> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const DynMatrix<icl8u> &src,
                                                      DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const DynMatrix<icl16u> &src,
                                                      DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const DynMatrix<icl16s> &src,
                                                      DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const DynMatrix<icl32u> &src,
                                                      DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const DynMatrix<icl32s> &src,
                                                      DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  fft2D_cpp(const DynMatrix<icl32f> &src,
                                                      DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D_cpp(const DynMatrix<icl32f> &src,
                                                     DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D_cpp(const DynMatrix<icl64f> &src,
                                                     DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D_cpp(const DynMatrix<icl64f> &src,
                                                     DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D_cpp(const DynMatrix<icl32c > &src,
                                                     DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D_cpp(const DynMatrix<icl32c > &src,
                                                     DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D_cpp(const DynMatrix<std::complex<icl64f> > &src,
                                                     DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D_cpp(const DynMatrix<std::complex<icl64f> > &src,
                                                     DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);

    // Helper: is T a std::complex type?
    template<class T> struct is_complex_t : std::false_type {};
    template<class T> struct is_complex_t<std::complex<T>> : std::true_type {};

    template<typename T1, typename T2>
    DynMatrix<std::complex<T2> >& fft2D(const DynMatrix<T1> &src,
                                         DynMatrix<std::complex<T2> > &dst, DynMatrix<std::complex<T2> > &buf){
      unsigned int rows = src.rows();
      unsigned int cols = src.cols();
      if(dst.isNull() || dst.cols() != cols || dst.rows() != rows) {
        dst.setBounds(cols, rows);
      }

      auto& ops = FFTOps<T2>::instance();

      if constexpr (is_complex_t<T1>::value) {
        // Complex-to-complex forward FFT
        using IS = typename T1::value_type;
        auto* impl = ops.template getSelector<typename FFTOps<T2>::C2CSig>(FFTOp::c2c).resolveOrThrow();
        if constexpr (std::is_same_v<IS, T2>) {
          impl->apply(reinterpret_cast<const std::complex<T2>*>(src.data()), rows, cols, dst.data());
        } else {
          std::vector<std::complex<T2>> conv(rows * cols);
          const T1* s = src.data();
          for(unsigned int i = 0; i < rows * cols; ++i) conv[i] = std::complex<T2>(T2(s[i].real()), T2(s[i].imag()));
          impl->apply(conv.data(), rows, cols, dst.data());
        }
      } else {
        // Real-to-complex forward FFT
        auto* impl = ops.template getSelector<typename FFTOps<T2>::R2CSig>(FFTOp::r2c).resolveOrThrow();
        if constexpr (std::is_same_v<T1, T2>) {
          impl->apply(src.data(), rows, cols, dst.data());
        } else {
          std::vector<T2> conv(rows * cols);
          const T1* s = src.data();
          for(unsigned int i = 0; i < rows * cols; ++i) conv[i] = T2(s[i]);
          impl->apply(conv.data(), rows, cols, dst.data());
        }
      }
      return dst;
    }
    // icl32f output
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl8u> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl16u> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl32u> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl16s> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl32s> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl32f> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl32c > &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<icl64f> &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& fft2D(const DynMatrix<std::complex<icl64f> > &src,
                               DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    // icl64f output
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl8u> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl16u> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl32u> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl16s> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl32s> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl32f> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl64f> &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<icl32c > &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& fft2D(const DynMatrix<std::complex<icl64f> > &src,
                                             DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);

    template<typename T1, typename T2>
    std::complex<T2>*  dft(unsigned int n, T1* src){
	std::complex<T2>* d = new std::complex<T2>[n];
	std::complex<T2> x = std::complex<T2>(0.0,0.0);
	T2 f= -FFT_2_PI/n;
	T2 g = 0.0;
	T2 h = 0.0;
	for(unsigned int i=0;i<n;++i){
        h = f*i;
        for(unsigned int j=0;j<n;++j){
          g=h*j;
          x = x+(CreateComplex<T1,T2>::create_complex((src[j])) * (std::complex<T2>(std::cos(g),std::sin(g))));
        }
        d[i] = x;
        x = std::complex<T2>(0.0,0.0);
	}
	return d;
    }

    template ICLMath_API icl32c*  dft(unsigned int n, icl8u* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl8u* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl16u* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl16u* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl32u* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl32u* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl16s* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl16s* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl32s* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl32s* src);

    template ICLMath_API icl32c*  dft(unsigned int n, icl32f* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl32f* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl64f* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl64f* src);
    template ICLMath_API icl32c*  dft(unsigned int n, icl32c* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, icl32c* src);
    template ICLMath_API std::complex<icl64f>*  dft(unsigned int n, std::complex<icl64f>* src);
    template ICLMath_API icl32c*  dft(unsigned int n, std::complex<icl64f>* src);

    template<typename T1,typename T2>
    DynMatrix<std::complex<T2> >&  dft2D(DynMatrix<T1> &src,
                                              DynMatrix<std::complex<T2> >&dst, DynMatrix<std::complex<T2> >&buf){
	std::complex<T2> *temp=0;
	for(unsigned int i=0;i<src.rows();++i){
        temp = dft<T1,T2>(src.cols(),(src.row(i)).data());
        //already transposed
        for(unsigned int j=0;j<src.cols();++j){
          buf.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	for(unsigned int i=0;i<buf.rows();++i){
        temp = dft<std::complex<T2>,T2>(buf.cols(),(buf.row(i)).data());
        for(unsigned int j=0;j<buf.cols();++j){
          dst.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl8u>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl8u>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl16u>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl16u>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl16s>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl16s>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl32u>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl32u>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl32s>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl32s>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl32f>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl32f>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl64f>& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl64f>& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<icl32c >& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<std::complex<icl64f> >& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  dft2D(DynMatrix<icl32c >& src,
                                                  DynMatrix<std::complex<icl64f> >& dst, DynMatrix<std::complex<icl64f> >& buf);
    template ICLMath_API
    DynMatrix<icl32c >&  dft2D(DynMatrix<std::complex<icl64f> >& src,
                                                  DynMatrix<icl32c >& dst, DynMatrix<icl32c >& buf);

    template<typename T1,typename T2>
    static std::complex<T2>*  ifft_(unsigned int n, const T1* a){
	if(n==1){
        std::complex<T2> *cpya = new std::complex<T2>[1];
        cpya[0] = CreateComplex<T1,T2>::create_complex(*a);
        return cpya;
	} else if (n%2==0){
        unsigned int halfsize = n/2;
        T1* even = new T1[halfsize];
        T1* odd = new T1[halfsize];
        unsigned int i;
        //split array
        for(i=0;i<halfsize;++i){
          even[i] = a[2*i];
          odd[i] = a[2*i+1];
        }
        std::complex<T2>* g = ifft_<T1,T2>(halfsize,even);
        std::complex<T2>* u = ifft_<T1,T2>(halfsize,odd);

        std::complex<T2>* c = new std::complex<T2>[n];
        T2 cp = FFT_2_PI/n;
        std::complex<T2> fac(0.0,0.0);
        for(unsigned int k=0;k<halfsize;++k){
          double e=cp*k;
          fac = u[k]*std::complex<T2>(std::cos(e),std::sin(e));
          c[k]=g[k]+fac;
          c[k+halfsize]=g[k]-fac;
        }
        delete[] even;
        delete[] odd;
        delete[] g;
        delete[] u;
        return c;
	} else {
        T2 omega = FFT_2_PI/n;
        std::complex<T2> *m3 = new std::complex<T2>[n];
        std::complex<T2> x(0.0,0.0);
        T2 y = (T2)0;
        for(unsigned int p =0;p<n;++p){
          x = std::complex<T2>(0.0,0.0);
          y=p*omega;
          for(unsigned int q = 0;q<n;++q){
            //x=x+(exp(std::complex<T2>(0.0,y*q))*(CreateComplex<T1,T2>::create_complex(a[q])));
            x=x+(std::complex<T2>(std::cos(y*q),std::sin(y*q))*(CreateComplex<T1,T2>::create_complex(a[q])));
          }
          m3[p]=x;
        }
        return m3;
	}
    }
    template icl32c*  ifft_(unsigned int n, const icl8u* a);
    template icl32c*  ifft_(unsigned int n, const icl16u* a);
    template icl32c*  ifft_(unsigned int n, const icl32u* a);
    template icl32c*  ifft_(unsigned int n, const icl16s* a);
    template icl32c*  ifft_(unsigned int n, const icl32s* a);
    template icl32c*  ifft_(unsigned int n, const icl32f* a);
    template icl32c*  ifft_(unsigned int n, const icl64f* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl8u* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl16u* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl32u* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl16s* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl32s* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl32f* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl64f* a);

    template icl32c*  ifft_(unsigned int n, const icl32c* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const icl32c* a);
    template std::complex<icl64f>*  ifft_(unsigned int n, const std::complex<icl64f>* a);
    template icl32c*  ifft_(unsigned int n, const std::complex<icl64f>* a);

    template<typename T1, typename T2>
    std::complex<T2>*  ifft_cpp(unsigned int n, const T1* a){
	std::complex<T2>* tempMat = ifft_<T1,T2>(n,a);
	double lambda = 1.0/n;
	for(unsigned int index = 0;index<n;++index){
        tempMat[index] *= lambda;
	}
	return tempMat;
    }

    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl8u* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl16u* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl32u* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl16s* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl32s* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl32f* a);

    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl8u* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl16u* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32u* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl16s* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32s* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32f* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl64f* a);

    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const icl32c* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32c* a);
    template ICLMath_API std::complex<icl64f>*  ifft_cpp(unsigned int n, const std::complex<icl64f>* a);
    template ICLMath_API icl32c*  ifft_cpp(unsigned int n, const std::complex<icl64f>* a);

// IPP IFFT wrappers removed — ippiFFTInitAlloc_* dropped from modern IPP.
// MKL IFFT wrappers removed — dispatch now goes through FFTOps<T>.

    template<typename T1, typename T2>
    DynMatrix<std::complex<T2> >&   ifft2D_cpp(const DynMatrix<T1> &src,DynMatrix<std::complex<T2> > &dst,DynMatrix<std::complex<T2> > &buf){
	if(buf.isNull() || buf.cols() != src.rows() || buf.rows() != src.cols()){
        buf.setBounds(src.rows(),src.cols());
	}
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	std::complex<T2> *temp=0;
	//already transposed
	for(unsigned int i=0;i<rows;++i){
        temp = ifft_cpp<T1,T2>(cols,src.row_begin(i));
        for(unsigned int j=0;j<cols;++j){
          buf.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	for(unsigned int i=0;i<buf.rows();++i){
        temp = ifft_cpp<std::complex<T2>,T2>(buf.cols(),buf.row_begin(i));
        for(unsigned int j=0;j<buf.cols();++j){
          dst.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl8u> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl16u> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl32u> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl16s> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl32s> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl8u> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl16u> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl32u> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl16s> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl32s> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl32f> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl64f> &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl32f> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl64f> &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<icl32c > &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<icl32c > &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  ifft2D_cpp(const DynMatrix<std::complex<icl64f> > &src,
                                                       DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const DynMatrix<std::complex<icl64f> > &src,
                                                       DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);

    template<typename T1, typename T2>
    DynMatrix<std::complex<T2> >& ifft2D(const DynMatrix<T1> &src,
                                          DynMatrix<std::complex<T2> > &dst, DynMatrix<std::complex<T2> > &buf){
      unsigned int rows = src.rows();
      unsigned int cols = src.cols();
      if(dst.isNull() || dst.cols() != cols || dst.rows() != rows) {
        dst.setBounds(cols, rows);
      }

      auto& ops = FFTOps<T2>::instance();
      auto* impl = ops.template getSelector<typename FFTOps<T2>::InvC2CSig>(FFTOp::inv_c2c).resolveOrThrow();

      if constexpr (is_complex_t<T1>::value) {
        using IS = typename T1::value_type;
        if constexpr (std::is_same_v<IS, T2>) {
          impl->apply(reinterpret_cast<const std::complex<T2>*>(src.data()), rows, cols, dst.data());
        } else {
          std::vector<std::complex<T2>> conv(rows * cols);
          const T1* s = src.data();
          for(unsigned int i = 0; i < rows * cols; ++i) conv[i] = std::complex<T2>(T2(s[i].real()), T2(s[i].imag()));
          impl->apply(conv.data(), rows, cols, dst.data());
        }
      } else {
        // Real input → convert to complex<T2>, then inverse FFT
        std::vector<std::complex<T2>> conv(rows * cols);
        const T1* s = src.data();
        for(unsigned int i = 0; i < rows * cols; ++i) conv[i] = std::complex<T2>(T2(s[i]), T2(0));
        impl->apply(conv.data(), rows, cols, dst.data());
      }
      return dst;
    }
    // icl32f output
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl8u> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl16u> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl32u> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl16s> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl32s> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl32f> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl64f> &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<icl32c > &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >& ifft2D(const DynMatrix<std::complex<icl64f> > &src,
                                DynMatrix<icl32c > &dst, DynMatrix<icl32c > &buf);
    // icl64f output
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl8u> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl16u> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl32u> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl16s> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl32s> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl32f> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl64f> &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<icl32c > &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >& ifft2D(const DynMatrix<std::complex<icl64f> > &src,
                                              DynMatrix<std::complex<icl64f> > &dst, DynMatrix<std::complex<icl64f> > &buf);

    template<typename T1,typename T2>
    std::complex<T2>*  idft(unsigned int n, T1* src){
	std::complex<T2> *d = new std::complex<T2>[n];
	std::complex<T2> x= std::complex<T2>(0,0);
	T2 f=FFT_2_PI/n;
	T2 g = 0.0;
	T2 h = 1.0/n;
	T2 k = 0.0;
	for(unsigned int i=0;i<n;++i){
        k=f*((T2)i);
        for(unsigned int j=0;j<n;++j){
          g=k*((T2)j);
          x += ((std::complex<T2>)src[j])*std::complex<T2>(std::cos(g),std::sin(g));
        }
        d[i] = x*h;
        g = 0.0;
        x = std::complex<T2>((T2)0,(T2)0);
	}
	return d;
    }

    template ICLMath_API icl32c*  idft(unsigned int n, icl8u* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl8u* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl16u* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl16u* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl16s* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl16s* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl32u* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl32u* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl32s* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl32s* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl32f* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl32f* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl64f* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl64f* src);
    template ICLMath_API icl32c*  idft(unsigned int n, icl32c* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, icl32c* src);
    template ICLMath_API std::complex<icl64f>*  idft(unsigned int n, std::complex<icl64f>* src);
    template ICLMath_API icl32c*  idft(unsigned int n, std::complex<icl64f>* src);

    template<typename T1, typename T2>
    DynMatrix<std::complex<T2> >&   idft2D(DynMatrix<T1>& src,DynMatrix<std::complex<T2> > &dst,DynMatrix<std::complex<T2> > &buf){
	std::complex<T2> *temp = 0;
	for(unsigned int i=0;i<src.rows();++i){
        temp = idft<T1,T2>(src.cols(),(src.row(i)).data());
        //already transposed
        for(unsigned int j=0;j<src.cols();++j){
          buf.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	for(unsigned int i=0;i<buf.cols();++i){
        temp =  idft<std::complex<T2>,T2>(buf.rows(),(buf.row(i)).data());
        for(unsigned int j=0;j<buf.rows();++j){
          dst.index_yx(j, i)=temp[j];
        }
        delete[] temp;
	}
	return dst;
    }
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl8u>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl8u>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl16u>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl16u>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl32u>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl32u>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl16s>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl16s>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl32s>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl32s>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl32f>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl32f>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl64f>& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl64f>& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<icl32c >& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<icl32c >& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<std::complex<icl64f> >&  idft2D(DynMatrix<std::complex<icl64f> >& src,
                                                   DynMatrix<std::complex<icl64f> > &dst,DynMatrix<std::complex<icl64f> > &buf);
    template ICLMath_API
    DynMatrix<icl32c >&  idft2D(DynMatrix<std::complex<icl64f> >& src,
                                                   DynMatrix<icl32c > &dst,DynMatrix<icl32c > &buf);
  } // namespace fft
  } // namespace icl::math
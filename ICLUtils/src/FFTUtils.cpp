#include <ICLUtils/FFTUtils.h>
#include <limits>
//#define FFT_DEBUG(X) std::cout << X << std::endl;
#define FFT_DEBUG(X)
namespace icl{
namespace fft{


template<class T1, class T2>
struct CreateComplex{
	static inline std::complex<T2> create_complex(const T1 &x){
		return std::complex<T2>(x);
	}
};

#define ICL_INSTANTIATE_DEPTH(D)                                        \
		template<class T2>                                                      \
		struct CreateComplex<icl##D,T2>{ \
	static inline std::complex<T2> create_complex(const icl##D &x){ \
			return std::complex<T2>((T2)x,0.0); \
		} \
}; \
template<class T2> \
struct CreateComplex<std::complex<icl##D>,T2>{ \
	static inline std::complex<T2> create_complex(const std::complex<icl##D> &x){ \
	return std::complex<T2>((T2)x.real(),(T2)x.imag()); \
} \
};
ICL_INSTANTIATE_ALL_DEPTHS;
ICL_INSTANTIATE_DEPTH(32u);
ICL_INSTANTIATE_DEPTH(16u);
#undef ICL_INSTANTIATE_DEPTH

template<typename T>
icl::DynMatrix<T>&  fftshift(icl::DynMatrix<T> &src,icl::DynMatrix<T> &dst) throw (InvalidMatrixDimensionException){
	if(src.cols() != dst.cols() || src.rows() != dst.rows())
		throw InvalidMatrixDimensionException("number of columns(rows) of sourcematrix must be equal to number of colums(rows) of destinationmatrix");
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	unsigned int cols2 = cols/2;
	unsigned int rows2 = rows/2;
	if(cols >1 && rows > 1){
		for(unsigned int y=0;y<rows;++y){
			for(unsigned int x=0;x<cols;++x){
				dst.operator ()( ((x+cols2))%cols,((y+rows2))%rows) = src.operator ()(x,y);
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
template
icl::DynMatrix<icl8u>&  fftshift(icl::DynMatrix<icl8u> &src,icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  fftshift(icl::DynMatrix<icl16u> &src,icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl16s>&  fftshift(icl::DynMatrix<icl16s> &src,icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32u>&  fftshift(icl::DynMatrix<icl32u> &src,icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl32s>&  fftshift(icl::DynMatrix<icl32s> &src,icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  fftshift(icl::DynMatrix<icl32f> &src,icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<std::complex<icl32f> >&  fftshift(icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<icl64f>&  fftshift(icl::DynMatrix<icl64f> &src,icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<std::complex<icl64f> >&  fftshift(icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst);

template<typename T>
icl::DynMatrix<T>&  ifftshift(icl::DynMatrix<T> &src,icl::DynMatrix<T> &dst) throw (InvalidMatrixDimensionException){
	if(src.cols() != dst.cols() || src.rows() != dst.rows())
		throw InvalidMatrixDimensionException("number of columns(rows) of sourcematrix must be equal to number of colums(rows) of destinationmatrix");
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	unsigned int cols2 = cols/2;
	unsigned int rows2 = rows/2;
	if(cols >1 && rows > 1){
		for(unsigned int y=0;y<rows;++y){
			for(unsigned int x=0;x<cols;++x){
				dst.operator ()( ((x+cols2))%cols,((y+rows2))%rows) = src.operator ()(x,y);
			}
		}
		if(cols%2==0 && rows%2==0){
			//nothing to do
		} else  if(cols%2==1 && rows%2==0){
			for(unsigned int y=0;y<rows;++y){
				for(unsigned int x=cols-1;x>0;--x){
					std::swap(dst.operator ()(x,y),dst.operator ()(x-1,y));
				}
			}
		} else if(cols%2==0 && rows%2==1){
			for(unsigned int x=0;x<cols;++x){
				for(unsigned int y=rows-1;y>0;--y){
					std::swap(dst.operator ()(x,y),dst.operator ()(x,y-1));
				}
			}
		} else {
			for(unsigned int y=0;y<rows;++y){
				for(unsigned int x=cols-1;x>0;--x){
					std::swap(dst.operator ()(x,y),dst.operator ()(x-1,y));
				}
			}
			for(unsigned int x=0;x<cols;++x){
				for(unsigned int y=rows-1;y>0;--y){
					std::swap(dst.operator ()(x,y),dst.operator ()(x,y-1));
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
template
icl::DynMatrix<icl8u>&  ifftshift(icl::DynMatrix<icl8u> &src,icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  ifftshift(icl::DynMatrix<icl16u> &src,icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl16s>&  ifftshift(icl::DynMatrix<icl16s> &src,icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32u>&  ifftshift(icl::DynMatrix<icl32u> &src,icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl32s>&  ifftshift(icl::DynMatrix<icl32s> &src,icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  ifftshift(icl::DynMatrix<icl32f> &src,icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<std::complex<icl32f> >&  ifftshift(icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<icl64f>&  ifftshift(icl::DynMatrix<icl64f> &src,icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<std::complex<icl64f> >&  ifftshift(icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst);

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

static bool  isPowerOfTwo(int n){
	int p = 1;
	while(p<n){
		p*=2;
	}
	if(p==n){
		return true;
	} else {
		return false;
	}
}

template<typename T>
icl::DynMatrix<T>&  logpowerspectrum(const icl::DynMatrix<std::complex<T> > &src,
		icl::DynMatrix<T> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
		dst.setBounds(src.cols(),src.rows());
	}
	std::complex<T> temp = (T)0.0;
	const std::complex<T>* srcdata = src.data();
	T* dstdata = dst.data();
        T epsilon = std::numeric_limits<T>::min(); // 1.17549e-38 in order to avoid log(0)
	for(unsigned int i=0;i<src.dim();++i){
		temp = srcdata[i];
		dstdata[i] = log2(epsilon + temp.real()*temp.real()+temp.imag()*temp.imag());
	}
	return dst;
}
template
icl::DynMatrix<icl32f>&  logpowerspectrum(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  logpowerspectrum(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T>
icl::DynMatrix<T>&  powerspectrum(const icl::DynMatrix<std::complex<T> > &src,
		icl::DynMatrix<T> &dst){
	if(dst.cols() != src.cols() || dst.rows() != src.rows()){
		dst.setBounds(src.cols(),src.rows());
	}
	const std::complex<T>* srcdata = src.data();
	std::complex<T> temp = (T)0.0;
	T* dstdata = dst.data();
	for(unsigned int i=0;i<src.dim();++i){
		temp = srcdata[i];
		dstdata[i] = temp.real()*temp.real()+temp.imag()*temp.imag();
	}
	return dst;
}
template icl::
DynMatrix<icl32f>&  powerspectrum(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  powerspectrum(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T>
icl::DynMatrix<T>&  continueMatrixToPowerOf2(const icl::DynMatrix<T> &src,
		icl::DynMatrix<T> &dst){
	unsigned int newCols = nextPowerOf2(src.cols());
	unsigned int newRows = nextPowerOf2(src.rows());
	if(dst.cols() != newCols || dst.rows() != newRows){
		dst.setBounds(newCols,newRows);
	}
	unsigned int nc = src.cols();
	unsigned int nr = src.rows();
	for(unsigned int i=0;i<newRows;++i){
		for(unsigned int j=0;j<newCols;++j){
			dst.operator()(j,i) = src.operator()(j%nc,i%nr);
		}
	}
	return dst;
}

template
icl::DynMatrix<icl8u>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<std::complex<icl32f> >&  continueMatrixToPowerOf2(
		const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<icl64f>&  continueMatrixToPowerOf2(const icl::DynMatrix<icl64f> &,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<std::complex<icl64f> >&  continueMatrixToPowerOf2(
		const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst);

template<typename T>
icl::DynMatrix<T>  &mirrorOnCenter(const icl::DynMatrix<T> &src,
		icl::DynMatrix<T> &dst){
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
			dst.operator ()(j+nc,i+nr) = src.operator()(j,i);
		}
	}
	for(unsigned int i=0;i<newRows;++i){
		for(unsigned int j=0;j<newCols;++j){
			dst.operator ()(j,i) = src.operator()((srcCols-(nc%srcCols)+j)%srcCols,(srcRows-(nr%srcRows)+i)%srcRows);
		}
	}
	return dst;
}
template
icl::DynMatrix<icl8u>  &mirrorOnCenter(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u> &mirrorOnCenter(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u> &mirrorOnCenter(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s> &mirrorOnCenter(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s> &mirrorOnCenter(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f> &mirrorOnCenter(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<std::complex<icl32f> > &mirrorOnCenter(
		const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<icl64f> &mirrorOnCenter(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<std::complex<icl64f> > &mirrorOnCenter(
		const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst);

template<typename T>
icl::DynMatrix<T> & makeborder(const icl::DynMatrix<T> &src,
		icl::DynMatrix<T> &dst, T borderFill){
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
			dst.operator()(j+nc,i+nr) = src.operator()(j,i);
		}
	}
	return dst;
}
template
icl::DynMatrix<icl8u>&  makeborder(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<icl8u> &dst, icl8u borderFill);
template
icl::DynMatrix<icl16u>&  makeborder(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<icl16u> &dst, icl16u borderFill);
template
icl::DynMatrix<icl32u>&  makeborder(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<icl32u> &dst, icl32u borderFill);
template
icl::DynMatrix<icl16s>&  makeborder(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<icl16s> &dst, icl16s borderFill);
template
icl::DynMatrix<icl32s>&  makeborder(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<icl32s> &dst, icl32s borderFill);
template
icl::DynMatrix<icl32f>&  makeborder(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<icl32f> &dst,icl32f borderFill);
template
icl::DynMatrix<std::complex<icl32f> >&  makeborder(
		const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,std::complex<icl32f> borderFill);
template
icl::DynMatrix<icl64f>&  makeborder(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<icl64f> &dst,icl64f borderFill);
template
icl::DynMatrix<std::complex<icl64f> >&  makeborder(
		const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,std::complex<icl64f> borderFill);

template<typename T1,typename T2>
icl::DynMatrix<T2> &imagpart(const icl::DynMatrix<std::complex<T1> > &src,
		icl::DynMatrix<T2> &dst){
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
template
icl::DynMatrix<icl8u> &imagpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u> &imagpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u> &imagpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl32f> &imagpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f> &imagpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<icl8u> &imagpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u> &imagpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u> &imagpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl32f> &imagpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f> &imagpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T1,typename T2>
icl::DynMatrix<T2> &realpart(const icl::DynMatrix<std::complex<T1> > &src,
		icl::DynMatrix<T2> &dst){
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
template
icl::DynMatrix<icl8u>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  realpart(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<icl8u>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  realpart(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T>
void split_complex(const icl::DynMatrix<std::complex<T> > &src,
		icl::DynMatrix<T> &real,icl::DynMatrix<T> &img){
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
		realdata[i] = (T) srcdata[i].real();
		imagdata[i] = (T)srcdata[i].imag();
	}
}
template void
split_complex(const icl::DynMatrix<std::complex<icl32f> > &src,icl::DynMatrix<icl32f> &real,
		icl::DynMatrix<icl32f> &img);
template void
split_complex(const icl::DynMatrix<std::complex<icl64f> > &src,icl::DynMatrix<icl64f> &real,
		icl::DynMatrix<icl64f> &img);

template<typename T1,typename T2>
icl::DynMatrix<T2>&  magnitude(const icl::DynMatrix<std::complex<T1> > &src,
		icl::DynMatrix<T2> &dst){
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
template
icl::DynMatrix<icl8u>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  magnitude(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<icl8u>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  magnitude(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T1,typename T2>
icl::DynMatrix<T2>&  phase(const icl::DynMatrix<std::complex<T1> > &src,
		icl::DynMatrix<T2> &dst){
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
template
icl::DynMatrix<icl8u>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl64f> &dst);
template
icl::DynMatrix<icl8u>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl8u> &dst);
template
icl::DynMatrix<icl16u>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16u> &dst);
template
icl::DynMatrix<icl32u>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32u> &dst);
template
icl::DynMatrix<icl16s>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl16s> &dst);
template
icl::DynMatrix<icl32s>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32s> &dst);
template
icl::DynMatrix<icl32f>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl32f> &dst);
template
icl::DynMatrix<icl64f>&  phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &dst);

template<typename T>
void  split_magnitude_phase(const icl::DynMatrix<std::complex<T> > &src,
		icl::DynMatrix<T> &magnitude,icl::DynMatrix<T> &phase){
	if(magnitude.cols() != src.cols() || magnitude.rows() != src.rows()){
		magnitude.setBounds(src.cols(),src.rows());
	}
	if(phase.cols() != src.cols() || phase.rows() != src.rows()){
		phase.setBounds(src.cols(),src.rows());
	}
	T im;
	T re;
	T zero = (T) 0;
	const std::complex<T> *srcdata = src.data();
	T *magnitudedata = magnitude.data();
	T *phasedata = phase.data();
	for(unsigned int i=0;i<src.dim();++i){
		im = srcdata[i].imag();
		re = srcdata[i].real();
		magnitudedata[i] = (T)(std::sqrt(re*re+im*im));
		if(re==zero && im==zero){
			//unbestimmt
			phasedata[i] = zero;
		} else if(re<zero && im>=zero){
			phasedata[i] = (T) atan2(im,re)+FFT_PI;
		} else if(re<zero && im<zero){
			phasedata[i] = (T) atan2(im,re)-FFT_PI;
		} else if(re==zero && im>zero){
			phasedata[i] = (T) FFT_PI_HALF;
		} else if(re==zero && im<zero){
			phasedata[i] = (T) -FFT_PI_HALF;
		} else {
			//real==0 && im whaterver else
			phasedata[i] = (T) std::atan2(im,re);
		}
	}
}
template
void  split_magnitude_phase(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<icl32f> &magnitude, icl::DynMatrix<icl32f> &phase);
template
void  split_magnitude_phase(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<icl64f> &magnitude, icl::DynMatrix<icl64f> &phase);

template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> > &joinComplex(const icl::DynMatrix<T1> &real,
		const icl::DynMatrix<T1> &im, icl::DynMatrix<std::complex<T2> > &dst){
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
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl8u> &real,
		const icl::DynMatrix<icl8u> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl16u> &real,
		const icl::DynMatrix<icl16u> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl32u> &real,
		const icl::DynMatrix<icl32u> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl16s> &real,
		const icl::DynMatrix<icl16s> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl32s> &real,
		const icl::DynMatrix<icl32s> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl32f> &real,
		const icl::DynMatrix<icl32f> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl32f> > &joinComplex(const icl::DynMatrix<icl64f> &real,
		const icl::DynMatrix<icl64f> &im,icl::DynMatrix<std::complex<icl32f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl8u> &real,
		const icl::DynMatrix<icl8u> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl16u> &real,
		const icl::DynMatrix<icl16u> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl32u> &real,
		const icl::DynMatrix<icl32u> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl16s> &real,
		const icl::DynMatrix<icl16s> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl32s> &real,
		const icl::DynMatrix<icl32s> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl32f> &real,
		const icl::DynMatrix<icl32f> &im,icl::DynMatrix<std::complex<icl64f> > &dst);
template
icl::DynMatrix<std::complex<icl64f> > &joinComplex(const icl::DynMatrix<icl64f> &real,
		const icl::DynMatrix<icl64f> &im,icl::DynMatrix<std::complex<icl64f> > &dst);

double f = 0.0;
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
			f=cp*k;
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
template std::complex<icl32f>*  fft(unsigned int n, const icl8u* a);
template std::complex<icl32f>*  fft(unsigned int n, const icl16u* a);
template std::complex<icl32f>*  fft(unsigned int n, const icl32u* a);
template std::complex<icl32f>*  fft(unsigned int n, const icl16s* a);
template std::complex<icl32f>*  fft(unsigned int n, const icl32s* a);

template std::complex<icl64f>*  fft(unsigned int n, const icl8u* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl16u* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl32u* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl16s* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl32s* a);

template std::complex<icl32f>*  fft(unsigned int n, const icl32f* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl32f* a);
template std::complex<icl64f>*  fft(unsigned int n, const icl64f* a);
template std::complex<icl32f>*  fft(unsigned int n, const icl64f* a);
template std::complex<icl32f>*  fft(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  fft(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  fft(unsigned int n, const std::complex<icl64f>* a);
template std::complex<icl32f>*  fft(unsigned int n, const std::complex<icl64f>* a);

#ifdef HAVE_IPP
template<typename T>
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<T> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException){
	FFT_DEBUG("using ipp fft");
	IppiFFTSpec_R_32f *spec = 0;
	//IppHintAlgorithm hint;
	IppStatus status = ippiFFTInitAlloc_R_32f(&spec,log2(src.cols()),log2(src.rows()),
			IPP_FFT_DIV_INV_BY_N, ippAlgHintAccurate); //or use ippAlgHintNone
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	int dim = src.cols()*src.rows();

	int minBufSize = 0;
	ippiFFTGetBufSize_R_32f(spec,&minBufSize);

	int currBufSize  = buf.cols()*buf.rows()*sizeof(std::complex<icl32f>)*sizeof(std::complex<icl32f>);
	Ipp8u *buffer=0;
	if(currBufSize >= minBufSize){
		buffer = reinterpret_cast<Ipp8u*>(buf.data());
	}else{
		buf.setBounds(src.cols(),src.rows());
		buffer = reinterpret_cast<Ipp8u*>(buf.data());
	}
	//needed for type conversation
	Ipp32f *srcbuf= new Ipp32f[dim];
	for(int i=0;i<dim;++i){
		srcbuf[i]=Ipp32f(src.data()[i]);
	}
	Ipp32f *dstbuf = new Ipp32f[dim];
	int srcStep = src.cols()*sizeof(Ipp32f);
	status = ippiFFTFwd_RToPack_32f_C1R(srcbuf, srcStep,
			dstbuf, srcStep, spec,buffer);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	status = ippiFFTFree_R_32f(spec);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	IppiSize size;
	size.width = src.cols();
	size.height = src.rows();
	status = ippiPackToCplxExtend_32f32fc_C1R(dstbuf, size,srcStep,
			reinterpret_cast<Ipp32fc*>(dst.data()),src.cols()*sizeof(Ipp32fc));
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	delete[] srcbuf;
	delete[] dstbuf;
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);

icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_fft_icl32fc(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException){

	IppiFFTSpec_C_32fc *spec = 0;
	//IppHintAlgorithm hint;
	IppStatus status = ippiFFTInitAlloc_C_32fc(&spec,log2(src.cols()),log2(src.rows()),
			IPP_FFT_DIV_INV_BY_N, ippAlgHintAccurate); //or use ippAlgHintNone
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	int minBufSize = 0;
	ippiFFTGetBufSize_C_32fc(spec,&minBufSize);

	int currBufSize  = buf.cols()*buf.rows()*sizeof(std::complex<icl32f>)*sizeof(std::complex<icl32f>);
	Ipp8u *buffer=0;
	if(currBufSize >= minBufSize){
		buffer = reinterpret_cast<Ipp8u*>(buf.data());
	}else{
		buf.setBounds(src.cols(),src.rows());
		buffer = reinterpret_cast<Ipp8u*>(buf.data());
	}

	int srcStep = src.cols()*sizeof(Ipp32fc);
	status = ippiFFTFwd_CToC_32fc_C1R(reinterpret_cast<const Ipp32fc*>(src.data()), srcStep,
			reinterpret_cast<Ipp32fc*>(dst.data()), srcStep, spec, buffer);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	status = ippiFFTFree_C_32fc(spec);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	return dst;
}
#endif

#ifdef HAVE_MKL
#include <mkl_dfti.h>
template<typename T>
void unpack_mkl_fft(T *src,std::complex<T> *dst, unsigned int cols, unsigned int rows){

	unsigned int dim = cols*rows;
	T re = 0.0;
	T im = 0.0;
	//first element
	dst[0]=std::complex<T>(src[0],0);
	unsigned int j=1;
	unsigned int offrow = cols*rows/2;
	//first row
	for(unsigned int i=1;i<cols-1;i+=2){
		re =src[i];
		im = src[i+1];
		dst[j] = std::complex<T>(re,im);
		dst[cols-j] = std::complex<T>(re,-im);
		++j;
	}
	if(rows%2 ==0){
		dst[offrow]=std::complex<T>(src[dim-cols],0);
	}
	if(cols%2 ==0){
		dst[cols/2]=std::complex<T>(src[cols-1],0);
	}
	if(cols%2==0 && rows%2==0){
		dst[offrow+cols/2]=std::complex<T>(src[dim-1],0);
	}
	j=cols;
	unsigned int offcol = cols/2;

	for(unsigned int i=cols;i<dim-cols-1;i+=2*cols){
		re =src[i];
		im = src[i+cols];
		dst[j] = std::complex<T>(re,im);
		dst[dim-j] = std::complex<T>(re,-im);
		if(cols%2==0 ){
			re =src[i+cols-1];
			im = src[i+2*cols-1];
			dst[j+offcol] = std::complex<T>(re,im);
			dst[dim-j+offcol] = std::complex<T>(re,-im);
		}
		j+=cols;
	}
	unsigned int a = cols/2;
	if(cols%2==1){
		a=cols/2+1;
	}
	unsigned int cindex=1;
	for(unsigned int r=1;r<a;++r){
		j=cols+r;
		for(unsigned int i=cols+cindex;i<dim;i+=cols){
			re =src[i];
			im = src[i+1];
			dst[j] = std::complex<T>(re,im);
			dst[dim-j+cols] = std::complex<T>(re,-im);
			j+=cols;
		}
		cindex+=2;
	}
}

template
void  unpack_mkl_fft(float *src, std::complex<float> *dst, unsigned int cols,
		unsigned int rows);
template
void  unpack_mkl_fft(double *src, std::complex<double> *dst, unsigned int cols,
		unsigned int rows);

template<class T2> DFTI_CONFIG_VALUE getMKLDftiType(){ return DFTI_DOUBLE; }
template<> DFTI_CONFIG_VALUE getMKLDftiType<float>() { return DFTI_SINGLE; }

template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> >&  mkl_wrapper_function_result_fft(
		const icl::DynMatrix<T1> &src, icl::DynMatrix<std::complex<T2> > &dst,
		icl::DynMatrix<std::complex<T2> > &buffer) throw (FFTException){
	FFT_DEBUG("using mkl fft2d");
	unsigned int dimx = src.cols();
	unsigned int dimy = src.rows();
	T2 *srcbuf = new T2[dimy*dimx];
	for(unsigned int i=0;i<src.rows()*src.cols();++i){
		srcbuf[i] =T2(src.data()[i]);
	}
	MKL_LONG status, l[2]={dimy,dimx};
	DFTI_DESCRIPTOR_HANDLE my_desc1_handle=0;
	status = DftiCreateDescriptor( &my_desc1_handle, getMKLDftiType<T2>(), DFTI_REAL, 2,l);
	MKL_LONG strides_in[3]={0,dimx,1};
	MKL_LONG strides_out[3]={0,dimx,1};
	status = DftiSetValue(my_desc1_handle,DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_PACKED_FORMAT, DFTI_PACK_FORMAT);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_INPUT_STRIDES,strides_in);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_OUTPUT_STRIDES,strides_out);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiCommitDescriptor( my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiCommitDescriptorError");
	}
	T2 *buf = reinterpret_cast<T2*>(buffer.data());
	status = DftiComputeForward( my_desc1_handle, srcbuf, buf);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiComputeForwardError");
	}
	status = DftiFreeDescriptor(&my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftifreeDescriptorError");
	}
	unpack_mkl_fft(buf,dst.data(),dimx,dimy);
	delete[] srcbuf;
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);

template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf)throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf) throw (FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_fft(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw (FFTException);

template<typename T1,typename T2>
void mkl_wrapper_function_result_fft_complex(DFTI_DESCRIPTOR_HANDLE &my_desc1_handle,T1 *src,
		std::complex<T2>*dst, std::complex<T2> *buffer, unsigned int dimx, unsigned int dimy) throw (FFTException){
	FFT_DEBUG("using mkl fft2d_complex");

	MKL_LONG status;
	MKL_LONG strides_in[3]={0,dimx,1};
	MKL_LONG strides_out[3]={0,dimx,1};
	status = DftiSetValue(my_desc1_handle,DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_INPUT_STRIDES,strides_in);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_OUTPUT_STRIDES,strides_out);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiCommitDescriptor( my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiCommitDescriptorError");
	}
	status = DftiComputeForward( my_desc1_handle, src, reinterpret_cast<T1*>(dst));
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiComputeForwardError");
	}
	status = DftiFreeDescriptor(&my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiFreeDescriptorError");
	}
}
template
void mkl_wrapper_function_result_fft_complex(DFTI_DESCRIPTOR_HANDLE &my_desc1_handle,_MKL_Complex8 *src,
		std::complex<icl32f> *dst, std::complex<icl32f> *buffer,unsigned int dimx, unsigned int dimy) throw (FFTException);
template
void mkl_wrapper_function_result_fft_complex(DFTI_DESCRIPTOR_HANDLE &my_desc1_handle,_MKL_Complex16 *src,
		std::complex<icl64f> *dst, std::complex<icl64f> *buffer,unsigned int dimx, unsigned int dimy) throw (FFTException);


icl::DynMatrix<std::complex<icl32f> >& mkl_wrapper_function_result_fft_icl32fc(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException){
	unsigned int dimx = src.cols();
	unsigned int dimy = src.rows();
	_MKL_Complex8 *srcbuf = new _MKL_Complex8[dimy*dimx];
	for(unsigned int i=0;i<src.rows()*src.cols();++i){
		srcbuf[i] =*(reinterpret_cast<const _MKL_Complex8*>(&(src.data()[i])));
	}
	MKL_LONG status, l[2]={dimy,dimx};
	DFTI_DESCRIPTOR_HANDLE my_desc1_handle;
	status = DftiCreateDescriptor( &my_desc1_handle, DFTI_SINGLE, DFTI_COMPLEX, 2,l);
	mkl_wrapper_function_result_fft_complex(my_desc1_handle,srcbuf,dst.data(),buffer.data(),dimx,dimy);
	delete[] srcbuf;
	return dst;
}

icl::DynMatrix<std::complex<icl64f> >& mkl_wrapper_function_result_fft_icl64fc(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw (FFTException){
	unsigned int dimx = src.cols();
	unsigned int dimy = src.rows();
	_MKL_Complex16 *srcbuf = new _MKL_Complex16[dimy*dimx];
	for(unsigned int i=0;i<src.rows()*src.cols();++i){
		srcbuf[i] =*(reinterpret_cast<const _MKL_Complex16*>(&(src.data()[i])));
	}
	MKL_LONG status, l[2]={dimy,dimx};
	DFTI_DESCRIPTOR_HANDLE my_desc1_handle=0;
	status = DftiCreateDescriptor( &my_desc1_handle, DFTI_DOUBLE, DFTI_COMPLEX, 2,l);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiCreateDescriptorError");
	}
	mkl_wrapper_function_result_fft_complex(my_desc1_handle,srcbuf,dst.data(),buffer.data(),dimx,dimy);
	delete[] srcbuf;
	return dst;
}
#endif

template<typename T1, typename T2>
icl::DynMatrix<std::complex<T2> >& fft2D_cpp(const icl::DynMatrix<T1> &src,icl::DynMatrix<std::complex<T2> > &dst,
		icl::DynMatrix<std::complex<T2> > &buf){
	FFT_DEBUG("fft2D_cpp");
	//check buffer
	if(buf.isNull() || buf.cols() != src.rows() || buf.rows() != src.cols()){
		//always wrong, but in this case really right!!!
		buf(src.rows(),src.cols());
	}
	unsigned int cols = src.cols();
	unsigned int rows = src.rows();
	std::complex<T2> *temp=0;
	//already transposed
	for(unsigned int i=0;i<rows;++i){
		temp = fft<T1,T2>(cols,src.row_begin(i));
		for(unsigned int j=0;j<cols;++j){
			//buf.operator()(i,j)=temp[j];
			buf(i,j)=temp[j];
		}
		delete[] temp;
	}

	for(unsigned int i=0;i<cols;++i){
		temp = fft<std::complex<T2>,T2 >(rows,buf.row_begin(i));
		for(unsigned int j=0;j<rows;++j){
			//dst.operator()(i,j)=temp[j];
			dst(i,j)=temp[j];
		}
		delete[] temp;
	}
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  fft2D_cpp(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  fft2D_cpp(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >& fft2D_cpp(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >& fft2D_cpp(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D_cpp(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >& fft2D_cpp(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D_cpp(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >& fft2D_cpp(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D_cpp(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);

template<typename T1, typename T2>
icl::DynMatrix<std::complex<T2> >& fft2D(const icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> > &dst, icl::DynMatrix<std::complex<T2> > &buf){
	if(isPowerOfTwo(src.cols()) && isPowerOfTwo(src.rows())){
#ifdef HAVE_IPP
		buf.setBounds(src.cols(),src.rows());
		return ipp_wrapper_function_result_fft(src,dst,buf);
#endif
	}
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,	icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,	icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,	icl::DynMatrix<std::complex<icl32f> > &buf);

//double
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  fft2D(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl32f> >&  fft2D(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
//complex
template<>
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf){
	if(isPowerOfTwo(src.cols()) && isPowerOfTwo(src.rows())){
#ifdef HAVE_IPP
		buf.setBounds(src.cols(),src.rows());
		return ipp_wrapper_function_result_fft_icl32fc(src,dst,buf);
#endif
	}
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft_icl32fc(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >& fft2D(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_fft_icl64fc(src,dst,buf);
#endif
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >& fft2D(const icl::DynMatrix<std::complex<icl32f> > &src,icl::DynMatrix<std::complex<icl64f> > &dst,
		icl::DynMatrix<std::complex<icl64f> > &buf){
	return fft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl32f> >& fft2D(const icl::DynMatrix<std::complex<icl64f> > &src,icl::DynMatrix<std::complex<icl32f> > &dst,
		icl::DynMatrix<std::complex<icl32f> > &buf){
	return fft2D_cpp(src,dst,buf);
}

template<typename T1, typename T2>
std::complex<T2>*  dft(unsigned int n, T1* src){
	std::complex<T2>* d = new std::complex<T2>[n];
	std::complex<T2> x = std::complex<T2>(0.0,0.0);
	T2 f= -FFT_2_PI/n;
	T2 g = 0.0;
	T2 h = 0.0;
	std::complex<T2> temp(0,0);
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

template std::complex<icl32f>*  dft(unsigned int n, icl8u* src);
template std::complex<icl64f>*  dft(unsigned int n, icl8u* src);
template std::complex<icl32f>*  dft(unsigned int n, icl16u* src);
template std::complex<icl64f>*  dft(unsigned int n, icl16u* src);
template std::complex<icl32f>*  dft(unsigned int n, icl32u* src);
template std::complex<icl64f>*  dft(unsigned int n, icl32u* src);
template std::complex<icl32f>*  dft(unsigned int n, icl16s* src);
template std::complex<icl64f>*  dft(unsigned int n, icl16s* src);
template std::complex<icl32f>*  dft(unsigned int n, icl32s* src);
template std::complex<icl64f>*  dft(unsigned int n, icl32s* src);

template std::complex<icl32f>*  dft(unsigned int n, icl32f* src);
template std::complex<icl64f>*  dft(unsigned int n, icl32f* src);
template std::complex<icl64f>*  dft(unsigned int n, icl64f* src);
template std::complex<icl32f>*  dft(unsigned int n, icl64f* src);
template std::complex<icl32f>*  dft(unsigned int n, std::complex<icl32f>* src);
template std::complex<icl64f>*  dft(unsigned int n, std::complex<icl32f>* src);
template std::complex<icl64f>*  dft(unsigned int n, std::complex<icl64f>* src);
template std::complex<icl32f>*  dft(unsigned int n, std::complex<icl64f>* src);

template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> >&  dft2D(icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> >&dst, icl::DynMatrix<std::complex<T2> >&buf){
	std::complex<T2> *temp=0;
	for(unsigned int i=0;i<src.rows();++i){
		temp = dft<T1,T2>(src.cols(),(src.row(i)).data());
		//already transposed
		for(unsigned int j=0;j<src.cols();++j){
			buf(i,j)=temp[j];
		}
		delete[] temp;
	}
	for(unsigned int i=0;i<buf.rows();++i){
		temp = dft<std::complex<T2>,T2>(buf.cols(),(buf.row(i)).data());
		for(unsigned int j=0;j<buf.cols();++j){
			dst(i,j)=temp[j];
		}
		delete[] temp;
	}
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl8u>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl8u>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl16u>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl16u>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl16s>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl16s>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl32u>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl32u>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl32s>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl32s>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl32f>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl32f>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<icl64f>& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<icl64f>& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<std::complex<icl32f> >& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<std::complex<icl64f> >& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl64f> >&  dft2D(icl::DynMatrix<std::complex<icl32f> >& src,
		icl::DynMatrix<std::complex<icl64f> >& dst, icl::DynMatrix<std::complex<icl64f> >& buf);
template
icl::DynMatrix<std::complex<icl32f> >&  dft2D(icl::DynMatrix<std::complex<icl64f> >& src,
		icl::DynMatrix<std::complex<icl32f> >& dst, icl::DynMatrix<std::complex<icl32f> >& buf);

double e = 0.0;
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
			e=cp*k;
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
template std::complex<icl32f>*  ifft_(unsigned int n, const icl8u* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl16u* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl32u* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl16s* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl32s* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl32f* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const icl64f* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl8u* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl16u* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl32u* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl16s* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl32s* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl32f* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const icl64f* a);

template std::complex<icl32f>*  ifft_(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  ifft_(unsigned int n, const std::complex<icl64f>* a);
template std::complex<icl32f>*  ifft_(unsigned int n, const std::complex<icl64f>* a);

template<typename T1, typename T2>
std::complex<T2>*  ifft_cpp(unsigned int n, const T1* a){
	std::complex<T2>* tempMat = ifft_<T1,T2>(n,a);
	double lambda = 1.0/n;
	for(unsigned int index = 0;index<n;++index){
		tempMat[index] *= lambda;
	}
	return tempMat;
}

template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl8u* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl16u* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl32u* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl16s* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl32s* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const icl32f* a);

template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl8u* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl16u* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32u* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl16s* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32s* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl32f* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const icl64f* a);

template std::complex<icl32f>*  ifft_cpp(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const std::complex<icl32f>* a);
template std::complex<icl64f>*  ifft_cpp(unsigned int n, const std::complex<icl64f>* a);
template std::complex<icl32f>*  ifft_cpp(unsigned int n, const std::complex<icl64f>* a);

#ifdef HAVE_IPP
template<typename T>
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<T> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException){
	FFT_DEBUG("using ipp ifft fc");
	int dim = src.cols()*src.rows();
	IppiFFTSpec_C_32fc *spec = 0;
	//IppHintAlgorithm hint;
	IppStatus status = ippiFFTInitAlloc_C_32fc(&spec,log2(src.cols()),log2(src.rows()),
			IPP_FFT_DIV_INV_BY_N, ippAlgHintAccurate); //or use ippAlgHintNone
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	int minBufSize = 0;
	ippiFFTGetBufSize_C_32fc(spec,&minBufSize);
	int currBufSize = dim*sizeof(std::complex<icl32f>)*sizeof(std::complex<icl32f>);
	Ipp8u *buffer=0;
	if(currBufSize<minBufSize){
		buf.setBounds(src.cols(),src.rows());
	}
	buffer = reinterpret_cast<Ipp8u*>(buf.data());

	Ipp32fc *srcbuf = new Ipp32fc[dim];
	const T *srcdata = src.data();
	std::complex<icl32f> t(0,0);
	for(int i=0;i<dim;++i){
		t = CreateComplex<T,icl32f>::create_complex(srcdata[i]);
		Ipp32fc f={t.real(),t.imag()};
		srcbuf[i]=f;
	}
	int srcStep = src.cols()*sizeof(Ipp32fc);
	status = ippiFFTInv_CToC_32fc_C1R(srcbuf, srcStep,
			reinterpret_cast<Ipp32fc*>(dst.data()), srcStep, spec, buffer);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	status = ippiFFTFree_C_32fc(spec);
	if(status != ippStsOk){
		std::string msg = "Error in IPP call!:";
		msg +=ippGetStatusString(status);
		throw FFTException(msg);
	}
	return dst;
}

template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  ipp_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf) throw(FFTException);
#endif

#ifdef HAVE_MKL
template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buffer) throw(FFTException){
	FFT_DEBUG("using mkl ifft2d fc");
	int dim = src.cols()*src.rows();
	unsigned int dimx = src.cols();
	unsigned int dimy = src.rows();
	MKL_LONG status, l[2]={dimy,dimx};
	DFTI_DESCRIPTOR_HANDLE my_desc1_handle=0;
	status = DftiCreateDescriptor( &my_desc1_handle, getMKLDftiType<T2>(), DFTI_COMPLEX, 2,l);
	MKL_LONG strides_in[3]={0,dimx,1};
	MKL_LONG strides_out[3]={0,dimx,1};
	status = DftiSetValue(my_desc1_handle,DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_PACKED_FORMAT, DFTI_PACK_FORMAT);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_INPUT_STRIDES,strides_in);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_OUTPUT_STRIDES,strides_out);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	T2 scale = 1.0/(T2)(dimx*dimy);
	status = DftiSetValue(my_desc1_handle, DFTI_BACKWARD_SCALE, scale);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiCommitDescriptor( my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiCommitDescriptorError");
	}
	_MKL_Complex8 *srcbuf = new _MKL_Complex8[dim];
	std::complex<T2> t(0,0);
	for(int i=0;i<dim;++i){
		t = CreateComplex<T1,T2>::create_complex(src.data()[i]);
		_MKL_Complex8 temp;
		temp.real = T2(t.real());
		temp.imag = T2(t.imag());
		srcbuf[i] = temp;
	}
	status = DftiComputeBackward( my_desc1_handle, srcbuf,
			reinterpret_cast<_MKL_Complex8*>(dst.data()));
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiComputeBackwardError");
	}
	status = DftiFreeDescriptor(&my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiFreeDescriptorError");
	}
	delete[] srcbuf;
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl32f> >&  mkl_wrapper_function_result_ifft_icl32fc(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buffer) throw(FFTException);

template<typename T1,typename T2>
icl::DynMatrix<std::complex<T2> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buffer) throw(FFTException){
	FFT_DEBUG("using mkl ifft2dfc");
	int dim = src.cols()*src.rows();
	unsigned int dimx = src.cols();
	unsigned int dimy = src.rows();
	MKL_LONG status, l[2]={dimy,dimx};
	DFTI_DESCRIPTOR_HANDLE my_desc1_handle=0;
	status = DftiCreateDescriptor( &my_desc1_handle, getMKLDftiType<T2>(), DFTI_COMPLEX, 2,l);
	MKL_LONG strides_in[3]={0,dimx,1};
	MKL_LONG strides_out[3]={0,dimx,1};
	status = DftiSetValue(my_desc1_handle,DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_PACKED_FORMAT, DFTI_PACK_FORMAT);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_INPUT_STRIDES,strides_in);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiSetValue(my_desc1_handle,DFTI_OUTPUT_STRIDES,strides_out);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	T2 scale = 1.0/(T2)(dimx*dimy);
	status = DftiSetValue(my_desc1_handle, DFTI_BACKWARD_SCALE, scale);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiSetValueError");
	}
	status = DftiCommitDescriptor( my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiCommitDescriptorError");
	}
	_MKL_Complex16 *srcbuf = new _MKL_Complex16[dim];
	std::complex<T2> t(0,0);
	for(int i=0;i<dim;++i){
		t = CreateComplex<T1,T2>::create_complex(src.data()[i]);
		_MKL_Complex16 temp;
		temp.real = T2(t.real());
		temp.imag = T2(t.imag());
		srcbuf[i] = temp;
	}
	status = DftiComputeBackward( my_desc1_handle, srcbuf,
			reinterpret_cast<_MKL_Complex16*>(dst.data()));
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiComputeBackwardError");
	}
	status = DftiFreeDescriptor(&my_desc1_handle);
	if(!DftiErrorClass(status,DFTI_NO_ERROR)){
		throw FFTException("FFTException DftiFreeDescriptorError");
	}
	delete[] srcbuf;
	return dst;
}
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
template
icl::DynMatrix<std::complex<icl64f> >&  mkl_wrapper_function_result_ifft_icl64fc(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buffer) throw(FFTException);
#endif

template<typename T1, typename T2>
icl::DynMatrix<std::complex<T2> >&   ifft2D_cpp(const icl::DynMatrix<T1> &src,icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buf){
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
			buf(i,j)=temp[j];
		}
		delete[] temp;
	}
	for(unsigned int i=0;i<buf.rows();++i){
		temp = ifft_cpp<std::complex<T2>,T2>(buf.cols(),buf.row_begin(i));
		for(unsigned int j=0;j<buf.cols();++j){
			dst(i,j)=temp[j];
		}
		delete[] temp;
	}
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  ifft2D_cpp(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  ifft2D_cpp(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);

template<typename T1, typename T2>
icl::DynMatrix<std::complex<T2> >& ifft2D(const icl::DynMatrix<T1> &src,
		icl::DynMatrix<std::complex<T2> > &dst,	icl::DynMatrix<std::complex<T2> > &buf){
	if(isPowerOfTwo(src.cols()) && isPowerOfTwo(src.rows())){
#ifdef HAVE_IPP
		buf.setBounds(src.cols(),src.rows());
		return ipp_wrapper_function_result_ifft_icl32fc(src,dst,buf);

#endif
	}
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl32fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >& ifft2D(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl32f> > &dst, icl::DynMatrix<std::complex<icl32f> > &buf);
//double
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl8u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl16u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl32u> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl16s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl32s> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl32f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<icl64f> &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}

//complex
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<std::complex<icl32f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}
template<>
icl::DynMatrix<std::complex<icl64f> >&  ifft2D(const icl::DynMatrix<std::complex<icl64f> > &src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf){
#ifdef HAVE_MKL
	return mkl_wrapper_function_result_ifft_icl64fc(src,dst,buf);
#endif
	return ifft2D_cpp(src,dst,buf);
}

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

template std::complex<icl32f>*  idft(unsigned int n, icl8u* src);
template std::complex<icl64f>*  idft(unsigned int n, icl8u* src);
template std::complex<icl32f>*  idft(unsigned int n, icl16u* src);
template std::complex<icl64f>*  idft(unsigned int n, icl16u* src);
template std::complex<icl32f>*  idft(unsigned int n, icl16s* src);
template std::complex<icl64f>*  idft(unsigned int n, icl16s* src);
template std::complex<icl32f>*  idft(unsigned int n, icl32u* src);
template std::complex<icl64f>*  idft(unsigned int n, icl32u* src);
template std::complex<icl32f>*  idft(unsigned int n, icl32s* src);
template std::complex<icl64f>*  idft(unsigned int n, icl32s* src);
template std::complex<icl32f>*  idft(unsigned int n, icl32f* src);
template std::complex<icl64f>*  idft(unsigned int n, icl32f* src);
template std::complex<icl64f>*  idft(unsigned int n, icl64f* src);
template std::complex<icl32f>*  idft(unsigned int n, icl64f* src);
template std::complex<icl32f>*  idft(unsigned int n, std::complex<icl32f>* src);
template std::complex<icl64f>*  idft(unsigned int n, std::complex<icl32f>* src);
template std::complex<icl64f>*  idft(unsigned int n, std::complex<icl64f>* src);
template std::complex<icl32f>*  idft(unsigned int n, std::complex<icl64f>* src);

template<typename T1, typename T2>
icl::DynMatrix<std::complex<T2> >&   idft2D(icl::DynMatrix<T1>& src,icl::DynMatrix<std::complex<T2> > &dst,icl::DynMatrix<std::complex<T2> > &buf){
	std::complex<T2> *temp = 0;
	for(unsigned int i=0;i<src.rows();++i){
		temp = idft<T1,T2>(src.cols(),(src.row(i)).data());
		//already transposed
		for(unsigned int j=0;j<src.cols();++j){
			buf(i,j)=temp[j];
		}
		delete[] temp;
	}
	for(unsigned int i=0;i<buf.cols();++i){
		temp =  idft<std::complex<T2>,T2>(buf.rows(),(buf.row(i)).data());
		for(unsigned int j=0;j<buf.rows();++j){
			dst(i,j)=temp[j];
		}
		delete[] temp;
	}
	return dst;
}
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl8u>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl8u>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl16u>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl16u>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl32u>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl32u>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl16s>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl16s>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl32s>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl32s>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl32f>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl32f>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<icl64f>& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<icl64f>& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<std::complex<icl32f> >& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<std::complex<icl32f> >& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl64f> >&  idft2D(icl::DynMatrix<std::complex<icl64f> >& src,
		icl::DynMatrix<std::complex<icl64f> > &dst,icl::DynMatrix<std::complex<icl64f> > &buf);
template
icl::DynMatrix<std::complex<icl32f> >&  idft2D(icl::DynMatrix<std::complex<icl64f> >& src,
		icl::DynMatrix<std::complex<icl32f> > &dst,icl::DynMatrix<std::complex<icl32f> > &buf);
}
}

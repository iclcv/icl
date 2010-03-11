/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLFilter/FFTOp.h>

//#define FFTOp_DEBUG(X) std::cout << X << std::endl;
#define FFTOp_DEBUG(X)

using namespace std;
using namespace icl;
using namespace icl::fft;
namespace icl{

struct FFTOp::Data{
	ResultMode m_rm;
	SizeAdaptionMode m_sam;
	bool m_fftshift;
	bool m_forceDFT;
	ImgBase *m_adaptedSource;
	DynMatrix<std::complex<float> > m_buf32f;
	DynMatrix<std::complex<double> > m_buf64f;
	DynMatrix<std::complex<float> > m_dstBuf32f;
	DynMatrix<std::complex<double> > m_dstBuf64f;

	Data(ResultMode rm=LOG_POWER_SPECTRUM, SizeAdaptionMode sam=NO_SCALE,bool fftshift=true,bool forceDFT=false):
		m_rm(rm),m_sam(sam), m_fftshift(fftshift),m_forceDFT(forceDFT),m_adaptedSource(0){}
	~Data(){
		ICL_DELETE(m_adaptedSource);
	}
};

void FFTOp::setResultMode(ResultMode rm){
	m_data->m_rm = rm;
}

int FFTOp::getResultMode(){
	return m_data->m_rm;
}

void FFTOp::setSizeAdaptionMode(SizeAdaptionMode sam){
	m_data->m_sam = sam;
}

int FFTOp::getSizeAdaptionMode(){
	return m_data->m_sam;
}

void FFTOp::setForceDFT(bool pForceDFT){
	m_data->m_forceDFT = pForceDFT;
}

bool FFTOp::getForceDFT(){
	return m_data->m_forceDFT;
}

void FFTOp::setFFTShift(bool pFFTShift){
	m_data->m_fftshift = pFFTShift;
}

bool FFTOp::getFFTShift(){
	return m_data->m_fftshift;
}

FFTOp::FFTOp(ResultMode rm, SizeAdaptionMode zam, bool fftshift, bool forceDFT):
										m_data(new FFTOp::Data(rm, zam, fftshift, forceDFT)){}

FFTOp::~FFTOp(){
	delete m_data;
}

template<typename T>
void FFTOp::apply_inplace_fftshift(DynMatrix<T> &mat){
	unsigned int cols = mat.cols();
	unsigned int cols2 = cols/2;
	unsigned int rows = mat.rows();
	unsigned int rows2 = rows/2;
	if(cols>1 && rows>1){
		if(cols%2==0 || rows%2==0){
			T temp = (T)0;
			for(unsigned int y=0;y<rows2;++y){
				for(unsigned int x=0;x<cols2;++x){
					temp = mat.operator ()(x,y);
					mat.operator ()(x,y) = mat.operator ()(x+cols2,y+rows2);
					mat.operator ()(x+cols2,y+rows2) = temp;
					temp = mat.operator ()(x+cols2,y);
					mat.operator ()(x+cols2,y) = mat.operator ()(x,y+rows2);
					mat.operator ()(x,y+rows2) = temp;
				}
			}
			if(mat.cols()%2==0 && mat.rows()%2==0){
				//nothing to do
			}else if(mat.cols()%2==1 && mat.rows()%2==0){
				T t2 = 0;
				T t3 = 0;
				for(unsigned int y=0;y<rows2;++y){
					t2 = mat.operator ()(cols-1,y);
					t3 = mat.operator ()(cols-1,y+rows2);
					mat.operator ()(cols-1,y) = mat.operator ()(0,y);
					mat.operator ()(cols-1,y+rows2) = mat.operator ()(0,y+rows2);
					for(unsigned int x=1;x<cols2;++x){
						mat.operator ()(x-1,y) = mat.operator ()(x,y);
						mat.operator ()(x-1,y+rows2) = mat.operator ()(x,y+rows2);
					}
					mat.operator ()(cols2-1,y) = t3;
					mat.operator ()(cols2-1,y+rows2) = t2;
				}
			} else if(mat.cols()%2==0 && mat.rows()%2==1){
				T t2 = 0;
				T t3 = 0;
				for(unsigned int x=0;x<cols2;++x){
					t2 = mat.operator ()(x,rows-1);
					t3 = mat.operator ()(x+cols2,rows-1);
					mat.operator ()(x,rows-1) = mat.operator ()(x,0);
					mat.operator ()(x+cols2,rows-1) = mat.operator ()(x+cols2,0);
					for(unsigned int y=1;y<rows2;++y){
						mat.operator ()(x,y-1) = mat.operator ()(x,y);
						mat.operator ()(x+cols2,y-1) = mat.operator ()(x+cols2,y);
					}
					mat.operator ()(x,rows2-1) = t3;
					mat.operator ()(x+cols2,rows2-1) = t2;
				}
			}
		} else {
			unsigned int dim = cols;
			unsigned int dim2 = dim/2;
			for(unsigned int k=0;k<rows;++k){
				T *dat = mat.row_begin(k);
				for(unsigned int i=0;i<dim2;++i){
					std::swap(dat[i],dat[i+dim2]);
				}
				if(cols%2==1 && rows%2==1){
					T temp = dat[dim-1];
					dat[dim-1] = dat[0];
					for(unsigned int i=1;i<dim2;++i){
						dat[i-1] = dat[i];
					}
					if(dim2>0)
						dat[dim2-1]=temp;
				}
			}
			dim = rows;
			dim2 = dim/2;
			for(unsigned int k=0;k<cols;++k){
				for(unsigned int i=0;i<dim2;++i){
					std::swap(mat.operator ()(k,i),mat.operator ()(k,i+dim2));
				}
				if(cols%2==1 && rows%2==1){
					T temp = mat.operator ()(k,dim-1);
					mat.operator ()(k,dim-1) = mat.operator ()(k,0);
					for(unsigned int i=1;i<dim2;++i){
						mat.operator ()(k,i-1) = mat.operator ()(k,i);
					}
					if(dim2>0)
						mat.operator ()(k,dim2-1)=temp;
				}
			}
		}
	}else {
		unsigned int dim = cols*rows;
		unsigned int dim2 = dim/2;
		for(unsigned int i=0;i<dim2;++i){
			std::swap(mat.data()[i],mat.data()[i+dim2]);
		}
		if(cols%2==1 && rows%2==1){
			T temp = mat.data()[dim-1];
			mat.data()[dim-1] = mat.data()[0];
			for(unsigned int i=1;i<dim2;++i){
				mat.data()[i-1] = mat.data()[i];
			}
			if(dim2>0)
				mat.data()[dim2-1]=temp;
		}
	}
}

template void FFTOp::apply_inplace_fftshift(DynMatrix<icl32f> &m);
template void FFTOp::apply_inplace_fftshift(DynMatrix<icl64f> &m);

template<class SrcT, class DstT>
void FFTOp::apply_internal(const Img<SrcT> &src, Img<DstT> &dst,
		DynMatrix<std::complex<DstT> > &buf, DynMatrix<std::complex<DstT> > &dstBuf){
	FFTOp_DEBUG("f: apply_internal");
	dstBuf.setBounds(dst.getWidth(),dst.getHeight());
	buf.setBounds(dst.getHeight(),dst.getWidth());
	for(int lChannel = 0;lChannel<src.getChannels();++lChannel){
		DynMatrix<SrcT> srcMat = src.extractDynMatrix(lChannel);
		if(m_data->m_forceDFT){
			FFTOp_DEBUG("compute dft on "<<lChannel);
			dft2D(srcMat,dstBuf,buf);
		} else {
			FFTOp_DEBUG("compute fft on "<<lChannel);
			fft2D(srcMat,dstBuf,buf);
		}
		switch(m_data->m_rm){
		case TWO_CHANNEL_COMPLEX:{
			FFTOp_DEBUG("two channel complex");
			DynMatrix<DstT> mm1 = dst.extractDynMatrix(2*lChannel);
			DynMatrix<DstT> mm2 = dst.extractDynMatrix(2*lChannel+1);
			split_complex(dstBuf,mm1,mm2);
			break;}
		case IMAG_ONLY:{
			FFTOp_DEBUG("imag only");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			imagpart(dstBuf,mm);
			break;}
		case REAL_ONLY:{
			FFTOp_DEBUG("real only");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			realpart(dstBuf,mm);
			break;}
		case POWER_SPECTRUM:{
			FFTOp_DEBUG("power spectrum");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			powerspectrum(dstBuf,mm);
			break;}
		case LOG_POWER_SPECTRUM:{
			FFTOp_DEBUG("log power spectrum");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			logpowerspectrum<DstT>(dstBuf,mm);
			break;}
		case MAGNITUDE_ONLY:{
			FFTOp_DEBUG("magnitude");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			magnitude(dstBuf,mm);
			break;}
		case PHASE_ONLY:{
			FFTOp_DEBUG("phase");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			phase(dstBuf,mm);
			break;}
		case TWO_CHANNEL_MAGNITUDE_PHASE:{
			FFTOp_DEBUG("two  channel magnitude phase");
			DynMatrix<DstT> mm1 = dst.extractDynMatrix(2*lChannel);
			DynMatrix<DstT> mm2 = dst.extractDynMatrix(2*lChannel+1);
			split_magnitude_phase(dstBuf,mm1,mm2);
			break;
		}
		}
	}
	if(m_data->m_fftshift){
		FFTOp_DEBUG("start shifft");
		for(int i=0;i<dst.getChannels();++i){
			DynMatrix<DstT> mm = dst.extractDynMatrix(i);
			apply_inplace_fftshift(mm);
		}
	}
}

template void FFTOp::apply_internal(const Img<icl8u> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl16s> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl32s> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl32f> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl64f> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);

template void FFTOp::apply_internal(const Img<icl8u> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl16s> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl32s> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl32f> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void FFTOp::apply_internal(const Img<icl64f> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);

template<class T>
const Img<T> *FFTOp::adapt_source(const Img<T> *src){
	FFTOp_DEBUG("f: adapt_source");
	switch(m_data->m_sam){
	case NO_SCALE:
		FFTOp_DEBUG("NO_SCALE");
		return src;
	case PAD_ZERO:{
		FFTOp_DEBUG("PAD_ZERO");
		int newHeight = nextPowerOf2(src->getHeight());
		int newWidth = nextPowerOf2(src->getWidth());
		m_data->m_adaptedSource = new Img<T>(Size(newWidth,newHeight), src->getChannels(), formatMatrix);
		for(int i=0;i<src->getChannels();++i){
			DynMatrix<T> m = ((m_data->m_adaptedSource)->asImg<T>())->extractDynMatrix(i);
			makeborder(src->extractDynMatrix(i),m,(T)0);
		}
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);}
	case PAD_COPY:{
		FFTOp_DEBUG("PAD_COPY");
		int newHeight = nextPowerOf2(src->getHeight());
		int newWidth = nextPowerOf2(src->getWidth());
		m_data->m_adaptedSource = new Img<T>(Size(newWidth,newHeight), src->getChannels(), formatMatrix);
		for(int i=0;i<src->getChannels();++i){
			DynMatrix<T> m = ((m_data->m_adaptedSource)->asImg<T>())->extractDynMatrix(i);
			continueMatrixToPowerOf2(src->extractDynMatrix(i),m);
		}
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);}
	case PAD_MIRROR:{
		FFTOp_DEBUG("PAD_MIRROR");
		int newHeight = nextPowerOf2(src->getHeight());
		int newWidth = nextPowerOf2(src->getWidth());
		m_data->m_adaptedSource = new Img<T>(Size(newWidth,newHeight), src->getChannels(), formatMatrix);
		for(int i=0;i<src->getChannels();++i){
			DynMatrix<T> m = ((m_data->m_adaptedSource)->asImg<T>())->extractDynMatrix(i);
			mirrorOnCenter(src->extractDynMatrix(i),m);
		}
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);}
	case SCALE_UP:{
		FFTOp_DEBUG("SCALE_UP");
		int newHeight = nextPowerOf2(src->getHeight());
		int newWidth = nextPowerOf2(src->getWidth());
		m_data->m_adaptedSource = src->scaledCopy(Size(newWidth,newHeight),icl::interpolateLIN);
		m_data->m_adaptedSource->detach(-1);
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);}
	case SCALE_DOWN:{
		FFTOp_DEBUG("SCALE_DOWN");
		int newHeight = priorPowerOf2(src->getHeight());
		int newWidth = priorPowerOf2(src->getWidth());
		m_data->m_adaptedSource = src->scaledCopy(Size(newWidth,newHeight),icl::interpolateRA);
		m_data->m_adaptedSource->detach(-1);
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);}
	default:
		return src;
	}
}

template const Img<icl8u> *FFTOp::adapt_source(const Img<icl8u> *src);
template const Img<icl16s> *FFTOp::adapt_source(const Img<icl16s> *src);
template const Img<icl32s> *FFTOp::adapt_source(const Img<icl32s> *src);
template const Img<icl32f> *FFTOp::adapt_source(const Img<icl32f> *src);
template const Img<icl64f> *FFTOp::adapt_source(const Img<icl64f> *src);

void FFTOp::apply(const ImgBase *src, ImgBase **dst){
	FFTOp_DEBUG("f: apply");
	ICLASSERT_RETURN(src);
	ICLASSERT_RETURN(dst);
	if(!*dst){
		depth srcDepth = (src->getDepth() == depth64f) ? depth64f : depth32f;
		*dst = imgNew(srcDepth, Size(0,0), formatMatrix,Rect(0,0,0,0));
	}
	depth dstDepth = ((*dst)->getDepth() == depth64f) ? depth64f : depth32f;
	switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                            \
		case depth##D:														\
		src=adapt_source(src->asImg<icl##D>());						\
		break;
	ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
	}

	//if TWO_CHANNEL_? is needed, the channelcount of the destinationimage is two times channelcount of sourceimmage
	int nChannels = (m_data->m_rm == TWO_CHANNEL_COMPLEX || m_data->m_rm == TWO_CHANNEL_MAGNITUDE_PHASE)
															? 2*src->getChannels() : src->getChannels();

	if(!prepare(dst,dstDepth,src->getSize(), formatMatrix, nChannels,
			Rect(Point::null,src->getSize()), src->getTime())){
		throw ICLException("preparation of  destinationimage failed");
	}
	FFTOp_DEBUG("size of src:"<<src->getSize().width << "  " << src->getSize().height);
	switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                            \
		case depth##D:														\
		if(dstDepth == depth32f){                                          \
			apply_internal(*src->asImg<icl##D>(),*(*dst)->asImg<icl32f>(),m_data->m_buf32f,m_data->m_dstBuf32f); \
		}else{                                                              \
			apply_internal(*src->asImg<icl##D>(),*(*dst)->asImg<icl64f>(),m_data->m_buf64f,m_data->m_dstBuf64f); \
		}                                                                   \
		break;
	ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
	}
}
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/IFFTOp.cpp                               **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLFilter/IFFTOp.h>
//#define IFFTOp_DEBUG(X) std::cout << X << std::endl;
#define IFFTOp_DEBUG(X)
using namespace std;
using namespace icl;
using namespace icl::fft;
namespace icl{

struct IFFTOp::Data{
	ResultMode m_rm;
	SizeAdaptionMode m_sam;
	Rect m_roi;
	bool m_join;
	bool m_ifftshift;
	bool m_forceIDFT;
	ImgBase *m_adaptedSource;
	DynMatrix<std::complex<float> > m_buf32f;
	DynMatrix<std::complex<double> > m_buf64f;
	DynMatrix<std::complex<float> > m_dstBuf32f;
	DynMatrix<std::complex<double> > m_dstBuf64f;
	Data(ResultMode rm, SizeAdaptionMode sam, Rect roi,bool join, bool ifftshift, bool forceIDFT):
		m_rm(rm),m_sam(sam),m_roi(roi),m_join(join),m_ifftshift(ifftshift),m_forceIDFT(forceIDFT),m_adaptedSource(0){}
	~Data(){
		ICL_DELETE(m_adaptedSource);
	}
};

IFFTOp::IFFTOp(ResultMode rm, SizeAdaptionMode zam, Rect roi, bool join, bool ifftshift,bool forceIDFT):
						m_data(new IFFTOp::Data(rm, zam, roi, join, ifftshift,forceIDFT)){
}

void IFFTOp::setResultMode(ResultMode rm){
	m_data->m_rm = rm;
}

int IFFTOp::getResultMode(){
	return m_data->m_rm;
}

void IFFTOp::setSizeAdaptionMode(SizeAdaptionMode sam){
	m_data->m_sam = sam;
}

int IFFTOp::getSizeAdaptionMode(){
	return m_data->m_sam;
}

void IFFTOp::setForceIDFT(bool pForceDFT){
	m_data->m_forceIDFT = pForceDFT;
}

bool IFFTOp::getForceIDFT(){
	return m_data->m_forceIDFT;
}

void IFFTOp::setJoinMatrix(bool pJoin){
	m_data->m_join = pJoin;
}

bool IFFTOp::getJoinMatrix(){
	return m_data->m_join;
}

void IFFTOp::setROI(Rect roi){
	m_data->m_roi = roi;
}

Rect IFFTOp::getRoi(){
	return m_data->m_roi;
}

IFFTOp::~IFFTOp(){
	delete m_data;
}

template<typename T>
void IFFTOp::apply_inplace_ifftshift(DynMatrix<T> &mat){
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
				for(unsigned int y=0;y<rows;++y){
					for(unsigned int x=cols-1;x>0;--x){
						std::swap(mat.operator ()(x,y),mat.operator ()(x-1,y));
					}
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
				for(unsigned int x=0;x<cols;++x){
					for(unsigned int y=rows-1;y>0;--y){
						std::swap(mat.operator ()(x,y),mat.operator ()(x,y-1));
					}
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
				for(unsigned int i=dim-1;i>dim2;--i){
					std::swap(dat[i],dat[i-1]);
				}
			}
			dim = rows;
			dim2 = dim/2;
			for(unsigned int k=0;k<cols;++k){
				for(unsigned int i=0;i<dim2;++i){
					std::swap(mat.operator ()(k,i),mat.operator ()(k,i+dim2));
				}
				for(unsigned int i=dim-1;i>dim2;--i){
					std::swap(mat.operator ()(k,i),mat.operator ()(k,i-1));
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
			for(unsigned int i=dim-1;i>dim2;--i){
				std::swap(mat.data()[i],mat.data()[i-1]);
			}
		}
	}
}

template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl8u> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl16u> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl32u> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl16s> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl32s> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl32f> &m);
template void IFFTOp::apply_inplace_ifftshift(DynMatrix<icl64f> &m);

template<class SrcT, class DstT>
void IFFTOp::apply_internal(const Img<SrcT> &src, Img<DstT> &dst,
		DynMatrix<std::complex<DstT> > &buf,
		DynMatrix<std::complex<DstT> > &dstBuf){
	dstBuf.setBounds(dst.getWidth(),dst.getHeight());
	buf.setBounds(dst.getHeight(),dst.getWidth());
	DynMatrix<SrcT> srcMat;
	DynMatrix<complex<DstT> > joinMat(src.getSize().width,src.getSize().height);
	for(int lChannel = 0;lChannel<dst.getChannels();++lChannel){
		IFFTOp_DEBUG("processing channel "<< lChannel);
		if(m_data->m_join){
			joinComplex(src.extractDynMatrix(2*lChannel),src.extractDynMatrix(2*lChannel+1),joinMat);
		} else {
			srcMat = src.extractDynMatrix(lChannel);
		}
		if(m_data->m_forceIDFT){
			if(m_data->m_join){
				idft2D(joinMat,dstBuf,buf);
			}else {
				idft2D(srcMat,dstBuf,buf);
			}
		} else {
			if(m_data->m_join){
				ifft2D(joinMat,dstBuf,buf);
			}else {
				ifft2D(srcMat,dstBuf,buf);
			}
		}
		switch(m_data->m_rm){
		case TWO_CHANNEL_COMPLEX:{
			IFFTOp_DEBUG("two channel complex");
			DynMatrix<DstT> mm1 = dst.extractDynMatrix(2*lChannel);
			DynMatrix<DstT> mm2 = dst.extractDynMatrix(2*lChannel+1);
			split_complex(dstBuf,mm1,mm2);
			break;}
		case IMAG_ONLY:{
			IFFTOp_DEBUG("imag only");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			imagpart(dstBuf,mm);
			break;}
		case REAL_ONLY:{
			IFFTOp_DEBUG("real only");
			DynMatrix<DstT> mm = dst.extractDynMatrix(lChannel);
			realpart(dstBuf,mm);
			break;
		}
		}
	}
}

template void IFFTOp::apply_internal(const Img<icl8u> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl16s> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl32s> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl32f> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl64f> &src, Img<icl32f> &dst,
		DynMatrix<std::complex<icl32f> > &buf, DynMatrix<std::complex<icl32f> > &dstBuf);

template void IFFTOp::apply_internal(const Img<icl8u> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl16s> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl32s> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl32f> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);
template void IFFTOp::apply_internal(const Img<icl64f> &src, Img<icl64f> &dst,
		DynMatrix<std::complex<icl64f> > &buf, DynMatrix<std::complex<icl64f> > &dstBuf);

template<class T>
const Img<T> *IFFTOp::adapt_source(const Img<T> *src){
	if(m_data->m_ifftshift){
		for(int i=0;i<src->getChannels();++i){
			DynMatrix<T> t = src->extractDynMatrix(i);
			apply_inplace_ifftshift(t);
		}
	}
	switch(m_data->m_sam){
	case NO_SCALE:{
		return src;
	}
	case PAD_REMOVE:{
		m_data->m_adaptedSource = new Img<T>(Size(m_data->m_roi.width,m_data->m_roi.height),
				src->getChannels(), formatMatrix);
		for(int i=0;i<src->getChannels();++i){
			DynMatrix<T> m = ((m_data->m_adaptedSource)->asImg<T>())->extractDynMatrix(i);
			DynMatrix<T> m2 =  src->extractDynMatrix(i);
			for(int k=0;k<m_data->m_roi.height;++k){
				for(int j=0;j<m_data->m_roi.width;++j){
					m.operator()(j,k) = m2.operator()(j+m_data->m_roi.x,k+m_data->m_roi.y);
				}
			}
		}
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
	}
	case SCALE_UP:{
		int newHeight = nextPowerOf2(src->getHeight());
		int newWidth = nextPowerOf2(src->getWidth());
		m_data->m_adaptedSource = src->scaledCopy(Size(newWidth,newHeight),icl::interpolateLIN);
		m_data->m_adaptedSource->detach(-1);
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
	}
	case SCALE_DOWN:{
		int newHeight = priorPowerOf2(src->getHeight());
		int newWidth = priorPowerOf2(src->getWidth());
		m_data->m_adaptedSource = src->scaledCopy(Size(newWidth,newHeight),icl::interpolateRA);
		m_data->m_adaptedSource->detach(-1);
		return reinterpret_cast<Img<T>*>(m_data->m_adaptedSource);
	}
	default:{
		return src;
	}
	}
}

template const Img<icl8u> *IFFTOp::adapt_source(const Img<icl8u> *src);
template const Img<icl16s> *IFFTOp::adapt_source(const Img<icl16s> *src);
template const Img<icl32s> *IFFTOp::adapt_source(const Img<icl32s> *src);
template const Img<icl32f> *IFFTOp::adapt_source(const Img<icl32f> *src);
template const Img<icl64f> *IFFTOp::adapt_source(const Img<icl64f> *src);


void IFFTOp::apply(const ImgBase *src, ImgBase **dst){
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
	int nChannels = src->getChannels();
	if(m_data->m_join &&(m_data->m_rm == IMAG_ONLY || m_data->m_rm == REAL_ONLY)) {
		nChannels = src->getChannels()/2;
	} else if(m_data->m_rm == TWO_CHANNEL_COMPLEX){
		if(!(m_data->m_join)){
			nChannels = src->getChannels()*2;
		}
	}
	if(!prepare(dst,dstDepth,src->getSize(), formatMatrix, nChannels,
			Rect(Point::null,src->getSize()), src->getTime())){
		throw ICLException("preparation of  destinationimage failed");
	}
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

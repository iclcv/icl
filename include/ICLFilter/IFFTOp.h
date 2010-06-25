/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/IFFTOp.h                             **
** Module : ICLFilter                                              **
** Authors: Christian Groszewski, Christof Elbrechter              **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_IFFTOP_H_
#define ICL_IFFTOP_H_
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLUtils/FFTUtils.h>
#include <cmath>
#include <complex>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>
namespace icl{
/**
 This class implements the unary operator for the inverse fast and discrete 2D fourier transformation.
  As known the ifft can only be applied if the datasize is a power of 2.
  This implementation uses the ifft as far as it can be applied and switches to the idft,
  so you can use it if datasize is not a power of 2 too. If MKL or IPP is available, IFFTOp tries
  to use it if possible.*/
class IFFTOp : public UnaryOp{

private:
	///Forwarddeklaration.
	class Data;

	/// Class for internal params and buffers.
	Data *m_data;

	//Applies ifft/idft on all channel of sourceimage.
	/**Called by apply. Aplies ifft/idft and resultmode.
	   Possible sourceparam is: Img<icl8u>, Img<icl16s>, Img<icl32s>,
	   Img<icl32f>, Img<icl64f>.
	   Possible destinationparam is: Img<icl32f> and Img<icl64f>.*/
	template<class SrcT, class DstT>
	void apply_internal(const Img<SrcT> &src, Img<DstT> &dst,
			icl::DynMatrix<std::complex<DstT> > &buf, icl::DynMatrix<std::complex<DstT> > &dstBuf);

	//Adapts sourceimage before ifft/idft computation.
	/**Called by apply. Adapts sourceimage to specified values(scaling, padding on so on).
	   Possible sourceparam is: Img<icl8u>, Img<icl16s>, Img<icl32s>,
		  Img<icl32f>, Img<icl64f>*/
	template<class T>
	const Img<T> *adapt_source(const Img<T> *src);

	//Applies an inplace ifftshift  after computation of the ifft.
	/** Applies inplace ifftshift on destinationimage after ifftcomputation.
		 Possible sourceparam is: Img<icl32f> and Img<icl64f>*/
	template<typename T>
	void apply_inplace_ifftshift(DynMatrix<T> &m);

public:

	///Modes how the sourceimage is to adapt before fftcomputation.
	/**Several sizeadaptionmodes for sourceimage.*/
	enum SizeAdaptionMode{
		NO_SCALE,//!< sourceimage stays as is
		PAD_REMOVE,//!< removes PAD_MIRROR,PAD_ZERO and PAD_COPY from the sourceimage(new size is given from contructorparameter roi)
		SCALE_UP,//!< zooms to next higher power of 2 of originsize, or origanalsize if it is power of 2
		SCALE_DOWN//!< zooms to next lower power of 2 of originsize, or origanalsize if it is power of 2
	};


	//Modes how the destinationimage will be created.
	/**Several resultmodes for destinationimage.*/
	enum ResultMode{
		REAL_ONLY,//!< alternates real- and imaginarypart of ifftcomputation
		IMAG_ONLY,//!< imaginarypart of ifftcomputation
		TWO_CHANNEL_COMPLEX//!< realpart of ifftcomputation
	};

	///Creates a new IFFTOp-object.
	/**Constructor. Params can be changed later.
	   @param rm the resultmode
	   @param sam the sizeadaptionmode before applying IFFTOp
	   @param roi determinates the size if removing padding is needed
	   @param join wether to join to channel to one complex for the ifftcomputation or not
	   @param ifftshift undo fftshift before the isfftcomputation or not
	   @param forceIDFT wether to apply idft or ifft*/
	IFFTOp(ResultMode rm=REAL_ONLY, SizeAdaptionMode sam=NO_SCALE,
			Rect roi=Rect(0,0,0,0),bool join=true, bool ifftshift=true, bool forceIDFT=false);

	/**Destructor*/
	~IFFTOp();

	///Sets if two channels shall be joined to one complex.
	/**@param pJoin true if channels shall be joined to one complex*/
	void setJoinMatrix(bool pJoin);

	///Returns if two channels shall be joined to one complex.
	/**@return the current value*/
	bool getJoinMatrix();

	///Sets the roi.
	/**@param roi the new roi*/
	void setROI(Rect roi);

	///Returns the current roi
	/**@return roi the current roi*/
	Rect getRoi();

	///Sets the resultmode.
	/*@param rm the resultmode to be set*/
	void setResultMode(ResultMode rm);

	///Returns the resultmode as int.
	/**@return the current resultmode.*/
	int getResultMode();

	///Sets the sizeadaptionmode.
	/**@param sam the sizeadaptionmode to be set*/
	void setSizeAdaptionMode(SizeAdaptionMode sam);

	///Returns the sizeadaptionmode.
	/**@return the current sizeadationmode*/
	int getSizeAdaptionMode();

	///Returns true if the discrete inverse fourier transfarmation shall be used, else false.
	 /**@return true for idft, false for ifft*/
	bool getForceIDFT();

	///Set wether to force the discrete inverse fourier transformation or to use the fast inverse fourier transformation
	/**@param pForceDFT*/
	void setForceIDFT(bool pForceDFT);

	///Call this method to start ifftcomputation.
	/**Applies IFFTOp on src and dst.
	  @param *src pointer to sourceimage
	  @param **dst pointer to pointer to destinationimage*/
	virtual void apply(const ImgBase *src, ImgBase **dst);

	/// Import unaryOps apply function without destination image
	UnaryOp::apply;
};
}

#endif /* ICL_IFFTOP_H_ */

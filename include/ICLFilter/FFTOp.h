/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/FFTOp.h                              **
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

#ifndef ICL_FFTOP_H_
#define ICL_FFTOP_H_
#include <ICLUtils/FFTUtils.h>
#include <ICLCore/ImgBase.h>
#include <ICLQuick/Common.h>
#include <ICLFilter/UnaryOp.h>
#include <cmath>

namespace icl{


/**
 This class implements the unary operator for the fast and discrete 2D fourier transformation.
  As known the fft can only be applied if the datasize is a power of 2.
  This implementation uses the fft as far as it can be applied and switches to the dft,
  so you can use it if datasize is not a power of 2 too. If MKL or IPP is available, FFTOp tries
  to use it if possible.

  \section EXAMPLE Simple FFT-computation demo

    <TABLE border=0><TR><TD>
    \code
    #include <ICLQuick/Common.h>

    GUI gui;

    int main(int n, char **args){
	painit(n,args,"-input|-i(filename)",init);
	Img32 src = load(*pa("-i"));
    FFTOp fftop(FFTOp::LOG_POWER_SPECTRUM,FFTOp::NO_SCALE);
	const ImgBase *dst = fftop.apply(&src);
    // note: icl-xv must be within your PATH
	show(norm(cvt(dst)));
    }


    \endcode

    </TD><TD>
    \image html fft.jpg
    </TD></TR></TABLE>
    Save the above sourcecode to a file, compile and link it.
    Execute it: ./appname -input pic.jpg

	\section RM Result modes
	These modes configure the way the destinationimage (the result of the fft) shall
	be returned. Different results like logpowerspectrum, real part or
	phase and  magnitude can be obtained. (see icl::FFTOp::ResultMode)

	\section SM Sizeadaption modes
	These modes configure the sourceimage before processing the fft.
	The sourceimage can be scaled up, so that the datasize is a power of 2
	(this is needed if you want to use the IPP acceleration or the faster
	part of the fallback). You can also scale the image down, leave it as it is
	or create a border and fill it with several methods. (see icl::FFTOp::SizeAdaptionMode)

	\section IPP_MKL_ACCEL Intel Ipp/Intel MKL acceleration
	If you own the Intel IPP or MKL library, the computation of the fft
	can be accelarated by using it. The IPP functions assume a datasize of power of 2 (see icl::FFTOp::SizeAdaptionMode).
	If IPP is available on you system and the datasize is a power of 2, it will be used
	automatacally. If MKL is available on your system, it will be used if IPP is not available or
	the datasize is not a power of 2. If IPP and MKL are not available, the fallback
	will be used. The MKL and fallback functions support also datasizes which are not power of 2.
	Supported source and destinationtypes:
	<TABLE border=2>
	<TR><TD>dst/src</TD><TD>icl8u</TD><TD>icl16s</TD><TD>icl32s</TD><TD>icl32f</TD><TD>icl64f</TD></TR>
	<TR><TD>icl32f</TD><TD>IPP/MKL</TD><TD>IPP/MKL</TD><TD>IPP/MKL</TD><TD>IPP/MKL</TD><TD>MKL</TD></TR>
	<TR><TD>icl64f</TD><TD>MKL</TD><TD>MKL</TD><TD>MKL</TD><TD>MKL</TD><TD>MKL</TD></TR>
	</TABLE>
	\section BENCH Benchmark results
	The following benchmark has been done on Intel Dual Core 2.4GHz CPU on a PC with 2GiB RAM.
	Since IPP and MKL provide their fft results in a packed format, the time for unpacking is included in this benchmarks.
	Also IPP and MKL do not support every possible sourcetype, so time for typeconversation is also included if necessary.
	All times are provided in ms per channel.:
	<TABLE border=2>
	<TR><TD>lib/size (format)</TD><TD>QVGA(320x240)</TD><TD>HVGA(480x320)</TD><TD>VGA(640x480)</TD><TD>SVGA(800x600)</TD><TD>XGA(1024x768)</TD><TD>DSVGA(1200x800)</TD><TD>512x512</TD><TD>1024x1024</TD></TR>
	<TR><TD>IPP</TD><TD> - </TD><TD> - </TD><TD> - </TD><TD> - </TD><TD> - </TD><TD> - </TD><TD> 4 </TD><TD> 21 </TD></TR>
	<TR><TD>MKL</TD><TD> 1 </TD><TD> 2 </TD><TD> 7 </TD><TD> 12 </TD><TD> 36 </TD><TD>26</TD><TD> 6 </TD><TD> 58 </TD></TR>
	<TR><TD>FB</TD><TD> 214 </TD><TD> 440 </TD><TD> 896 </TD><TD> 5509 </TD><TD> 1395 </TD><TD> 11064 </TD><TD> 438 </TD><TD> 1915 </TD></TR>
	</TABLE>
  */
class FFTOp : public UnaryOp{

private:
	///Forwarddeklaration.
	class Data;

	///Class for internal params and buffers.
	Data *m_data;

	//Applies fft/dft on all channel of sourceimage.
	/**Called by apply. Applies fft/dft and resultmode.
	  Possible sourceparam is: Img<icl8u>, Img<icl16s>, Img<icl32s>,
	  Img<icl32f>, Img<icl64f>.
	  Possible destinationparam is: Img<icl32f> and Img<icl64f>.*/
	template<class SrcT, class DstT>
	void apply_internal(const Img<SrcT> &src, Img<DstT> &dst,
			icl::DynMatrix<std::complex<DstT> > &buf, icl::DynMatrix<std::complex<DstT> > &dstBuf);

	//Adapts sourceimage before fft/dft computation.
	/**Called by apply. Adapts sourceimage to specified
	 values(scaling, padding on so on).Possible sourceparam is: Img<icl8u>, Img<icl16s>, Img<icl32s>,
	  Img<icl32f>, Img<icl64f>*/
	template<class T>
	const Img<T> *adapt_source(const Img<T> *src);

	//Applies an inplace fftshift  after computation of the fft (if possible).
	/** Applies inplace fftshift on destinationimage after fftcomputation.
	 Possible sourceparam is: Img<icl32f> and Img<icl64f>*/
	template<typename T>
	void apply_inplace_fftshift(DynMatrix<T> &m);

public:
	///Modes how the sourceimage is to adapt before fftcomputation.
	/**Several sizeadaptionmodes for sourceimageadaption*/
	enum SizeAdaptionMode{
		NO_SCALE,//!< sourceimage stays as is
		PAD_ZERO,//!< creates a border with zeros around the sourceimage(new size is next power of 2 after originsize, or origanalsize if it is power of 2)
		PAD_COPY,//!< continues the image with copies the sourceimage(new size is next power of 2 after originsize, or origanalsize if it is power of 2)
		PAD_MIRROR,//!< mirrors the image on the edges of the sourceimage(new size is next power of 2 after originsize, or origanalsize if it is power of 2)
		SCALE_UP,//!< zooms to next higher power of 2 of originsize, or origanalsize if it is power of 2
		SCALE_DOWN //!< zooms to next lower power of 2 of originsize, or origanalsize if it is power of 2
	};

	///Modes how the destinationimage will be created.
	/**Several resultmodes for destinationimage.*/
	enum ResultMode{
		TWO_CHANNEL_COMPLEX,//!< alternates real- and imaginarypart of fftcomputation
		IMAG_ONLY,//!< imaginarypart of fftcomputation
		REAL_ONLY,//!< realpart of fftcomputation
		POWER_SPECTRUM,//!< powerspectrum of fftcomputation
		LOG_POWER_SPECTRUM,//!< logpowerspectrum of fftcomputation
		MAGNITUDE_ONLY,//!< magnitude of fftcomputation
		PHASE_ONLY,//!< phase of fftcomputation
		TWO_CHANNEL_MAGNITUDE_PHASE//!< alternates magnitude and phase of fftcomputation
	};

	///Creates a new FFTOp-object.
	/**Constructor. Params can be changed later.
	  @param rm the resultmode
	  @param sam the sizeadaptionmode before applying FFTOp
	  @param fftshift wether to apply fftshift to destinationimage after fftcomputation or not*
	  @param forceDFT wether to apply dft or fft*/
	FFTOp(ResultMode rm=LOG_POWER_SPECTRUM, SizeAdaptionMode sam=NO_SCALE,
			bool fftshift=true,bool forceDFT=false);

	/**Destructor*/
	~FFTOp();

	///Sets the resultmode.
	/**@param rm the resultmode to be set*/
	void setResultMode(ResultMode rm);

	///Returns the resultmode as int.
	/**@return the current resultmode.*/
	int getResultMode();

	///Sets the sizeadaptionmode.
	/**@param sm the sizeadaptionmode to be set*/
	void setSizeAdaptionMode(SizeAdaptionMode sam);

	///Returns the sizeadaptionmode.
	/**@return the current sizeadationmode*/
	int getSizeAdaptionMode();

	///Returns true if the diskrete fourier transfarmation shall be used, else false.
	/**@return true if dft should be used, false if fft should be used*/
	bool getForceDFT();

	///Set wether to force the diskrete fourier transformation or to use the fast fourier transformation
	/**@param pForceDFT*/
	void setForceDFT(bool pForceDFT);

	///Set wether to fftshift the destinationimage or not
	/**@param pFFTShift true if the destinationimage should be shifted, else false*/
	void setFFTShift(bool pFFTShift);

	///Returns the current value for fftshift.
	/**@return true if destinationimage  is to be fftshifted, else false*/
	bool getFFTShift();

	///Call this method to start fftcomputation.
	/**Applies FFTOp on src and dst.
	  @param *src pointer to sourceimage
	  @param **dst pointer to pointer to destinationimage*/
	virtual void apply(const ImgBase *src, ImgBase **dst);

	/// Import unaryOps apply function without destination image
	UnaryOp::apply;


};
}

#endif /* ICL_FFTOP_H_ */

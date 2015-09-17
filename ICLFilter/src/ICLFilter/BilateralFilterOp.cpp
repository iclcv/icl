/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLFilter/src/ICLFilter/BilateralOp.cpp                **
 ** Module : ICLFilter                                              **
 ** Authors: Tobias Roehlig                                         **
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

#include "BilateralFilterOp.h"

#include <ICLFilter/ICLFilterConfig.h>

#include <fstream>

#include <ICLCore/CCFunctions.h>

#include <ICLUtils/CLIncludes.h>
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>

#include <ICLUtils/StackTimer.h>


namespace icl {

namespace filter {

// /////////////////////////////////////////////////////////////////////////////////////////////////

struct BilateralFilterOp::Impl {

	Impl() {}
	virtual ~Impl() {}
	virtual void apply(const core::ImgBase *in, core::ImgBase **out, int radius, float sigma_s, float sigma_r, bool _use_lab) = 0;

};

// /////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ICL_HAVE_OPENCL

struct BilateralFilterOp::GPUImpl : BilateralFilterOp::Impl {
public:

	GPUImpl()
		: width(0), height(0) {

		std::ifstream kernel_stream;
		std::string kernel_name = "/BilateralFilterOp.cl";
		std::string kernel_path = ICLFILTER_OPENCL_KERNEL_PATH+kernel_name;
		kernel_stream.open(kernel_path.c_str());
		if (!kernel_stream.is_open()) {
			throw icl::utils::ICLException(std::string("No kernel file found in \""+kernel_path+"\""));
		}

		program = utils::CLProgram("gpu",kernel_stream);

		rgb_to_lab = program.createKernel("rgbToLABCIE");
		filter[(int)UCHAR_3COLORS] = program.createKernel("bilateral_filter_color");
		filter[(int)UCHAR_SINGLE_CHANNEL] = program.createKernel("bilateral_filter_mono");
		filter[(int)FLOAT_SINGLE_CHANNEL] = program.createKernel("bilateral_filter_mono_float");

		//program.listSelectedDevice();
	}

	~GPUImpl() {}

	void apply(const core::ImgBase *in, core::ImgBase **out, int radius, float sigma_s, float sigma_r, bool _use_lab) {

		int w = in->getWidth();
		int h = in->getHeight();

		bool reset_buffers = this->width != w
							|| this->height != h
							|| this->depth != in->getDepth()
							|| this->channels != in->getChannels();

		this->width = w;
		this->height = h;
		this->depth = in->getDepth();
		this->channels = in->getChannels();

		if (in->getDepth() == core::depth8u) {
			const core::Img8u &_in = *in->as8u();
			core::Img8u *_out = (*out)->as8u();
			if (_in.getChannels() == 1) {

				core::Channel8u ch_out = (*_out)[0];
				core::Channel8u ch = _in[0];
				_out->setSize(_in.getSize());
				_out->setFormat(_in.getFormat());
				if (reset_buffers) {
					image_buffer_in = program.createImage2D("r",w,h,core::depth8u,&ch(0,0));
					image_buffer_out = program.createImage2D("w",w,h,core::depth8u,0);
				} else {
					image_buffer_in.write(&ch(0,0));
				}
				utils::CLKernel &kernel = filter[UCHAR_SINGLE_CHANNEL];
				kernel.setArgs(image_buffer_in,image_buffer_out,w,h,radius,sigma_s,sigma_r);
				kernel.apply(w,h);
				kernel.finish();
				image_buffer_out.read(&ch_out(0,0));

			} else if(_in.getChannels() == 3) {

				const core::Channel8u r = _in[0];
				const core::Channel8u g = _in[1];
				const core::Channel8u b = _in[2];

				if (reset_buffers) {
					in_r = program.createImage2D("r",w,h,core::depth8u,&r(0,0));
					in_g = program.createImage2D("r",w,h,core::depth8u,&g(0,0));
					in_b = program.createImage2D("r",w,h,core::depth8u,&b(0,0));
					out_r = program.createImage2D("w",w,h,core::depth8u,0);
					out_g = program.createImage2D("w",w,h,core::depth8u,0);
					out_b = program.createImage2D("w",w,h,core::depth8u,0);
					lab_l = program.createImage2D("rw",w,h,core::depth8u,0);
					lab_a = program.createImage2D("rw",w,h,core::depth8u,0);
					lab_b = program.createImage2D("rw",w,h,core::depth8u,0);
				} else {
					in_r.write(&r(0,0));
					in_g.write(&g(0,0));
					in_b.write(&b(0,0));
				}

				utils::CLKernel &kernel = filter[UCHAR_3COLORS];
				if (_use_lab) {
					rgb_to_lab.setArgs(in_r,in_g,in_b,lab_l,lab_a,lab_b);
					rgb_to_lab.apply(w,h);
					rgb_to_lab.finish();
					kernel.setArgs(lab_l,lab_a,lab_b,out_r,out_g,out_b,w,h,radius,sigma_s,sigma_r,(int)_use_lab);
				} else {
					kernel.setArgs(in_r,in_g,in_b,out_r,out_g,out_b,w,h,radius,sigma_s,sigma_r,(int)_use_lab);
				}
				kernel.apply(w,h);
				kernel.finish();

				_out->setSize(_in.getSize());
				_out->setFormat(_in.getFormat());
				core::Channel8u _r = (*_out)[0];
				core::Channel8u _g = (*_out)[1];
				core::Channel8u _b = (*_out)[2];

				out_r.read(&_r(0,0));
				out_g.read(&_g(0,0));
				out_b.read(&_b(0,0));

			} else {
				ERROR_LOG("Wrong number of channels. Expected 3 or 1 channels.");
			}
		} else if(in->getDepth() == core::depth32f && in->getChannels() == 1) {

			const core::Img32f &_in = *in->as32f();
			const core::Channel32f ch = _in[0];
			core::Img32f *_out = (*out)->as32f();
			_out->setSize(_in.getSize());
			_out->setFormat(_in.getFormat());
			core::Channel32f ch_out = (*_out)[0];

			if (reset_buffers) {
				image_buffer_in = program.createImage2D("r",w,h,core::depth32f,&ch(0,0));
				image_buffer_out = program.createImage2D("w",w,h,core::depth32f,0);
			} else {
				image_buffer_in.write(&ch(0,0));
			}
			utils::CLKernel &kernel = filter[FLOAT_SINGLE_CHANNEL];
			kernel.setArgs(image_buffer_in,image_buffer_out,w,h,radius,sigma_s,sigma_r);
			kernel.apply(w,h);
			kernel.finish();
			image_buffer_out.read(&ch_out(0,0));

		} else {
			throw utils::InvalidDepthException(ICL_FILE_LOCATION);
		}
	}

protected:
	enum ImageType {
		FLOAT_SINGLE_CHANNEL,
		UCHAR_SINGLE_CHANNEL,
		UCHAR_3COLORS
	};

	/// Main OpenCL program
	utils::CLProgram program;
	/// All known filters
	std::map<int,utils::CLKernel> filter;
	/// Additional kernel for rgb-to-lab converter
	utils::CLKernel rgb_to_lab;

	//@{
	/// buffers used for image transfer and converting
	utils::CLImage2D image_buffer_in;
	utils::CLImage2D image_buffer_out;
	utils::CLImage2D in_r;
	utils::CLImage2D in_g;
	utils::CLImage2D in_b;
	utils::CLImage2D lab_l;
	utils::CLImage2D lab_a;
	utils::CLImage2D lab_b;
	utils::CLImage2D out_r;
	utils::CLImage2D out_g;
	utils::CLImage2D out_b;
	//@}

	//@{
	/// Internal storage
	int width, height;
	int depth;
	int channels;
	//@}
};

#endif

// /////////////////////////////////////////////////////////////////////////////////////////////////

struct BilateralFilterOp::CPUImpl : public BilateralFilterOp::Impl {
public:
	CPUImpl()
		: BilateralFilterOp::Impl() {}
	~CPUImpl() {}

	void apply(const core::ImgBase *in, core::ImgBase **out, int radius, float sigma_s, float sigma_r, bool _use_lab) {
		ERROR_LOG("NOT IMPLEMENTED! PLEASE USE GPU VERSION!")
	}

protected:
};

// /////////////////////////////////////////////////////////////////////////////////////////////////

BilateralFilterOp::BilateralFilterOp(int radius,
									   float sigma_s, float sigma_r, bool _use_lab, Mode mode)
	: filter::UnaryOp(), utils::Uncopyable(), use_lab(_use_lab), radius(radius),
	  sigma_s(sigma_s), sigma_r(sigma_r), impl(0) {
	init(mode);
}

BilateralFilterOp::BilateralFilterOp(Mode mode)
	: filter::UnaryOp(), utils::Uncopyable(), use_lab(true), radius(2), sigma_s(1), sigma_r(1), impl(0) {
	init(mode);
}

void BilateralFilterOp::init(Mode mode) {
	if (impl)
		delete impl;
	if (mode == GPU) {
		#ifdef ICL_HAVE_OPENCL
			impl = new GPUImpl();
		#else
			WARNING_LOG"OpenCL is not available");
		#endif
	} else if (mode == CPU) {
		impl = new CPUImpl();
	} else if (mode == BEST) {
		#ifdef ICL_HAVE_OPENCL
			impl = new GPUImpl();
		#else
			impl = new CPUImpl();
		#endif
	}
}

BilateralFilterOp::~BilateralFilterOp() {
}

void BilateralFilterOp::apply(const core::ImgBase *in, core::ImgBase **out) throw() {
	impl->apply(in,out,radius,sigma_s,sigma_r,use_lab);
}

} // namespace filter
} // namespace icl


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

#include <ICLFilter/BilateralFilterOp.h>

#include <fstream>

#include <ICLCore/CCFunctions.h>

#include <ICLMath/FixedVector.h>

#include <ICLUtils/CLIncludes.h>
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLImage2D.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>

#include <ICLFilter/OpenCL/BilateralFilterOpKernel.h>

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

		std::string kernel_source = BilateralFilterOpKernelSource;

		program = utils::CLProgram("gpu",kernel_source);

		rgb_to_lab = program.createKernel("rgbToLABCIE");
		filter[(int)UCHAR_3COLORS] = program.createKernel("bilateral_filter_color");
		filter[(int)UCHAR_SINGLE_CHANNEL] = program.createKernel("bilateral_filter_mono");
		filter[(int)FLOAT_SINGLE_CHANNEL] = program.createKernel("bilateral_filter_mono_float");

		program.listSelectedDevice();
	}

	~GPUImpl() {}

	void apply(const core::ImgBase *in, core::ImgBase **out, int radius, float sigma_s, float sigma_r, bool _use_lab) {

		int w = in->getWidth();//in->getROIWidth();//in->getWidth();
		int h = in->getHeight();//in->getROIHeight();//in->getHeight();

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
				_out->setSize(utils::Size(w,h));
				_out->setFormat(_in.getFormat());
//				unsigned char *buff = new unsigned char[w*h];
//				std::copy(ch.beginROI(),ch.endROI(),buff);
				if (reset_buffers) {
					//image_buffer_in = program.createImage2D("r",w,h,core::depth8u,buff);
					image_buffer_in = program.createImage2D("r",w,h,core::depth8u,1,&ch(0,0));
					image_buffer_out = program.createImage2D("w",w,h,core::depth8u,1,0);
				} else {
//					image_buffer_in.write(buff);
					image_buffer_in.write(&ch(0,0));
				}
				//delete [] buff;
				utils::CLKernel &kernel = filter[UCHAR_SINGLE_CHANNEL];
				kernel.setArgs(image_buffer_in,image_buffer_out,w,h,radius,sigma_s,sigma_r);
				kernel.apply(w,h);
				kernel.finish();
				image_buffer_out.read(&ch_out(0,0));

			} else if(_in.getChannels() == 3) {

				const core::Channel8u r = _in[0];
				const core::Channel8u g = _in[1];
				const core::Channel8u b = _in[2];

				/*unsigned char *buff_r = new unsigned char[w*h];
				unsigned char *buff_g = new unsigned char[w*h];
				unsigned char *buff_b = new unsigned char[w*h];
				std::copy(r.beginROI(),r.endROI(),buff_r);
				std::copy(g.beginROI(),g.endROI(),buff_g);
				std::copy(b.beginROI(),b.endROI(),buff_b);*/
				if (reset_buffers) {
					/*in_r = program.createImage2D("r",w,h,core::depth8u,buff_r);
					in_g = program.createImage2D("r",w,h,core::depth8u,buff_g);
					in_b = program.createImage2D("r",w,h,core::depth8u,buff_b);*/
					in_r = program.createImage2D("r",w,h,core::depth8u,1,&r(0,0));
					in_g = program.createImage2D("r",w,h,core::depth8u,1,&g(0,0));
					in_b = program.createImage2D("r",w,h,core::depth8u,1,&b(0,0));
					out_r = program.createImage2D("w",w,h,core::depth8u,1,0);
					out_g = program.createImage2D("w",w,h,core::depth8u,1,0);
					out_b = program.createImage2D("w",w,h,core::depth8u,1,0);
					lab_l = program.createImage2D("rw",w,h,core::depth8u,1,0);
					lab_a = program.createImage2D("rw",w,h,core::depth8u,1,0);
					lab_b = program.createImage2D("rw",w,h,core::depth8u,1,0);
				} else {
					/*in_r.write(buff_r);
					in_g.write(buff_g);
					in_b.write(buff_b);*/
					in_r.write(&r(0,0));
					in_g.write(&g(0,0));
					in_b.write(&b(0,0));
				}
				/*delete [] buff_r;
				delete [] buff_g;
				delete [] buff_b;*/

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

				_out->setSize(utils::Size(w,h));
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
			_out->setSize(utils::Size(w,h));
			_out->setFormat(_in.getFormat());
			core::Channel32f ch_out = (*_out)[0];

			//float *buff = new float[w*h];
			//std::copy(ch.beginROI(),ch.endROI(),buff);
			if (reset_buffers) {
				//image_buffer_in = program.createImage2D("r",w,h,core::depth32f,buff);
				image_buffer_in = program.createImage2D("r",w,h,core::depth32f,1,&ch(0,0));
				image_buffer_out = program.createImage2D("w",w,h,core::depth32f,1,0);
			} else {
				//image_buffer_in.write(buff);
				image_buffer_in.write(&ch(0,0));
			}
			//delete [] buff;
			utils::CLKernel &kernel = filter[FLOAT_SINGLE_CHANNEL];
			kernel.setArgs(image_buffer_in,image_buffer_out,w,h,radius,sigma_s,sigma_r);
			kernel.apply(w,h);
			kernel.finish();
			image_buffer_out.read(&ch_out(0,0));

		} else {
			throw utils::InvalidDepthException(ICL_FILE_LOCATION);
		}
	}

	void apply2(const core::ImgBase *in, core::ImgBase **out, int radius, float sigma_s, float sigma_r, bool _use_lab) {

		int w = in->getWidth();//in->getROIWidth();//in->getWidth();
		int h = in->getHeight();//in->getROIHeight();//in->getHeight();

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
				_out->setSize(utils::Size(w,h));
				_out->setFormat(_in.getFormat());
				if (reset_buffers) {
					image_buffer_in = program.createImage2D("r",w,h,core::depth8u,1,&ch(0,0));
					image_buffer_out = program.createImage2D("w",w,h,core::depth8u,1,0);
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
					in_r = program.createImage2D("r",w,h,core::depth8u,1,&r(0,0));
					in_g = program.createImage2D("r",w,h,core::depth8u,1,&g(0,0));
					in_b = program.createImage2D("r",w,h,core::depth8u,1,&b(0,0));
					out_r = program.createImage2D("w",w,h,core::depth8u,1,0);
					out_g = program.createImage2D("w",w,h,core::depth8u,1,0);
					out_b = program.createImage2D("w",w,h,core::depth8u,1,0);
					lab_l = program.createImage2D("rw",w,h,core::depth8u,1,0);
					lab_a = program.createImage2D("rw",w,h,core::depth8u,1,0);
					lab_b = program.createImage2D("rw",w,h,core::depth8u,1,0);
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

				_out->setSize(utils::Size(w,h));
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
			_out->setSize(utils::Size(w,h));
			_out->setFormat(_in.getFormat());
			core::Channel32f ch_out = (*_out)[0];
			if (reset_buffers) {
				image_buffer_in = program.createImage2D("r",w,h,core::depth32f,1,&ch(0,0));
				image_buffer_out = program.createImage2D("w",w,h,core::depth32f,1,0);
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

	template<typename TYPE, int n_ch>
	void filter(core::Img<TYPE> const &in, core::Img<TYPE> &out, int radius, float sigma_s, float sigma_r) {

		/*int w = in.getWidth();
		int h = in.getHeight();

		int channel = in.getChannels();
		core::Channel<TYPE> _channel[n_ch];
		for(int i = 0; i < channel; ++i) {
			_channel[i] = in[i];
		}

		for(int y = 0; y < h; ++y) {
			for(int x = 0; x < w; ++x) {

				int tlx = max(x-radius, 0);
				int tly = max(y-radius, 0);
				int brx = min(x+radius, w);
				int bry = min(y+radius, h);

				float sum = 0.f;
				float wp = 0.f;

				icl::math::FixedColVector<n_ch> cur();

				for(int i=tlx; i< brx; i++) {
					for(int j=tly; j<bry; j++) {

						//float4 d4 = read_imagef(in,sampler,coords2);
						float d = ;//d4.x;
						float delta_depth = (src_depth - d) * (src_depth - d);
						float weight = native_exp( -(delta_dist / s2 + delta_depth / r2) ); //cost
						sum += weight * d;
						wp += weight;
					}
				}

			}
		}*/

	}

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
			WARNING_LOG("OpenCL is not available");
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


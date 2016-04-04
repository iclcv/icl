/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BilateralOp.h                  **
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

#pragma once

#include <ICLFilter/UnaryOp.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>

namespace icl {

namespace filter {

/**
 * @brief BilateralFilterICL class Gaussian bilateral filtering
 * Implements the gaussian bilateral filtering like described in
 * "A Fast Approximation of the Bilateral Filter using a Signal Processing Approach"
 * (http://people.csail.mit.edu/sparis/publi/2006/tr/Paris_06_Fast_Bilateral_Filter_MIT_TR_low-res.pdf)
 * on the GPU using OpenCL (no CPU-backend at the moment).
 */
class ICLFilter_API BilateralFilterOp : public filter::UnaryOp, public utils::Uncopyable {

public:

	enum Mode {BEST, GPU, CPU};
	/**
	 * @brief BilateralFilterICL Standard constructor
	 */
	BilateralFilterOp(Mode mode = BEST);
	/**
	 * @brief BilateralFilterICL Custom constructor to init radius, sigma_s and sigma_r
	 * @param radius kernel radius
	 * @param sigma_s sigma_s component
	 * @param sigma_r sigma_r component
	 */
	BilateralFilterOp(int radius, float sigma_s, float sigma_r, bool _use_lab = true, Mode mode = BEST);
	/// Destructor
	virtual ~BilateralFilterOp();

	// We make use of the apply functions:
	using UnaryOp::apply;

	/**
	 * Applies the bilateral filter operation. Supported are grayvalue-images, mono float images and color images like rgb.
	 * Internally, the color images are converted to Lab-color-space for filtering if the flag use_lab is set to true. It will use
	 * the given image format instead. The output image will
	 * have the same size and format like the input image. There is no ROI-support at the moment.
	 * @brief apply Applies the bilateral filter operation.
	 * @param in Image to filter.
	 * @param out Filter result (must be of the same size and format)
	 */
	void apply(const core::ImgBase *in, core::ImgBase **out) throw();

	/// Sets the kernel radius
	void setRadius(int radius) { this->radius = radius; }
	/// Sets the sigma_s component
	void setSigmaS(float sigmaS) { this->sigma_s = sigmaS; }
	/// Sets the sigma_r component
	void setSigmaR(float sigmaR) { this->sigma_r = sigmaR; }
	/// Sets whether to use lab-color space or rgb
	void setUseLAB(bool _use_lab) { this->use_lab = _use_lab; }

	int getRadius() { return this->radius; }
	float getSigmaS() { return this->sigma_s; }
	float getSigmaR() { return this->sigma_r; }

protected:

	bool use_lab;
	/// Kernel radius
	int radius;
	/// Spatial extent of the kernel, size of the considered neighborhood
	float sigma_s;
	/// “Minimum” amplitude of an edge
	float sigma_r;

private:

	struct Impl;	//!< internal data type
	struct GPUImpl;	//!< internal data type derived from Impl
	struct CPUImpl;	//!< internal data type derived from Impl
	Impl *impl;

	/**
	 * @brief init internal initialization function
	 */
	void init(Mode mode);

};

} // namespace filter
} // namespace icl

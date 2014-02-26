/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLCV/src/ICLCV/CLSurfLib.cpp                          **
 ** Module : ICLCV                                                  **
 ** Authors: Christof Elbrechter                                    **
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

/****************************************************************************\ 
 * Copyright (c) 2011, Advanced Micro Devices, Inc.                           *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * Redistributions of source code must retain the above copyright notice,     *
 * this list of conditions and the following disclaimer.                      *
 *                                                                            *
 * Redistributions in binary form must reproduce the above copyright notice,  *
 * this list of conditions and the following disclaimer in the documentation  *
 * and/or other materials provided with the distribution.                     *
 *                                                                            *
 * Neither the name of the copyright holder nor the names of its contributors *
 * may be used to endorse or promote products derived from this software      *
 * without specific prior written permission.                                 *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 * If you use the software (in whole or in part), you shall adhere to all     *
 * applicable U.S., European, and other export laws, including but not        *
 * limited to the U.S. Export Administration Regulations ('EAR'), (15 C.F.R.  *
 * Sections 730 through 774), and E.U. Council Regulation (EC) No 1334/2000   *
 * of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR, you       *
 * hereby certify that, except pursuant to a license granted by the United    *
 * States Department of Commerce Bureau of Industry and Security or as        *
 * otherwise permitted pursuant to a License Exception under the U.S. Export  *
 * Administration Regulations ("EAR"), you will not (1) export, re-export or  *
 * release to a national of a country in Country Groups D:1, E:1 or E:2 any   *
 * restricted technology, software, or source code you receive hereunder,     *
 * or (2) export to Country Groups D:1, E:1 or E:2 the direct product of such *
 * technology or software, if such foreign produced direct product is subject *
 * to national security controls as identified on the Commerce Control List   *
 *(currently found in Supplement 1 to Part 774 of EAR).  For the most current *
 * Country Group listings, or for additional information about the EAR or     *
 * your obligations under those regulations, please refer to the U.S. Bureau  *
 * of Industry and Security's website at http://www.bis.doc.gov/.             *
 \****************************************************************************/

#include <ICLCV/CLSurfLib.h>
#include <ICLCV/CLSurfLibKernels.h>

#include <ICLCore/CCFunctions.h>

#include <CL/cl.h>
#include "cstdio"
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <sstream>

#define DESC_SIZE 64

#define MAX_ERR_VAL 64

namespace icl {
using namespace utils;
using namespace math;
using namespace core;
namespace cv {
namespace clsurf {

typedef struct {
	int x;
	int y;
} int2;

typedef struct {
	float x;
	float y;
} float2;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} float4;

class ResponseLayer;
class FastHessian;

struct Surf::Data {
	IpVec m_outputBuffer;

	icl::core::Img32f m_grayBuffer;

	// The actual number of ipoints for this image
	int numIpts;

	//! The amount of ipoints we have allocated space for
	int maxIpts;

	//! A fast hessian object that will be used for detecting ipoints
	FastHessian* fh;

	//! The integral image
	CLBuffer d_intImage;
	CLBuffer d_tmpIntImage;   // orig orientation
	CLBuffer d_tmpIntImageT1; // transposed
	CLBuffer d_tmpIntImageT2; // transposed

	//! Number of surf descriptors
	CLBuffer d_length;

	//! Array of Descriptors for each Ipoint
	CLBuffer d_desc;

	//! Orientation of each Ipoint an array of float
	CLBuffer d_orientation;

	CLBuffer d_gauss25;

	CLBuffer d_id;

	CLBuffer d_i;

	CLBuffer d_j;

	//! Position data on the host
	float2* pixPos;

	//! Scale data on the host
	float* scale;

	//! Laplacian data on the host
	int* laplacian;

	//! Descriptor data on the host
	float* desc;

	//! Orientation data on the host
	float* orientation;

	//! Position buffer on the device
	CLBuffer d_pixPos;

	//! Scale buffer on the device
	CLBuffer d_scale;

	//! Laplacian buffer on the device
	CLBuffer d_laplacian;

	//! Res buffer on the device
	CLBuffer d_res;

	const static int j[16];

	const static int i[16];

	const static unsigned int id[13];

	const static float gauss25[49];
};

typedef double cl_time;

class ResponseLayer {

public:

	ResponseLayer(int width, int height, int step, int filter,
			CLProgram &program);

	~ResponseLayer();

	int getWidth();

	int getHeight();

	int getStep();

	int getFilter();

	CLBuffer getResponses();

	CLBuffer getLaplacian();

private:

	int width;

	int height;

	int step;

	int filter;

	CLProgram &program;

	CLBuffer d_responses;

	CLBuffer d_laplacian;
};

static const int OCTAVES = 5;
static const int INTERVALS = 4;
static const float THRES = 0.0001f;
static const int SAMPLE_STEP = 2;

//! FastHessian Calculates array of hessian and co-ordinates of ipoints
/*!
 FastHessian declaration\n
 Calculates array of hessian and co-ordinates of ipoints
 */
class FastHessian {
public:

	//! Destructor
	~FastHessian();

	//! Constructor without image
	FastHessian(int i_height, int i_width, CLProgram &program,
			CLKernel &hessian_detKernel, CLKernel &non_max_supressionKernel,
			const int octaves = OCTAVES, const int intervals = INTERVALS,
			const int sample_step = SAMPLE_STEP, const float thres = THRES);

	// TODO Fix this name
	void selectIpoints(CLBuffer d_laplacian, CLBuffer d_pixPos,
			CLBuffer d_scale, int maxPoints);

	// TODO Fix this name
	void computeHessianDet(CLBuffer d_intImage, int i_width, int i_height);

	//! Find the image features and write into vector of features
	int getIpoints(const icl::core::Img32f &image, CLBuffer d_intImage,
			CLBuffer d_laplacian, CLBuffer d_pixPos, CLBuffer d_scale,
			int maxIpts);

	//! Resets the information required for the next frame to compute
	void reset();

private:

	void createResponseMap(int octaves, int imgWidth, int imgHeight,
			int sample_step);

	CLProgram &program;
	CLKernel &hessian_detKernel;
	CLKernel &non_max_supressionKernel;

	//! Number of Ipoints
	int num_ipts;

	//! Number of Octaves
	int octaves;

	//! Number of Intervals per octave
	int intervals;

	//! Initial sampling step for Ipoint detection
	int sample_step;

	//! Threshold value for blob resonses
	float thres;

	std::vector<ResponseLayer*> responseMap;

	//! Number of Ipoints on GPU
	CLBuffer d_ipt_count;
};

// Rounds up size to the nearest multiple of multiple
unsigned int roundUp(unsigned int value, unsigned int multiple) {
	unsigned int remainder = value % multiple;
	// Make the value a multiple of multiple
	if (remainder != 0) {
		value += (multiple - remainder);
	}
	return value;
}

//////////////////////////////////////////////////////////////////////////////
// utils functions ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Wrapper for malloc
void* alloc(size_t size) {
	void* ptr = NULL;
	ptr = malloc(size);
	if (ptr == NULL) {
		perror("malloc");
		exit(-1);
	}
	return ptr;
}

//! OpenCl error code list
/*!
 An array of character strings used to give the error corresponding to the error code \n

 The error code is the index within this array
 */
const char *cl_errs[MAX_ERR_VAL] = { "CL_SUCCESS",                         // 0
		"CL_DEVICE_NOT_FOUND",                //-1
		"CL_DEVICE_NOT_AVAILABLE",            //-2
		"CL_COMPILER_NOT_AVAILABLE",          //-3
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",   //-4
		"CL_OUT_OF_RESOURCES",                //-5
		"CL_OUT_OF_HOST_MEMORY",              //-6
		"CL_PROFILING_INFO_NOT_AVAILABLE",    //-7
		"CL_MEM_COPY_OVERLAP",                //-8
		"CL_IMAGE_FORMAT_MISMATCH",           //-9
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",      //-10
		"CL_BUILD_PROGRAM_FAILURE",           //-11
		"CL_MAP_FAILURE",                     //-12
		"",                                   //-13
		"",                                   //-14
		"",                                   //-15
		"",                                   //-16
		"",                                   //-17
		"",                                   //-18
		"",                                   //-19
		"",                                   //-20
		"",                                   //-21
		"",                                   //-22
		"",                                   //-23
		"",                                   //-24
		"",                                   //-25
		"",                                   //-26
		"",                                   //-27
		"",                                   //-28
		"",                                   //-29
		"CL_INVALID_VALUE",                   //-30
		"CL_INVALID_DEVICE_TYPE",             //-31
		"CL_INVALID_PLATFORM",                //-32
		"CL_INVALID_DEVICE",                  //-33
		"CL_INVALID_CONTEXT",                 //-34
		"CL_INVALID_QUEUE_PROPERTIES",        //-35
		"CL_INVALID_COMMAND_QUEUE",           //-36
		"CL_INVALID_HOST_PTR",                //-37
		"CL_INVALID_MEM_OBJECT",              //-38
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR", //-39
		"CL_INVALID_IMAGE_SIZE",              //-40
		"CL_INVALID_SAMPLER",                 //-41
		"CL_INVALID_BINARY",                  //-42
		"CL_INVALID_BUILD_OPTIONS",           //-43
		"CL_INVALID_PROGRAM",                 //-44
		"CL_INVALID_PROGRAM_EXECUTABLE",      //-45
		"CL_INVALID_KERNEL_NAME",             //-46
		"CL_INVALID_KERNEL_DEFINITION",       //-47
		"CL_INVALID_KERNEL",                  //-48
		"CL_INVALID_ARG_INDEX",               //-49
		"CL_INVALID_ARG_VALUE",               //-50
		"CL_INVALID_ARG_SIZE",                //-51
		"CL_INVALID_KERNEL_ARGS",             //-52
		"CL_INVALID_WORK_DIMENSION ",         //-53
		"CL_INVALID_WORK_GROUP_SIZE",         //-54
		"CL_INVALID_WORK_ITEM_SIZE",          //-55
		"CL_INVALID_GLOBAL_OFFSET",           //-56
		"CL_INVALID_EVENT_WAIT_LIST",         //-57
		"CL_INVALID_EVENT",                   //-58
		"CL_INVALID_OPERATION",               //-59
		"CL_INVALID_GL_OBJECT",               //-60
		"CL_INVALID_BUFFER_SIZE",             //-61
		"CL_INVALID_MIP_LEVEL",               //-62
		"CL_INVALID_GLOBAL_WORK_SIZE" };       //-63

//////////////////////////////////////////////////////////////////////////////
// ResponseLayer Hessian class implementation ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ResponseLayer::ResponseLayer(int width, int height, int step, int filter,
		CLProgram &program) :
		program(program) {
	this->width = width;
	this->height = height;
	this->step = step;
	this->filter = filter;

	this->d_laplacian = program.createBuffer("rw",
			sizeof(int) * width * height);
	this->d_responses = program.createBuffer("rw",
			sizeof(float) * width * height);

}

ResponseLayer::~ResponseLayer() {

}

int ResponseLayer::getWidth() {

	return this->width;
}

int ResponseLayer::getHeight() {

	return this->height;
}

int ResponseLayer::getStep() {

	return this->step;
}

int ResponseLayer::getFilter() {

	return this->filter;
}

CLBuffer ResponseLayer::getLaplacian() {

	return this->d_laplacian;
}

CLBuffer ResponseLayer::getResponses() {

	return this->d_responses;
}

//////////////////////////////////////////////////////////////////////////////
// Fast Hessian class implementation /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// Based on the octave (row) and interval (column), this lookup table
// identifies the appropriate determinant layer
const int filter_map[OCTAVES][INTERVALS] = { { 0, 1, 2, 3 }, { 1, 3, 4, 5 }, {
		3, 5, 6, 7 }, { 5, 7, 8, 9 }, { 7, 9, 10, 11 } };

//-------------------------------------------------------

//! Constructor
FastHessian::FastHessian(int i_height, int i_width, CLProgram &program,
		CLKernel &hessian_detKernel, CLKernel &non_max_supressionKernel,
		const int octaves, const int intervals, const int sample_step,
		const float thres) :
		program(program), hessian_detKernel(hessian_detKernel), non_max_supressionKernel(
				non_max_supressionKernel) {
	// Initialise variables with bounds-checked values
	this->octaves = (octaves > 0 && octaves <= 4 ? octaves : OCTAVES);
	this->intervals = (intervals > 0 && intervals <= 4 ? intervals : INTERVALS);
	this->sample_step = (
			sample_step > 0 && sample_step <= 6 ? sample_step : SAMPLE_STEP);
	this->thres = (thres >= 0 ? thres : THRES);

	this->num_ipts = 0;

	// TODO implement this as device zero-copy memory
	this->d_ipt_count = program.createBuffer("rw", sizeof(int),
			&this->num_ipts);

	// Create the hessian response map objects

	this->createResponseMap(octaves, i_width, i_height, sample_step);
}

//! Destructor
FastHessian::~FastHessian() {
	for (unsigned int i = 0; i < this->responseMap.size(); i++) {
		delete responseMap.at(i);
	}
}

void FastHessian::createResponseMap(int octaves, int imgWidth, int imgHeight,
		int sample_step) {
	int w = (imgWidth / sample_step);
	int h = (imgHeight / sample_step);
	int s = (sample_step);

	// Calculate approximated determinant of hessian values
	if (octaves >= 1) {
		this->responseMap.push_back(new ResponseLayer(w, h, s, 9, program));
		this->responseMap.push_back(new ResponseLayer(w, h, s, 15, program));
		this->responseMap.push_back(new ResponseLayer(w, h, s, 21, program));
		this->responseMap.push_back(new ResponseLayer(w, h, s, 27, program));
	}

	if (octaves >= 2) {
		this->responseMap.push_back(
				new ResponseLayer(w / 2, h / 2, s * 2, 39, program));
		this->responseMap.push_back(
				new ResponseLayer(w / 2, h / 2, s * 2, 51, program));
	}

	if (octaves >= 3) {
		this->responseMap.push_back(
				new ResponseLayer(w / 4, h / 4, s * 4, 75, program));
		this->responseMap.push_back(
				new ResponseLayer(w / 4, h / 4, s * 4, 99, program));
	}

	if (octaves >= 4) {
		this->responseMap.push_back(
				new ResponseLayer(w / 8, h / 8, s * 8, 147, program));
		this->responseMap.push_back(
				new ResponseLayer(w / 8, h / 8, s * 8, 195, program));
	}

	if (octaves >= 5) {
		this->responseMap.push_back(
				new ResponseLayer(w / 16, h / 16, s * 16, 291, program));
		this->responseMap.push_back(
				new ResponseLayer(w / 16, h / 16, s * 16, 387, program));
	}
}

//! Hessian determinant for the image using approximated box filters
/*!
 \param d_intImage Integral Image
 \param i_width Image Width
 \param i_height Image Height
 */
void FastHessian::computeHessianDet(CLBuffer d_intImage, int i_width,
		int i_height) {
	// set matrix size and x,y threads per block
	const int BLOCK_DIM = 16;

	size_t localWorkSize[2] = { BLOCK_DIM, BLOCK_DIM };
	hessian_detKernel.setArgs(d_intImage, i_width, i_height);
	for (unsigned int i = 0; i < this->responseMap.size(); i++) {

		CLBuffer responses = this->responseMap.at(i)->getResponses();
		CLBuffer laplacian = this->responseMap.at(i)->getLaplacian();
		int step = this->responseMap.at(i)->getStep();
		int filter = this->responseMap.at(i)->getFilter();
		int layerWidth = this->responseMap.at(i)->getWidth();
		int layerHeight = this->responseMap.at(i)->getHeight();

		hessian_detKernel[3] = responses;
		hessian_detKernel[4] = laplacian;
		hessian_detKernel[5] = layerWidth;
		hessian_detKernel[6] = layerHeight;
		hessian_detKernel[7] = step;
		hessian_detKernel[8] = filter;

		hessian_detKernel.apply(roundUp(layerWidth, localWorkSize[0]),
				roundUp(layerHeight, localWorkSize[1]), 0, localWorkSize[0],
				localWorkSize[1]);
	}
}

/*!
 Find the image features and write into vector of features
 Determine what points are interesting and store them
 \param img
 \param d_intImage The integral image pointer on the device
 \param d_laplacian
 \param d_pixPos
 \param d_scale
 */
int FastHessian::getIpoints(const icl::core::Img32f &image, CLBuffer d_intImage,
		CLBuffer d_laplacian, CLBuffer d_pixPos, CLBuffer d_scale,
		int maxIpts) {

	// Compute the hessian determinants
	// GPU kernels: init_det and build_det kernels
	//    this->computeHessianDet(d_intImage, img->width, img->height);
	this->computeHessianDet(d_intImage, image.getWidth(), image.getHeight());

	// Determine which points are interesting
	// GPU kernels: non_max_suppression kernel
	this->selectIpoints(d_laplacian, d_pixPos, d_scale, maxIpts);

	// Copy the number of interesting points back to the host
	d_ipt_count.read(&this->num_ipts, sizeof(int));

	// Sanity check
	if (this->num_ipts < 0) {
		printf("Invalid number of Ipoints\n");
		exit(-1);
	};

	return num_ipts;
}

/*!
 //! Calculate the position of ipoints (gpuIpoint::d_pixPos) using non maximal suppression

 Convert d_m_det which is a array of all the hessians into d_pixPos
 which is a float2 array of the (x,y) of all ipoint locations
 \param i_width The width of the image
 \param i_height The height of the image
 \param d_laplacian
 \param d_pixPos
 \param d_scale
 */
void FastHessian::selectIpoints(CLBuffer d_laplacian, CLBuffer d_pixPos,
		CLBuffer d_scale, int maxPoints) {

	// The search for exterema (the most interesting point in a neighborhood)
	// is done by non-maximal suppression

	int BLOCK_W = 16;
	int BLOCK_H = 16;
	non_max_supressionKernel[14] = this->d_ipt_count;
	non_max_supressionKernel[15] = d_pixPos;
	non_max_supressionKernel[16] = d_scale;
	non_max_supressionKernel[17] = d_laplacian;
	non_max_supressionKernel[18] = maxPoints;
	non_max_supressionKernel[19] = this->thres;

	// Run the kernel for each octave
	for (int o = 0; o < octaves; o++) {
		for (int i = 0; i <= 1; i++) {

			CLBuffer bResponse =
					this->responseMap.at(filter_map[o][i])->getResponses();
			int bWidth = this->responseMap.at(filter_map[o][i])->getWidth();
			int bHeight = this->responseMap.at(filter_map[o][i])->getHeight();
			int bFilter = this->responseMap.at(filter_map[o][i])->getFilter();

			CLBuffer mResponse =
					this->responseMap.at(filter_map[o][i + 1])->getResponses();
			int mWidth = this->responseMap.at(filter_map[o][i + 1])->getWidth();
			int mHeight =
					this->responseMap.at(filter_map[o][i + 1])->getHeight();
			int mFilter =
					this->responseMap.at(filter_map[o][i + 1])->getFilter();
			CLBuffer mLaplacian =
					this->responseMap.at(filter_map[o][i + 1])->getLaplacian();

			CLBuffer tResponse =
					this->responseMap.at(filter_map[o][i + 2])->getResponses();
			int tWidth = this->responseMap.at(filter_map[o][i + 2])->getWidth();
			int tHeight =
					this->responseMap.at(filter_map[o][i + 2])->getHeight();
			int tFilter =
					this->responseMap.at(filter_map[o][i + 2])->getFilter();
			int tStep = this->responseMap.at(filter_map[o][i + 2])->getStep();

			size_t localWorkSize[2] = { (size_t) BLOCK_W, (size_t) BLOCK_H };
			size_t globalWorkSize[2] = { (size_t) roundUp(mWidth, BLOCK_W),
					(size_t) roundUp(mHeight, BLOCK_H) };

			non_max_supressionKernel[0] = tResponse;
			non_max_supressionKernel[1] = tWidth;
			non_max_supressionKernel[2] = tHeight;
			non_max_supressionKernel[3] = tFilter;
			non_max_supressionKernel[4] = tStep;
			non_max_supressionKernel[5] = mResponse;
			non_max_supressionKernel[6] = mLaplacian;
			non_max_supressionKernel[7] = mWidth;
			non_max_supressionKernel[8] = mHeight;
			non_max_supressionKernel[9] = mFilter;
			non_max_supressionKernel[10] = bResponse;
			non_max_supressionKernel[11] = bWidth;
			non_max_supressionKernel[12] = bHeight;
			non_max_supressionKernel[13] = bFilter;
			// Call non-max supression kernel
			non_max_supressionKernel.apply(globalWorkSize[0], globalWorkSize[1],
					0, localWorkSize[0], localWorkSize[1]);

			// TODO Verify that a clFinish is not required (setting an argument
			//      to the loop counter without it may be problematic, but it
			//      really kills performance on AMD parts)
			//cl_sync();
		}
	}
}

//! Reset the state of the data
void FastHessian::reset() {
	int numIpts = 0;
	d_ipt_count.write(&numIpts, sizeof(int));
}

//////////////////////////////////////////////////////////////////////////////
// Surf class implementation /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// TODO Get rid of these arrays (i and j).  Have the values computed
//      dynamically within the kernel
const int Surf::Data::j[] = { -12, -7, -2, 3, -12, -7, -2, 3, -12, -7, -2, 3,
		-12, -7, -2, 3 };

const int Surf::Data::i[] = { -12, -12, -12, -12, -7, -7, -7, -7, -2, -2, -2,
		-2, 3, 3, 3, 3 };

const unsigned int Surf::Data::id[] = { 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6 };

const float Surf::Data::gauss25[] = { 0.02350693969273f, 0.01849121369071f,
		0.01239503121241f, 0.00708015417522f, 0.00344628101733f,
		0.00142945847484f, 0.00050524879060f, 0.02169964028389f,
		0.01706954162243f, 0.01144205592615f, 0.00653580605408f,
		0.00318131834134f, 0.00131955648461f, 0.00046640341759f,
		0.01706954162243f, 0.01342737701584f, 0.00900063997939f,
		0.00514124713667f, 0.00250251364222f, 0.00103799989504f,
		0.00036688592278f, 0.01144205592615f, 0.00900063997939f,
		0.00603330940534f, 0.00344628101733f, 0.00167748505986f,
		0.00069579213743f, 0.00024593098864f, 0.00653580605408f,
		0.00514124713667f, 0.00344628101733f, 0.00196854695367f,
		0.00095819467066f, 0.00039744277546f, 0.00014047800980f,
		0.00318131834134f, 0.00250251364222f, 0.00167748505986f,
		0.00095819467066f, 0.00046640341759f, 0.00019345616757f,
		0.00006837798818f, 0.00131955648461f, 0.00103799989504f,
		0.00069579213743f, 0.00039744277546f, 0.00019345616757f,
		0.00008024231247f, 0.00002836202103f };

void Surf::createKernels() {
	createDescrtptorsKernel = program.createKernel("createDescriptors_kernel");
	getOrientationStep1Kernel = program.createKernel("getOrientationStep1");
	getOrientationStep2Kernel = program.createKernel("getOrientationStep2");
	hessian_detKernel = program.createKernel("hessian_det");
	scanKernel = program.createKernel("scan");
	scan4Kernel = program.createKernel("scan4");
	scanImageKernel = program.createKernel("scanImage");
	transposeKernel = program.createKernel("transpose");
	transposeImageKernel = program.createKernel("transposeImage");
	nearestNeighborKernel = program.createKernel("NearestNeighbor");
	non_max_supressionKernel = program.createKernel(
			"non_max_supression_kernel");
	normalizeDescriptorsKernel = program.createKernel("normalizeDescriptors");
}

//! Constructor
Surf::Surf(int initialPoints, int i_height, int i_width, int octaves,
		int intervals, int sample_step, float threshold) :
		m_data(new Data) {

	stringstream ss;
	ss << utilityKernels << endl << createDescriptors_kernel << endl
			<< getOrientation_kernels << endl << hessianDet_kernel << endl
			<< integralImage_kernels << endl << nearestNeighbor_kernel << endl
			<< nonMaxSuppression_kernel << endl << normalizeDescriptors_kernel
			<< endl;
	program = CLProgram("gpu", ss.str());
	createKernels();

	this->m_data->fh = new FastHessian(i_height, i_width, program,
			hessian_detKernel, non_max_supressionKernel, octaves, intervals, sample_step,
			threshold);

	// Once we know the size of the image, successive frames should stay
	// the same size, so we can just allocate the space once for the integral
	// image and intermediate data
	this->m_data->d_intImage = program.createBuffer("rw",
			sizeof(float) * i_width * i_height);

	this->m_data->d_tmpIntImage = program.createBuffer("rw",
			sizeof(float) * i_width * i_height);

	// These two are unnecessary for buffers, but required for images, so
	// we'll use them for buffers as well to keep the code clean
	this->m_data->d_tmpIntImageT1 = program.createBuffer("rw",
			sizeof(float) * i_width * i_height);
	this->m_data->d_tmpIntImageT2 = program.createBuffer("rw",
			sizeof(float) * i_width * i_height);

	// Allocate constant data on device
	this->m_data->d_gauss25 = program.createBuffer("r", sizeof(float) * 49,
			(void*) Surf::Data::gauss25);

	this->m_data->d_id = program.createBuffer("r", sizeof(unsigned int) * 13,
			(void*) Surf::Data::id);

	this->m_data->d_i = program.createBuffer("r", sizeof(unsigned int) * 16,
			(void*) Surf::Data::i);

	this->m_data->d_j = program.createBuffer("r", sizeof(int) * 16,
			(void*) Surf::Data::j);

	// Allocate buffers for each of the interesting points.  We don't know
	// how many there are initially, so must allocate more than enough space

	this->m_data->d_scale = program.createBuffer("rw",
			initialPoints * sizeof(float));
	this->m_data->d_pixPos = program.createBuffer("rw",
			initialPoints * sizeof(float2));
	this->m_data->d_laplacian = program.createBuffer("rw",
			initialPoints * sizeof(int));

	// These buffers used to wait for the number of actual ipts to be known
	// before being allocated, instead now we'll only allocate them once
	// so that we can take advantage of optimized data transfers and reallocate
	// them if there's not enough space available

	this->m_data->d_length = program.createBuffer("rw",
			initialPoints * DESC_SIZE * sizeof(float));
	this->m_data->d_desc = program.createBuffer("rw",
			initialPoints * DESC_SIZE * sizeof(float));
	this->m_data->d_res = program.createBuffer("rw",
			initialPoints * 109 * sizeof(float4));
	this->m_data->d_orientation = program.createBuffer("rw",
			initialPoints * sizeof(float));

	// Allocate buffers to store the output data (descriptor information)
	// on the host

	this->m_data->scale = (float*) alloc(initialPoints * sizeof(float));
	this->m_data->pixPos = (float2*) alloc(initialPoints * sizeof(float2));
	this->m_data->laplacian = (int*) alloc(initialPoints * sizeof(int));
	this->m_data->desc = (float*) alloc(
			initialPoints * DESC_SIZE * sizeof(float));
	this->m_data->orientation = (float*) alloc(initialPoints * sizeof(float));
	// This is how much space is available for Ipts
	this->m_data->maxIpts = initialPoints;

	this->m_data->m_grayBuffer = icl::core::Img32f(icl::utils::Size(1, 1),
			icl::core::formatGray);
}

//! Destructor
Surf::~Surf() {

	free(this->m_data->orientation);
	free(this->m_data->scale);
	free(this->m_data->laplacian);
	free(this->m_data->desc);
	free(this->m_data->pixPos);

	delete this->m_data->fh;

	delete m_data;
}

//! Computes the integral image of image img.
//! Assumes source image to be a  32-bit floating point.
/*!
 Saves integral Image in d_intImage on the GPU
 \param source Input Image as grabbed by OpenCv
 */

inline void scale_to_01(float &f) {
	static const float s = 1.0f / 255.0f;
	f *= s;
}

void Surf::computeIntegralImage(const icl::core::Img32f &image) {
	//! convert the image to single channel 32f

	// TODO This call takes about 4ms (is there any way to speed it up?)
	//IplImage *img = getGray(source);
	//cc(&image,&this->m_data->m_grayBuffer);

	// set up variables for data access
	int height = image.getHeight(); //img->height;
	int width = image.getWidth(); //img->width;
	float *data = (float*) image.begin(0); //img->imageData;

	//m_grayBuffer = m_grayBuffer/(1.0f/255.0f);

	//        this->m_data->m_grayBuffer.forEach(scale_to_01);

	CLKernel scan_kernel;
	{
		// Copy the data to the GPU
		this->m_data->d_intImage.write(data, sizeof(float) * width * height);

		scan_kernel = scanKernel;
	}

	// -----------------------------------------------------------------
	// Step 1: Perform integral summation on the rows
	// -----------------------------------------------------------------

	size_t localWorkSize1[2] = { 64, 1 };
	size_t globalWorkSize1[2] = { (size_t) 64, (size_t) height };

	scan_kernel.setArgs(this->m_data->d_intImage, this->m_data->d_tmpIntImage,
			height, width);
	scan_kernel.apply(globalWorkSize1[0], globalWorkSize1[1], 0,
			localWorkSize1[0], localWorkSize1[1]);

	// -----------------------------------------------------------------
	// Step 2: Transpose
	// -----------------------------------------------------------------

	size_t localWorkSize2[] = { 16, 16 };
	size_t globalWorkSize2[] = { roundUp(width, 16), roundUp(height, 16) };
	transposeKernel.setArgs(this->m_data->d_tmpIntImage,
			this->m_data->d_tmpIntImageT1, height, width);
	transposeKernel.apply(globalWorkSize2[0], globalWorkSize2[1], 0,
			localWorkSize2[0], localWorkSize2[1]);

	// -----------------------------------------------------------------
	// Step 3: Run integral summation on the rows again (same as columns
	//         integral since we've transposed).
	// -----------------------------------------------------------------

	int heightT = width;
	int widthT = height;

	size_t localWorkSize3[2] = { 64, 1 };
	size_t globalWorkSize3[2] = { (size_t) 64, (size_t) heightT };

	scan_kernel.setArgs(this->m_data->d_tmpIntImageT1,
			this->m_data->d_tmpIntImageT2, heightT, widthT);

	scan_kernel.apply(globalWorkSize3[0], globalWorkSize3[1], 0,
			localWorkSize3[0], localWorkSize3[1]);

	// -----------------------------------------------------------------
	// Step 4: Transpose back
	// -----------------------------------------------------------------

	size_t localWorkSize4[] = { 16, 16 };
	size_t globalWorkSize4[] = { roundUp(widthT, 16), roundUp(heightT, 16) };

	transposeKernel.setArgs(this->m_data->d_tmpIntImageT2,
			this->m_data->d_intImage, heightT, widthT);
	transposeKernel.apply(globalWorkSize4[0], globalWorkSize4[1], 0,
			localWorkSize4[0], localWorkSize4[1]);
	// release the gray image
	//cvReleaseImage(&img);
}

//! Create the SURF descriptors
/*!
 Calculate orientation for all ipoints using the
 sliding window technique from OpenSurf
 \param d_intImage The integral image
 \param width The width of the image
 \param height The height of the image
 */
void Surf::createDescriptors(int i_width, int i_height) {

	const size_t threadsPerWG = 81;
	const size_t wgsPerIpt = 16;

	size_t localWorkSizeSurf64[2] = { threadsPerWG, 1 };
	size_t globalWorkSizeSurf64[2] = { (wgsPerIpt * threadsPerWG),
			(size_t) this->m_data->numIpts };

	createDescrtptorsKernel.setArgs(this->m_data->d_intImage, i_width, i_height,
			this->m_data->d_scale, this->m_data->d_desc, this->m_data->d_pixPos,
			this->m_data->d_orientation, this->m_data->d_length,
			this->m_data->d_j, this->m_data->d_i);

	createDescrtptorsKernel.apply(globalWorkSizeSurf64[0],
			globalWorkSizeSurf64[1], 0, localWorkSizeSurf64[0],
			localWorkSizeSurf64[1]);

	size_t localWorkSizeNorm64[] = { DESC_SIZE };
	size_t globallWorkSizeNorm64[] = { (size_t) this->m_data->numIpts
			* DESC_SIZE };

	normalizeDescriptorsKernel.setArgs(this->m_data->d_desc,
			this->m_data->d_length);

	normalizeDescriptorsKernel.apply(globallWorkSizeNorm64[0], 0, 0,
			localWorkSizeNorm64[0]);
}

//! Calculate orientation for all ipoints
/*!
 Calculate orientation for all ipoints using the
 sliding window technique from OpenSurf
 \param i_width The image width
 \param i_height The image height
 */
void Surf::getOrientations(int i_width, int i_height) {

	size_t localWorkSize1[] = { 169 };
	size_t globalWorkSize1[] = { (size_t) this->m_data->numIpts * 169 };

	/*!
	 Assign the supplied Ipoint an orientation
	 */
	getOrientationStep1Kernel.setArgs(this->m_data->d_intImage,
			this->m_data->d_scale, this->m_data->d_pixPos,
			this->m_data->d_gauss25, this->m_data->d_id, i_width, i_height,
			this->m_data->d_res);
	getOrientationStep1Kernel.apply(globalWorkSize1[0], 0, 0,
			localWorkSize1[0]);

	getOrientationStep2Kernel.setArgs(this->m_data->d_orientation,
			this->m_data->d_res);

	size_t localWorkSize2[] = { 42 };
	size_t globalWorkSize2[] = { (size_t) this->m_data->numIpts * 42 };

	getOrientationStep2Kernel.apply(globalWorkSize2[0], 0, 0,
			localWorkSize2[0]);
}

//! Allocates the memory objects requried for the ipt descriptor information
void Surf::reallocateIptBuffers() {

	// Release the old memory objects (that were too small)

	free(this->m_data->orientation);
	free(this->m_data->scale);
	free(this->m_data->laplacian);
	free(this->m_data->desc);
	free(this->m_data->pixPos);

	int newSize = this->m_data->maxIpts;

	// Allocate new memory objects based on the new size

	this->m_data->d_scale = program.createBuffer("rw", newSize * sizeof(float));
	this->m_data->d_pixPos = program.createBuffer("rw",
			newSize * sizeof(float2));
	this->m_data->d_laplacian = program.createBuffer("rw",
			newSize * sizeof(int));
	this->m_data->d_length = program.createBuffer("rw",
			newSize * DESC_SIZE * sizeof(float));

	this->m_data->d_desc = program.createBuffer("rw",
			newSize * DESC_SIZE * sizeof(float));

	this->m_data->d_res = program.createBuffer("rw",
			newSize * 121 * sizeof(float4));

	this->m_data->d_orientation = program.createBuffer("rw",
			newSize * sizeof(float));

	this->m_data->scale = (float*) alloc(newSize * sizeof(float));
	this->m_data->pixPos = (float2*) alloc(newSize * sizeof(float2));
	this->m_data->laplacian = (int*) alloc(newSize * sizeof(int));
	this->m_data->desc = (float*) alloc(newSize * DESC_SIZE * sizeof(float));
	this->m_data->orientation = (float*) alloc(newSize * sizeof(float));

}

//! This function gets called each time SURF is run on a new frame.  It prevents
//! having to create and destroy the object each time (lots of OpenCL overhead)
void Surf::reset() {
	this->m_data->fh->reset();
}

//! Retreive the descriptors from the GPU
/*!
 Copy data back from the GPU into an IpVec structure on the host
 */
const IpVec &Surf::retrieveDescriptors() {
	m_data->m_outputBuffer.resize(m_data->numIpts);
	if (!m_data->m_outputBuffer.size()) {
		return m_data->m_outputBuffer;
	}

	// Copy back the output data

	// Copy back Laplacian information
	this->m_data->d_laplacian.read(this->m_data->laplacian,
			(this->m_data->numIpts) * sizeof(int), false);

	// Copy back scale data
	this->m_data->d_scale.read(this->m_data->scale,
			this->m_data->numIpts * sizeof(float), false);

	// Copy back pixel positions
	this->m_data->d_pixPos.read(this->m_data->pixPos,
			(this->m_data->numIpts) * sizeof(float2), false);

	// Copy back descriptors
	this->m_data->d_desc.read(this->m_data->desc,
			(this->m_data->numIpts) * DESC_SIZE * sizeof(float), false);

	// Copy back orientation data
	this->m_data->d_orientation.read(this->m_data->orientation,
			(this->m_data->numIpts) * sizeof(float));

	// Parse the data into Ipoint structures
	for (int i = 0; i < (this->m_data->numIpts); i++) {
		Ipoint &ipt = m_data->m_outputBuffer[i];
		ipt.x = this->m_data->pixPos[i].x;
		ipt.y = this->m_data->pixPos[i].y;
		ipt.scale = this->m_data->scale[i];
		ipt.laplacian = this->m_data->laplacian[i];
		ipt.orientation = this->m_data->orientation[i];
		memcpy(ipt.descriptor, &this->m_data->desc[i * 64], sizeof(float) * 64);
		//            m_data->m_outputBuffer[i] = ipts->push_back(ipt);
	}

	return m_data->m_outputBuffer; //ipts;
}

//! Function that builds vector of interest points.  This is the main SURF function
//! that will be called for any type of input.
/*!
 High level driver function for entire OpenSurfOpenCl
 \param img image to find Ipoints within
 \param upright Switch for future functionality of upright surf
 \param fh FastHessian object
 */
void Surf::run(const icl::core::Img32f &image, bool upright) {

	if (upright) {
		// Extract upright (i.e. not rotation invariant) descriptors
		printf("Upright surf not supported\n");
		exit(1);
	}

	// Perform the scan sum of the image (populates d_intImage)
	// GPU kernels: scan (x2), tranpose (x2)
	this->computeIntegralImage(image);

	// Determines the points of interest
	// GPU kernels: init_det, hessian_det (x12), non_max_suppression (x3)
	// GPU mem transfer: copies back the number of ipoints
	this->m_data->numIpts = this->m_data->fh->getIpoints(image,
			this->m_data->d_intImage, this->m_data->d_laplacian,
			this->m_data->d_pixPos, this->m_data->d_scale,
			this->m_data->maxIpts);

	// Verify that there was enough space allocated for the number of
	// Ipoints found
	if (this->m_data->numIpts >= this->m_data->maxIpts) {
		// If not enough space existed, we need to reallocate space and
		// run the kernels again

		printf(
				"Not enough space for Ipoints, reallocating and running again\n");
		this->m_data->maxIpts = this->m_data->numIpts * 2;
		this->reallocateIptBuffers();
		// XXX This was breaking sometimes
		this->m_data->fh->reset();
		this->m_data->numIpts = this->m_data->fh->getIpoints(image,
				this->m_data->d_intImage, this->m_data->d_laplacian,
				this->m_data->d_pixPos, this->m_data->d_scale,
				this->m_data->maxIpts);
	}

	// printf("There were %d interest points\n", this->m_data->numIpts);

	// Main SURF-64 loop assigns orientations and gets descriptors
	if (this->m_data->numIpts == 0)
		return;

	// GPU kernel: getOrientation1 (1x), getOrientation2 (1x)
	this->getOrientations(image.getWidth(), image.getHeight());

	// GPU kernel: surf64descriptor (1x), norm64descriptor (1x)
	this->createDescriptors(image.getWidth(), image.getHeight());
}

const IpVec &Surf::detect(const ImgBase *image) {
	ICLASSERT_THROW(image,
			ICLException("CLSurfLib::Surf::detect: given image was null"));

	cc(image, &this->m_data->m_grayBuffer);
	this->m_data->m_grayBuffer.forEach(scale_to_01);
	run(m_data->m_grayBuffer, false);
	const IpVec &ipts = retrieveDescriptors();
	reset();
	return ipts;
}
}
}
}

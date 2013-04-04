/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLFilter/src/ICLFilter/MotionSensitiveTemporalSmoothi **
 **          ng.cpp                                                 **
 ** Module : ICLFilter                                              **
 ** Authors: Andre Ueckermann                                       **
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

#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLCore/Img.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
namespace filter {

#ifdef HAVE_OPENCL
static char timeSmoothingKernel[] =
"__kernel void                                                                                                                  \n"
"temporalSmoothingFloat(__global float const * inputImages, __global float * outputImage, int const filterSize, int const imgCount, int const w, int const h, int const difference, int const nullvalue, __global float * motionImage)                                                                                              \n"
"{                                                                                                                              \n"
"   size_t id =  get_global_id(0);                                                                                              \n"
"   int count=0;                                                                                                                \n"
"   float value=0;                                                                                                              \n"
"   float min=100000;                                                                                                           \n"
"   float max=0;                                                                                                                \n"
"   for(int i=0; i<filterSize; i++){                                                                                            \n"
"     if(inputImages[i*w*h+id]!=nullvalue){                                                                                     \n"
"       count++;                                                                                                                \n"
"       value+=inputImages[i*w*h+id];                                                                                           \n"
"       if(inputImages[i*w*h+id]<min) min=inputImages[i*w*h+id];                                                                \n"
"       if(inputImages[i*w*h+id]>max) max=inputImages[i*w*h+id];                                                                \n"
"     }                                                                                                                         \n"
"   }                                                                                                                           \n"
"   if(count==0){                                                                                                               \n"
"     outputImage[id]=nullvalue;                                                                                                \n"
"     motionImage[id]=0.0;                                                                                                      \n"
"   }else{                                                                                                                      \n"
"     if((max-min)>difference){                                                                                                 \n"
"       outputImage[id]=inputImages[(imgCount-1)*w*h+id];                                                                       \n"
"       motionImage[id]=255.0;                                                                                                  \n"
"     }else{                                                                                                                    \n"
"       outputImage[id]=value/(float)count;                                                                                     \n"
"       motionImage[id]=0.0;                                                                                                    \n"
"     }                                                                                                                         \n"
"   }                                                                                                                           \n"
"}                                                                                                                              \n"
"__kernel void                                                                                                                  \n"
"temporalSmoothingChar(__global uchar const * inputImages, __global uchar * outputImage, int const filterSize, int const imgCount, int const w, int const h, int difference, int nullvalue, __global float * motionImage)                                                                                                         \n"
"{                                                                                                                              \n"
"   size_t id =  get_global_id(0);                                                                                              \n"
"   int count=0;                                                                                                                \n"
"   uint value=0;                                                                                                               \n"
"   uint min=100000;                                                                                                            \n"
"   uint max=0;                                                                                                                 \n"
"   for(int i=0; i<filterSize; i++){                                                                                            \n"
"     if(inputImages[i*w*h+id]!=nullvalue){                                                                                     \n"
"       count++;                                                                                                                \n"
"       value+=inputImages[i*w*h+id];                                                                                           \n"
"       if(inputImages[i*w*h+id]<min) min=inputImages[i*w*h+id];                                                                \n"
"       if(inputImages[i*w*h+id]>max) max=inputImages[i*w*h+id];                                                                \n"
"     }                                                                                                                         \n"
"   }                                                                                                                           \n"
"   if(count==0){                                                                                                               \n"
"     outputImage[id]=nullvalue;                                                                                                \n"
"     motionImage[id]=0.0;                                                                                                      \n"
"   }else{                                                                                                                      \n"
"     if((max-min)>difference){                                                                                                 \n"
"       outputImage[id]=inputImages[(imgCount-1)*w*h+id];                                                                       \n"
"       motionImage[id]=255.0;                                                                                                  \n"
"     }else{                                                                                                                    \n"
"       outputImage[id]=(uchar)((float)value/(float)count);                                                                     \n"
"       motionImage[id]=0.0;                                                                                                    \n"
"     }                                                                                                                         \n"
"   }                                                                                                                           \n"
"}                                                                                                                              \n"
;
#endif

MotionSensitiveTemporalSmoothing::MotionSensitiveTemporalSmoothing(
		int iNullValue, int iMaxFilterSize) {
	//addProperty("use opencl","flag","",true);
	//addProperty("filter size","range:slider","[1,15]",10);
	//addProperty("difference","range:slider","[1,15]",10);

	size = utils::Size(0, 0);
	numChannels = 0;
	maxFilterSize = iMaxFilterSize;
	nullValue = iNullValue;

	currentFilterSize = 6;
	currentDifference = 10;
	useCL = true;
}

MotionSensitiveTemporalSmoothing::~MotionSensitiveTemporalSmoothing() {
	for (int i = clPointer.size() - 1; i >= 0; i--) {
		delete &clPointer.at(i);
	}
	clPointer.clear();
}

void MotionSensitiveTemporalSmoothing::init(int iChannels, core::depth iDepth,
		utils::Size iSize) {
	std::cout << "maxFilterSize: " << maxFilterSize << " , nullValue: "
			<< nullValue << std::endl;
	numChannels = iChannels;
	depth = iDepth;
	size = iSize;
	std::cout << "channels: " << numChannels << " , imageSize: " << size
			<< " , imageDepth: " << depth << std::endl;
	for (int i = clPointer.size() - 1; i >= 0; i--) {
		delete &clPointer.at(i);
	}
	clPointer.clear();
	for (int i = 0; i < numChannels; i++) {
		TemporalSmoothingCL* clElement = new TemporalSmoothingCL(size, depth,
				maxFilterSize, nullValue);
		clElement->setFilterSize(currentFilterSize);
		clElement->setDifference(currentDifference);
		clElement->setUseCL(useCL);
		clPointer.push_back(clElement);
	}
}

void MotionSensitiveTemporalSmoothing::apply(const ImgBase *poSrc,
		ImgBase **ppoDst) {
	ICLASSERT_RETURN(poSrc);
	ICLASSERT_RETURN(ppoDst);
	ICLASSERT_RETURN(poSrc != *ppoDst);

	//filterSize = getPropertyValue("filter size");
	//useCL = getPropertyValue("use opencl");
	//difference = getPropertyValue("difference");

	if (!poSrc->hasFullROI())
		throw ICLException(
				"MotionSensitiveTemporalSmoothing::apply: no roi supported");

	if (poSrc->getDepth() != depth8u && poSrc->getDepth() != depth32f)
		throw ICLException(
				"MotionSensitiveTemporalSmoothing::apply: depth 32f and 8u only");

	if (!prepare(ppoDst, poSrc))
		return;

	if (poSrc->getChannels() != numChannels || poSrc->getDepth() != depth
			|| poSrc->getSize() != size) {
		init(poSrc->getChannels(), poSrc->getDepth(), poSrc->getSize());
	}

	if (poSrc->getDepth() == depth8u) {
		Img8u src = *poSrc->as8u();
		Img8u &dst = *(*ppoDst)->as8u();
		for (int i = 0; i < numChannels; i++) {
			Img8u in = (*src.selectChannel(i));
			Img8u out = clPointer.at(i)->temporalSmoothingC(in);
			dst.replaceChannel(i, &out, 0);
		}
	} else {
		Img32f src = *poSrc->as32f();
		Img32f &dst = *(*ppoDst)->as32f();
		for (int i = 0; i < numChannels; i++) {
			Img32f in = (*src.selectChannel(i));
			Img32f out = clPointer.at(i)->temporalSmoothingF(in);
			dst.replaceChannel(i, &out, 0);
		}
	}
}

void MotionSensitiveTemporalSmoothing::setUseCL(bool use) {
	//setPropertyValue("use opencl",use);
	useCL = use;
	for (unsigned int i = 0; i < clPointer.size(); i++) {
		clPointer.at(i)->setUseCL(use);
	}
}

void MotionSensitiveTemporalSmoothing::setFilterSize(int filterSize) {
	//setPropertyValue("filter size",filterSize);
	currentFilterSize = filterSize;
	for (unsigned int i = 0; i < clPointer.size(); i++) {
		clPointer.at(i)->setFilterSize(filterSize);
	}
}

void MotionSensitiveTemporalSmoothing::setDifference(int difference) {
	//setPropertyValue("difference",difference);
	currentDifference = difference;
	for (unsigned int i = 0; i < clPointer.size(); i++) {
		clPointer.at(i)->setDifference(difference);
	}
}

Img32f MotionSensitiveTemporalSmoothing::getMotionImage() {
	return clPointer.at(0)->getMotionImage();
}

bool MotionSensitiveTemporalSmoothing::isCLActive() {
	return useCL;
}

TemporalSmoothingCL::TemporalSmoothingCL(utils::Size size, core::depth depth,
		int iMaxFilterSize, int iNullValue) {
	w = size.width;
	h = size.height;
	d = depth;
	maxFilterSize = iMaxFilterSize;
	nullValue = iNullValue;

	filterSize = maxFilterSize / 2;
	currentFilterSize = maxFilterSize / 2;
	currentDifference = 10;

	imgCount = 0;
	useCL = true;

#ifdef HAVE_OPENCL
	//create openCL context
	motionImage.setSize(Size(w,h));
	motionImage.setChannels(1);
	if(depth==depth32f) {
		for(int i=0; i<maxFilterSize; i++) {
			Img32f inputImage;
			inputImage.setSize(Size(w,h));
			inputImage.setChannels(1);
			inputImagesF.push_back(inputImage);
		}
		outputImageF.setSize(Size(w,h));
		outputImageF.setChannels(1);
	} else if(depth==depth8u) {
		for(int i=0; i<maxFilterSize; i++) {
			Img8u inputImage;
			inputImage.setSize(Size(w,h));
			inputImage.setChannels(1);
			inputImagesC.push_back(inputImage);
		}
		outputImageC.setSize(Size(w,h));
		outputImageC.setChannels(1);
	}

	motionImageArray = new float[w*h];
	if(depth==depth32f) {
		inputImage1ArrayF = new float[w*h];
		inputImagesArrayF = new float[w*h*maxFilterSize];
		outputImageArrayF = new float[w*h];
	} else if(depth==depth8u) {
		inputImage1ArrayC = new unsigned char[w*h];
		inputImagesArrayC = new unsigned char[w*h*maxFilterSize];
		outputImageArrayC = new unsigned char[w*h];
	} else {
		std::cout<<"Unsupported Depth"<<std::endl;
	}

	//init
	if(depth==depth32f) {
		for(int i=0; i<w*h; i++) {
			inputImage1ArrayF[i]=0;
			outputImageArrayF[i]=0;
			motionImageArray[i]=0;
		}
		for(int i=0; i<w*h*maxFilterSize; i++) {
			inputImagesArrayF[i]=0;
		}
	} else if(depth==depth8u) {
		for(int i=0; i<w*h; i++) {
			inputImage1ArrayC[i]=0;
			outputImageArrayC[i]=0;
			motionImageArray[i]=0;
		}
		for(int i=0; i<w*h*maxFilterSize; i++) {
			inputImagesArrayC[i]=0;
		}
	}

	try {
		program = CLProgram("gpu", timeSmoothingKernel);
		clReady = true;
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
		clReady = false;
	}
	if(clReady==true) { //only if CL context is available
		try {
			motionImageBuffer = program.createBuffer("rw", w*h * sizeof(float), motionImageArray);

			if(depth==depth32f) {
				inputImageBufferF = program.createBuffer("rw", w*h * sizeof(float) * maxFilterSize, inputImagesArrayF);
				outputImageBufferF = program.createBuffer("rw", w*h * sizeof(float), outputImageArrayF);
			} else if(depth==depth8u) {
				inputImageBufferC = program.createBuffer("rw", w*h * sizeof(unsigned char) * maxFilterSize, inputImagesArrayC);
				inputImageBufferC = program.createBuffer("rw", w*h * sizeof(unsigned char), outputImageArrayC);
			}
			kernelTemporalSmoothingFloat = program.createKernel("temporalSmoothingFloat");
			kernelTemporalSmoothingChar = program.createKernel("temporalSmoothingChar");
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
			clReady = false;
		}

	}

#else
	std::cout << "no openCL parallelization available" << std::endl;
	clReady = false;
#endif

}

TemporalSmoothingCL::~TemporalSmoothingCL() {
#ifdef HAVE_OPENCL
	if(d==depth32f) {
		delete[] inputImage1ArrayF;
		delete[] inputImagesArrayF;
		delete[] outputImageArrayF;
	} else if(d==depth8u) {
		delete[] inputImage1ArrayC;
		delete[] inputImagesArrayC;
		delete[] outputImageArrayC;
	}
	delete[] motionImageArray;
#endif
}

Img32f TemporalSmoothingCL::temporalSmoothingF(Img32f &inputImage) {
	if (filterSize > maxFilterSize) {
		filterSize = maxFilterSize;
		std::cout << "set filter size to maximum (" << maxFilterSize << ")"
				<< std::endl;
	}
	if (currentFilterSize != filterSize) {
		currentFilterSize = filterSize;
		imgCount = 0;
	}

	if (imgCount % currentFilterSize == 0) {
		imgCount = 0;
	}
	inputImage.deepCopy(&inputImagesF.at(imgCount % currentFilterSize));
	imgCount++;

	if (useCL == true && clReady == true) {
#ifdef HAVE_OPENCL
		try {
			inputImage1ArrayF=inputImagesF.at(imgCount-1).begin(0);
			inputImageBufferF.write(inputImage1ArrayF, w*h * sizeof(float), (imgCount-1)*w*h* sizeof(float));

			kernelTemporalSmoothingFloat.setArgs(inputImageBufferF,
					outputImageBufferF,
					currentFilterSize,
					imgCount,
					w,
					h,
					currentDifference,
					nullValue,
					motionImageBuffer);//set parameter for kernel
			kernelTemporalSmoothingFloat.apply(w*h);
			outputImageBufferF.read(outputImageArrayF, w*h * sizeof(float));
			outputImageF = Img32f(Size(w,h),1,std::vector<float*>(1,outputImageArrayF),false);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				int count = 0;
				float value = 0;
				float min = 100000;
				float max = 0;

				for (int i = 0; i < currentFilterSize; i++) {
					if (inputImagesF.at(i)(x, y, 0) != nullValue) {
						count++;
						value += inputImagesF.at(i)(x, y, 0);
						if (inputImagesF.at(i)(x, y, 0) < min)
							min = inputImagesF.at(i)(x, y, 0);
						if (inputImagesF.at(i)(x, y, 0) > max)
							max = inputImagesF.at(i)(x, y, 0);
					}
				}

				if (count == 0) {
					outputImageF(x, y, 0) = nullValue;
				} else {
					if ((max - min) > currentDifference) {
						outputImageF(x, y, 0) = inputImagesF.at(imgCount - 1)(x,
								y, 0);
					} else {
						outputImageF(x, y, 0) = value / count;
					}
				}
			}
		}
	}
	return outputImageF;
}

Img8u TemporalSmoothingCL::temporalSmoothingC(Img8u &inputImage) {
	if (filterSize > maxFilterSize) {
		filterSize = maxFilterSize;
		std::cout << "set filter size to maximum (" << maxFilterSize << ")"
				<< std::endl;
	}
	if (currentFilterSize != filterSize) {
		currentFilterSize = filterSize;
		imgCount = 0;
	}

	if (imgCount % currentFilterSize == 0) {
		imgCount = 0;
	}
	inputImage.deepCopy(&inputImagesC.at(imgCount % currentFilterSize));
	imgCount++;

	if (useCL == true && clReady == true) {
#ifdef HAVE_OPENCL
		try {
			inputImage1ArrayC=inputImagesC.at(imgCount-1).begin(0);

			inputImageBufferC.write(inputImage1ArrayC, w*h * sizeof(unsigned char), (imgCount-1)*w*h* sizeof(unsigned char));

			kernelTemporalSmoothingChar.setArgs(inputImageBufferC,
					outputImageBufferC,
					currentFilterSize,
					imgCount,
					w,
					h,
					currentDifference,
					nullValue,
					motionImageBuffer);//set parameter for kernel
			kernelTemporalSmoothingChar.apply(w*h);
			outputImageBufferC.read(outputImageArrayC, w*h * sizeof(unsigned char));
			outputImageC = Img8u(Size(w,h),1,std::vector<unsigned char*>(1,outputImageArrayC),false);

		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				int count = 0;
				int value = 0;
				int min = 100000;
				int max = 0;

				for (int i = 0; i < currentFilterSize; i++) {
					if (inputImagesC.at(i)(x, y, 0) != nullValue) {
						count++;
						value += inputImagesC.at(i)(x, y, 0);
						if (inputImagesC.at(i)(x, y, 0) < min)
							min = inputImagesC.at(i)(x, y, 0);
						if (inputImagesC.at(i)(x, y, 0) > max)
							max = inputImagesC.at(i)(x, y, 0);
					}
				}

				if (count == 0) {
					outputImageC(x, y, 0) = nullValue;
				} else {
					if ((max - min) > currentDifference) {
						outputImageC(x, y, 0) = inputImagesC.at(imgCount - 1)(x,
								y, 0);
					} else {
						outputImageC(x, y, 0) = (unsigned char) ((float) value
								/ (float) count);
					}
				}
			}
		}
	}
	return outputImageC;
}

void TemporalSmoothingCL::setUseCL(bool use) {
	useCL = use;
}

void TemporalSmoothingCL::setFilterSize(int iFilterSize) {
	filterSize = iFilterSize;
}

void TemporalSmoothingCL::setDifference(int iDifference) {
	currentDifference = iDifference;
}

Img32f TemporalSmoothingCL::getMotionImage() {
	if (useCL == true && clReady == true) {
#ifdef HAVE_OPENCL
		motionImageBuffer.read(motionImageArray, w*h * sizeof(float));
		motionImage = Img32f(Size(w,h),1,std::vector<float*>(1,motionImageArray),false);
#endif
	}
	return motionImage;
}

bool TemporalSmoothingCL::isCLReady() {
	return clReady;
}

bool TemporalSmoothingCL::isCLActive() {
	return useCL;
}

} // namespace filter
}

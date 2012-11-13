/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/MotionSensitiveTemporalSmoothing.cpp     **
** Module : ICLFilter                                              **
** Authors: Andre Ueckermann                                       **
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

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching

#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLCore/Img.h>

#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
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

   
    MotionSensitiveTemporalSmoothing::MotionSensitiveTemporalSmoothing(int iNullValue, int iMaxFilterSize){
      //addProperty("use opencl","flag","",true);
      //addProperty("filter size","range:slider","[1,15]",10);
      //addProperty("difference","range:slider","[1,15]",10); 
      
      size=utils::Size(0,0);
      numChannels=0;  
      maxFilterSize=iMaxFilterSize;
      nullValue=iNullValue;
      
      currentFilterSize=6;
      currentDifference=10;
      useCL=true;
    }
    
    
    MotionSensitiveTemporalSmoothing::~MotionSensitiveTemporalSmoothing(){
      for(int i=clPointer.size()-1; i>=0; i--){
        delete &clPointer.at(i);
      }
      clPointer.clear(); 
    }

    
    void MotionSensitiveTemporalSmoothing::init(int iChannels, core::depth iDepth, utils::Size iSize){
      std::cout<<"maxFilterSize: "<<maxFilterSize<<" , nullValue: "<<nullValue<<std::endl;
      numChannels=iChannels;
      depth=iDepth;
      size=iSize;
      std::cout<<"channels: "<<numChannels<<" , imageSize: "<<size<<" , imageDepth: "<<depth<<std::endl;
      for(int i=clPointer.size()-1; i>=0; i--){
        delete &clPointer.at(i);
      }
      clPointer.clear();
      for(int i=0; i<numChannels; i++){
        TemporalSmoothingCL* clElement = new TemporalSmoothingCL(size, depth, maxFilterSize, nullValue);
        clElement->setFilterSize(currentFilterSize);
        clElement->setDifference(currentDifference);
        clElement->setUseCL(useCL);
        clPointer.push_back(clElement);
      }
    }
  
  
    void MotionSensitiveTemporalSmoothing::apply(const ImgBase *poSrc, ImgBase **ppoDst){
      ICLASSERT_RETURN( poSrc );
      ICLASSERT_RETURN( ppoDst );
      ICLASSERT_RETURN( poSrc != *ppoDst);
      
      //filterSize = getPropertyValue("filter size");
      //useCL = getPropertyValue("use opencl");
      //difference = getPropertyValue("difference");
        
      if(!poSrc->hasFullROI()) throw ICLException("MotionSensitiveTemporalSmoothing::apply: no roi supported");
   
      if(poSrc->getDepth () != depth8u && poSrc->getDepth() != depth32f) throw ICLException("MotionSensitiveTemporalSmoothing::apply: depth 32f and 8u only");
        
      if(!prepare (ppoDst, poSrc)) return;
  
      if(poSrc->getChannels() != numChannels || poSrc->getDepth() != depth || poSrc->getSize() != size){
        init(poSrc->getChannels(), poSrc->getDepth(), poSrc->getSize());
      }
  
      if(poSrc->getDepth() == depth8u){
        Img8u src = *poSrc->as8u();
        Img8u &dst = *(*ppoDst)->as8u();
        for(int i=0; i<numChannels; i++){
          Img8u in = (*src.selectChannel(i));
          Img8u out = clPointer.at(i)->temporalSmoothingC(in);
          dst.replaceChannel(i, &out, 0);
        }
      }else{
        Img32f src = *poSrc->as32f();
        Img32f &dst = *(*ppoDst)->as32f();
        for(int i=0; i<numChannels; i++){
          Img32f in = (*src.selectChannel(i));
          Img32f out = clPointer.at(i)->temporalSmoothingF(in);
          dst.replaceChannel(i, &out, 0);
        }
      }
    }
    
    
    void MotionSensitiveTemporalSmoothing::setUseCL(bool use){
      //setPropertyValue("use opencl",use);
      useCL=use;
      for(unsigned int i=0; i<clPointer.size(); i++){
        clPointer.at(i)->setUseCL(use);
      }
    }
    
    
    void MotionSensitiveTemporalSmoothing::setFilterSize(int filterSize){
      //setPropertyValue("filter size",filterSize);
      currentFilterSize=filterSize;
      for(unsigned int i=0; i<clPointer.size(); i++){
        clPointer.at(i)->setFilterSize(filterSize);
      }
    }
    
    
    void MotionSensitiveTemporalSmoothing::setDifference(int difference){
      //setPropertyValue("difference",difference);
      currentDifference=difference;
      for(unsigned int i=0; i<clPointer.size(); i++){
        clPointer.at(i)->setDifference(difference);
      }
    }
    
  
	  bool MotionSensitiveTemporalSmoothing::isCLActive(){
	    return useCL;
	  }
    
    
    
    TemporalSmoothingCL::TemporalSmoothingCL(utils::Size size, core::depth depth, int iMaxFilterSize, int iNullValue){
      w=size.width;
      h=size.height;
      d=depth;
      maxFilterSize=iMaxFilterSize;
      nullValue=iNullValue;
      
      filterSize=maxFilterSize/2;
      currentFilterSize=maxFilterSize/2;
      currentDifference=10;
      
      imgCount=0;
      useCL=true;
      
      #ifdef HAVE_OPENCL
      //create openCL context
      motionImage.setSize(Size(w,h));
      motionImage.setChannels(1);
      if(depth==depth32f){
        for(int i=0; i<maxFilterSize; i++){
          Img32f inputImage;
          inputImage.setSize(Size(w,h));
          inputImage.setChannels(1);
          inputImagesF.push_back(inputImage);
        }
        outputImageF.setSize(Size(w,h));
        outputImageF.setChannels(1);
      }else if(depth==depth8u){
        for(int i=0; i<maxFilterSize; i++){
          Img8u inputImage;
          inputImage.setSize(Size(w,h));
          inputImage.setChannels(1);
          inputImagesC.push_back(inputImage);
        }
        outputImageC.setSize(Size(w,h));
        outputImageC.setChannels(1);
      }
      
      motionImageArray = new float[w*h];
      if(depth==depth32f){
        inputImage1ArrayF = new float[w*h];
        inputImagesArrayF = new float[w*h*maxFilterSize];
        outputImageArrayF = new float[w*h];
      }else if(depth==depth8u){
        inputImage1ArrayC = new cl_uchar[w*h];
        inputImagesArrayC = new cl_uchar[w*h*maxFilterSize];
        outputImageArrayC = new cl_uchar[w*h];
      }else{
        std::cout<<"Unsupported Depth"<<std::endl;
      }
      
      //init
      if(depth==depth32f){
        for(int i=0; i<w*h; i++){
          inputImage1ArrayF[i]=0;
          outputImageArrayF[i]=0;
          motionImageArray[i]=0;
        }
        for(int i=0; i<w*h*maxFilterSize; i++){
          inputImagesArrayF[i]=0;
        }
      }else if(depth==depth8u){
        for(int i=0; i<w*h; i++){
          inputImage1ArrayC[i]=0;
          outputImageArrayC[i]=0;
          motionImageArray[i]=0;
        }
        for(int i=0; i<w*h*maxFilterSize; i++){
          inputImagesArrayC[i]=0;
        }
      }
      std::vector<cl::Platform> platformList;//get number of available openCL platforms
      int selectedDevice=0;//initially select platform 0
      try{
        if(cl::Platform::get(&platformList)==CL_SUCCESS){
          std::cout<<"openCL platform found"<<std::endl;
        }else{
          std::cout<<"no openCL platform available"<<std::endl;
        }
        std::cout<<"number of openCL platforms: "<<platformList.size()<<std::endl;
          
        //check devices on platforms
        for(unsigned int i=0; i<platformList.size(); i++){//check all platforms
          std::cout<<"platform "<<i+1<<":"<<std::endl;
          std::vector<cl::Device> deviceList;
          if(platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList)==CL_SUCCESS){
            std::cout<<"GPU-DEVICE(S) FOUND"<<std::endl;
            selectedDevice=i; //if GPU found on platform, select this platform
            clReady=true; //and mark CL context as available
          }else if(platformList.at(i).getDevices(CL_DEVICE_TYPE_CPU, &deviceList)==CL_SUCCESS){
            std::cout<<"CPU-DEVICE(S) FOUND"<<std::endl;
          }else{
            std::cout<<"UNKNOWN DEVICE(S) FOUND"<<std::endl;
          }
          std::cout<<"number of devices: "<<deviceList.size()<<std::endl;
        }
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        std::cout<<"OpenCL not ready"<<std::endl;
        clReady=false;//disable openCL on error
      }  
        
      if(clReady==true){//only if CL context is available
        try{
          cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[selectedDevice])(), 0};//get context properties of selected platform
          context = cl::Context(CL_DEVICE_TYPE_GPU, cprops);//select GPU device
          devices = context.getInfo<CL_CONTEXT_DEVICES>();
                
          std::cout<<"selected devices: "<<devices.size()<<std::endl;
                
          cl::Program::Sources sources(1, std::make_pair(timeSmoothingKernel, 0)); //kernel source
          program=cl::Program(context, sources); //program (bind context and source)
          program.build(devices);//build program
          
          motionImageBuffer = cl::Buffer(
				    context, 
				    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				    w*h * sizeof(float), 
				    (void *) &motionImageArray[0]);
          if(depth==depth32f){
            inputImageBufferF = cl::Buffer(
			          context, 
			          CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
			          w*h * sizeof(float) * maxFilterSize, 
				        (void *) &inputImagesArrayF[0]);
		        outputImageBufferF = cl::Buffer(
				        context, 
				        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				        w*h * sizeof(float), 
				        (void *) &outputImageArrayF[0]);
		      }else if(depth==depth8u){
            inputImageBufferC = cl::Buffer(
				        context, 
				        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				        w*h * sizeof(cl_uchar) * maxFilterSize, 
				        (void *) &inputImagesArrayC[0]);
		        outputImageBufferC = cl::Buffer(
				        context, 
				        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				        w*h * sizeof(cl_uchar), 
				        (void *) &outputImageArrayC[0]);
		     }
				                    
          //create kernels    
          kernelTemporalSmoothingFloat=cl::Kernel(program, "temporalSmoothingFloat");
          kernelTemporalSmoothingChar=cl::Kernel(program, "temporalSmoothingChar");
          
          queue=cl::CommandQueue(context, devices[0], 0);//create command queue        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
          clReady=false;//disable openCL on error
        }
      }
        
    #else
      std::cout<<"no openCL parallelization available"<<std::endl;
      clReady=false;
    #endif   
    
    }
    
    
    TemporalSmoothingCL::~TemporalSmoothingCL(){
      #ifdef HAVE_OPENCL
      if(d==depth32f){
        delete[] inputImage1ArrayF;
        delete[] inputImagesArrayF;
        delete[] outputImageArrayF;
      }else if(d==depth8u){  
        delete[] inputImage1ArrayC;
        delete[] inputImagesArrayC;
        delete[] outputImageArrayC;
      }
      delete[] motionImageArray;
      #endif
    }

     
    Img32f TemporalSmoothingCL::temporalSmoothingF(Img32f &inputImage){
      if(filterSize>maxFilterSize){
        filterSize=maxFilterSize;
        std::cout<<"set filter size to maximum ("<<maxFilterSize<<")"<<std::endl;
      }
      if(currentFilterSize!=filterSize){
        currentFilterSize=filterSize;
        imgCount=0;
      }
            
      if(imgCount%currentFilterSize==0){
    	  imgCount=0;
    	}
    	inputImage.deepCopy(&inputImagesF.at(imgCount%currentFilterSize));
    	imgCount++;

      if(useCL==true && clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          inputImage1ArrayF=inputImagesF.at(imgCount-1).begin(0);
          queue.enqueueWriteBuffer(//read output from kernel
			          inputImageBufferF,
			          CL_TRUE, // block 
			          (imgCount-1)*w*h* sizeof(float),
			          w*h * sizeof(float),
			          (float*) inputImage1ArrayF);
          
          kernelTemporalSmoothingFloat.setArg(0, inputImageBufferF);//set parameter for kernel
          kernelTemporalSmoothingFloat.setArg(1, outputImageBufferF);
          kernelTemporalSmoothingFloat.setArg(2, currentFilterSize);
          kernelTemporalSmoothingFloat.setArg(3, imgCount);
          kernelTemporalSmoothingFloat.setArg(4, w);
          kernelTemporalSmoothingFloat.setArg(5, h);
          kernelTemporalSmoothingFloat.setArg(6, currentDifference);
          kernelTemporalSmoothingFloat.setArg(7, nullValue);
          kernelTemporalSmoothingFloat.setArg(8, motionImageBuffer);
                  
          queue.enqueueNDRangeKernel(//run kernel
				     kernelTemporalSmoothingFloat, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);   
    	    
          queue.enqueueReadBuffer(//read output from kernel
			          outputImageBufferF,
			          CL_TRUE, // block 
			          0,
			          w*h * sizeof(float),
			          (float*) outputImageArrayF);
			    outputImageF = Img32f(Size(w,h),1,std::vector<float*>(1,outputImageArrayF),false);
			          
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        for(int x=0; x<w; x++){
      	  for(int y=0; y<h; y++){
      	    int count=0;
      	    float value=0;
      	    float min=100000;
      	    float max=0;
      	    
      	    for(int i=0; i<currentFilterSize; i++){
      	      if(inputImagesF.at(i)(x,y,0)!=nullValue){
      	        count++;
      	        value+=inputImagesF.at(i)(x,y,0);
      	        if(inputImagesF.at(i)(x,y,0)<min) min=inputImagesF.at(i)(x,y,0);
      	        if(inputImagesF.at(i)(x,y,0)>max) max=inputImagesF.at(i)(x,y,0);
      	      }
      	    }
      	    
      	    if(count==0){
      	      outputImageF(x,y,0)=nullValue;
      	    }else{
      	      if((max-min)>currentDifference){
      	        outputImageF(x,y,0)=inputImagesF.at(imgCount-1)(x,y,0);
      	      }else{
      	        outputImageF(x,y,0)=value/count;
      	      }
      	    }
      	  }
      	}
      }
      return outputImageF;
    }
    
    
    Img8u TemporalSmoothingCL::temporalSmoothingC(Img8u &inputImage){
      if(filterSize>maxFilterSize){
        filterSize=maxFilterSize;
        std::cout<<"set filter size to maximum ("<<maxFilterSize<<")"<<std::endl;
      }
      if(currentFilterSize!=filterSize){
        currentFilterSize=filterSize;
        imgCount=0;
      }
            
      if(imgCount%currentFilterSize==0){
    	  imgCount=0;
    	}
    	inputImage.deepCopy(&inputImagesC.at(imgCount%currentFilterSize));
    	imgCount++;

      if(useCL==true && clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          inputImage1ArrayC=inputImagesC.at(imgCount-1).begin(0);
          queue.enqueueWriteBuffer(//read output from kernel
			          inputImageBufferC,
			          CL_TRUE, // block 
			          (imgCount-1)*w*h* sizeof(cl_uchar),
			          w*h * sizeof(cl_uchar),
			          (cl_uchar*) inputImage1ArrayC);
          
          kernelTemporalSmoothingChar.setArg(0, inputImageBufferC);//set parameter for kernel
          kernelTemporalSmoothingChar.setArg(1, outputImageBufferC);
          kernelTemporalSmoothingChar.setArg(2, currentFilterSize);
          kernelTemporalSmoothingChar.setArg(3, imgCount);
          kernelTemporalSmoothingChar.setArg(4, w);
          kernelTemporalSmoothingChar.setArg(5, h);
          kernelTemporalSmoothingChar.setArg(6, currentDifference);
          kernelTemporalSmoothingChar.setArg(7, nullValue);
          kernelTemporalSmoothingChar.setArg(8, motionImageBuffer);
                  
          queue.enqueueNDRangeKernel(//run kernel
				     kernelTemporalSmoothingChar, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);   
    	    
          queue.enqueueReadBuffer(//read output from kernel
			          outputImageBufferC,
			          CL_TRUE, // block 
			          0,
			          w*h * sizeof(cl_uchar),
			          (cl_uchar*) outputImageArrayC);
			    outputImageC = Img8u(Size(w,h),1,std::vector<unsigned char*>(1,outputImageArrayC),false);
			          
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        for(int x=0; x<w; x++){
      	  for(int y=0; y<h; y++){
      	    int count=0;
      	    int value=0;
      	    int min=100000;
      	    int max=0;
      	    
      	    for(int i=0; i<currentFilterSize; i++){
      	      if(inputImagesC.at(i)(x,y,0)!=nullValue){
      	        count++;
      	        value+=inputImagesC.at(i)(x,y,0);
      	        if(inputImagesC.at(i)(x,y,0)<min) min=inputImagesC.at(i)(x,y,0);
      	        if(inputImagesC.at(i)(x,y,0)>max) max=inputImagesC.at(i)(x,y,0);
      	      }
      	    }
      	    
      	    if(count==0){
      	      outputImageC(x,y,0)=nullValue;
      	    }else{
      	      if((max-min)>currentDifference){
      	        outputImageC(x,y,0)=inputImagesC.at(imgCount-1)(x,y,0);
      	      }else{
      	        outputImageC(x,y,0)=(unsigned char)((float)value/(float)count);
      	      }
      	    }
      	  }
      	}
      }
      return outputImageC;
    }
   
    
    void TemporalSmoothingCL::setUseCL(bool use){
      useCL=use;
    }
    
    
    void TemporalSmoothingCL::setFilterSize(int iFilterSize){
      filterSize=iFilterSize;
    }
    
    void TemporalSmoothingCL::setDifference(int iDifference){
      currentDifference=iDifference;
    }
    
    Img32f TemporalSmoothingCL::getMotionImage(){
      if(useCL==true && clReady==true){
      #ifdef HAVE_OPENCL
        queue.enqueueReadBuffer(//read output from kernel
			            motionImageBuffer,
			            CL_TRUE, // block 
			            0,
			            w*h * sizeof(float),
			            (float*) motionImageArray);
		    motionImage = Img32f(Size(w,h),1,std::vector<float*>(1,motionImageArray),false);
      #endif
      }
      return motionImage;
    }
    
    bool TemporalSmoothingCL::isCLReady(){
      return clReady;
    }
    
    
	  bool TemporalSmoothingCL::isCLActive(){
	    return useCL;
	  }
   
  } // namespace filter
}

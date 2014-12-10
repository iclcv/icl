/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SegmenterUtils.cpp                 **
** Module : ICLGeom                                                **
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

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching

#include <ICLGeom/SegmenterUtils.h>

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLMath/DynMatrix.h>

#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#endif

namespace icl{
  namespace geom{
    
    #ifdef ICL_HAVE_OPENCL
    //OpenCL kernel code
    static char utilsKernel[] = 
      "  #pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable                                                           \n"
      "__kernel void                                                                                                                  \n"
      "segmentColoring(__global int const * assignment, __global uchar * colorR, __global uchar * colorG, __global uchar * colorB)    \n"
      "{                                                                                                                              \n"
      "  size_t id =  get_global_id(0);                                                                                               \n"
      "  if(assignment[id]==0)                                                                                                        \n"
      "  {                                                                                                                            \n"
      "    colorR[id]=128;                                                                                                            \n"
      "    colorG[id]=128;                                                                                                            \n"
      "    colorB[id]=128;                                                                                                            \n"
      "  }                                                                                                                            \n"
      "  else                                                                                                                         \n"
      "  {                                                                                                                            \n"
      "    int H=(int)(assignment[id]*35.)%360;                                                                                       \n"
      "    float S=1.0-assignment[id]*0.01;                                                                                           \n"
      "    float hi=floor((float)H/60.);                                                                                              \n"
      "	   float f=((float)H/60.)-hi;                                                                                                 \n"
      "	   float pp=1.0-S;                                                                                                            \n"
      "	   float qq=1.0-S*f;                                                                                                          \n"
      "	   float tt=1.0-S*(1.-f);                                                                                                     \n"
      "	   float newR=0;                                                                                                              \n"
      "	   float newG=0;                                                                                                              \n"
      "	   float newB=0;                                                                                                              \n"
      "	   if((int)hi==0 || (int)hi==6){                                                                                              \n"
      "	  	 newR=1.0;                                                                                                                \n"
      "	     newG=tt;                                                                                                                 \n"
      "		   newB=pp;                                                                                                                 \n"
      "	   }else if((int)hi==1){                                                                                                      \n"
      "		   newR=qq;                                                                                                                 \n"
      "		   newG=1.0;                                                                                                                \n"
      "		   newB=pp;                                                                                                                 \n"
      "	   }else if((int)hi==2){                                                                                                      \n"
      "		   newR=pp;                                                                                                                 \n"
      "		   newG=1.0;                                                                                                                \n"
      "	     newB=tt;                                                                                                                 \n"
      "	   }else if((int)hi==3){                                                                                                      \n"
      "		   newR=pp;                                                                                                                 \n"
      "	     newG=qq;                                                                                                                 \n"
      "	     newB=1.0;                                                                                                                \n"
      "	   }else if((int)hi==4){                                                                                                      \n"
      "		   newR=tt;                                                                                                                 \n"
      "		   newG=pp;                                                                                                                 \n"
      "		   newB=1.0;                                                                                                                \n"
      "	   }else if((int)hi==5){                                                                                                      \n"
      "	     newR=1.0;                                                                                                                \n"
      "		   newG=pp;                                                                                                                 \n"
      "		   newB=qq;                                                                                                                 \n"
      "	   }                                                                                                                          \n"
      "    colorR[id]=(unsigned char)(newR*255.);                                                                                     \n"
      "    colorG[id]=(unsigned char)(newG*255.);                                                                                     \n"
      "    colorB[id]=(unsigned char)(newB*255.);                                                                                     \n"
      "  }                                                                                                                            \n"
      "}                                                                                                                              \n"
      "__kernel void                                                                                                                  \n"
      "calculatePointAssignment(__global float4 const * xyz, __global uchar * mask, __global int const * assignment,                  \n"
      "                         int const radius, int const numFaces, __global uchar * neighbours, __global int * assignmentOut,       \n"
      "                         int const w, int const h, float const maxDist)                                                        \n"
      "{                                                                                                                              \n"
      "  int x = get_global_id(0);                                                                                                    \n"
      "  int y = get_global_id(1);                                                                                                    \n"
      "  size_t id = x+y*w;                                                                                                           \n"
      "  float dist=100000;                                                                                                           \n"
      "  int ass=0;                                                                                                                   \n"
      "  bool assigned=false;                                                                                                         \n"
      "  if(mask[id]==0 && assignment[id]==0){                                                                                        \n"
      "    bool adj[100];                                                                                                             \n"
      "    for(int a=0; a<numFaces; a++){                                                                                             \n"
      "      adj[a]=false;                                                                                                            \n"
      "    }                                                                                                                          \n"
      "    for(int xx=-radius; xx<=radius; xx++){                                                                                     \n"
      "      for(int yy=-radius; yy<=radius; yy++){                                                                                   \n"
      "        if(x+xx>=0 && x+xx<w && y+yy>=0 && y+yy<h && assignment[(x+xx)+w*(y+yy)]!=0){                                          \n"
      "          float4 pointa=xyz[id];                                                                                               \n"
      "          pointa.w=1.0;                                                                                                        \n"
      "          float4 pointb=xyz[(x+xx)+w*(y+yy)];                                                                                  \n"
      "          pointb.w=1.0;                                                                                                        \n"
      "          float dist3 = distance(pointa,pointb);                                                                               \n"
      "          if(dist3<maxDist){                                                                                                   \n"
      "            adj[assignment[(x+xx)+w*(y+yy)]-1]=true;                                                                           \n"
      "          }                                                                                                                    \n"
      "          if(dist3<dist && dist3<maxDist){                                                                                     \n"
      "            dist=dist3;                                                                                                        \n"
      "            ass=assignment[(x+xx)+w*(y+yy)];                                                                                   \n"
      "            assigned=true;                                                                                                     \n"
      "          }                                                                                                                    \n"
      "        }                                                                                                                      \n"
      "      }                                                                                                                        \n"
      "    }                                                                                                                          \n"
      "    for(int a=0; a<numFaces-1; a++){                                                                                           \n"
      "      for (int b=a+1; b<numFaces; b++){                                                                                        \n"
      "        if(adj[a]==true && adj[b]==true){                                                                                      \n"
      "          neighbours[a*numFaces+b]=1;                                                                                          \n"
      "          neighbours[b*numFaces+a]=1;                                                                                          \n"
      "        }                                                                                                                      \n"
      "      }                                                                                                                        \n"
      "    }                                                                                                                          \n"
      "    if(assigned==true){                                                                                                        \n"
      "      assignmentOut[id]=ass;                                                                                                   \n"
      "      mask[id]=1;                                                                                                              \n"
      "    }                                                                                                                          \n"
      "    else                                                                                                                       \n"
      "    {                                                                                                                          \n"
      "      assignmentOut[id]=assignment[id];                                                                                        \n"
      "    }                                                                                                                          \n"
      "  }                                                                                                                            \n"
      "  else                                                                                                                         \n"
      "  {                                                                                                                            \n"
      "    assignmentOut[id]=assignment[id];                                                                                          \n"
      "  }                                                                                                                            \n"
      "}                                                                                                                              \n"
    ;
    #endif  
    
    
    struct SegmenterUtils::Data {
	    Data(Mode mode) {
		    clReady = false;
		    kernelSegmentColoringInitialized=false;
	      kernelPointAssignmentInitialized=false;
	      size = utils::Size(0,0);
	      stabelizeCounter=0;

        if(mode==BEST || mode==GPU){
          useCL=true;
        }else{
          useCL=false;
        } 
	    }

	    ~Data() {
	    }

	    bool clReady;
	    bool useCL;
	    
	    utils::Size size;	    
	    bool kernelSegmentColoringInitialized;
	    bool kernelPointAssignmentInitialized;
	    
	    int stabelizeCounter;
	    core::Img32s lastLabelImage;
	    
	    #ifdef ICL_HAVE_OPENCL
        //OpenCL data
        std::vector<unsigned char> segmentColorImageRArray;
        std::vector<unsigned char> segmentColorImageGArray;
        std::vector<unsigned char> segmentColorImageBArray;
        
        std::vector<unsigned char> maskArray;
        std::vector<int> assignmentArray;
        
        //OpenCL    
        utils::CLProgram program;
        utils::CLKernel kernelSegmentColoring;
        utils::CLKernel kernelPointAssignment;

        //OpenCL buffer
        utils::CLBuffer segmentColorImageRBuffer;
        utils::CLBuffer segmentColorImageGBuffer;
        utils::CLBuffer segmentColorImageBBuffer;
        utils::CLBuffer assignmentBuffer;
        
        utils::CLBuffer neighboursBuffer;
	      utils::CLBuffer xyzBuffer;
	      utils::CLBuffer maskBuffer;
	      utils::CLBuffer assignmentOutBuffer;
      #endif
    };
    

    SegmenterUtils::SegmenterUtils(Mode mode) :
	    m_data(new Data(mode)) {
	    
	    if(m_data->useCL==true){
	      #ifdef ICL_HAVE_OPENCL
	        try
	        {
		        m_data->program = utils::CLProgram("gpu", utilsKernel);
		        m_data->kernelSegmentColoring = m_data->program.createKernel("segmentColoring");
		        m_data->kernelPointAssignment = m_data->program.createKernel("calculatePointAssignment");
		        m_data->clReady = true;
	        } catch (utils::CLException &err) { //catch openCL errors
		        std::cout<< "ERROR: "<< err.what()<< std::endl;
		        m_data->clReady = false;

	        }

        #else
	        std::cout << "no openCL parallelization available" << std::endl;
          m_data->clReady = false;
        #endif
	    }
    }


    SegmenterUtils::~SegmenterUtils() {
	    delete m_data;
    }
    
    
    core::Img8u SegmenterUtils::createColorImage(core::Img32s &labelImage){
      core::Img8u colorImage;
      if(m_data->useCL==true && m_data->clReady==true){
        createColorImageCL(labelImage, colorImage);
      }else{
        createColorImageCPU(labelImage, colorImage);
      }
      return colorImage;
    }
    
        
    core::Img8u SegmenterUtils::createROIMask(core::DataSegment<float,4> &xyzh, core::Img32f &depthImage, 
                float xMin, float xMax, float yMin, float yMax, float zMin, float zMax){
      utils::Size size = depthImage.getSize();
      core::Img8u maskImage(size,1,core::formatMatrix);
      core::Channel8u maskImageC = maskImage[0];
      core::Channel32f depthImageC = depthImage[0];
      for(int y=0;y<size.height;++y){
        for(int x=0;x<size.width;++x){
          int i = x+size.width*y;	 
          if(xyzh[i][0]<xMin || xyzh[i][0]>xMax || xyzh[i][1]<yMin || xyzh[i][1]>yMax || xyzh[i][2]<zMin || xyzh[i][2]>zMax){           
            maskImageC(x,y)=1;
          }else{		
            maskImageC(x,y)=0;
          }
          if(depthImageC(x,y)==2047){ 
            maskImageC(x,y)=1;
          }
        }    
      }
      return maskImage;
    }
    
    
    core::Img8u SegmenterUtils::createMask(core::Img32f &depthImage){
      utils::Size size = depthImage.getSize();
      core::Img8u maskImage(size,1,core::formatMatrix);
      core::Channel8u maskImageC = maskImage[0];
      core::Channel32f depthImageC = depthImage[0];
      for(int y=0;y<size.height;++y){
        for(int x=0;x<size.width;++x){
          maskImageC(x,y)=0;
          if(depthImageC(x,y)==2047){ 
            maskImageC(x,y)=1;
          }
        }
      }
      return maskImage;
    }
    
    
    core::Img32s SegmenterUtils::stabelizeSegmentation(core::Img32s &labelImage){
      core::Img32s stableLabelImage(labelImage.getSize(),1,core::formatMatrix);
     	
     	core::Channel32s labelImageC = labelImage[0];
      core::Channel32s lastLabelImageC = m_data->lastLabelImage[0];
      core::Channel32s stableLabelImageC = stableLabelImage[0];
     	
     	utils::Size size = labelImage.getSize();			 
      if(m_data->stabelizeCounter==0){//first image
        labelImage.deepCopy(&m_data->lastLabelImage);
      }else{
        //count number of segments of previous and current label image
        int countCur=0;
        int countLast=0;
        for(int y=0; y<size.height; y++){
          for(int x=0; x<size.width; x++){
            if(labelImageC(x,y)>countCur){
              countCur=labelImageC(x,y);
            }
            if(lastLabelImageC(x,y)>countLast){
              countLast=lastLabelImageC(x,y);
            }
          }
        }
        
        if(countCur==0 || countLast==0){//no relabeling possible
            labelImage.deepCopy(&m_data->lastLabelImage);
            return labelImage;
        }
        
        std::vector<int> curAss = calculateLabelReassignment(countCur, countLast, labelImageC, lastLabelImageC, size);
				 	
        for(int y=0; y<size.height; y++){//reassign label
          for(int x=0; x<size.width; x++){   
            if(labelImageC(x,y)>0){
              stableLabelImageC(x,y)=curAss[labelImageC(x,y)-1];
            }else{
              stableLabelImageC(x,y)=0;
            }
          }
        }
				 	
				stableLabelImage.deepCopy(&m_data->lastLabelImage);//copy image for next iteration 	
				 	
      }
      m_data->stabelizeCounter=1;
				 
      return stableLabelImage;
    }
    
    
    math::DynMatrix<bool> SegmenterUtils::calculateAdjacencyMatrix(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage, 
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces){
      math::DynMatrix<bool> adjacencyMatrix;
    	if(m_data->useCL==true && m_data->clReady==true){
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCL(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, false);
    	}else{
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCPU(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, false);
    	}
      return adjacencyMatrix;
    }
    
    
    void SegmenterUtils::edgePointAssignment(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage, 
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces){
      math::DynMatrix<bool> adjacencyMatrix;
      if(m_data->useCL==true && m_data->clReady==true){
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCL(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, true);
    	}else{
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCPU(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, true);
    	}
    }
    
    
    math::DynMatrix<bool> SegmenterUtils::edgePointAssignmentAndAdjacencyMatrix(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage, 
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces){
      math::DynMatrix<bool> adjacencyMatrix;
    	if(m_data->useCL==true && m_data->clReady==true){
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCL(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, true);
    	}else{
    	  adjacencyMatrix=edgePointAssignmentAndAdjacencyMatrixCPU(xyzh, labelImage, maskImage, radius, euclideanDistance, numSurfaces, true);
    	}
      return adjacencyMatrix;
    }
        
    
    std::vector<std::vector<int> > SegmenterUtils::extractSegments(core::Img32s &labelImage){
      int h=labelImage.getSize().height;
      int w=labelImage.getSize().width;
      core::Channel32s labelImageC = labelImage[0];
      std::vector<std::vector<int> > segments;
      for(int y=0; y<h; y++){
        for(int x=0; x<w; x++){
          if(labelImageC(x,y)>(int)segments.size()){
            segments.resize(labelImageC(x,y));
          }
          if(labelImageC(x,y)>0){
            segments.at(labelImageC(x,y)-1).push_back(x+y*w);
          }
        }
      }
      return segments;
    }
    
    
    void SegmenterUtils::relabel(core::Img32s &labelImage, std::vector<std::vector<int> > &assignment, int maxOldLabel){
      std::vector<int> mapping;
      if(maxOldLabel>0){
        mapping.resize(maxOldLabel,0);
      }else{
        int maxLabel=0;
        for(unsigned int i=0; i<assignment.size(); i++){
          for(unsigned int j=0; j<assignment[i].size(); j++){
            if(assignment[i][j]+1>maxLabel){
              maxLabel=assignment[i][j]+1;//assignment [0..n-1], label [1..n]
            }
          }
        }
        mapping.resize(maxLabel,0);
      }
      for(unsigned int i=0; i<assignment.size(); i++){//calculate mapping
        for(unsigned int j=0; j<assignment[i].size(); j++){
          mapping[assignment[i][j]]=i;
        }
      }
      int w = labelImage.getSize().width;
      int h = labelImage.getSize().height;
      core::Channel32s labelImageC = labelImage[0];
      for(int y=0; y<h; y++){//map
        for(int x=0; x<w; x++){
          if(labelImageC(x,y)>0){
            labelImageC(x,y)=mapping[labelImageC(x,y)-1]+1;
          }
        }
      }  
    }
    
    
    bool SegmenterUtils::occlusionCheck(core::Img32f &depthImage, utils::Point p1, utils::Point p2, float distanceTolerance, float outlierTolerance){
      core::Channel32f depthImageC = depthImage[0];
      bool sampleX=false;//over x or y
      int step=0;//positive or negative
      float startValue=depthImageC(p1.x,p1.y);
      float endValue=depthImageC(p2.x,p2.y);
      float depthGradient, gradient;

      if(abs(p2.x-p1.x)>abs(p2.y-p1.y)){//sample x init
        sampleX=true;
        gradient = (float)(p2.y-p1.y)/(float)(p2.x-p1.x);
        depthGradient=(endValue-startValue)/(p2.x-p1.x);
        if(p2.x-p1.x>0){
          step=1;
        }else{
          step=-1;
        }
      }else{//sample y init
        sampleX=false;
        gradient = (float)(p2.x-p1.x)/(float)(p2.y-p1.y);
        depthGradient=(endValue-startValue)/(p2.y-p1.y);
        if(p2.y-p1.y>0){
          step=1;
        }else{
          step=-1;
        }
      }
      
      int numReject=0;
      if(sampleX){//sample x process
        for(int i=p1.x; (i-p2.x)*step<=0; i+=step){
          int newY=(int)round(p1.y+(i-p1.x)*gradient);
          float realValue = depthImageC(i,newY);
          float augmentedValue = startValue+(i-p1.x)*depthGradient;
          float s1 = realValue-augmentedValue;//minus -> real closer than augmented
          if(s1-distanceTolerance>0 && depthImageC(i,newY)!=2047){//not occluding
            numReject++;
          }     
        }
        if((float)numReject/(float)(abs(p2.x-p1.x)+1)>outlierTolerance/100.){
          return false;
        }
      }else{//sample y process 
        for(int i=p1.y; (i-p2.y)*step<=0; i+=step){
          int newX=(int)round(p1.x+(i-p1.y)*gradient);
          float realValue = depthImageC(newX,i);
          float augmentedValue = startValue+(i-p1.y)*depthGradient;
          float s1 = realValue-augmentedValue;
          if(s1-distanceTolerance>0 && depthImageC(newX,i)!=2047){
            numReject++;
          }
        }
        if((float)numReject/(float)(abs(p2.y-p1.y)+1)>outlierTolerance/100.){
          return false;
        }
      }
      return true;             
    }
    
    
    void SegmenterUtils::createColorImageCL(core::Img32s &labelImage, core::Img8u &colorImage){
      #ifdef ICL_HAVE_OPENCL
        utils::Size s = labelImage.getSize();
        if(s!=m_data->size || m_data->kernelSegmentColoringInitialized==false){//reinit	      
	        m_data->size = s;
	        int w = s.width;
	        int h = s.height;
 
          m_data->segmentColorImageRArray.resize(w*h);
          m_data->segmentColorImageGArray.resize(w*h);
          m_data->segmentColorImageBArray.resize(w*h);
          m_data->segmentColorImageRBuffer = m_data->program.createBuffer("rw", w*h * sizeof(unsigned char));
          m_data->segmentColorImageGBuffer = m_data->program.createBuffer("rw", w*h * sizeof(unsigned char));
          m_data->segmentColorImageBBuffer = m_data->program.createBuffer("rw", w*h * sizeof(unsigned char));      
          m_data->assignmentBuffer = m_data->program.createBuffer("r", w*h * sizeof(int));
        
          m_data->kernelSegmentColoringInitialized=true;
        }
        
		    try {
		      int w = m_data->size.width;
	        int h = m_data->size.height;
			    m_data->assignmentBuffer.write(labelImage.begin(0),w*h*sizeof(int));
			    m_data->kernelSegmentColoring.setArgs(m_data->assignmentBuffer,
					    m_data->segmentColorImageRBuffer,
					    m_data->segmentColorImageGBuffer,
					    m_data->segmentColorImageBBuffer);

			    m_data->kernelSegmentColoring.apply(w*h);
			    m_data->segmentColorImageRBuffer.read(&m_data->segmentColorImageRArray[0], w*h * sizeof(unsigned char));
			    m_data->segmentColorImageGBuffer.read(&m_data->segmentColorImageGArray[0], w*h * sizeof(unsigned char));
			    m_data->segmentColorImageBBuffer.read(&m_data->segmentColorImageBArray[0], w*h * sizeof(unsigned char));

			    std::vector<icl8u*> data(3);
			    data[0] = m_data->segmentColorImageRArray.data();
			    data[1] = m_data->segmentColorImageGArray.data();
			    data[2] = m_data->segmentColorImageBArray.data();
			    colorImage = core::Img8u(utils::Size(w,h),3,data,false);

		    } catch (utils::CLException &err) { //catch openCL errors
			    std::cout<< "ERROR: "<< err.what() <<std::endl;
		    }
      #endif
    }
    
        
    void SegmenterUtils::createColorImageCPU(core::Img32s &labelImage, core::Img8u &colorImage){
      utils::Size s = labelImage.getSize();
      colorImage.setSize(s);
      colorImage.setChannels(3);
      for (int y = 0; y < s.height; y++) {
			  for (int x = 0; x < s.width; x++) {
				  if (labelImage(x,y,0) == 0) {
					  colorImage(x, y, 0) = 128;
					  colorImage(x, y, 1) = 128;
					  colorImage(x, y, 2) = 128;
				  } else {
					  int H = (int) (labelImage(x,y,0) * 35.) % 360;
					  float S = 1.0 - labelImage(x,y,0) * 0.01;
					  float hi = floor((float) H / 60.);
					  float f = ((float) H / 60.) - hi;
					  float pp = 1.0 - S;
					  float qq = 1.0 - S * f;
					  float tt = 1.0 - S * (1. - f);
					  float newR = 0;
					  float newG = 0;
					  float newB = 0;
					  if ((int) hi == 0 || (int) hi == 6) {
						  newR = 1.0;
						  newG = tt;
						  newB = pp;
					  } else if ((int) hi == 1) {
						  newR = qq;
						  newG = 1.0;
						  newB = pp;
					  } else if ((int) hi == 2) {
						  newR = pp;
						  newG = 1.0;
						  newB = tt;
					  } else if ((int) hi == 3) {
						  newR = pp;
						  newG = qq;
						  newB = 1.0;
					  } else if ((int) hi == 4) {
						  newR = tt;
						  newG = pp;
						  newB = 1.0;
					  } else if ((int) hi == 5) {
						  newR = 1.0;
						  newG = pp;
						  newB = qq;
					  }
					  colorImage(x, y, 0) = (unsigned char) (newR * 255.);
					  colorImage(x, y, 1) = (unsigned char) (newG * 255.);
					  colorImage(x, y, 2) = (unsigned char) (newB * 255.);
				  }
			  }
		  }
		  
    }
    
    
    std::vector<int> SegmenterUtils::calculateLabelReassignment(int countCur, int countLast, core::Channel32s &labelImageC, core::Channel32s &lastLabelImageC, utils::Size size){
      math::DynMatrix<int> assignmentMatrix(countCur,countLast,0);
      std::vector<int> lastNum(countLast,0);
      std::vector<int> curNum(countCur,0);				 	
      std::vector<int> curAss(countCur,0);
      std::vector<float> curVal(countCur,0);
      
      for(int y=0; y<size.height; y++){//count overlap points (cross-correlated)
        for(int x=0; x<size.width; x++){            
          if(labelImageC(x,y)>0 && lastLabelImageC(x,y)>0){
            assignmentMatrix(labelImageC(x,y)-1, lastLabelImageC(x,y)-1)++;//num match points
            lastNum[lastLabelImageC(x,y)-1]++;//num segment points
            curNum[labelImageC(x,y)-1]++;//num segment points
          }
        }
      }
      
      for(int i=0; i<countCur; i++){//calculate assignment score
        for(int j=0; j<countLast; j++){
          float curScore;
          float lastScore;
          float compScore;
          if(assignmentMatrix(i,j)>0){
            curScore=(float)assignmentMatrix(i,j)/(float)curNum[i];
            lastScore=(float)assignmentMatrix(i,j)/(float)lastNum[j];
            compScore=(curScore+lastScore)/2.;
          }
          else{
            curScore=0;
            lastScore=0;
            compScore=0;
          }
				 		
          if(curVal[i]<compScore){//assign highest score
            curVal[i]=compScore;
            curAss[i]=j+1;
          }
        }
      }
			 	
      std::vector<bool> empties(countCur,true);//find unassigned ids
      for(int i=0; i<countCur; i++){
        if(curAss[i]!=0){
          empties[curAss[i]-1]=false;
        }
        for(int j=i+1; j<countCur; j++){//multiple use of id (use highest score)
          if(curAss[i]==curAss[j]){
            if(curVal[i]<curVal[j]){
              curAss[i]=0;
            }else{
              curAss[j]=0;
            }
          }
        }
      }

      for(int i=0; i<countCur; i++){//assign unassigned ids to first empty id
        for(int j=0; j<countCur; j++){
          if(curAss[i]==0 && empties[j]==true){
            empties[j]=false;
            curAss[i]=j+1;
            break;
          }
        }
      }
      
      return curAss;
    }
    
    
    math::DynMatrix<bool> SegmenterUtils::edgePointAssignmentAndAdjacencyMatrixCL(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage, 
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces, bool pointAssignment){
      #ifdef ICL_HAVE_OPENCL
        utils::Size s = labelImage.getSize();
        math::DynMatrix<bool> neighbours(numSurfaces,numSurfaces,false);
        math::DynMatrix<unsigned char> neighboursC(numSurfaces,numSurfaces,(unsigned char)0);
        if(s!=m_data->size || m_data->kernelPointAssignmentInitialized==false){//reinit	      
	        m_data->size = s;
	        int w = s.width;
	        int h = s.height;
          
          m_data->maskArray.resize(w*h);
          m_data->assignmentArray.resize(w*h);
          
          m_data->assignmentBuffer = m_data->program.createBuffer("r", w*h * sizeof(int));
          m_data->assignmentOutBuffer = m_data->program.createBuffer("rw", w*h * sizeof(int));
          m_data->xyzBuffer = m_data->program.createBuffer("r", w*h * sizeof(Vec));
		      m_data->maskBuffer = m_data->program.createBuffer("rw", w*h * sizeof(unsigned char));
		      
		      m_data->kernelPointAssignmentInitialized=true;
        }
        
		    try {
		      int w = s.width;
	        int h = s.height;

          core::Img32s labelImageOut(labelImage.getSize(),1,core::formatMatrix);
          
          m_data->assignmentBuffer.write(labelImage.begin(0),w*h*sizeof(int));
          m_data->maskBuffer.write(maskImage.begin(0),w*h*sizeof(unsigned char));
          m_data->xyzBuffer.write(&xyzh[0][0],w*h*sizeof(Vec));//FixedColVector<float, 4>));
          
		      m_data->neighboursBuffer = m_data->program.createBuffer("rw", numSurfaces*numSurfaces * sizeof(unsigned char), &neighboursC[0]);
		      
		      m_data->kernelPointAssignment.setArgs(m_data->xyzBuffer,
				      m_data->maskBuffer,
				      m_data->assignmentBuffer,
				      radius,
				      numSurfaces,
				      m_data->neighboursBuffer,
				      m_data->assignmentOutBuffer,
				      w,
				      h,
				      euclideanDistance);
		      m_data->kernelPointAssignment.apply(w,h);
		      m_data->neighboursBuffer.read(neighboursC.data(),
				      numSurfaces*numSurfaces * sizeof(unsigned char));
				  for(unsigned int i=0; i<neighboursC.rows(); i++){
				    for(unsigned int j=0; j<neighboursC.cols(); j++){
				      neighbours(i,j)=(bool)neighboursC(i,j);
				    }
				  }
		      if(pointAssignment){
				    m_data->assignmentOutBuffer.read(m_data->assignmentArray.data(), w*h * sizeof(int));
				    labelImage = core::Img32s(utils::Size(w,h),1,std::vector<int*>(1,m_data->assignmentArray.data()),false);
				    m_data->maskBuffer.read(m_data->maskArray.data(), w*h * sizeof(unsigned char));
				    maskImage = core::Img8u(utils::Size(w,h),1,std::vector<unsigned char*>(1,m_data->maskArray.data()),false);    				        
          }
		      for(int i=0; i<numSurfaces; i++) {
			      neighbours(i,i)=true;
		      }
	      } catch (utils::CLException &err) { //catch openCL errors
		      std::cout<< "ERROR: "<< err.what()<< std::endl;
	      }
	      
	      return neighbours;
      #else
        return math::DynMatrix<bool>();
	    #endif
    }
    
        
    math::DynMatrix<bool> SegmenterUtils::edgePointAssignmentAndAdjacencyMatrixCPU(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage, 
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces, bool pointAssignment){                              
      utils::Size s = labelImage.getSize();
      int w = s.width;
      int h = s.height;
      math::DynMatrix<bool> neighbours(numSurfaces, numSurfaces, false);
      core::Img32s labelImageOut(labelImage.getSize(),1,core::formatMatrix);
      
      core::Channel32s labelImageC = labelImage[0];
      core::Channel32s labelImageOutC = labelImageOut[0];
      core::Channel8u maskImageC = maskImage[0];
      
      for (int x=0; x<w; x++) {
	      for(int y=0; y<h; y++) {
		      int i=x+w*y;
		      float dist=100000;
		      int ass=0;
		      bool assigned=false;
		      if(maskImageC(x,y)==0 && labelImageC(x,y)==0) {
			      std::vector<bool> adj(numSurfaces,false);
			      for(int xx=-radius; xx<=radius; xx++) {
				      for(int yy=-radius; yy<=radius; yy++) {
					      if(x+xx>=0 && x+xx<w && y+yy>=0 && y+yy<h && labelImageC(x+xx,y+yy)!=0) {
						      Vec p1=xyzh[i];
						      Vec p2=xyzh[(x+xx)+w*(y+yy)];
						      float distance=dist3(p1, p2);
						      if(distance<euclideanDistance) {
							      adj[labelImageC(x+xx,y+yy)-1]=true;
						      }
						      if(distance<dist && distance<euclideanDistance) {
							      dist=distance;
							      ass=labelImageC(x+xx,y+yy);
							      assigned=true;
						      }
					      }
				      }
			      }
			      for(int a=0; a<numSurfaces-1; a++) {
				      for (int b=a+1; b<numSurfaces; b++) {
					      if(adj[a]==true && adj[b]==true) {
						      neighbours(a,b)=true;
						      neighbours(b,a)=true;
					      }
				      }
			      }
			      if(pointAssignment){
			        if(assigned==true) {
				        maskImageC(x,y)=1;
				        labelImageOutC(x,y)=ass;
			        }
			        else {
				        labelImageOutC(x,y)=labelImageC(x,y);
			        }
			      }
		      }
		      else {
			      labelImageOutC(x,y)=labelImageC(x,y);
		      }
	      }
      }
      for (int i = 0; i < numSurfaces; i++) {
	      neighbours(i, i) = true;
      }
      if(pointAssignment==true) {
        labelImageOut.deepCopy(&labelImage);
      }
      return neighbours;
    }


  }
}

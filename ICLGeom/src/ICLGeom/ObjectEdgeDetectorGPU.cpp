/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorGPU.cpp          **
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
#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLProgram.h>

#include <ICLCore/Channel.h>
#include <ICLGeom/ObjectEdgeDetectorGPU.h>
#include <ICLGeom/ObjectEdgeDetectorData.h>

namespace icl {

using namespace utils;
using namespace math;
using namespace core;

namespace geom {

//OpenCL kernel code
static char normalEstimationKernel[] =
		"__kernel void                                                                                                                      \n"
				"medianFilter(__global float const * in, __global float * out, int const w, int const h, int const medianFilterSize)        \n"
				"{                                                                                                                          \n"
				"    int x = get_global_id(0);                                                                                              \n"
				"    int y = get_global_id(1);                                                                                              \n"
				"    size_t id = x+y*w;                                                                                                     \n"
				"    if(y<(medianFilterSize-1)/2 || y>=h-(medianFilterSize-1)/2 || x<(medianFilterSize-1)/2 || x>=w-(medianFilterSize-1)/2) \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float mask[81];                                                                                                     \n"
				"  		  for(int sy=-(medianFilterSize-1)/2; sy<=(medianFilterSize-1)/2; sy++){                                            \n"
				"           for(int sx=-(medianFilterSize-1)/2; sx<=(medianFilterSize-1)/2; sx++){                                          \n"
				" 			        mask[(sx+(medianFilterSize-1)/2)+(sy+(medianFilterSize-1)/2)*medianFilterSize]=in[(x+sx)+w*(y+sy)];     \n"
				" 		      }                                                                                                             \n"
				"       }                                                                                                                   \n"
				"       //bubble-sort: slow O(n^2), but easy to implement without recursion or complex data structurs                       \n"
				"       bool sw=true;                                                                                                       \n"
				"       while(sw==true){                                                                                                    \n"
				"           sw=false;                                                                                                       \n"
				"           for(int i=0; i<medianFilterSize*medianFilterSize-1; i++){                                                       \n"
				"               if(mask[i+1]<mask[i]){                                                                                      \n"
				"                    float tmp=mask[i];                                                                                     \n"
				"                    mask[i]=mask[i+1];                                                                                     \n"
				"                    mask[i+1]=tmp;                                                                                         \n"
				"                    sw=true;                                                                                               \n"
				"               }                                                                                                           \n"
				"           }                                                                                                               \n"
				"       }                                                                                                                   \n"
				"  		  out[id]=mask[(medianFilterSize*medianFilterSize)/2];                                                              \n"
				"    }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"normalCalculation(__global float const * in, __global float4 * out, int const w, int const h, int const normalRange)       \n"
				"{                                                                                                                          \n"
				"    int x = get_global_id(0);                                                                                              \n"
				"    int y = get_global_id(1);                                                                                              \n"
				"    size_t id = x+y*w;                                                                                                     \n"
				"    if(y<normalRange || y>=h-normalRange || x<normalRange || x>=w-normalRange)                                             \n"
				"    {                                                                                                                      \n"
				"       out[id]=(0,0,0,0);                                                                                                  \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float4 fa;                                                                                                          \n"
				"       fa.x=(x+normalRange)-(x-normalRange);                                                                               \n"
				"       fa.y=(y-normalRange)-(y-normalRange);                                                                               \n"
				"       fa.z=in[(x+normalRange)+w*(y-normalRange)]-in[(x-normalRange)+w*(y-normalRange)];                                   \n"
				"       fa.w=1;                                                                                                             \n"
				"       float4 fb;                                                                                                          \n"
				"       fb.x=(x)-(x-normalRange);                                                                                           \n"
				"       fb.y=(y+normalRange)-(y-normalRange);                                                                               \n"
				"       fb.z=in[(x)+w*(y+normalRange)]-in[(x-normalRange)+w*(y-normalRange)];                                               \n"
				"       fb.w=1;                                                                                                             \n"
				"       float4 n1=cross(fa,fb);                                                                                             \n"
				"       out[id]=fast_normalize(n1);                                                                                         \n"
				"    }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"normalAveraging(__global float4 const * in, __global float4 * out, int const w, int const h, int const normalAveragingRange)\n"
				"{                                                                                                                          \n"
				"    int x = get_global_id(0);                                                                                              \n"
				"    int y = get_global_id(1);                                                                                              \n"
				"    size_t id = x+y*w;                                                                                                     \n"
				"    if(y<normalAveragingRange || y>=h-normalAveragingRange || x<normalAveragingRange || x>=w-normalAveragingRange)         \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float4 avg;                                                                                                         \n"
				"	      avg.x=0, avg.y=0, avg.z=0, avg.w=0;                                                                               \n"
				"       for(int sx=-normalAveragingRange; sx<=normalAveragingRange; sx++){                                                  \n"
				"      		  for(int sy=-normalAveragingRange; sy<=normalAveragingRange; sy++){                                            \n"
				"      		      avg.x+=in[(x+sx)+w*(y+sy)].x;                                                                             \n"
				"     			    avg.y+=in[(x+sx)+w*(y+sy)].y;                                                                           \n"
				"     			    avg.z+=in[(x+sx)+w*(y+sy)].z;                                                                           \n"
				"       	  }                                                                                                             \n"
				"       }                                                                                                                   \n"
				"       avg.x/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.y/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.z/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.w=1;                                                                                                            \n"
				"	      out[id]=avg;                                                                                                      \n"
				"  	 }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"angleImageCalculation(__global float4 const * normals, __global float * out, int const w, int const h, int const neighborhoodRange, int const neighborhoodMode)\n"
				"{                                                                                                                          \n"
				"    int x = get_global_id(0);                                                                                              \n"
				"    int y = get_global_id(1);                                                                                              \n"
				"    size_t id = x+y*w;                                                                                                     \n"
				"    if(y<neighborhoodRange || y>=h-(neighborhoodRange) || x<neighborhoodRange || x>=w-(neighborhoodRange))                 \n"
				"    {                                                                                                                      \n"
				"       out[id]=0;                                                                                                          \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float snr=0;//sum right                                                                                             \n"
				"       float snl=0;//sum left                                                                                              \n"
				"       float snt=0;//sum top                                                                                               \n"
				"       float snb=0;//sum bottom                                                                                            \n"
				"       float sntr=0;//sum top-right                                                                                        \n"
				"       float sntl=0;//sum top-left                                                                                         \n"
				"       float snbr=0;//sum bottom-right                                                                                     \n"
				"       float snbl=0;//sum bottom-left                                                                                      \n"
				"       for(int z=1; z<=neighborhoodRange; z++){                                                                            \n"
				"           float nr=(normals[id].x*normals[id+z].x+normals[id].y*normals[id+z].y+normals[id].z*normals[id+z].z);           \n"
				"	          float nl=(normals[id].x*normals[id-z].x+normals[id].y*normals[id-z].y+normals[id].z*normals[id-z].z);         \n"
				"	          float nt=(normals[id].x*normals[id+w*z].x+normals[id].y*normals[id+w*z].y+normals[id].z*normals[id+w*z].z);   \n"
				"	          float nb=(normals[id].x*normals[id-w*z].x+normals[id].y*normals[id-w*z].y+normals[id].z*normals[id-w*z].z);   \n"
				"	          float ntr=(normals[id].x*normals[id+w*z+z].x+normals[id].y*normals[id+w*z+z].y+normals[id].z*normals[id+w*z+z].z); \n"
				"	          float ntl=(normals[id].x*normals[id+w*z-z].x+normals[id].y*normals[id+w*z-z].y+normals[id].z*normals[id+w*z-z].z); \n"
				"	          float nbr=(normals[id].x*normals[id-w*z+z].x+normals[id].y*normals[id-w*z+z].y+normals[id].z*normals[id-w*z+z].z); \n"
				"	          float nbl=(normals[id].x*normals[id-w*z-z].x+normals[id].y*normals[id-w*z-z].y+normals[id].z*normals[id-w*z-z].z); \n"
				"	          if(nr<cos(M_PI/2)) {nr=cos(M_PI-acos(nr));}                                                                   \n"
				"	          if(nl<cos(M_PI/2)) {nl=cos(M_PI-acos(nl));}                                                                   \n"
				"	          if(nt<cos(M_PI/2)) {nt=cos(M_PI-acos(nt));}                                                                   \n"
				"	          if(nb<cos(M_PI/2)) {nb=cos(M_PI-acos(nb));}                                                                   \n"
				"	          if(ntr<cos(M_PI/2)) {ntr=cos(M_PI-acos(ntr));}                                                                \n"
				"	          if(ntl<cos(M_PI/2)) {ntl=cos(M_PI-acos(ntl));}                                                                \n"
				"	          if(nbr<cos(M_PI/2)) {nbr=cos(M_PI-acos(nbr));}                                                                \n"
				"	          if(nbl<cos(M_PI/2)) {nbl=cos(M_PI-acos(nbl));}	                                                            \n"
				"	          snr+=nr;                                                                                                      \n"
				"           snl+=nl;                                                                                                        \n"
				"           snt+=nt;                                                                                                        \n"
				"           snb+=nb;                                                                                                        \n"
				"           sntr+=ntr;                                                                                                      \n"
				"           sntl+=ntl;                                                                                                      \n"
				"           snbr+=nbr;                                                                                                      \n"
				"           snbl+=nbl;                                                                                                      \n"
				"	      }                                                                                                                 \n"
				"	      snr/=neighborhoodRange;                                                                                           \n"
				"       snl/=neighborhoodRange;                                                                                             \n"
				"       snt/=neighborhoodRange;                                                                                             \n"
				"       snb/=neighborhoodRange;                                                                                             \n"
				"       sntr/=neighborhoodRange;                                                                                            \n"
				"       sntl/=neighborhoodRange;                                                                                            \n"
				"       snbr/=neighborhoodRange;                                                                                            \n"
				"       snbl/=neighborhoodRange;                                                                                            \n"
				"	      if(neighborhoodMode==0){                                                                                          \n"
				"	          out[id]=snr;                                                                                                  \n"
				"	          if(out[id]>snl){                                                                                              \n"
				"		            out[id]=snl;                                                                                            \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>snt){                                                                                              \n"
				"		            out[id]=snt;                                                                                            \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>snb){                                                                                              \n"
				"		            out[id]=snb;                                                                                            \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>snbl){                                                                                             \n"
				"		            out[id]=snbl;                                                                                           \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>snbr){                                                                                             \n"
				"		            out[id]=snbr;                                                                                           \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>sntl){                                                                                             \n"
				"		            out[id]=sntl;                                                                                           \n"
				"	          }                                                                                                             \n"
				"	          if(out[id]>sntr){                                                                                             \n"
				"		            out[id]=sntr;                                                                                           \n"
				"	          }                                                                                                             \n"
				"	      }                                                                                                                 \n"
				"	      else if(neighborhoodMode==1){                                                                                     \n"
				"	          out[id]=(snr+snl+snt+snb+sntr+sntl+snbr+snbl)/8;                                                              \n"
				"	      }                                                                                                                 \n"
				"	      else{                                                                                                             \n"
				"	          out[id]=0;                                                                                                    \n"
				"	      }                                                                                                                 \n"
				"    }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"imageBinarization(__global float const * in, __global uchar * out, int const w, int const h, float const binarizationThreshold)\n"
				"{                                                                                                                          \n"
				"   size_t id =  get_global_id(0);                                                                                          \n"
				"   if(in[id]>binarizationThreshold)                                                                                        \n"
				"   {                                                                                                                       \n"
				"	      out[id]=255;                                                                                                      \n"
				"   }                                                                                                                       \n"
				"   else                                                                                                                    \n"
				"   {                                                                                                                       \n"
				"	      out[id]=0;                                                                                                        \n"
				"   }                                                                                                                       \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"worldNormalCalculation(__global float const * depth, __global uchar * outCR, __global uchar * outCG, __global uchar * outCB, __global float4 * normals, __global float const * cam, __global float4 * outNormals)                                                                                            \n"
				"{                                                                                                                          \n"
				"   size_t id =  get_global_id(0);                                                                                          \n"
				"   float4 pWN;                                                                                                             \n"
				"   float4 n= normals[id];                                                                                                  \n"
				"   pWN.x=-(n.x*cam[0]+n.y*cam[1]+n.z*cam[2]);                                                                              \n"
				"   pWN.y=-(n.x*cam[4]+n.y*cam[5]+n.z*cam[6]);                                                                              \n"
				"   pWN.z=-(n.x*cam[8]+n.y*cam[9]+n.z*cam[10]);                                                                             \n"
				"   pWN.w=1;                                                                                                                \n"
				"   if(depth[id]==2047)                                                                                                     \n"
				"   {                                                                                                                       \n"
				"     outCR[id]=0;                                                                                                          \n"
				"     outCG[id]=0;                                                                                                          \n"
				"     outCB[id]=0;                                                                                                          \n"
				"     outNormals[id]=(0,0,0,0);                                                                                             \n"
				"   }                                                                                                                       \n"
				"   else                                                                                                                    \n"
				"   {                                                                                                                       \n"
				"     uchar cx=(int)(fabs(pWN.x)*255.);                                                                                     \n"
				"     uchar cy=(int)(fabs(pWN.y)*255.);                                                                                     \n"
				"     uchar cz=(int)(fabs(pWN.z)*255.);                                                                                     \n"
				"     outCR[id] = cx;                                                                                                       \n"
				"     outCG[id] = cy;                                                                                                       \n"
				"     outCB[id] = cz;                                                                                                       \n"
				"     outNormals[id]=pWN;                                                                                                   \n"
				"   }                                                                                                                       \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"normalGaussSmoothing(__global float4 const * in, __global float4 * out, int const w, int const h, int const l, float const norm, __global float const * kern, int const rowSize)                                                                                                                              \n"
				"{                                                                                                                          \n"
				"    int x = get_global_id(0);                                                                                              \n"
				"    int y = get_global_id(1);                                                                                              \n"
				"    size_t id = x+y*w;                                                                                                     \n"
				"    if(y<l || y>=h-l || x<l || x>=w-l || l==0)                                                                             \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float4 avg;                                                                                                         \n"
				"	      avg.x=0, avg.y=0, avg.z=0, avg.w=0;                                                                               \n"
				"       for(int sx=-l; sx<=l; sx++){                                                                                        \n"
				"      		  for(int sy=-l; sy<=l; sy++){                                                                                  \n"
				"      		      avg.x+=in[(x+sx)+w*(y+sy)].x*kern[(sx+l)+rowSize*(sy+l)];                                                 \n"
				"     			    avg.y+=in[(x+sx)+w*(y+sy)].y*kern[(sx+l)+rowSize*(sy+l)];                                               \n"
				"     			    avg.z+=in[(x+sx)+w*(y+sy)].z*kern[(sx+l)+rowSize*(sy+l)];                                               \n"
				"       	  }                                                                                                             \n"
				"       }                                                                                                                   \n"
				"       avg.x/=norm;                                                                                                        \n"
				"      	avg.y/=norm;                                                                                                        \n"
				"      	avg.z/=norm;                                                                                                        \n"
				"      	avg.w=1;                                                                                                            \n"
				"	      out[id]=avg;                                                                                                      \n"
				"  	 }                                                                                                                      \n"
				"}                                                                                                                          \n";

typedef FixedColVector<float, 4> Vec4;

struct ObjectEdgeDetectorGPU::Data {
	Data() {
		
		oedData = new ObjectEdgeDetectorData();
		
		//set default values
		params = oedData->getParameters();
		clReady = false;
		isInitialized=false;

		try {
			program = CLProgram("gpu", normalEstimationKernel);
			clReady=true; //and mark CL context as available
			
			//create kernels
		    kernelMedianFilter = program.createKernel("medianFilter");
		    kernelNormalCalculation = program.createKernel("normalCalculation");
		    kernelNormalAveraging = program.createKernel("normalAveraging");
		    kernelAngleImageCalculation = program.createKernel("angleImageCalculation");
		    kernelImageBinarization = program.createKernel("imageBinarization");
		    kernelWorldNormalCalculation = program.createKernel("worldNormalCalculation");
		    kernelNormalGaussSmoothing = program.createKernel("normalGaussSmoothing");
		    
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
			clReady = false;
		}

	}
	~Data() {
	}

	int w, h;
	bool clReady;
	DataSegment<float,4> normals;
	DataSegment<float,4> avgNormals;
	DataSegment<float,4> worldNormals;
	Array2D<Vec4> normalsA;
	Array2D<Vec4> avgNormalsA;
	Array2D<Vec4> worldNormalsA;
	core::Img32f rawImage;
	core::Img32f filteredImage;
	core::Img32f angleImage;
	core::Img8u binarizedImage;
	core::Img8u normalImage;
	bool isInitialized;

	//OpenCL data
	std::vector<float> outputFilteredImage;//output of kernel for image
	std::vector<float> outputAngleImage;
	std::vector<unsigned char> outputBinarizedImage;
	std::vector<unsigned char> binarizedImageArray;
	std::vector<unsigned char> normalImageRArray;
	std::vector<unsigned char> normalImageGArray;
	std::vector<unsigned char> normalImageBArray;

	//OpenCL
	utils::CLProgram program;

	utils::CLKernel kernelMedianFilter;
	utils::CLKernel kernelNormalCalculation;
	utils::CLKernel kernelNormalAveraging;
	utils::CLKernel kernelAngleImageCalculation;
	utils::CLKernel kernelImageBinarization;
	utils::CLKernel kernelWorldNormalCalculation;
	utils::CLKernel kernelNormalGaussSmoothing;

	//OpenCL buffer
	utils::CLBuffer rawImageBuffer;
	utils::CLBuffer filteredImageBuffer;
	utils::CLBuffer normalsBuffer;
	utils::CLBuffer avgNormalsBuffer;
	utils::CLBuffer angleImageBuffer;
	utils::CLBuffer binarizedImageBuffer;
	utils::CLBuffer worldNormalsBuffer;
	utils::CLBuffer normalImageRBuffer;
	utils::CLBuffer normalImageGBuffer;
	utils::CLBuffer normalImageBBuffer;
	utils::CLBuffer camBuffer;
	utils::CLBuffer gaussKernelBuffer;
	
	ObjectEdgeDetectorData* oedData;
	ObjectEdgeDetectorData::m_params params;
};

ObjectEdgeDetectorGPU::ObjectEdgeDetectorGPU() :
		m_data(new Data()) {
}

ObjectEdgeDetectorGPU::~ObjectEdgeDetectorGPU() {
	delete m_data;
}


void ObjectEdgeDetectorGPU::initialize(Size size){
    if(m_data->isInitialized){
		m_data->isInitialized=false;
    }
    
	m_data->w = size.width;
	m_data->h = size.height;
    
    m_data->normalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->avgNormalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->worldNormalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->normals=DataSegment<float,4>(&m_data->normalsA(0,0)[0], sizeof(Vec4), m_data->normalsA.getDim(), m_data->normalsA.getWidth());
	m_data->avgNormals=DataSegment<float,4>(&m_data->avgNormalsA(0,0)[0], sizeof(Vec4), m_data->avgNormalsA.getDim(), m_data->avgNormalsA.getWidth());
	m_data->worldNormals=DataSegment<float,4>(&m_data->worldNormalsA(0,0)[0], sizeof(Vec4), m_data->worldNormalsA.getDim(), m_data->worldNormalsA.getWidth());

	m_data->rawImage.setSize(Size(m_data->w, m_data->h));
	m_data->rawImage.setChannels(1);
	m_data->filteredImage.setSize(Size(m_data->w, m_data->h));
	m_data->filteredImage.setChannels(1);
	m_data->angleImage.setSize(Size(m_data->w, m_data->h));
	m_data->angleImage.setChannels(1);
	m_data->binarizedImage.setSize(Size(m_data->w, m_data->h));
	m_data->binarizedImage.setChannels(1);
	m_data->normalImage.setSize(Size(m_data->w, m_data->h));
	m_data->normalImage.setChannels(3);

	m_data->binarizedImageArray.resize(m_data->w*m_data->h);
	m_data->normalImageRArray.resize(m_data->w*m_data->h);
	m_data->normalImageGArray.resize(m_data->w*m_data->h);
	m_data->normalImageBArray.resize(m_data->w*m_data->h);

	m_data->outputFilteredImage.resize(m_data->w*m_data->h);
	m_data->outputAngleImage.resize(m_data->w*m_data->h);
	m_data->outputBinarizedImage.resize(m_data->w*m_data->h);
	
	try {
		//create buffer for memory access and allocation
        m_data->rawImageBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(float));
        m_data->filteredImageBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(float));
		m_data->normalsBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
		m_data->avgNormalsBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
		m_data->angleImageBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(float));
		m_data->binarizedImageBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(unsigned char));
		m_data->normalImageRBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(unsigned char));
		m_data->normalImageGBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(unsigned char));
		m_data->normalImageBBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(unsigned char));
		m_data->worldNormalsBuffer = m_data->program.createBuffer("rw", m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));

	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	
	m_data->isInitialized=true;
}


void ObjectEdgeDetectorGPU::setDepthImage(const Img32f &depthImg) {
	m_data->rawImage = depthImg;
	try {
        m_data->rawImageBuffer.write(depthImg.begin(0),m_data->w*m_data->h*sizeof(float));
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

void ObjectEdgeDetectorGPU::applyMedianFilter() {
	try {
		m_data->kernelMedianFilter.setArgs(m_data->rawImageBuffer,
				m_data->filteredImageBuffer,
				m_data->w,
				m_data->h,
				m_data->params.medianFilterSize);
		
		m_data->kernelMedianFilter.apply(m_data->w,m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

const Img32f &ObjectEdgeDetectorGPU::getFilteredDepthImage() {
	try {
		m_data->filteredImageBuffer.read( //read output from kernel
				m_data->outputFilteredImage.data(),
				m_data->w*m_data->h * sizeof(float));

		m_data->filteredImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputFilteredImage.data()),false);
	}
	catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->filteredImage;
}

void ObjectEdgeDetectorGPU::setFilteredDepthImage(const Img32f &filteredImg) {
    m_data->filteredImage = filteredImg;
    try {
      m_data->filteredImageBuffer.write(filteredImg.begin(0),m_data->w*m_data->h*sizeof(float));
	} catch (CLException &err) { //catch openCL errors
      std::cout<< "ERROR: "<< err.what()<< std::endl;
    }
}

void ObjectEdgeDetectorGPU::applyNormalCalculation() {
	try {
		m_data->kernelNormalCalculation.setArgs(m_data->filteredImageBuffer,
				m_data->normalsBuffer,
				m_data->w,
				m_data->h,
				m_data->params.normalRange);
		m_data->kernelNormalCalculation.apply(m_data->w,m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}

	if (m_data->params.useNormalAveraging && !m_data->params.useGaussSmoothing) {
		applyLinearNormalAveraging();
	} else if (m_data->params.useNormalAveraging && m_data->params.useGaussSmoothing) {
		applyGaussianNormalSmoothing();
	}
}

void ObjectEdgeDetectorGPU::applyLinearNormalAveraging() {
	try {
		m_data->kernelNormalAveraging.setArgs(m_data->normalsBuffer,
				m_data->avgNormalsBuffer,
				m_data->w,
				m_data->h,
				m_data->params.normalAveragingRange);
		m_data->kernelNormalAveraging.apply(m_data->w,m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

void ObjectEdgeDetectorGPU::applyGaussianNormalSmoothing() {
	ObjectEdgeDetectorData::m_kernel kernelData = m_data->oedData->getKernel(m_data->params.normalAveragingRange);
	float norm = kernelData.norm;
	int l = kernelData.l;
	DynMatrix<float> kernel = kernelData.kernel;
	int kSize = kernelData.kSize;
	int rowSize = kernelData.rowSize;

	try {

		m_data->gaussKernelBuffer = m_data->program.createBuffer("rw", kSize * sizeof(float), (void *) &kernel[0]);

		m_data->kernelNormalGaussSmoothing.setArgs(m_data->normalsBuffer,
				m_data->avgNormalsBuffer,
				m_data->w,
				m_data->h,
				l,
				norm,
				m_data->gaussKernelBuffer,
				rowSize); //set parameter for kernel
		m_data->kernelNormalGaussSmoothing.apply(m_data->w,m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

const DataSegment<float,4> ObjectEdgeDetectorGPU::getNormals() {
	try {
		if(m_data->params.useNormalAveraging==true) {
			m_data->avgNormalsBuffer.read((float*) &m_data->avgNormals[0][0], m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
			return m_data->avgNormals;
		} else {
			m_data->normalsBuffer.read( //read output from kernel
					(float*) &m_data->normals[0][0],
					m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
			return m_data->normals;
		}
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->normals;
}

void ObjectEdgeDetectorGPU::applyWorldNormalCalculation(const Camera &cam) {
	Mat T = cam.getCSTransformationMatrix();
	FixedMatrix<float, 3, 3> R = T.part<0, 0, 3, 3>();
	Mat T2 = R.transp().resize<4, 4>(0);
	T2(3, 3) = 1;
	try {
		m_data->camBuffer = m_data->program.createBuffer("rw", 16 * sizeof(float), (void *) &T2[0]);

		m_data->kernelWorldNormalCalculation.setArgs(m_data->rawImageBuffer,
				m_data->normalImageRBuffer,
				m_data->normalImageGBuffer,
				m_data->normalImageBBuffer); //set parameter for kernel
		if(m_data->params.useNormalAveraging==true) {
			m_data->kernelWorldNormalCalculation[4] = m_data->avgNormalsBuffer;
		} else {
			m_data->kernelWorldNormalCalculation[4] = m_data->normalsBuffer;
		}
		m_data->kernelWorldNormalCalculation[5] = m_data->camBuffer;
		m_data->kernelWorldNormalCalculation[6] = m_data->worldNormalsBuffer;
		m_data->kernelWorldNormalCalculation.apply(m_data->w*m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

const DataSegment<float,4> ObjectEdgeDetectorGPU::getWorldNormals() {
	try {
		m_data->worldNormalsBuffer.read((float*) &m_data->worldNormals[0][0],
				m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
		return m_data->worldNormals;
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->worldNormals;
}

const core::Img8u &ObjectEdgeDetectorGPU::getRGBNormalImage() {
	try {
		m_data->normalImageRBuffer.read(m_data->normalImageRArray.data(),
				m_data->w*m_data->h * sizeof(unsigned char));

		m_data->normalImageGBuffer.read(m_data->normalImageGArray.data(),
				m_data->w*m_data->h * sizeof(unsigned char));
		m_data->normalImageBBuffer.read(m_data->normalImageBArray.data(),
				m_data->w*m_data->h * sizeof(unsigned char));

		std::vector<icl8u*> data(3);
		data[0] = m_data->normalImageRArray.data();
		data[1] = m_data->normalImageGArray.data();
		data[2] = m_data->normalImageBArray.data();
		m_data->normalImage = Img8u(Size(m_data->w,m_data->h),3,data,false);

		return m_data->normalImage;

	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->normalImage;
}

void ObjectEdgeDetectorGPU::setNormals(DataSegment<float,4> pNormals) {
	if (m_data->params.useNormalAveraging == true) {
		pNormals.deepCopy(m_data->avgNormals);
	} else {
		pNormals.deepCopy(m_data->avgNormals);
	}
	try {
		if(m_data->params.useNormalAveraging==true) {
			m_data->avgNormalsBuffer.write(&m_data->avgNormals[0][0],m_data->w*m_data->h*sizeof(FixedColVector<float,4>));
		} else {
			m_data->normalsBuffer.write(&m_data->normals[0][0],m_data->w*m_data->h*sizeof(FixedColVector<float,4>));
		}
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

void ObjectEdgeDetectorGPU::applyAngleImageCalculation() {
	try {
		if(m_data->params.useNormalAveraging==true) {
			m_data->kernelAngleImageCalculation[0] = m_data->avgNormalsBuffer;

		} else {
			m_data->kernelAngleImageCalculation[0] = m_data->normalsBuffer;
		}
		m_data->kernelAngleImageCalculation[1] = m_data->angleImageBuffer;
		m_data->kernelAngleImageCalculation[2] = m_data->w;
		m_data->kernelAngleImageCalculation[3] = m_data->h;
		m_data->kernelAngleImageCalculation[4] = m_data->params.neighborhoodRange;
		m_data->kernelAngleImageCalculation[5] = m_data->params.neighborhoodMode;
		m_data->kernelAngleImageCalculation.apply(m_data->w,m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

const Img32f &ObjectEdgeDetectorGPU::getAngleImage() {
	try {
		m_data->angleImageBuffer.read(m_data->outputAngleImage.data(), m_data->w*m_data->h * sizeof(float));
		m_data->angleImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputAngleImage.data()),false);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->angleImage;
}

void ObjectEdgeDetectorGPU::setAngleImage(const Img32f &angleImg) {
    m_data->angleImage = angleImg;
    try {
      m_data->angleImageBuffer.write(angleImg.begin(0),m_data->w*m_data->h*sizeof(float));
    } catch (CLException &err) { //catch openCL errors
      std::cout<< "ERROR: "<< err.what()<< std::endl;
    }
}

void ObjectEdgeDetectorGPU::applyImageBinarization() {
	try {
		m_data->kernelImageBinarization.setArgs(m_data->angleImageBuffer,
				m_data->binarizedImageBuffer,
				m_data->w,
				m_data->h,
				m_data->params.binarizationThreshold);

		m_data->kernelImageBinarization.apply(m_data->w*m_data->h);
	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
}

const Img8u &ObjectEdgeDetectorGPU::getBinarizedAngleImage() {
	try {
		m_data->binarizedImageBuffer.read(m_data->outputBinarizedImage.data(), m_data->w*m_data->h * sizeof(unsigned char));
		m_data->binarizedImage = Img8u(Size(m_data->w,m_data->h),1,std::vector<unsigned char*>(1,m_data->outputBinarizedImage.data()),false);

	} catch (CLException &err) { //catch openCL errors
		std::cout<< "ERROR: "<< err.what()<< std::endl;
	}
	return m_data->binarizedImage;
}

void ObjectEdgeDetectorGPU::setMedianFilterSize(int size) {
	m_data->params.medianFilterSize = size;
}

void ObjectEdgeDetectorGPU::setNormalCalculationRange(int range) {
	m_data->params.normalRange = range;
}

void ObjectEdgeDetectorGPU::setNormalAveragingRange(int range) {
	m_data->params.normalAveragingRange = range;
}

void ObjectEdgeDetectorGPU::setAngleNeighborhoodMode(int mode) {
	m_data->params.neighborhoodMode = mode;
}

void ObjectEdgeDetectorGPU::setAngleNeighborhoodRange(int range) {
	m_data->params.neighborhoodRange = range;
}

void ObjectEdgeDetectorGPU::setBinarizationThreshold(float threshold) {
	m_data->params.binarizationThreshold = threshold;
}

void ObjectEdgeDetectorGPU::setUseNormalAveraging(bool use) {
	m_data->params.useNormalAveraging = use;
}

void ObjectEdgeDetectorGPU::setUseGaussSmoothing(bool use) {
	m_data->params.useGaussSmoothing = use;
}

bool ObjectEdgeDetectorGPU::isCLReady() {
	return m_data->clReady;
}

const Img8u &ObjectEdgeDetectorGPU::calculate(const Img32f &depthImage,
		bool filter, bool average, bool gauss) {
	
	if (filter == false) {
		setFilteredDepthImage(depthImage);
	} else {
		setDepthImage(depthImage);
		applyMedianFilter();
	}
	m_data->params.useNormalAveraging = average;
	m_data->params.useGaussSmoothing = gauss;
	applyNormalCalculation();
	applyAngleImageCalculation();
	applyImageBinarization();
	return getBinarizedAngleImage();
}
} // namespace geom
}

#endif

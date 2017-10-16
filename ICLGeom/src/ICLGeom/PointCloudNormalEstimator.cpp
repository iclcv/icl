/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/PointCloudNormalEstimator.cpp      **
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
#endif

#include <ICLGeom/PointCloudNormalEstimator.h>

namespace icl {

using namespace utils;
using namespace math;
using namespace core;

namespace geom {

//OpenCL kernel code
static char normalEstimationKernel[] =
		"__kernel void                                                                                                              \n"
				"medianFilter(__global float const * in, __global float * out, int const w, int const h, int const medianFilterSize)        \n"
				"{                                                                                                                          \n"
				"    size_t id =  get_global_id(0);                                                                                         \n"
				"    int y = (int)floor((float)id/(float)w);                                                                                \n"
				"    int x = id-y*w;                                                                                                        \n"
				"    if(y<(medianFilterSize-1)/2 || y>=h-(medianFilterSize-1)/2 || x<(medianFilterSize-1)/2 || x>=w-(medianFilterSize-1)/2) \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float mask[81];                                                                                                     \n"
				"  		  for(int sy=-(medianFilterSize-1)/2; sy<=(medianFilterSize-1)/2; sy++){                                              \n"
				"           for(int sx=-(medianFilterSize-1)/2; sx<=(medianFilterSize-1)/2; sx++){                                          \n"
				" 			        mask[(sx+(medianFilterSize-1)/2)+(sy+(medianFilterSize-1)/2)*medianFilterSize]=in[(x+sx)+w*(y+sy)];         \n"
				" 		      }                                                                                                               \n"
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
				"  		  out[id]=mask[(medianFilterSize*medianFilterSize)/2];                                                                \n"
				"    }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"normalCalculation(__global float const * in, __global float4 * out, int const w, int const h, int const normalRange)       \n"
				"{                                                                                                                          \n"
				"    size_t id =  get_global_id(0);                                                                                         \n"
				"    int y = (int)floor((float)id/(float)w);                                                                                \n"
				"    int x = id-y*w;                                                                                                        \n"
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
				"    size_t id =  get_global_id(0);                                                                                         \n"
				"    int y = (int)floor((float)id/(float)w);                                                                                \n"
				"    int x = id-y*w;                                                                                                        \n"
				"    if(y<normalAveragingRange || y>=h-normalAveragingRange || x<normalAveragingRange || x>=w-normalAveragingRange)         \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float4 avg;                                                                                                         \n"
				"	      avg.x=0, avg.y=0, avg.z=0, avg.w=0;                                                                                 \n"
				"       for(int sx=-normalAveragingRange; sx<=normalAveragingRange; sx++){                                                  \n"
				"      		  for(int sy=-normalAveragingRange; sy<=normalAveragingRange; sy++){                                              \n"
				"      		      avg.x+=in[(x+sx)+w*(y+sy)].x;                                                                               \n"
				"     			    avg.y+=in[(x+sx)+w*(y+sy)].y;                                                                               \n"
				"     			    avg.z+=in[(x+sx)+w*(y+sy)].z;                                                                               \n"
				"       	  }                                                                                                               \n"
				"       }                                                                                                                   \n"
				"       avg.x/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.y/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.z/=((1+2*normalAveragingRange)*(1+2*normalAveragingRange));                                                     \n"
				"      	avg.w=1;                                                                                                            \n"
				"	      out[id]=avg;                                                                                                        \n"
				"  	 }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"angleImageCalculation(__global float4 const * normals, __global float * out, int const w, int const h, int const neighborhoodRange, int const neighborhoodMode)\n"
				"{                                                                                                                          \n"
				"    size_t id =  get_global_id(0);                                                                                         \n"
				"    int y = (int)floor((float)id/(float)w);                                                                                \n"
				"    int x = id-y*w;                                                                                                        \n"
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
				"	          float nl=(normals[id].x*normals[id-z].x+normals[id].y*normals[id-z].y+normals[id].z*normals[id-z].z);           \n"
				"	          float nt=(normals[id].x*normals[id+w*z].x+normals[id].y*normals[id+w*z].y+normals[id].z*normals[id+w*z].z);     \n"
				"	          float nb=(normals[id].x*normals[id-w*z].x+normals[id].y*normals[id-w*z].y+normals[id].z*normals[id-w*z].z);     \n"
				"	          float ntr=(normals[id].x*normals[id+w*z+z].x+normals[id].y*normals[id+w*z+z].y+normals[id].z*normals[id+w*z+z].z);  \n"
				"	          float ntl=(normals[id].x*normals[id+w*z-z].x+normals[id].y*normals[id+w*z-z].y+normals[id].z*normals[id+w*z-z].z);  \n"
				"	          float nbr=(normals[id].x*normals[id-w*z+z].x+normals[id].y*normals[id-w*z+z].y+normals[id].z*normals[id-w*z+z].z);  \n"
				"	          float nbl=(normals[id].x*normals[id-w*z-z].x+normals[id].y*normals[id-w*z-z].y+normals[id].z*normals[id-w*z-z].z);  \n"
				"	          if(nr<cos(M_PI/2)) {nr=cos(M_PI-acos(nr));}                                                                     \n"
				"	          if(nl<cos(M_PI/2)) {nl=cos(M_PI-acos(nl));}                                                                     \n"
				"	          if(nt<cos(M_PI/2)) {nt=cos(M_PI-acos(nt));}                                                                     \n"
				"	          if(nb<cos(M_PI/2)) {nb=cos(M_PI-acos(nb));}                                                                     \n"
				"	          if(ntr<cos(M_PI/2)) {ntr=cos(M_PI-acos(ntr));}                                                                  \n"
				"	          if(ntl<cos(M_PI/2)) {ntl=cos(M_PI-acos(ntl));}                                                                  \n"
				"	          if(nbr<cos(M_PI/2)) {nbr=cos(M_PI-acos(nbr));}                                                                  \n"
				"	          if(nbl<cos(M_PI/2)) {nbl=cos(M_PI-acos(nbl));}	                                                                \n"
				"	          snr+=nr;                                                                                                        \n"
				"           snl+=nl;                                                                                                        \n"
				"           snt+=nt;                                                                                                        \n"
				"           snb+=nb;                                                                                                        \n"
				"           sntr+=ntr;                                                                                                      \n"
				"           sntl+=ntl;                                                                                                      \n"
				"           snbr+=nbr;                                                                                                      \n"
				"           snbl+=nbl;                                                                                                      \n"
				"	      }                                                                                                                   \n"
				"	      snr/=neighborhoodRange;                                                                                             \n"
				"       snl/=neighborhoodRange;                                                                                             \n"
				"       snt/=neighborhoodRange;                                                                                             \n"
				"       snb/=neighborhoodRange;                                                                                             \n"
				"       sntr/=neighborhoodRange;                                                                                            \n"
				"       sntl/=neighborhoodRange;                                                                                            \n"
				"       snbr/=neighborhoodRange;                                                                                            \n"
				"       snbl/=neighborhoodRange;                                                                                            \n"
				"	      if(neighborhoodMode==0){                                                                                            \n"
				"	          out[id]=snr;                                                                                                    \n"
				"	          if(out[id]>snl){                                                                                                \n"
				"		            out[id]=snl;                                                                                                \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>snt){                                                                                                \n"
				"		            out[id]=snt;                                                                                                \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>snb){                                                                                                \n"
				"		            out[id]=snb;                                                                                                \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>snbl){                                                                                               \n"
				"		            out[id]=snbl;                                                                                               \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>snbr){                                                                                               \n"
				"		            out[id]=snbr;                                                                                               \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>sntl){                                                                                               \n"
				"		            out[id]=sntl;                                                                                               \n"
				"	          }                                                                                                               \n"
				"	          if(out[id]>sntr){                                                                                               \n"
				"		            out[id]=sntr;                                                                                               \n"
				"	          }                                                                                                               \n"
				"	      }                                                                                                                   \n"
				"	      else if(neighborhoodMode==1){                                                                                       \n"
				"	          out[id]=(snr+snl+snt+snb+sntr+sntl+snbr+snbl)/8;                                                                \n"
				"	      }                                                                                                                   \n"
				"	      else{                                                                                                               \n"
				"	          out[id]=0;                                                                                                      \n"
				"	      }                                                                                                                   \n"
				"    }                                                                                                                      \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"imageBinarization(__global float const * in, __global uchar * out, int const w, int const h, float const binarizationThreshold)\n"
				"{                                                                                                                          \n"
				"   size_t id =  get_global_id(0);                                                                                          \n"
				"   if(in[id]>binarizationThreshold)                                                                                        \n"
				"   {                                                                                                                       \n"
				"	      out[id]=255;                                                                                                        \n"
				"   }                                                                                                                       \n"
				"   else                                                                                                                    \n"
				"   {                                                                                                                       \n"
				"	      out[id]=0;                                                                                                          \n"
				"   }                                                                                                                       \n"
				"}                                                                                                                          \n"
				"__kernel void                                                                                                              \n"
				"worldNormalCalculation(__global float const * depth, __global uchar * outCR, __global uchar * outCG, __global uchar * outCB, __global float4 * normals, __global float const * cam, __global float4 * outNormals)                                                                                                              \n"
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
				"normalGaussSmoothing(__global float4 const * in, __global float4 * out, int const w, int const h, int const l, float const norm, __global float const * kern, int const rowSize)\n"
				"{                                                                                                                          \n"
				"    size_t id =  get_global_id(0);                                                                                         \n"
				"    int y = (int)floor((float)id/(float)w);                                                                                \n"
				"    int x = id-y*w;                                                                                                        \n"
				"    if(y<l || y>=h-l || x<l || x>=w-l || l==0)                                                                             \n"
				"    {                                                                                                                      \n"
				"       out[id]=in[id];                                                                                                     \n"
				"    }                                                                                                                      \n"
				"    else                                                                                                                   \n"
				"    {                                                                                                                      \n"
				"       float4 avg;                                                                                                         \n"
				"	      avg.x=0, avg.y=0, avg.z=0, avg.w=0;                                                                                 \n"
				"       for(int sx=-l; sx<=l; sx++){                                                                                        \n"
				"      		  for(int sy=-l; sy<=l; sy++){                                                                                    \n"
				"      		      avg.x+=in[(x+sx)+w*(y+sy)].x*kern[(sx+l)+rowSize*(sy+l)];                                                   \n"
				"     			    avg.y+=in[(x+sx)+w*(y+sy)].y*kern[(sx+l)+rowSize*(sy+l)];                                                   \n"
				"     			    avg.z+=in[(x+sx)+w*(y+sy)].z*kern[(sx+l)+rowSize*(sy+l)];                                                   \n"
				"       	  }                                                                                                               \n"
				"       }                                                                                                                   \n"
				"       avg.x/=norm;                                                                                                        \n"
				"      	avg.y/=norm;                                                                                                        \n"
				"      	avg.z/=norm;                                                                                                        \n"
				"      	avg.w=1;                                                                                                            \n"
				"	      out[id]=avg;                                                                                                        \n"
				"  	 }                                                                                                                      \n"
				"}                                                                                                                          \n";

#ifdef ICL_HAVE_OPENCL
typedef FixedColVector<float, 4> Vec4;
#else
struct Vec4 {
	inline Vec4() {
	}
	inline Vec4(float x, float y, float z, float w) :
			x(x), y(y), z(z), w(w) {
	}
	float x, y, z, w;
};
#endif

struct PointCloudNormalEstimator::Data {
	Data(const Size &size) {
		//set default values
		medianFilterSize = 3;
		normalRange = 2;
		normalAveragingRange = 1;
		neighborhoodMode = 0;
		neighborhoodRange = 3;
		binarizationThreshold = 0.89;
		clReady = false;
		useCL = true;
		useNormalAveraging = true;
		useGaussSmoothing = false;

		//create arrays and images in given size
		if (size == Size::QVGA) {
			std::cout << "Resolution: 320x240" << std::endl;
			w = 320;
			h = 240;

		} else if (size == Size::VGA) {
			std::cout << "Resolution: 640x480" << std::endl;
			w = 640;
			h = 480;
		} else {
			std::cout << "Unknown Resolution" << std::endl;
			w = size.width;
			h = size.height;
		}

		normals = new Vec4[w * h];avgNormals
		= new Vec4[w * h];worldNormals
		= new Vec4[w * h];for
(		int i=0; i<w*h;i++) {
			normals[i].x=0;
			normals[i].y=0;
			normals[i].z=0;
			normals[i].w=0;
			avgNormals[i].x=0;
			avgNormals[i].y=0;
			avgNormals[i].z=0;
			avgNormals[i].w=0;
			worldNormals[i].x=0;
			worldNormals[i].y=0;
			worldNormals[i].z=0;
			worldNormals[i].w=0;
		}

		rawImage.setSize(Size(w, h));
		rawImage.setChannels(1);
		filteredImage.setSize(Size(w, h));
		filteredImage.setChannels(1);
		angleImage.setSize(Size(w, h));
		angleImage.setChannels(1);
		binarizedImage.setSize(Size(w, h));
		binarizedImage.setChannels(1);
		normalImage.setSize(Size(w, h));
		normalImage.setChannels(3);

#ifdef ICL_HAVE_OPENCL
		//create openCL context
                //	rawImageArray = new float[w*h];
                //	filteredImageArray = new float[w*h];
                //		angleImageArray = new float[w*h];
		binarizedImageArray = new unsigned char[w*h];
		normalImageRArray = new unsigned char[w*h];
		normalImageGArray = new unsigned char[w*h];
		normalImageBArray = new unsigned char[w*h];

		outputNormals=new FixedColVector<float, 4>[w*h];
		outputFilteredImage=new float[w*h];
		outputAngleImage=new float[w*h];
		outputBinarizedImage=new unsigned char[w*h];
		outputWorldNormals=new FixedColVector<float, 4>[w*h];

		try {
			program = CLProgram("gpu", normalEstimationKernel);
			clReady=true; //and mark CL context as available

		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
			clReady = false;
		}

		if(clReady==true) { //only if CL context is available
			try {
				//create buffer for memory access and allocation
                          rawImageBuffer = program.createBuffer("rw", w*h * sizeof(float));//, rawImageArray);
                          filteredImageBuffer = program.createBuffer("rw", w*h * sizeof(float));//, filteredImageArray);
				normalsBuffer = program.createBuffer("rw", w*h * sizeof(FixedColVector<float, 4>), normals);
				avgNormalsBuffer = program.createBuffer("rw", w*h * sizeof(FixedColVector<float, 4>), avgNormals);
				angleImageBuffer = program.createBuffer("rw", w*h * sizeof(float));//, angleImageArray);
				binarizedImageBuffer = program.createBuffer("rw", w*h * sizeof(unsigned char), binarizedImageArray);
				normalImageRBuffer = program.createBuffer("rw", w*h * sizeof(unsigned char), normalImageRArray);
				normalImageGBuffer = program.createBuffer("rw", w*h * sizeof(unsigned char), normalImageGArray);
				normalImageBBuffer = program.createBuffer("rw", w*h * sizeof(unsigned char), normalImageBArray);
				worldNormalsBuffer = program.createBuffer("rw", w*h * sizeof(FixedColVector<float, 4>), worldNormals);

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

#else
		std::cout << "no openCL parallelization available" << std::endl;
		clReady = false;
#endif
	}
	~Data() {
		delete[] normals;
		delete[] avgNormals;
		delete[] worldNormals;
#ifdef ICL_HAVE_OPENCL
                //	delete[] rawImageArray;
                //		delete[] filteredImageArray;
                //		delete[] angleImageArray;
		delete[] binarizedImageArray;
		delete[] normalImageRArray;
		delete[] normalImageGArray;
		delete[] normalImageBArray;
		delete[] outputNormals;
		delete[] outputFilteredImage;
		delete[] outputAngleImage;
		delete[] outputBinarizedImage;
		delete[] outputWorldNormals;
#endif
	}
	int w, h;
	int medianFilterSize;
	int normalRange;
	int normalAveragingRange;
	int neighborhoodMode;
	int neighborhoodRange;
	float binarizationThreshold;
	bool clReady;
	bool useCL;
	bool useNormalAveraging;
	bool useGaussSmoothing;
	Vec4* normals;
	Vec4* avgNormals;
	Vec4* worldNormals;
	core::Img32f rawImage;
	core::Img32f filteredImage;
	core::Img32f angleImage;
	core::Img8u binarizedImage;
	core::Img8u normalImage;

#ifdef ICL_HAVE_OPENCL
	//OpenCL data
	Vec4 * outputNormals;
	Vec4 * outputWorldNormals;
	float* outputFilteredImage;//output of kernel for image
	float* outputAngleImage;
	unsigned char* outputBinarizedImage;
  //	float* rawImageArray;//input for image to kernel
  //	float* filteredImageArray;
  //	float* angleImageArray;
	unsigned char* binarizedImageArray;
	unsigned char* normalImageRArray;
	unsigned char* normalImageGArray;
	unsigned char* normalImageBArray;

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
#endif
};

PointCloudNormalEstimator::PointCloudNormalEstimator(Size size) :
		m_data(new Data(size)) {
}

PointCloudNormalEstimator::~PointCloudNormalEstimator() {
	delete m_data;
}

void PointCloudNormalEstimator::setDepthImage(const Img32f &depthImg) {
	m_data->rawImage = depthImg;
#ifdef ICL_HAVE_OPENCL
	if(m_data->useCL==true && m_data->clReady==true) {
          //	m_data->rawImageArray=m_data->rawImage.begin(0); //image to float array
		try {
                  //	m_data->rawImageBuffer = m_data->program.createBuffer("r", m_data->w*m_data->h * sizeof(float),
                  // depthImg.begin(0));
                  m_data->rawImageBuffer.write(depthImg.begin(0),m_data->w*m_data->h*sizeof(float));
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
	}
#endif
}

void PointCloudNormalEstimator::applyMedianFilter() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->kernelMedianFilter.setArgs(m_data->rawImageBuffer,
					m_data->filteredImageBuffer,
					m_data->w,
					m_data->h,
					m_data->medianFilterSize);
			m_data->kernelMedianFilter.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		// TODO: heavily optimize this (MedianOp ??)
		std::vector < icl32f > list;
		for (int y = 0; y < m_data->h; y++) {
			for (int x = 0; x < m_data->w; x++) {
				if (y < (m_data->medianFilterSize - 1) / 2
						|| y >= m_data->h - (m_data->medianFilterSize - 1) / 2
						|| x < (m_data->medianFilterSize - 1) / 2
						|| x >= m_data->w - (m_data->medianFilterSize - 1) / 2) {
					m_data->filteredImage(x, y, 0) = m_data->rawImage(x, y, 0); //pixel out of range
				} else {
					for (int sx = -(m_data->medianFilterSize - 1) / 2;
							sx <= (m_data->medianFilterSize - 1) / 2; sx++) {
						for (int sy = -(m_data->medianFilterSize - 1) / 2;
								sy <= (m_data->medianFilterSize - 1) / 2;
								sy++) {
							list.push_back(m_data->rawImage(x + sx, y + sy, 0));
						}
					}
					std::sort(list.begin(), list.end());
					m_data->filteredImage(x, y, 0) =
							list[m_data->medianFilterSize
									* m_data->medianFilterSize / 2];
					list.clear();
				}
			}
		}
	}
}

const Img32f &PointCloudNormalEstimator::getFilteredDepthImage() {
#ifdef ICL_HAVE_OPENCL
	if(m_data->useCL==true && m_data->clReady==true) {
		try {
			m_data->filteredImageBuffer.read( //read output from kernel
					(float*) m_data->outputFilteredImage,
					m_data->w*m_data->h * sizeof(float));

			m_data->filteredImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputFilteredImage),false);
		}
		catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
	}
#endif
	return m_data->filteredImage;
}

void PointCloudNormalEstimator::setFilteredDepthImage(const Img32f &filteredImg) {
  m_data->filteredImage = filteredImg;
#ifdef ICL_HAVE_OPENCL
  if(m_data->useCL==true && m_data->clReady==true) {
    //m_data->filteredImageArray=m_data->filteredImage.begin(0); //image to float array
    try {
      m_data->filteredImageBuffer = m_data->program.createBuffer("r", m_data->w*m_data->h * sizeof(float),
                                                                 filteredImg.begin(0));
		} catch (CLException &err) { //catch openCL errors
      std::cout<< "ERROR: "<< err.what()<< std::endl;
    }
  }
#endif
}

void PointCloudNormalEstimator::applyNormalCalculation() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->kernelNormalCalculation.setArgs(m_data->filteredImageBuffer,
					m_data->normalsBuffer,
					m_data->w,
					m_data->h,
					m_data->normalRange);
			m_data->kernelNormalCalculation.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		const int r = m_data->normalRange;
		Vec fa1, fb1, n1, n01;
		for (int y = 0; y < m_data->h; y++) {
			for (int x = 0; x < m_data->w; x++) {
				int i = x + m_data->w * y;
				if (y < r || y >= m_data->h - r || x < r
						|| x >= m_data->w - r) {
					m_data->normals[i].x = 0; //points out of range
					m_data->normals[i].y = 0;
					m_data->normals[i].z = 0;
				} else {
					//cross product normal determination
					fa1[0] = (x + r) - (x - r);
					fa1[1] = (y - r) - (y - r);
					fa1[2] = m_data->filteredImage(x + r, y - r, 0)
							- m_data->filteredImage(x - r, y - r, 0);
					fb1[0] = (x) - (x - r);
					fb1[1] = (y + r) - (y - r);
					fb1[2] = m_data->filteredImage(x, y + r, 0)
							- m_data->filteredImage(x - r, y - r, 0);

					n1[0] = fa1[1] * fb1[2] - fa1[2] * fb1[1];
					n1[1] = fa1[2] * fb1[0] - fa1[0] * fb1[2];
					n1[2] = fa1[0] * fb1[1] - fa1[1] * fb1[0];
					n01[0] = n1[0] / norm3(n1);
					n01[1] = n1[1] / norm3(n1);
					n01[2] = n1[2] / norm3(n1);

					m_data->normals[i].x = n01[0];
					m_data->normals[i].y = n01[1];
					m_data->normals[i].z = n01[2];
					m_data->normals[i].w = 1;
				}
			}
		}
	}

	if (m_data->useNormalAveraging && !m_data->useGaussSmoothing) {
		applyTemporalNormalAveraging();
	} else if (m_data->useNormalAveraging && m_data->useGaussSmoothing) {
		applyGaussianNormalSmoothing();
	}
}

void PointCloudNormalEstimator::applyTemporalNormalAveraging() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->kernelNormalAveraging.setArgs(m_data->normalsBuffer,
					m_data->avgNormalsBuffer,
					m_data->w,
					m_data->h,
					m_data->normalAveragingRange);
			m_data->kernelNormalAveraging.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		const int r = m_data->normalAveragingRange;
		for (int y = 0; y < m_data->h; y++) {
			for (int x = 0; x < m_data->w; x++) {
				int i = x + m_data->w * y;
				if (y < r || y >= m_data->h - r || x < r
						|| x >= m_data->w - r) {
					m_data->avgNormals[i] = m_data->normals[i];
				} else {
					Vec4 avg;
					avg.x = 0, avg.y = 0, avg.z = 0, avg.w = 0;
					for (int sx = -r; sx <= r; sx++) {
						for (int sy = -r; sy <= r; sy++) {
							avg.x += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].x;
							avg.y += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].y;
							avg.z += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].z;
						}
					}
					avg.x /= ((1 + 2 * r) * (1 + 2 * r));
					avg.y /= ((1 + 2 * r) * (1 + 2 * r));
					avg.z /= ((1 + 2 * r) * (1 + 2 * r));
					avg.w = 1;
					m_data->avgNormals[i] = avg;
				}
			}
		}
	}
}

void PointCloudNormalEstimator::applyGaussianNormalSmoothing() {
	float norm = 1;
	DynMatrix<float> kernel = DynMatrix<float>(1, 1, 0.0);
	int l = 0;
	int kSize = 1;
	int rowSize = 1;
        (void)kSize;
        (void)rowSize;

	if (m_data->normalAveragingRange <= 1) {
		// nothing!
	} else if (m_data->normalAveragingRange <= 3) {
		norm = 16.;
		l = 1;
		kSize = 3 * 3;
		rowSize = 3;
		DynMatrix<float> k1 = DynMatrix<float>(1, 3, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 2.;
		k1(0, 2) = 1.;
		kernel = k1 * k1.transp();
	} else if (m_data->normalAveragingRange <= 5) {
		norm = 256.;
		l = 2;
		kSize = 5 * 5;
		rowSize = 5;
		DynMatrix<float> k1 = DynMatrix<float>(1, 5, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 4.;
		k1(0, 2) = 6.;
		k1(0, 3) = 4.;
		k1(0, 4) = 1.;
		kernel = k1 * k1.transp();
	} else {
		norm = 4096.;
		l = 3;
		kSize = 7 * 7;
		rowSize = 7;
		DynMatrix<float> k1 = DynMatrix<float>(1, 7, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 6.;
		k1(0, 2) = 15.;
		k1(0, 3) = 20.;
		k1(0, 4) = 15.;
		k1(0, 5) = 6.;
		k1(0, 6) = 1.;
		kernel = k1 * k1.transp();
	}
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
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

			m_data->kernelNormalGaussSmoothing.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		for (int y = 0; y < m_data->h; y++) {
			for (int x = 0; x < m_data->w; x++) {
				int i = x + m_data->w * y;
				if (y < l || y >= m_data->h - l || x < l || x >= m_data->w - l
						|| l == 0) {
					m_data->avgNormals[i] = m_data->normals[i];
				} else {
					Vec4 avg;
					avg.x = 0, avg.y = 0, avg.z = 0, avg.w = 0;
					for (int sx = -l; sx <= l; sx++) {
						for (int sy = -l; sy <= l; sy++) {
							avg.x += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].x
									* kernel(sx + l, sy + l);
							avg.y += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].y
									* kernel(sx + l, sy + l);
							avg.z += m_data->normals[(x + sx)
									+ m_data->w * (y + sy)].z
									* kernel(sx + l, sy + l);
						}
					}
					avg.x /= norm;
					avg.y /= norm;
					avg.z /= norm;
					avg.w = 1;
					m_data->avgNormals[i] = avg;
				}
			}
		}
	}
}

const Vec *PointCloudNormalEstimator::getNormals() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			if(m_data->useNormalAveraging==true) {
				m_data->avgNormalsBuffer.read(m_data->outputNormals, m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
				m_data->avgNormals=m_data->outputNormals;
				return (const Vec*)m_data->avgNormals;
			} else {
				m_data->normalsBuffer.read( //read output from kernel
						m_data->outputNormals,
						m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
				m_data->normals=m_data->outputNormals;
				return (const Vec*)m_data->normals;
			}
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
		return (const Vec*) m_data->normals;
	} else {
		if (m_data->useNormalAveraging == true) {
			return (const Vec*) m_data->avgNormals;
		} else {
			return (const Vec*) m_data->normals;
		}
	}
}

void PointCloudNormalEstimator::applyWorldNormalCalculation(const Camera &cam) {
	Mat T = cam.getCSTransformationMatrix();
	FixedMatrix<float, 3, 3> R = T.part<0, 0, 3, 3>();
	Mat T2 = R.transp().resize<4, 4>(0);
	T2(3, 3) = 1;
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->camBuffer = m_data->program.createBuffer("rw", 16 * sizeof(float), (void *) &T2[0]);

			m_data->kernelWorldNormalCalculation.setArgs(m_data->rawImageBuffer,
					m_data->normalImageRBuffer,
					m_data->normalImageGBuffer,
					m_data->normalImageBBuffer); //set parameter for kernel
			if(m_data->useNormalAveraging==true) {
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
#endif
	} else {
		const core::Channel32f d = m_data->rawImage[0];
		for (int y = 0; y < m_data->h; ++y) {
			for (int x = 0; x < m_data->w; ++x) {
				int i = x + m_data->w * y;
				if (d[i] == 2047) {
					m_data->normalImage(x, y, 0) = 0;
					m_data->normalImage(x, y, 1) = 0;
					m_data->normalImage(x, y, 2) = 0;
				} else {
					Vec pWN;
					if (m_data->useNormalAveraging == true) {
						pWN = T2 * (Vec&) m_data->avgNormals[i];
					} else {
						pWN = T2 * (Vec&) m_data->normals[i];
					}
					m_data->worldNormals[i].x = -pWN[0];
					m_data->worldNormals[i].y = -pWN[1];
					m_data->worldNormals[i].z = -pWN[2];
					m_data->worldNormals[i].w = 1.;

					m_data->normalImage(x, y, 0) = (int) (fabs(pWN[0]) * 255.);
					m_data->normalImage(x, y, 1) = (int) (fabs(pWN[1]) * 255.);
					m_data->normalImage(x, y, 2) = (int) (fabs(pWN[2]) * 255.);
				}
			}
		}
	}
}

const Vec* PointCloudNormalEstimator::getWorldNormals() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->worldNormalsBuffer.read(m_data->outputWorldNormals,
					m_data->w*m_data->h * sizeof(FixedColVector<float, 4>));
			m_data->worldNormals=m_data->outputWorldNormals;
			return (const Vec*)m_data->worldNormals;
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
		return (const Vec*) m_data->worldNormals;
	} else {
		return (const Vec*) m_data->worldNormals;
	}
}

const core::Img8u &PointCloudNormalEstimator::getRGBNormalImage() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->normalImageRBuffer.read(m_data->normalImageRArray,
					m_data->w*m_data->h * sizeof(unsigned char));

			m_data->normalImageGBuffer.read(m_data->normalImageGArray,
					m_data->w*m_data->h * sizeof(unsigned char));
			m_data->normalImageBBuffer.read(m_data->normalImageBArray,
					m_data->w*m_data->h * sizeof(unsigned char));

			std::vector<icl8u*> data(3);
			data[0] = m_data->normalImageRArray;
			data[1] = m_data->normalImageGArray;
			data[2] = m_data->normalImageBArray;
			m_data->normalImage = Img8u(Size(m_data->w,m_data->h),3,data,false);

			return m_data->normalImage;

		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
		return m_data->normalImage;
	} else {
		return m_data->normalImage;
	}
}

void PointCloudNormalEstimator::setNormals(Vec* pNormals) {
	if (m_data->useNormalAveraging == true) {
		m_data->avgNormals = (Vec4*) pNormals;
	} else {
		m_data->normals = (Vec4*) pNormals;
	}
#ifdef ICL_HAVE_OPENCL
	if(m_data->useCL==true && m_data->clReady==true) {
		try {
			if(m_data->useNormalAveraging==true) {
				m_data->avgNormalsBuffer = m_data->program.createBuffer("r", m_data->w*m_data->h * sizeof(FixedColVector<float, 4>), m_data->avgNormals);
			} else {
				m_data->normalsBuffer = m_data->program.createBuffer("r", m_data->w*m_data->h * sizeof(FixedColVector<float, 4>), m_data->normals);
			}
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
	}
#endif
}

void PointCloudNormalEstimator::applyAngleImageCalculation() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			if(m_data->useNormalAveraging==true) {
				m_data->kernelAngleImageCalculation[0] = m_data->avgNormalsBuffer;

			} else {
				m_data->kernelAngleImageCalculation[0] = m_data->normalsBuffer;
			}
			m_data->kernelAngleImageCalculation[1] = m_data->angleImageBuffer;
			m_data->kernelAngleImageCalculation[2] = m_data->w;
			m_data->kernelAngleImageCalculation[3] = m_data->h;
			m_data->kernelAngleImageCalculation[4] = m_data->neighborhoodRange;
			m_data->kernelAngleImageCalculation[5] = m_data->neighborhoodMode;
			m_data->kernelAngleImageCalculation.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		Vec4 * norm;
		if (m_data->useNormalAveraging == true) {
			norm = m_data->avgNormals;
		} else {
			norm = m_data->normals;
		}
		const int w = m_data->w, h = m_data->h;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int i = x + w * y;
				if (y < m_data->neighborhoodRange
						|| y >= h - (m_data->neighborhoodRange)
						|| x < m_data->neighborhoodRange
						|| x >= w - (m_data->neighborhoodRange)) {
					m_data->angleImage(x, y, 0) = 0;
				} else {
					float snr = 0; //sum right
					float snl = 0; //sum left
					float snt = 0; //sum top
					float snb = 0; //sum bottom
					float sntr = 0; //sum top-right
					float sntl = 0; //sum top-left
					float snbr = 0; //sum bottom-right
					float snbl = 0; //sum bottom-left
					for (int z = 1; z <= m_data->neighborhoodRange; z++) {
						//angle between normals
						float nr = (norm[i].x * norm[i + z].x
								+ norm[i].y * norm[i + z].y
								+ norm[i].z * norm[i + z].z);
						float nl = (norm[i].x * norm[i - z].x
								+ norm[i].y * norm[i - z].y
								+ norm[i].z * norm[i - z].z);
						float nt = (norm[i].x * norm[i + w * z].x
								+ norm[i].y * norm[i + w * z].y
								+ norm[i].z * norm[i + w * z].z);
						float nb = (norm[i].x * norm[i - w * z].x
								+ norm[i].y * norm[i - w * z].y
								+ norm[i].z * norm[i - w * z].z);
						float ntr = (norm[i].x * norm[i + w * z + z].x
								+ norm[i].y * norm[i + w * z + z].y
								+ norm[i].z * norm[i + w * z + z].z);
						float ntl = (norm[i].x * norm[i + w * z - z].x
								+ norm[i].y * norm[i + w * z - z].y
								+ norm[i].z * norm[i + w * z - z].z);
						float nbr = (norm[i].x * norm[i - w * z + z].x
								+ norm[i].y * norm[i - w * z + z].y
								+ norm[i].z * norm[i - w * z + z].z);
						float nbl = (norm[i].x * norm[i - w * z - z].x
								+ norm[i].y * norm[i - w * z - z].y
								+ norm[i].z * norm[i - w * z - z].z);
						//flip if angle is bigger than 90°
						if (nr < cos(M_PI / 2))
							nr = cos(M_PI - acos(nr));
						if (nl < cos(M_PI / 2))
							nl = cos(M_PI - acos(nl));
						if (nt < cos(M_PI / 2))
							nt = cos(M_PI - acos(nt));
						if (nb < cos(M_PI / 2))
							nb = cos(M_PI - acos(nb));
						if (ntr < cos(M_PI / 2))
							ntr = cos(M_PI - acos(ntr));
						if (ntl < cos(M_PI / 2))
							ntl = cos(M_PI - acos(ntl));
						if (nbr < cos(M_PI / 2))
							nbr = cos(M_PI - acos(nbr));
						if (nbl < cos(M_PI / 2))
							nbl = cos(M_PI - acos(nbl));

						snr += nr;
						snl += nl;
						snt += nt;
						snb += nb;
						sntr += ntr;
						sntl += ntl;
						snbr += nbr;
						snbl += nbl;
					}
					snr /= m_data->neighborhoodRange;
					snl /= m_data->neighborhoodRange;
					snt /= m_data->neighborhoodRange;
					snb /= m_data->neighborhoodRange;
					sntr /= m_data->neighborhoodRange;
					sntl /= m_data->neighborhoodRange;
					snbr /= m_data->neighborhoodRange;
					snbl /= m_data->neighborhoodRange;

					if (m_data->neighborhoodMode == 0) {
						m_data->angleImage(x, y, 0) = snr;
						if (m_data->angleImage(x, y, 0) > snl) {
							m_data->angleImage(x, y, 0) = snl;
						}
						if (m_data->angleImage(x, y, 0) > snt) {
							m_data->angleImage(x, y, 0) = snt;
						}
						if (m_data->angleImage(x, y, 0) > snb) {
							m_data->angleImage(x, y, 0) = snb;
						}
						if (m_data->angleImage(x, y, 0) > snbl) {
							m_data->angleImage(x, y, 0) = snbl;
						}
						if (m_data->angleImage(x, y, 0) > snbr) {
							m_data->angleImage(x, y, 0) = snbr;
						}
						if (m_data->angleImage(x, y, 0) > sntl) {
							m_data->angleImage(x, y, 0) = sntl;
						}
						if (m_data->angleImage(x, y, 0) > sntr) {
							m_data->angleImage(x, y, 0) = sntr;
						}
					} else if (m_data->neighborhoodMode == 1) {
						m_data->angleImage(x, y, 0) = (snr + snl + snt + snb
								+ sntr + sntl + snbr + snbl) / 8;
					} else {
						std::cout << "Unknown neighborhood mode" << std::endl;
					}
				}
			}
		}
	}
}

const Img32f &PointCloudNormalEstimator::getAngleImage() {
#ifdef ICL_HAVE_OPENCL
	if(m_data->useCL==true && m_data->clReady==true) {
		try {
			m_data->angleImageBuffer.read(m_data->outputAngleImage, m_data->w*m_data->h * sizeof(float));
			m_data->angleImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputAngleImage),false);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
	}
#endif
	return m_data->angleImage;
}

void PointCloudNormalEstimator::setAngleImage(const Img32f &angleImg) {
  m_data->angleImage = angleImg;
#ifdef ICL_HAVE_OPENCL
  if(m_data->useCL==true && m_data->clReady==true) {
    //m_data->angleImageArray=m_data->angleImage.begin(0); //image to float array
    try {
      m_data->angleImageBuffer = m_data->program.createBuffer("r", m_data->w*m_data->h * sizeof(float),
                                                              angleImg.begin(0));
    } catch (CLException &err) { //catch openCL errors
      std::cout<< "ERROR: "<< err.what()<< std::endl;
    }
  }
#endif
}

void PointCloudNormalEstimator::applyImageBinarization() {
	if (m_data->useCL == true && m_data->clReady == true) {
#ifdef ICL_HAVE_OPENCL
		try {
			m_data->kernelImageBinarization.setArgs(m_data->angleImageBuffer,
					m_data->binarizedImageBuffer,
					m_data->w,
					m_data->h,
					m_data->binarizationThreshold);

			m_data->kernelImageBinarization.apply(m_data->w*m_data->h);
		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
#endif
	} else {
		for (int y = 0; y < m_data->h; y++) {
			for (int x = 0; x < m_data->w; x++) {
				if (m_data->angleImage(x, y, 0)
						> m_data->binarizationThreshold) {
					m_data->binarizedImage(x, y, 0) = 255;
				} else {
					m_data->binarizedImage(x, y, 0) = 0;
				}
			}
		}
	}
}

const Img8u &PointCloudNormalEstimator::getBinarizedAngleImage() {
#ifdef ICL_HAVE_OPENCL
	if(m_data->useCL==true && m_data->clReady==true) {
		try {
			m_data->binarizedImageBuffer.read(m_data->outputBinarizedImage, m_data->w*m_data->h * sizeof(unsigned char));
			m_data->binarizedImage = Img8u(Size(m_data->w,m_data->h),1,std::vector<unsigned char*>(1,m_data->outputBinarizedImage),false);

		} catch (CLException &err) { //catch openCL errors
			std::cout<< "ERROR: "<< err.what()<< std::endl;
		}
	}
#endif
	return m_data->binarizedImage;
}

void PointCloudNormalEstimator::setMedianFilterSize(int size) {
	m_data->medianFilterSize = size;
}

void PointCloudNormalEstimator::setNormalCalculationRange(int range) {
	m_data->normalRange = range;
}

void PointCloudNormalEstimator::setNormalAveragingRange(int range) {
	m_data->normalAveragingRange = range;
}

void PointCloudNormalEstimator::setAngleNeighborhoodMode(int mode) {
	m_data->neighborhoodMode = mode;
}

void PointCloudNormalEstimator::setAngleNeighborhoodRange(int range) {
	m_data->neighborhoodRange = range;
}

void PointCloudNormalEstimator::setBinarizationThreshold(float threshold) {
	m_data->binarizationThreshold = threshold;
}

void PointCloudNormalEstimator::setUseCL(bool use) {
	m_data->useCL = use;
}

void PointCloudNormalEstimator::setUseNormalAveraging(bool use) {
	m_data->useNormalAveraging = use;
}

void PointCloudNormalEstimator::setUseGaussSmoothing(bool use) {
	m_data->useGaussSmoothing = use;
}

bool PointCloudNormalEstimator::isCLReady() {
	return m_data->clReady;
}

bool PointCloudNormalEstimator::isCLActive() {
	return m_data->useCL;
}

const Img8u &PointCloudNormalEstimator::calculate(const Img32f &depthImage,
		bool filter, bool average, bool gauss) {
	if (filter == false) {
		setFilteredDepthImage(depthImage);
	} else {
		setDepthImage(depthImage);
		applyMedianFilter();
	}
	m_data->useNormalAveraging = average;
	m_data->useGaussSmoothing = gauss;
	applyNormalCalculation();
	applyAngleImageCalculation();
	applyImageBinarization();
	return getBinarizedAngleImage();
}
} // namespace geom
}

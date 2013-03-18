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
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#endif

#include <ICLGeom/PointCloudNormalEstimator.h>

namespace icl{

  using namespace utils;
  using namespace math;
  using namespace core;

  namespace geom{
    

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
      "}                                                                                                                          \n"
      ;


#ifdef HAVE_OPENCL
    typedef cl_float4 Vec4;
#else
    struct Vec4{
      inline Vec4(){}
      inline Vec4(float x,float y, float z, float w): x(x),y(y),z(z),w(w){}
      float x,y,z,w;
    };
#endif
    
    struct PointCloudNormalEstimator::Data{
      Data(const Size &size){
        //set default values
        medianFilterSize=3;
        normalRange=2;
        normalAveragingRange=1;
        neighborhoodMode=0;
        neighborhoodRange=3;
        binarizationThreshold=0.89;
        clReady=false;
        useCL=true;
        useNormalAveraging=true;
        useGaussSmoothing=false;
        
        //create arrays and images in given size
        if(size==Size::QVGA){
          std::cout<<"Resolution: 320x240"<<std::endl;
          w=320;
          h=240;
          
        }else if(size==Size::VGA){
          std::cout<<"Resolution: 640x480"<<std::endl;
          w=640;
          h=480;
        }else{
          std::cout<<"Unknown Resolution"<<std::endl;
          w=size.width;
          h=size.height;
        }
        
        normals = new Vec4[w*h];
        avgNormals = new Vec4[w*h];
        worldNormals = new Vec4[w*h];
        for(int i=0; i<w*h;i++){
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
  	
        rawImage.setSize(Size(w,h));
        rawImage.setChannels(1);
        filteredImage.setSize(Size(w,h));
        filteredImage.setChannels(1);
        angleImage.setSize(Size(w,h));
        angleImage.setChannels(1);
        binarizedImage.setSize(Size(w,h));
        binarizedImage.setChannels(1);
        normalImage.setSize(Size(w,h));
        normalImage.setChannels(3);
        
#ifdef HAVE_OPENCL
        //create openCL context
        rawImageArray = new float[w*h];
        filteredImageArray = new float[w*h];
        angleImageArray = new float[w*h];
        binarizedImageArray = new cl_uchar[w*h];
        normalImageRArray = new cl_uchar[w*h];
        normalImageGArray = new cl_uchar[w*h];
        normalImageBArray = new cl_uchar[w*h];
        
        outputNormals=new cl_float4[w*h];
        outputFilteredImage=new float[w*h];
        outputAngleImage=new float[w*h];
        outputBinarizedImage=new cl_uchar[w*h];
        outputWorldNormals=new cl_float4[w*h];
        
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
            
            cl::Program::Sources sources(1, std::make_pair(normalEstimationKernel, 0)); //kernel source
            program=cl::Program(context, sources); //program (bind context and source)
            program.build(devices);//build program
            
            //create buffer for memory access and allocation
            rawImageBuffer = cl::Buffer(
                                        context, 
                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                        w*h * sizeof(float), 
                                        (void *) &rawImageArray[0]);
                    
            filteredImageBuffer = cl::Buffer(
                                             context, 
                                             CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                             w*h * sizeof(float), 
                                             (void *) &filteredImageArray[0]);
              
            normalsBuffer = cl::Buffer(
                                       context, 
                                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                       w*h * sizeof(cl_float4),
                                       (void *) &normals[0]);
                    
            avgNormalsBuffer = cl::Buffer(
                                          context, 
                                          CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                          w*h * sizeof(cl_float4),
                                          (void *) &avgNormals[0]);
                    
            angleImageBuffer = cl::Buffer(
                                          context, 
                                          CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                          w*h * sizeof(float), 
                                          (void *) &angleImageArray[0]);
               
            binarizedImageBuffer = cl::Buffer(
                                              context, 
                                              CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
                                              w*h * sizeof(cl_uchar), 
                                              (void *) &binarizedImageArray[0]);
  			
            normalImageRBuffer = cl::Buffer(
                                            context, 
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
                                            w*h * sizeof(cl_uchar), 
                                            (void *) &normalImageRArray[0]);
  			
            normalImageGBuffer = cl::Buffer(
                                            context, 
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
                                            w*h * sizeof(cl_uchar), 
                                            (void *) &normalImageGArray[0]);
  			
            normalImageBBuffer = cl::Buffer(
                                            context, 
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
                                            w*h * sizeof(cl_uchar), 
                                            (void *) &normalImageBArray[0]);
  				
            worldNormalsBuffer = cl::Buffer(
                                            context, 
                                            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                            w*h * sizeof(cl_float4),
                                            (void *) &worldNormals[0]);
                
            //create kernels    
            kernelMedianFilter=cl::Kernel(program, "medianFilter");
            kernelNormalCalculation=cl::Kernel(program, "normalCalculation");
            kernelNormalAveraging=cl::Kernel(program, "normalAveraging");
            kernelAngleImageCalculation=cl::Kernel(program, "angleImageCalculation");
            kernelImageBinarization=cl::Kernel(program, "imageBinarization");
            kernelWorldNormalCalculation=cl::Kernel(program, "worldNormalCalculation");
            kernelNormalGaussSmoothing=cl::Kernel(program, "normalGaussSmoothing");
          
            queue=cl::CommandQueue(context, devices[0], 0);//create command queue        
          }catch (cl::Error err) {//catch openCL errors
            std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
            clReady=false;
          }
        }
        
#else
        std::cout<<"no openCL parallelization available"<<std::endl;
        clReady=false;
#endif   
      }
      ~Data(){
        delete[] normals;
        delete[] avgNormals;
        delete[] worldNormals;
#ifdef HAVE_OPENCL
        delete[] rawImageArray;
        delete[] filteredImageArray;
        delete[] angleImageArray;
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
      int w,h;
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
  	
    #ifdef HAVE_OPENCL
      //OpenCL data
      Vec4 * outputNormals;
      Vec4 * outputWorldNormals;
      float* outputFilteredImage;//output of kernel for image
      float* outputAngleImage;
      cl_uchar* outputBinarizedImage;
      float* rawImageArray;//input for image to kernel
      float* filteredImageArray;
      float* angleImageArray;
      cl_uchar* binarizedImageArray;
      cl_uchar* normalImageRArray;
      cl_uchar* normalImageGArray;
      cl_uchar* normalImageBArray;
  	
      //OpenCL    
      cl::Context context;
      std::vector<cl::Device> devices;
      cl::Program program;
      cl::CommandQueue queue;
        
      cl::Kernel kernelMedianFilter;
      cl::Kernel kernelNormalCalculation;
      cl::Kernel kernelNormalAveraging;
      cl::Kernel kernelAngleImageCalculation;
      cl::Kernel kernelImageBinarization;
      cl::Kernel kernelWorldNormalCalculation;
      cl::Kernel kernelNormalGaussSmoothing;
      
      //OpenCL buffer
      cl::Buffer rawImageBuffer;
      cl::Buffer filteredImageBuffer;
      cl::Buffer normalsBuffer;
      cl::Buffer avgNormalsBuffer;
      cl::Buffer angleImageBuffer;
      cl::Buffer binarizedImageBuffer;
      cl::Buffer worldNormalsBuffer;
      cl::Buffer normalImageRBuffer;
      cl::Buffer normalImageGBuffer;
      cl::Buffer normalImageBBuffer;
      cl::Buffer camBuffer;
      cl::Buffer gaussKernelBuffer;
    #endif
    };

  
                   
    PointCloudNormalEstimator::PointCloudNormalEstimator(Size size):
      m_data(new Data(size)){
    }
  
  
    PointCloudNormalEstimator::~PointCloudNormalEstimator(){
      delete m_data;
    }
  
  
    void PointCloudNormalEstimator::setDepthImage(const Img32f &depthImg){
      m_data->rawImage=depthImg;
    #ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        m_data->rawImageArray=m_data->rawImage.begin(0);//image to float array
        try{
          m_data->rawImageBuffer = cl::Buffer(//write new image to buffer
                                              m_data->context, 
                                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                              m_data->w*m_data->h * sizeof(float), 
                                              (void *) &m_data->rawImageArray[0]);
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      }
#endif
    }
  
  
    void PointCloudNormalEstimator::applyMedianFilter(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{   
          m_data->kernelMedianFilter.setArg(0, m_data->rawImageBuffer);//set parameter for kernel
          m_data->kernelMedianFilter.setArg(1, m_data->filteredImageBuffer);
          m_data->kernelMedianFilter.setArg(2, m_data->w);
          m_data->kernelMedianFilter.setArg(3, m_data->h);
          m_data->kernelMedianFilter.setArg(4, m_data->medianFilterSize);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
  				   m_data->kernelMedianFilter, 
  				   cl::NullRange, 
  				   cl::NDRange(m_data->w*m_data->h), //input size for get global id
  				   cl::NullRange);
                
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        // TODO: heavily optimize this (MedianOp ??)
        std::vector<icl32f> list;
        for(int y=0; y<m_data->h; y++){
          for(int x=0; x<m_data->w; x++){
            if(y<(m_data->medianFilterSize-1)/2 || 
               y>=m_data->h-(m_data->medianFilterSize-1)/2 || 
               x<(m_data->medianFilterSize-1)/2 ||
               x>=m_data->w-(m_data->medianFilterSize-1)/2){
              m_data->filteredImage(x,y,0)=m_data->rawImage(x,y,0); //pixel out of range
            }else{
              for(int sx=-(m_data->medianFilterSize-1)/2; sx<=(m_data->medianFilterSize-1)/2; sx++){
                for(int sy=-(m_data->medianFilterSize-1)/2; sy<=(m_data->medianFilterSize-1)/2; sy++){
                  list.push_back(m_data->rawImage(x+sx,y+sy,0));
                }
              }
              std::sort(list.begin(),list.end());
              m_data->filteredImage(x,y,0)=list[m_data->medianFilterSize*m_data->medianFilterSize/2];
              list.clear();
            }
          }
        }
      }
    }
  
  
    const Img32f &PointCloudNormalEstimator::getFilteredDepthImage(){
    #ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        try{        
          m_data->queue.enqueueReadBuffer(//read output from kernel
  			        m_data->filteredImageBuffer,
  			        CL_TRUE, // block 
  			        0,
  			        m_data->w*m_data->h * sizeof(float),
  			        (float*) m_data->outputFilteredImage);
                    
          m_data->filteredImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputFilteredImage),false);
        }
        catch (cl::Error err) {//catch openCL error
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        }   
      }  
    #endif
      return m_data->filteredImage;
    }
  
  
    void PointCloudNormalEstimator::setFilteredDepthImage(const Img32f &filteredImg){
      m_data->filteredImage=filteredImg;
    #ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        m_data->filteredImageArray=m_data->filteredImage.begin(0);//image to float array
        try{
          m_data->filteredImageBuffer = cl::Buffer(//write new image to buffer
                                                   m_data->context, 
                                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                                   m_data->w*m_data->h * sizeof(float), 
                                                   (void *) &m_data->filteredImageArray[0]);
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      }
    #endif    
    }
  
  
    void PointCloudNormalEstimator::applyNormalCalculation(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          m_data->kernelNormalCalculation.setArg(0, m_data->filteredImageBuffer);//set parameter for kernel
          m_data->kernelNormalCalculation.setArg(1, m_data->normalsBuffer);
          m_data->kernelNormalCalculation.setArg(2, m_data->w);
          m_data->kernelNormalCalculation.setArg(3, m_data->h);
          m_data->kernelNormalCalculation.setArg(4, m_data->normalRange);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
  				   m_data->kernelNormalCalculation, 
  				   cl::NullRange, 
  				   cl::NDRange(m_data->w*m_data->h), //input size for get global id
  				   cl::NullRange);
                
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        const int r = m_data->normalRange;
        Vec fa1,fb1, n1, n01;
        for(int y=0;y<m_data->h;y++){
          for (int x=0;x<m_data->w;x++){
            int i= x+m_data->w*y;
            if(y<r || y>=m_data->h-r || 
               x<r || x>=m_data->w-r){
              m_data->normals[i].x=0;//points out of range
              m_data->normals[i].y=0;
              m_data->normals[i].z=0;
            }else{
              //cross product normal determination
              fa1[0]=(x+r)-(x-r);
              fa1[1]=(y-r)-(y-r);
              fa1[2]=m_data->filteredImage(x+r,y-r,0)-m_data->filteredImage(x-r,y-r,0);
              fb1[0]=(x)-(x-r);
              fb1[1]=(y+r)-(y-r);	
              fb1[2]=m_data->filteredImage(x,y+r,0)-m_data->filteredImage(x-r,y-r,0);
              
              n1[0]=fa1[1]*fb1[2]-fa1[2]*fb1[1];
              n1[1]=fa1[2]*fb1[0]-fa1[0]*fb1[2];
              n1[2]=fa1[0]*fb1[1]-fa1[1]*fb1[0];
              n01[0]=n1[0]/norm3(n1);
              n01[1]=n1[1]/norm3(n1);
              n01[2]=n1[2]/norm3(n1);
              
              m_data->normals[i].x=n01[0];
              m_data->normals[i].y=n01[1];
              m_data->normals[i].z=n01[2];
              m_data->normals[i].w=1;
            }
          }
        }
      }
        
      if(m_data->useNormalAveraging && !m_data->useGaussSmoothing){
        applyTemporalNormalAveraging();
      }else if(m_data->useNormalAveraging && m_data->useGaussSmoothing){
        applyGaussianNormalSmoothing();
      }
    }
  
  
    void PointCloudNormalEstimator::applyTemporalNormalAveraging(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          m_data->kernelNormalAveraging.setArg(0, m_data->normalsBuffer);//set parameter for kernel
          m_data->kernelNormalAveraging.setArg(1, m_data->avgNormalsBuffer);
          m_data->kernelNormalAveraging.setArg(2, m_data->w);
          m_data->kernelNormalAveraging.setArg(3, m_data->h);
          m_data->kernelNormalAveraging.setArg(4, m_data->normalAveragingRange);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
  				   m_data->kernelNormalAveraging, 
  				   cl::NullRange, 
  				   cl::NDRange(m_data->w*m_data->h), //input size for get global id
  				   cl::NullRange);
                
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        const int r = m_data->normalAveragingRange;
        for(int y=0; y<m_data->h;y++){
          for(int x=0; x<m_data->w; x++){
            int i= x+m_data->w*y;
            if(y<r || y>=m_data->h-r || x<r || x>=m_data->w-r){
              m_data->avgNormals[i]=m_data->normals[i];
            }else{
              Vec4 avg;
              avg.x=0, avg.y=0, avg.z=0, avg.w=0;
              for(int sx=-r; sx<=r; sx++){
                for(int sy=-r; sy<=r; sy++){
                  avg.x+=m_data->normals[(x+sx)+m_data->w*(y+sy)].x;
                  avg.y+=m_data->normals[(x+sx)+m_data->w*(y+sy)].y;
                  avg.z+=m_data->normals[(x+sx)+m_data->w*(y+sy)].z;
                }
              }
              avg.x/=((1+2*r)*(1+2*r));
              avg.y/=((1+2*r)*(1+2*r));
              avg.z/=((1+2*r)*(1+2*r));
              avg.w=1;
              m_data->avgNormals[i]=avg;
            }
          }
        }
      }
    }
    
    
    void PointCloudNormalEstimator::applyGaussianNormalSmoothing(){
      float norm=1;
      DynMatrix<float> kernel = DynMatrix<float>(1,1,0.0);
      int l=0;
      int kSize=1;
      int rowSize=1;
      if(m_data->normalAveragingRange<=1){
        // nothing!
      }else if(m_data->normalAveragingRange<=3){
        norm=16.;
        l=1;
        kSize=3*3;
        rowSize=3;
        DynMatrix<float> k1 = DynMatrix<float>(1,3, 0.0);
        k1(0,0)=1.;
        k1(0,1)=2.;
        k1(0,2)=1.;
        kernel=k1*k1.transp();
      }else if(m_data->normalAveragingRange<=5){
        norm=256.;
        l=2;
        kSize=5*5;
        rowSize=5;
        DynMatrix<float> k1 = DynMatrix<float>(1,5, 0.0);
        k1(0,0)=1.;
        k1(0,1)=4.;
        k1(0,2)=6.;
        k1(0,3)=4.;
        k1(0,4)=1.;
        kernel=k1*k1.transp();
      }else{
        norm=4096.;
        l=3;
        kSize=7*7;
        rowSize=7;
        DynMatrix<float> k1 = DynMatrix<float>(1,7, 0.0);
        k1(0,0)=1.;
        k1(0,1)=6.;
        k1(0,2)=15.;
        k1(0,3)=20.;
        k1(0,4)=15.;
        k1(0,5)=6.;
        k1(0,6)=1.;
        kernel=k1*k1.transp();
      }
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          m_data->gaussKernelBuffer = cl::Buffer(m_data->context, 
                                                 CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                                 kSize * sizeof(float), 
                                                 (void *) &kernel[0]);
          
          m_data->kernelNormalGaussSmoothing.setArg(0, m_data->normalsBuffer);//set parameter for kernel
          m_data->kernelNormalGaussSmoothing.setArg(1, m_data->avgNormalsBuffer);
          m_data->kernelNormalGaussSmoothing.setArg(2, m_data->w);
          m_data->kernelNormalGaussSmoothing.setArg(3, m_data->h);
          m_data->kernelNormalGaussSmoothing.setArg(4, l);
          m_data->kernelNormalGaussSmoothing.setArg(5, norm);
          m_data->kernelNormalGaussSmoothing.setArg(6, m_data->gaussKernelBuffer);
          m_data->kernelNormalGaussSmoothing.setArg(7, rowSize);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
  				   m_data->kernelNormalGaussSmoothing, 
  				   cl::NullRange, 
  				   cl::NDRange(m_data->w*m_data->h), //input size for get global id
  				   cl::NullRange);
                    
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        for(int y=0; y<m_data->h;y++){
          for(int x=0; x<m_data->w; x++){
            int i= x+m_data->w*y;
            if(y<l || y>=m_data->h-l || x<l || x>=m_data->w-l || l==0){
              m_data->avgNormals[i]=m_data->normals[i];
            }else{
              Vec4 avg;
              avg.x=0, avg.y=0, avg.z=0, avg.w=0;
              for(int sx=-l; sx<=l; sx++){
                for(int sy=-l; sy<=l; sy++){
                  avg.x+=m_data->normals[(x+sx)+m_data->w*(y+sy)].x*kernel(sx+l, sy+l);
                  avg.y+=m_data->normals[(x+sx)+m_data->w*(y+sy)].y*kernel(sx+l, sy+l);
                  avg.z+=m_data->normals[(x+sx)+m_data->w*(y+sy)].z*kernel(sx+l, sy+l);
                }
              }
              avg.x/=norm;
              avg.y/=norm;
              avg.z/=norm;
              avg.w=1;
              m_data->avgNormals[i]=avg;
            }
          }
        }
      }
    }
  
  
    const Vec *PointCloudNormalEstimator::getNormals(){
      if(m_data->useCL==true && m_data->clReady==true){
#ifdef HAVE_OPENCL
        try{ 
          if(m_data->useNormalAveraging==true){
            m_data->queue.enqueueReadBuffer(//read output from kernel
                                            m_data->avgNormalsBuffer,
                                            CL_TRUE, // block 
                                            0,
                                            m_data->w*m_data->h * sizeof(cl_float4),
                                            (cl_float4*) m_data->outputNormals);
            m_data->avgNormals=m_data->outputNormals;
            return (const Vec*)m_data->avgNormals;
          }else{     
            m_data->queue.enqueueReadBuffer(//read output from kernel
                                            m_data->normalsBuffer,
                                            CL_TRUE, // block 
                                            0,
                                            m_data->w*m_data->h * sizeof(cl_float4),
                                            (cl_float4*) m_data->outputNormals);
            m_data->normals=m_data->outputNormals;
            return (const Vec*)m_data->normals;
          }
        }
        catch (cl::Error err) {//catch openCL error
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        }
    #endif
        return (const Vec*)m_data->normals;
      }
      else{
        if(m_data->useNormalAveraging==true){
          return (const Vec*)m_data->avgNormals;
        }
        else{
          return (const Vec*)m_data->normals;
        }
      } 
    }
  
  
    void PointCloudNormalEstimator::applyWorldNormalCalculation(const Camera &cam){
      Mat T = cam.getCSTransformationMatrix();
      FixedMatrix<float,3,3> R = T.part<0,0,3,3>();
      Mat T2 = R.transp().resize<4,4>(0);
      T2(3,3) =1;     
      if(m_data->useCL==true && m_data->clReady==true){
#ifdef HAVE_OPENCL
        try{                       
          m_data->camBuffer = cl::Buffer(m_data->context, 
                                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                         16 * sizeof(float), 
                                         (void *) &T2[0]);
          
          m_data->kernelWorldNormalCalculation.setArg(0, m_data->rawImageBuffer);//set parameter for kernel
          m_data->kernelWorldNormalCalculation.setArg(1, m_data->normalImageRBuffer);
          m_data->kernelWorldNormalCalculation.setArg(2, m_data->normalImageGBuffer);
          m_data->kernelWorldNormalCalculation.setArg(3, m_data->normalImageBBuffer);
          if(m_data->useNormalAveraging==true){ 
            m_data->kernelWorldNormalCalculation.setArg(4, m_data->avgNormalsBuffer);
          }else{
            m_data->kernelWorldNormalCalculation.setArg(4, m_data->normalsBuffer);
          }
          m_data->kernelWorldNormalCalculation.setArg(5, m_data->camBuffer);
          m_data->kernelWorldNormalCalculation.setArg(6, m_data->worldNormalsBuffer);
          
          m_data->queue.enqueueNDRangeKernel(//run kernel
				     m_data->kernelWorldNormalCalculation, 
				     cl::NullRange, 
				     cl::NDRange(m_data->w*m_data->h), //input size for get global id
				     cl::NullRange);
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        const core::Channel32f d=m_data->rawImage[0];
        for(int y=0;y<m_data->h;++y){
          for(int x=0;x<m_data->w;++x){
            int i = x+m_data->w*y;
            if(d[i] == 2047){
              m_data->normalImage(x,y,0)=0;
              m_data->normalImage(x,y,1)=0;
              m_data->normalImage(x,y,2)=0;
            }else{
              Vec pWN;
              if(m_data->useNormalAveraging==true){ 
                pWN = T2*(Vec&)m_data->avgNormals[i];
              }else{
                pWN = T2*(Vec&)m_data->normals[i];
              }
              m_data->worldNormals[i].x=-pWN[0];
              m_data->worldNormals[i].y=-pWN[1];
              m_data->worldNormals[i].z=-pWN[2];
              m_data->worldNormals[i].w=1.;
              
              m_data->normalImage(x,y,0)=(int)(fabs(pWN[0])*255.);
              m_data->normalImage(x,y,1)=(int)(fabs(pWN[1])*255.);
              m_data->normalImage(x,y,2)=(int)(fabs(pWN[2])*255.);
            }	
          }
        }
      }
    }
  	
  	  
    const Vec* PointCloudNormalEstimator::getWorldNormals(){
      if(m_data->useCL==true && m_data->clReady==true){
#ifdef HAVE_OPENCL
        try{ 
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                          m_data->worldNormalsBuffer,
                                          CL_TRUE, // block 
                                          0,
                                          m_data->w*m_data->h * sizeof(cl_float4),
                                          (cl_float4*) m_data->outputWorldNormals);
  	      m_data->worldNormals=m_data->outputWorldNormals;
  	  
  	      return (const Vec*)m_data->worldNormals;
        }
        catch (cl::Error err) {//catch openCL error
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        }
#endif
        return (const Vec*)m_data->worldNormals;
      }
      else{  
        return (const Vec*)m_data->worldNormals;
      } 
    }
  
  
    const core::Img8u &PointCloudNormalEstimator::getRGBNormalImage(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{  
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                          m_data->normalImageRBuffer,
                                          CL_TRUE, // block 
                                          0,
                                          m_data->w*m_data->h * sizeof(cl_uchar),
                                          (cl_uchar*) m_data->normalImageRArray);
          
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                          m_data->normalImageGBuffer,
                                          CL_TRUE, // block 
                                          0,
                                          m_data->w*m_data->h * sizeof(cl_uchar),
                                          (cl_uchar*) m_data->normalImageGArray);
          
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                          m_data->normalImageBBuffer,
                                          CL_TRUE, // block 
                                          0,
                                          m_data->w*m_data->h * sizeof(cl_uchar),
                                          (cl_uchar*) m_data->normalImageBArray);
          
          std::vector<icl8u*> data(3);
          data[0] = m_data->normalImageRArray; 
          data[1] = m_data->normalImageGArray;
          data[2] = m_data->normalImageBArray; 
          m_data->normalImage = Img8u(Size(m_data->w,m_data->h),3,data,false);
          
          return m_data->normalImage;
            	        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
        return m_data->normalImage;
      }
      else{
        return m_data->normalImage;
      }
    }
  
  
    void PointCloudNormalEstimator::setNormals(Vec* pNormals){
      if(m_data->useNormalAveraging==true){
        m_data->avgNormals=(Vec4*)pNormals;
      }
      else{
        m_data->normals=(Vec4*)pNormals;
      }
    #ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        try{
          if(m_data->useNormalAveraging==true){
            m_data->avgNormalsBuffer = cl::Buffer(m_data->context, 
                                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                                  m_data->w*m_data->h * sizeof(cl_float4),
                                                  (void *) &m_data->avgNormals[0]);
          }else{
            m_data->normalsBuffer = cl::Buffer(m_data->context, 
                                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                               m_data->w*m_data->h * sizeof(cl_float4),
                                               (void *) &m_data->normals[0]);
          }
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      }
    #endif    
    }
  
  
    void PointCloudNormalEstimator::applyAngleImageCalculation(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{
          if(m_data->useNormalAveraging==true){            
            m_data->kernelAngleImageCalculation.setArg(0, m_data->avgNormalsBuffer);
          }else{
            m_data->kernelAngleImageCalculation.setArg(0, m_data->normalsBuffer);
          }
          m_data->kernelAngleImageCalculation.setArg(1, m_data->angleImageBuffer);//set parameter for kernel
          m_data->kernelAngleImageCalculation.setArg(2, m_data->w);
          m_data->kernelAngleImageCalculation.setArg(3, m_data->h);
          m_data->kernelAngleImageCalculation.setArg(4, m_data->neighborhoodRange);
          m_data->kernelAngleImageCalculation.setArg(5, m_data->neighborhoodMode);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
  				   m_data->kernelAngleImageCalculation, 
  				   cl::NullRange, 
  				   cl::NDRange(m_data->w*m_data->h), //input size for get global id
  				   cl::NullRange);
                
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        Vec4 * norm;
        if(m_data->useNormalAveraging==true){ 
          norm=m_data->avgNormals;
        }else{
          norm=m_data->normals;
        }
        const int w = m_data->w, h = m_data->h;
        for(int y=0;y<h;y++){
          for (int x=0;x<w;x++){
            int i= x+w*y;
            if(y<m_data->neighborhoodRange || y>=h-(m_data->neighborhoodRange) || 
               x<m_data->neighborhoodRange || x>=w-(m_data->neighborhoodRange)){
              m_data->angleImage(x,y,0)=0;
            }
            else{
              float snr=0;//sum right
              float snl=0;//sum left
              float snt=0;//sum top
              float snb=0;//sum bottom
              float sntr=0;//sum top-right
              float sntl=0;//sum top-left
              float snbr=0;//sum bottom-right
              float snbl=0;//sum bottom-left
              for(int z=1; z<=m_data->neighborhoodRange; z++){
                //angle between normals
                float nr=(norm[i].x*norm[i+z].x+norm[i].y*norm[i+z].y+norm[i].z*norm[i+z].z);
                float nl=(norm[i].x*norm[i-z].x+norm[i].y*norm[i-z].y+norm[i].z*norm[i-z].z);
                float nt=(norm[i].x*norm[i+w*z].x+norm[i].y*norm[i+w*z].y+norm[i].z*norm[i+w*z].z);
                float nb=(norm[i].x*norm[i-w*z].x+norm[i].y*norm[i-w*z].y+norm[i].z*norm[i-w*z].z);
                float ntr=(norm[i].x*norm[i+w*z+z].x+norm[i].y*norm[i+w*z+z].y+norm[i].z*norm[i+w*z+z].z);
                float ntl=(norm[i].x*norm[i+w*z-z].x+norm[i].y*norm[i+w*z-z].y+norm[i].z*norm[i+w*z-z].z);
                float nbr=(norm[i].x*norm[i-w*z+z].x+norm[i].y*norm[i-w*z+z].y+norm[i].z*norm[i-w*z+z].z);
                float nbl=(norm[i].x*norm[i-w*z-z].x+norm[i].y*norm[i-w*z-z].y+norm[i].z*norm[i-w*z-z].z);
                //flip if angle is bigger than 90°
                if(nr<cos(M_PI/2)) nr=cos(M_PI-acos(nr));
                if(nl<cos(M_PI/2)) nl=cos(M_PI-acos(nl));
                if(nt<cos(M_PI/2)) nt=cos(M_PI-acos(nt));
                if(nb<cos(M_PI/2)) nb=cos(M_PI-acos(nb));
                if(ntr<cos(M_PI/2)) ntr=cos(M_PI-acos(ntr));
                if(ntl<cos(M_PI/2)) ntl=cos(M_PI-acos(ntl));
                if(nbr<cos(M_PI/2)) nbr=cos(M_PI-acos(nbr));
                if(nbl<cos(M_PI/2)) nbl=cos(M_PI-acos(nbl));
        	
                snr+=nr;
                snl+=nl;
                snt+=nt;
                snb+=nb;
                sntr+=ntr;
                sntl+=ntl;
                snbr+=nbr;
                snbl+=nbl;
              }
              snr/=m_data->neighborhoodRange;
              snl/=m_data->neighborhoodRange;
              snt/=m_data->neighborhoodRange;
              snb/=m_data->neighborhoodRange;
              sntr/=m_data->neighborhoodRange;
              sntl/=m_data->neighborhoodRange;
              snbr/=m_data->neighborhoodRange;
              snbl/=m_data->neighborhoodRange;
              
              if(m_data->neighborhoodMode==0){
                m_data->angleImage(x,y,0)=snr;
                if(m_data->angleImage(x,y,0)>snl){
                  m_data->angleImage(x,y,0)=snl;
                }
                if(m_data->angleImage(x,y,0)>snt){
                  m_data->angleImage(x,y,0)=snt;
                }
                if(m_data->angleImage(x,y,0)>snb){
                  m_data->angleImage(x,y,0)=snb;
                }
                if(m_data->angleImage(x,y,0)>snbl){
                  m_data->angleImage(x,y,0)=snbl;
                }
                if(m_data->angleImage(x,y,0)>snbr){
                  m_data->angleImage(x,y,0)=snbr;
                }
                if(m_data->angleImage(x,y,0)>sntl){
                  m_data->angleImage(x,y,0)=sntl;
                }
                if(m_data->angleImage(x,y,0)>sntr){
                  m_data->angleImage(x,y,0)=sntr;
                }
              }
              else if(m_data->neighborhoodMode==1){
                m_data->angleImage(x,y,0)=(snr+snl+snt+snb+sntr+sntl+snbr+snbl)/8;
              }
              else{
                std::cout<<"Unknown neighborhood mode"<<std::endl;
              }
            }
          }
        }
      }
    }
    
    
    const Img32f &PointCloudNormalEstimator::getAngleImage(){
#ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        try{
          m_data->queue.enqueueReadBuffer(//read output from kernel
  			        m_data->angleImageBuffer,
  			        CL_TRUE, // block 
  			        0,
  			        m_data->w*m_data->h * sizeof(float),
  			        (float*) m_data->outputAngleImage);
                
          m_data->angleImage = Img32f(Size(m_data->w,m_data->h),1,std::vector<float*>(1,m_data->outputAngleImage),false);
        }
        catch (cl::Error err) {//catch openCL error
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        }       
      }
    #endif  
      return m_data->angleImage;
    }
  
  
    void PointCloudNormalEstimator::setAngleImage(const Img32f &angleImg){
      m_data->angleImage=angleImg;
    #ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        m_data->angleImageArray=m_data->angleImage.begin(0);//image to float array
        try{
          m_data->angleImageBuffer = cl::Buffer(//write new image to buffer
  				      m_data->context, 
  				      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
  				      m_data->w*m_data->h * sizeof(float), 
  				      (void *) &m_data->angleImageArray[0]);
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      }
    #endif    
    }
  
  
    void PointCloudNormalEstimator::applyImageBinarization(){
      if(m_data->useCL==true && m_data->clReady==true){
    #ifdef HAVE_OPENCL
        try{            
          m_data->kernelImageBinarization.setArg(0, m_data->angleImageBuffer);//set parameter for kernel
          m_data->kernelImageBinarization.setArg(1, m_data->binarizedImageBuffer);
          m_data->kernelImageBinarization.setArg(2, m_data->w);
          m_data->kernelImageBinarization.setArg(3, m_data->h);
          m_data->kernelImageBinarization.setArg(4, m_data->binarizationThreshold);
                
          m_data->queue.enqueueNDRangeKernel(//run kernel
                                             m_data->kernelImageBinarization, 
                                             cl::NullRange, 
                                             cl::NDRange(m_data->w*m_data->h), //input size for get global id
                                             cl::NullRange);
                
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{ 
        for(int y=0;y<m_data->h;y++){
          for (int x=0;x<m_data->w;x++){
            if(m_data->angleImage(x,y,0)>m_data->binarizationThreshold){
              m_data->binarizedImage(x,y,0)=255;
            }else{
              m_data->binarizedImage(x,y,0)=0;
            }
          }
        }
      }
    }
    
    
    const Img8u &PointCloudNormalEstimator::getBinarizedAngleImage(){
#ifdef HAVE_OPENCL
      if(m_data->useCL==true && m_data->clReady==true){
        try{            
          m_data->queue.enqueueReadBuffer(//read output from kernel
                                          m_data->binarizedImageBuffer,
                                          CL_TRUE, // block 
                                          0,
                                          m_data->w*m_data->h * sizeof(cl_uchar),
                                          (cl_uchar*) m_data->outputBinarizedImage);
          
          m_data->binarizedImage = Img8u(Size(m_data->w,m_data->h),1,std::vector<cl_uchar*>(1,m_data->outputBinarizedImage),false);
        }
        catch (cl::Error err) {//catch openCL error
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        }       
      } 
    #endif 
      return m_data->binarizedImage;
    }
    
    
    void PointCloudNormalEstimator::setMedianFilterSize(int size){
      m_data->medianFilterSize=size;
    }
  
  
    void PointCloudNormalEstimator::setNormalCalculationRange(int range){
      m_data->normalRange=range;
    }
  
  
    void PointCloudNormalEstimator::setNormalAveragingRange(int range){
      m_data->normalAveragingRange=range;
    }
  
  
    void PointCloudNormalEstimator::setAngleNeighborhoodMode(int mode){
      m_data->neighborhoodMode=mode;
    }
  
    void PointCloudNormalEstimator::setAngleNeighborhoodRange(int range){
      m_data->neighborhoodRange=range;
    }
  
  
    void PointCloudNormalEstimator::setBinarizationThreshold(float threshold){
      m_data->binarizationThreshold=threshold;
    }
  
  
    void PointCloudNormalEstimator::setUseCL(bool use){
      m_data->useCL=use;
    }
  
  
    void PointCloudNormalEstimator::setUseNormalAveraging(bool use){
      m_data->useNormalAveraging=use;
    }

    void PointCloudNormalEstimator::setUseGaussSmoothing(bool use){
      m_data->useGaussSmoothing=use;
    }
  
  
    bool PointCloudNormalEstimator::isCLReady(){
      return m_data->clReady;
    }
  
  
    bool PointCloudNormalEstimator::isCLActive(){
      return m_data->useCL;
    }
  
  
    const Img8u &PointCloudNormalEstimator::calculate(const Img32f &depthImage, bool filter, bool average, bool gauss){
      if(filter==false){
        setFilteredDepthImage(depthImage);
      }
      else{
        setDepthImage(depthImage);
        applyMedianFilter();
      }
      m_data->useNormalAveraging=average;
      m_data->useGaussSmoothing=gauss;
      applyNormalCalculation();
      applyAngleImageCalculation();
      applyImageBinarization();
      return getBinarizedAngleImage();
    }
  } // namespace geom
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PointcloudSceneObject.cpp                  **
** Module : ICLGeom                                                **
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

#include <ICLGeom/PointcloudSceneObject.h>

#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#endif

#include <ICLQuick/Quick.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
  //OpenCL kernel code
  static char pointcloudViewerKernel[] = 
    "__kernel void                                                                                                                  \n"
    "calculateUniColor(__global float const * depth, __global float4 const * vrOffset, __global float4 const * vrDirection, __global float const * norms, __global float4 * outV, __global float4 * outC, float4 const color, float const scale)                                                                                        \n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float d = 1.046 * (depth[id]==2047 ? 0 : 1000. / (depth[id] * -0.0030711016 + 3.3309495161));                               \n"
    "   float lambda = d*norms[id]*scale;                                                                                           \n"
    "   float4 out = vrOffset[id]+vrDirection[id]*lambda;                                                                           \n"
    "   out.w=1;                                                                                                                    \n"
    "   outV[id]=out;                                                                                                               \n"
    "   if(depth[id]==2047)                                                                                                         \n"
    "   {                                                                                                                           \n"
    "     outC[id]=((float4)(0,0,0,0));                                                                                             \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     outC[id]=color;                                                                                                           \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "calculateRGBColor(__global float const * depth, __global float4 const * vrOffset, __global float4 const * vrDirection, __global float const * norms, __global float4 * outV, __global float4 * outC, __global uchar const * colorR, __global uchar const * colorG, __global uchar const * colorB, float const scale, __global float const * H, int const w, int const h)                                                                                                                                  \n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float d = 1.046 * (depth[id]==2047 ? 0 : 1000. / (depth[id] * -0.0030711016 + 3.3309495161));                               \n"
    "   float lambda = d*norms[id]*scale;                                                                                           \n"
    "   float4 out = vrOffset[id]+vrDirection[id]*lambda;                                                                           \n"
    "   out.w=1;                                                                                                                    \n"
    "   outV[id]=out;                                                                                                               \n"
    "   if(depth[id]==2047)                                                                                                         \n"
    "   {                                                                                                                           \n"
    "     outC[id]=((float4)(0,0,0,0));                                                                                             \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     int y = (int)floor((float)id/(float)w);                                                                                   \n"
    "     int x = id-y*w;                                                                                                           \n"
    "     float az = H[6]*x + H[7] * y + H[8];                                                                                      \n"
    "     int ax = (int)round(( H[0]*(float)x + H[1] * (float)y + H[2] ) / az);                                                     \n"
    "     int ay = (int)round(( H[3]*(float)x + H[4] * (float)y + H[5] ) / az);                                                     \n"     
    "     if(ax>=0 && ax<w && ay>=0 && ay<h)                                                                                        \n"
    "     {                                                                                                                         \n"
		"       outC[id] = ((float4)(colorR[ax+ay*w]/255., colorG[ax+ay*w]/255., colorB[ax+ay*w]/255.,1.));                             \n"
    "     }                                                                                                                         \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "calculatePseudoColor(__global float const * depth, __global float4 const * vrOffset, __global float4 const * vrDirection, __global float const * norms, __global float4 * outV, __global float4 * outC, __global uchar const * colorR, __global uchar const * colorG, __global uchar const * colorB, float const scale, int const w, int const h)\n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float d = 1.046 * (depth[id]==2047 ? 0 : 1000. / (depth[id] * -0.0030711016 + 3.3309495161));                               \n"
    "   float lambda = d*norms[id]*scale;                                                                                           \n"
    "   float4 out = vrOffset[id]+vrDirection[id]*lambda;                                                                           \n"
    "   out.w=1;                                                                                                                    \n"
    "   outV[id]=out;                                                                                                               \n"
    "   if(depth[id]==2047)                                                                                                         \n"
    "   {                                                                                                                           \n"
    "     outC[id]=((float4)(0,0,0,0));                                                                                             \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     int y = (int)floor((float)id/(float)w);                                                                                   \n"
    "     int x = id-y*w;                                                                                                           \n"
		"     outC[id] = ((float4)(colorR[x+y*w]/255., colorG[x+y*w]/255., colorB[x+y*w]/255.,1.));                                     \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "calculateNormalDirectionColorCam(__global float const * depth, __global float4 const * vrOffset, __global float4 const * vrDirection, __global float const * norms, __global float4 * outV, __global float4 * outC, __global float4 * normals, float const scale)                                                               \n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float d = 1.046 * (depth[id]==2047 ? 0 : 1000. / (depth[id] * -0.0030711016 + 3.3309495161));                               \n"
    "   float lambda = d*norms[id]*scale;                                                                                           \n"
    "   float4 out = vrOffset[id]+vrDirection[id]*lambda;                                                                           \n"
    "   out.w=1;                                                                                                                    \n"
    "   outV[id]=out;                                                                                                               \n"
    "   if(depth[id]==2047)                                                                                                         \n"
    "   {                                                                                                                           \n"
    "     outC[id]=((float4)(0,0,0,0));                                                                                             \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     float4 n = normals[id];                                                                                                   \n"
    "     float cx=fabs(n.x);                                                                                                       \n"
    "     float cy=fabs(n.y);                                                                                                       \n"
    "     float cz=fabs(n.z);                                                                                                       \n"
		"     outC[id] = ((float4)(cx, cy, cz,1.));                                                                                     \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "calculateNormalDirectionColorWorld(__global float const * depth, __global float4 const * vrOffset, __global float4 const * vrDirection, __global float const * norms, __global float4 * outV, __global float4 * outC, __global float4 * normals, float const scale, __global float const * cam, __global float4 * outNormals)     \n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float d = 1.046 * (depth[id]==2047 ? 0 : 1000. / (depth[id] * -0.0030711016 + 3.3309495161));                               \n"
    "   float lambda = d*norms[id]*scale;                                                                                           \n"
    "   float4 out = vrOffset[id]+vrDirection[id]*lambda;                                                                           \n"
    "   out.w=1;                                                                                                                    \n"
    "   outV[id]=out;                                                                                                               \n"
    "   if(depth[id]==2047)                                                                                                         \n"
    "   {                                                                                                                           \n"
    "     outC[id]=((float4)(0,0,0,0));                                                                                             \n"
    "   }                                                                                                                           \n"
    "   else                                                                                                                        \n"
    "   {                                                                                                                           \n"
    "     float4 pWN;                                                                                                               \n"
    "     float4 n= normals[id];                                                                                                    \n"
    "     pWN.x=-(n.x*cam[0]+n.y*cam[1]+n.z*cam[2]);                                                                                \n"
    "     pWN.y=-(n.x*cam[4]+n.y*cam[5]+n.z*cam[6]);                                                                                \n"
    "     pWN.z=-(n.x*cam[8]+n.y*cam[9]+n.z*cam[10]);                                                                               \n"
    "     pWN.w=1;                                                                                                                  \n"
    "     float cx=fabs(pWN.x);                                                                                                     \n"
    "     float cy=fabs(pWN.y);                                                                                                     \n"
    "     float cz=fabs(pWN.z);                                                                                                     \n"
		"     outC[id] = ((float4)(cx, cy, cz,1.));                                                                                     \n"
		"     outNormals[id]=pWN;                                                                                                       \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    "__kernel void                                                                                                                  \n"
    "calculatePseudoColorDepthImage(__global float const * depth, __global uchar * R, __global uchar * G, __global uchar * B)       \n"
    "{                                                                                                                              \n"
    "   size_t id =  get_global_id(0);                                                                                              \n"
    "   float v = depth[id]/2048.0;                                                                                                 \n"
	  "   v = pown(v,3)*6;                                                                                                            \n"
	  "   int pv = v*6*256;                                                                                                           \n"
    "   int lb = pv & 0xff;                                                                                                         \n"
	  "   switch (pv>>8) {                                                                                                            \n"
    "      case 0:                                                                                                                  \n"
	  "        R[id] = 255;                                                                                                           \n"
	  "        G[id] = 255-lb;                                                                                                        \n"
	  "        B[id] = 255-lb;                                                                                                        \n"
	  "        break;                                                                                                                 \n"
    "      case 1:                                                                                                                  \n"
	  "        R[id] = 255;                                                                                                           \n"
	  "        G[id] = lb;		                                                                                                        \n"
	  "        B[id] = 0;                                                                                                             \n"
	  "        break;                                                                                                                 \n"
    "      case 2:                                                                                                                  \n"
	  "        R[id] = 255-lb;                                                                                                        \n"
	  "        G[id] = 255;                                                                                                           \n"
	  "        B[id] = 0;                                                                                                             \n"
	  "        break;                                                                                                                 \n"
    "      case 3:                                                                                                                  \n"
	  "        R[id] = 0;                                                                                                             \n"
	  "        G[id] = 255;                                                                                                           \n"
	  "        B[id] = lb;                                                                                                            \n"
	  "        break;                                                                                                                 \n"
    "      case 4:                                                                                                                  \n"
	  "        R[id] = 0;                                                                                                             \n"
	  "        G[id] = 255-lb;                                                                                                        \n"
	  "        B[id] = 255;                                                                                                           \n"
	  "        break;                                                                                                                 \n"
    "      case 5:                                                                                                                  \n"
	  "        R[id] = 0;                                                                                                             \n"
	  "        G[id] = 0;                                                                                                             \n"
	  "        B[id] = 255-lb;                                                                                                        \n"
	  "        break;                                                                                                                 \n"
    "      default:                                                                                                                 \n"
	  "        R[id] = 0;                                                                                                             \n"
	  "        G[id] = 0;                                                                                                             \n"
	  "        B[id] = 0;                                                                                                             \n"
	  "        break;                                                                                                                 \n"
    "   }                                                                                                                           \n"
    "}                                                                                                                              \n"
    ;

      
  PointcloudSceneObject::PointcloudSceneObject(Size size, const Camera &cam){
    H.id();
    w=size.width;
    h=size.height;
    dim=w*h;
    s=size;
    
    setLockingEnabled(true);
  
    viewrayOffsets = new PointNormalEstimation::Vec4[w*h];
    viewrayDirections = new PointNormalEstimation::Vec4[w*h];
  
    highlightedIdx = -1;
    create_view_rays(w,h,cam);
        
    ViewRay v00 = cam.getViewRay(Point32f(w/2,h/2));
    int i=0;
    norms.resize(w*h);
    for(int y=0;y<h;++y){
      for(int x=0;x<w;++x,++i){
        addVertex(Vec(0,0,0,1),geom_blue());
        norms[i] = 1.0/getNormFactor(rays[i],v00);
        viewrayOffsets[i].x=rays[i].offset[0];
        viewrayOffsets[i].y=rays[i].offset[1];
        viewrayOffsets[i].z=rays[i].offset[2];
        viewrayOffsets[i].w=rays[i].offset[3];
        viewrayDirections[i].x=rays[i].direction[0];
        viewrayDirections[i].y=rays[i].direction[1];
        viewrayDirections[i].z=rays[i].direction[2];
        viewrayDirections[i].w=rays[i].direction[3];
      }
    }
    inverseCamCSMatrix = cam.getCSTransformationMatrix().inv();
    setVisible(Primitive::vertex,true);
    setPointSize(3);
    setLineWidth(4);
    
    depthScaling=1;
    useCL=true;
    normalLinesSet=false;
    useNormalLines=false;
    
    normalLinesLength=20.0;
    normalLinesGranularity=4;
    
    pWNormals=new PointNormalEstimation::Vec4[w*h];
    for(int i=0; i<w*h; i++){
      pWNormals[i].x=0;
      pWNormals[i].y=0;
      pWNormals[i].z=0;
      pWNormals[i].w=0;
    }
    
    #ifdef HAVE_OPENCL
    //create openCL context
    rawImageArray = new float[w*h];
    outputVertices=new cl_float4[w*h];
    outputColors=new cl_float4[w*h];
    colorImageRArray=new cl_uchar[w*h];
    colorImageGArray=new cl_uchar[w*h];
    colorImageBArray=new cl_uchar[w*h];
    pseudoImageRArray=new cl_uchar[w*h];
    pseudoImageGArray=new cl_uchar[w*h];
    pseudoImageBArray=new cl_uchar[w*h];
    
    //init
    for(int i=0; i<w*h; i++){
      rawImageArray[i]=0;
      outputVertices[i].x=0;
      outputVertices[i].y=0;
      outputVertices[i].z=0;
      outputVertices[i].w=0;
      outputColors[i].x=0;
      outputColors[i].y=0;
      outputColors[i].z=0;
      outputColors[i].w=0;
      colorImageRArray[i]=0;
      colorImageGArray[i]=0;
      colorImageBArray[i]=0;
      pseudoImageRArray[i]=0;
      pseudoImageGArray[i]=0;
      pseudoImageBArray[i]=0;
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
              
        cl::Program::Sources sources(1, std::make_pair(pointcloudViewerKernel, 0)); //kernel source
        program=cl::Program(context, sources); //program (bind context and source)
        program.build(devices);//build program
        
        //create buffer for memory access and allocation
        vrOffsetBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_float4), 
				         (void *) &viewrayOffsets[0]);
				
				vrDirectionBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_float4), 
				         (void *) &viewrayDirections[0]);
				
				normsBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &norms[0]);            
				
				pseudoRBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageRArray[0]); 
				
				pseudoGBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageGArray[0]);
				
				pseudoBBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageBArray[0]);
				         			
				verticesBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_float4), 
				         (void *) &m_vertices[0]); 
	
				vertColorsBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_float4), 
				         (void *) &m_vertexColors[0]);
				         
				outNormalsBuffer = cl::Buffer(
                  context, 
                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                  w*h * sizeof(cl_float4),
                  (void *) &pWNormals[0]);
                              
        //create kernels    
        kernelUnicolor=cl::Kernel(program, "calculateUniColor");
        kernelRGBcolor=cl::Kernel(program, "calculateRGBColor");
        kernelPseudocolor=cl::Kernel(program, "calculatePseudoColor");
        kernelPseudodepth=cl::Kernel(program, "calculatePseudoColorDepthImage");
        kernelNormalcolorCam=cl::Kernel(program, "calculateNormalDirectionColorCam");
        kernelNormalcolorWorld=cl::Kernel(program, "calculateNormalDirectionColorWorld");
        
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


  PointcloudSceneObject::~PointcloudSceneObject(){
    #ifdef HAVE_OPENCL
    delete[] rawImageArray;
    delete[] outputVertices;
    delete[] outputColors;
    delete[] colorImageRArray;
    delete[] colorImageGArray;
    delete[] colorImageBArray;
    delete[] pseudoImageRArray;
    delete[] pseudoImageGArray;
    delete[] pseudoImageBArray;
    #endif
    
    delete[] pWNormals;
  }


  Img8u PointcloudSceneObject::calculatePseudocolorDepthImage(const Img32f &depthImg, bool vSync){
    Img8u hsvImage(s,formatRGB);
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array
        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
				          
        kernelPseudodepth.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelPseudodepth.setArg(1, pseudoRBuffer);
        kernelPseudodepth.setArg(2, pseudoGBuffer);
        kernelPseudodepth.setArg(3, pseudoBBuffer);
              
        queue.enqueueNDRangeKernel(//run kernel
				   kernelPseudodepth, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);
        
        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
       
        queue.enqueueReadBuffer(//read output from kernel
            pseudoRBuffer,
            vS, // block 
            0,
            w*h * sizeof(cl_uchar),
            (cl_uchar*) pseudoImageRArray);
            
        queue.enqueueReadBuffer(//read output from kernel
            pseudoGBuffer,
            vS, // block 
            0,
            w*h * sizeof(cl_uchar),
            (cl_uchar*) pseudoImageGArray);
            
        queue.enqueueReadBuffer(//read output from kernel
            pseudoBBuffer,
            vS, // block 
            0,
            w*h * sizeof(cl_uchar),
            (cl_uchar*) pseudoImageBArray);
            
        std::vector<icl8u*> data(3);
        data[0] = pseudoImageRArray; 
        data[1] = pseudoImageGArray;
        data[2] = pseudoImageBArray; 
        hsvImage = Img8u(Size(w,h),3,data,false);
          	        
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      for(int x=0;x<hsvImage.getWidth();++x){
		    for(int y=0;y<hsvImage.getHeight();++y){      
       	  float v = depthImg(x,y,0)/2048.0;
      		v = powf(v,3)*6; 
     			int pv = v*6*256;
      	  int lb = pv & 0xff;
     			switch (pv>>8) {
				    case 0:
					    hsvImage(x,y,0) = 255;
					    hsvImage(x,y,1) = 255-lb;
					    hsvImage(x,y,2) = 255-lb;
					    break;
				    case 1:
					    hsvImage(x,y,0) = 255;
					    hsvImage(x,y,1) = lb;		
					    hsvImage(x,y,2) = 0;
					    break;
				    case 2:
					    hsvImage(x,y,0) = 255-lb;
					    hsvImage(x,y,1) = 255;
					    hsvImage(x,y,2) = 0;
					    break;
				    case 3:
					    hsvImage(x,y,0) = 0;
					    hsvImage(x,y,1) = 255;
					    hsvImage(x,y,2) = lb;
					    break;
				    case 4:
					    hsvImage(x,y,0) = 0;
					    hsvImage(x,y,1) = 255-lb;
					    hsvImage(x,y,2) = 255;
					    break;
				    case 5:
					    hsvImage(x,y,0) = 0;
					    hsvImage(x,y,1) = 0;
					    hsvImage(x,y,2) = 255-lb;
					    break;
				    default:
					    hsvImage(x,y,0) = 0;
					    hsvImage(x,y,1) = 0;
					    hsvImage(x,y,2) = 0;
					    break;
			    } 
		    }
		  }
	  }
	  return hsvImage;
  } 
 
  
  void PointcloudSceneObject::calculateUniColor(const Img32f &depthImg, GeomColor color, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array
        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
				          
				cl_float4 vcolor;
				vcolor.x=color[0]/255.;
				vcolor.y=color[1]/255.;
				vcolor.z=color[2]/255.;
				vcolor.w=1.;
        kernelUnicolor.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelUnicolor.setArg(1, vrOffsetBuffer);
        kernelUnicolor.setArg(2, vrDirectionBuffer);
        kernelUnicolor.setArg(3, normsBuffer);
        kernelUnicolor.setArg(4, verticesBuffer);
        kernelUnicolor.setArg(5, vertColorsBuffer);
        kernelUnicolor.setArg(6, vcolor);
        kernelUnicolor.setArg(7, depthScaling);
              
        queue.enqueueNDRangeKernel(//run kernel
				   kernelUnicolor, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);
        
        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
       
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));
	      
	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));
	    	      	        
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      Mutex::Locker l(mutex);
      const Channel32f d = depthImg[0];
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{
	          m_vertexColors[i] = color/255.;			
	        }	
        }
      }
    }
  }
  
  
  void PointcloudSceneObject::calculateRGBColor(const Img32f &depthImg, const Img8u &colorImg, FixedMatrix<float,3,3> homogeneity, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array
        colorImageRArray=(cl_uchar*)colorImg.begin(0);//image to char array
        colorImageGArray=(cl_uchar*)colorImg.begin(1);//image to char array
        colorImageBArray=(cl_uchar*)colorImg.begin(2);//image to char array

        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
  			colorRBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageRArray[0]);
			  colorGBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageGArray[0]);
        colorBBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageBArray[0]);
				homogeneityBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         9 * sizeof(float), 
				         (void *) &homogeneity[0]);

        kernelRGBcolor.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelRGBcolor.setArg(1, vrOffsetBuffer);
        kernelRGBcolor.setArg(2, vrDirectionBuffer);
        kernelRGBcolor.setArg(3, normsBuffer);
        kernelRGBcolor.setArg(4, verticesBuffer);
        kernelRGBcolor.setArg(5, vertColorsBuffer);
        kernelRGBcolor.setArg(6, colorRBuffer);
        kernelRGBcolor.setArg(7, colorGBuffer);
        kernelRGBcolor.setArg(8, colorBBuffer);
        kernelRGBcolor.setArg(9, depthScaling);
        kernelRGBcolor.setArg(10, homogeneityBuffer);
        kernelRGBcolor.setArg(11, w);
        kernelRGBcolor.setArg(12, h);
              
        queue.enqueueNDRangeKernel(//run kernel
				   kernelRGBcolor, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);

        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
         
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));

	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));
				      
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      H=homogeneity;
      Mutex::Locker l(mutex);
      const Channel32f d = depthImg[0];
      const Channel8u c[3] = { colorImg[0], colorImg[1], colorImg[2] };
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{
            float az = H(0,2)*x + H(1,2) * y + H(2,2);
	          int ax = round(( H(0,0)*x + H(1,0) * y + H(2,0) ) / az);
	          int ay = round(( H(0,1)*x + H(1,1) * y + H(2,1) ) / az);
	          if(r.contains(ax,ay)){
		          m_vertexColors[i] = GeomColor(c[0](ax,ay)/255., c[1](ax,ay)/255., c[2](ax,ay)/255.,1); 					
	          }
		      }	
        }
      }
    }
  }
  
  
  void PointcloudSceneObject::calculatePseudoColor(const Img32f &depthImg, Img8u &pseudoImg, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array
        colorImageRArray=(cl_uchar*)pseudoImg.begin(0);//image to char array
        colorImageGArray=(cl_uchar*)pseudoImg.begin(1);//image to char array
        colorImageBArray=(cl_uchar*)pseudoImg.begin(2);//image to char array

        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
    		colorRBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageRArray[0]);
			  colorGBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageGArray[0]);
        colorBBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &colorImageBArray[0]);

        kernelPseudocolor.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelPseudocolor.setArg(1, vrOffsetBuffer);
        kernelPseudocolor.setArg(2, vrDirectionBuffer);
        kernelPseudocolor.setArg(3, normsBuffer);
        kernelPseudocolor.setArg(4, verticesBuffer);
        kernelPseudocolor.setArg(5, vertColorsBuffer);
        kernelPseudocolor.setArg(6, colorRBuffer);
        kernelPseudocolor.setArg(7, colorGBuffer);
        kernelPseudocolor.setArg(8, colorBBuffer);
        kernelPseudocolor.setArg(9, depthScaling);
        kernelPseudocolor.setArg(10, w);
        kernelPseudocolor.setArg(11, h);
              
        queue.enqueueNDRangeKernel(//run kernel
				   kernelPseudocolor, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);

        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
         
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));

	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));
				      
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      Mutex::Locker l(mutex);
      const Channel32f d = depthImg[0];
      const Channel8u c[3] = { pseudoImg[0], pseudoImg[1], pseudoImg[2] };
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{
	          m_vertexColors[i] = GeomColor(c[0](x,y)/255., c[1](x,y)/255., c[2](x,y)/255.,1);			
	        }	
        }
      }
    }
  }
  
  
  void PointcloudSceneObject::calculatePseudoColor(const Img32f &depthImg, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array
        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
				pseudoRBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageRArray[0]); 
				pseudoGBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageGArray[0]);
				pseudoBBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(cl_uchar), 
				         (void *) &pseudoImageBArray[0]);
				          
        kernelPseudodepth.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelPseudodepth.setArg(1, pseudoRBuffer);
        kernelPseudodepth.setArg(2, pseudoGBuffer);
        kernelPseudodepth.setArg(3, pseudoBBuffer);
              
        queue.enqueueNDRangeKernel(//run kernel 1
				   kernelPseudodepth, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);
        
        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
       
        kernelPseudocolor.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelPseudocolor.setArg(1, vrOffsetBuffer);
        kernelPseudocolor.setArg(2, vrDirectionBuffer);
        kernelPseudocolor.setArg(3, normsBuffer);
        kernelPseudocolor.setArg(4, verticesBuffer);
        kernelPseudocolor.setArg(5, vertColorsBuffer);
        kernelPseudocolor.setArg(6, pseudoRBuffer);
        kernelPseudocolor.setArg(7, pseudoGBuffer);
        kernelPseudocolor.setArg(8, pseudoBBuffer);
        kernelPseudocolor.setArg(9, depthScaling);
        kernelPseudocolor.setArg(10, w);
        kernelPseudocolor.setArg(11, h);
              
        queue.enqueueNDRangeKernel(//run kernel 2
				   kernelPseudocolor, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);
 
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));

	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));
		      
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      Mutex::Locker l(mutex);
      Img8u pseudoImg=calculatePseudocolorDepthImage(depthImg,vSync);
      const Channel32f d = depthImg[0];
      const Channel8u c[3] = { pseudoImg[0], pseudoImg[1], pseudoImg[2] };
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{
	          m_vertexColors[i] = GeomColor(c[0](x,y)/255., c[1](x,y)/255., c[2](x,y)/255.,1);			
	        }	
        }
      }
    }
  }
  
  
  void PointcloudSceneObject::calculateNormalDirectionColor(const Img32f &depthImg, PointNormalEstimation::Vec4* pNormals, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array

        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
				normalsBuffer = cl::Buffer(
                    context, 
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                    w*h * sizeof(cl_float4),
                    (void *) &pNormals[0]);

        kernelNormalcolorCam.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelNormalcolorCam.setArg(1, vrOffsetBuffer);
        kernelNormalcolorCam.setArg(2, vrDirectionBuffer);
        kernelNormalcolorCam.setArg(3, normsBuffer);
        kernelNormalcolorCam.setArg(4, verticesBuffer);
        kernelNormalcolorCam.setArg(5, vertColorsBuffer);
        kernelNormalcolorCam.setArg(6, normalsBuffer);
        kernelNormalcolorCam.setArg(7, depthScaling);
              
        queue.enqueueNDRangeKernel(//run kernel
				   kernelNormalcolorCam, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);

        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
         
        if(useNormalLines==true){//No vSync if normal lines are shown 
          vS=CL_TRUE;
        }
        
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));

	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));
	      
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      Mutex::Locker l(mutex);
      const Channel32f d = depthImg[0];
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{		
            float cx=fabs(pNormals[i].x);
            float cy=fabs(pNormals[i].y);
            float cz=fabs(pNormals[i].z);
	          m_vertexColors[i] = GeomColor(cx, cy, cz,1.);			
	        }	
        }
      } 
    }
    if(useNormalLines==true){
      drawNormalLines(pNormals);
    }
  } 
  
  
  void PointcloudSceneObject::calculateNormalDirectionColor(const Img32f &depthImg, PointNormalEstimation::Vec4* pNormals, Camera cam, bool vSync){
    if(normalLinesSet==true){
      clearNormalLines();
    }
    Mat T = cam.getCSTransformationMatrix();
    FixedMatrix<float,3,3> R = T.part<0,0,3,3>();
    Mat T2 = R.transp().resize<4,4>(0);
    T2(3,3) =1;
    
    if(useCL==true && clReady==true){
  #ifdef HAVE_OPENCL
      try{  
        rawImageArray=(float*)depthImg.begin(0);//image to float array

        rawImageBuffer = cl::Buffer(//write new image to buffer
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(float), 
				         (void *) &rawImageArray[0]); 
				normalsBuffer = cl::Buffer(
                    context, 
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                    w*h * sizeof(cl_float4),
                    (void *) &pNormals[0]);
                    
        camBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				         16 * sizeof(float), 
				         (void *) &T2[0]);
				         
        kernelNormalcolorWorld.setArg(0, rawImageBuffer);//set parameter for kernel
        kernelNormalcolorWorld.setArg(1, vrOffsetBuffer);
        kernelNormalcolorWorld.setArg(2, vrDirectionBuffer);
        kernelNormalcolorWorld.setArg(3, normsBuffer);
        kernelNormalcolorWorld.setArg(4, verticesBuffer);
        kernelNormalcolorWorld.setArg(5, vertColorsBuffer);
        kernelNormalcolorWorld.setArg(6, normalsBuffer);
        kernelNormalcolorWorld.setArg(7, depthScaling);
        kernelNormalcolorWorld.setArg(8, camBuffer);
        kernelNormalcolorWorld.setArg(9, outNormalsBuffer);
        
        queue.enqueueNDRangeKernel(//run kernel
				   kernelNormalcolorWorld, 
				   cl::NullRange, 
				   cl::NDRange(w*h), //input size for get global id
				   cl::NullRange);

        cl_bool vS=CL_TRUE; //default: blocking (use after complete read)
        if(vSync==true){
          vS=CL_FALSE; //if vSync is activated: no block (read if needed, faster but incomplete update)
        }
        
        if(useNormalLines==true){//No vSync if normal lines are shown 
          vS=CL_TRUE;
        }
        
        queue.enqueueReadBuffer(//read output from kernel
				  verticesBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertices.data()));

	      queue.enqueueReadBuffer(//read output from kernel
				  vertColorsBuffer,
				  vS,
				  0,
				  w*h * sizeof(cl_float4),
				  reinterpret_cast<cl_float4*>(m_vertexColors.data()));

      queue.enqueueReadBuffer(//read output from kernel
                    outNormalsBuffer,
                    CL_TRUE, // block 
                    0,
                    w*h * sizeof(cl_float4),
                    (cl_float4*) pWNormals);	      
				      
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
      }
  #endif
    }
    else{
      Mutex::Locker l(mutex);
      const Channel32f d = depthImg[0];
      const Rect r(0,0,w,h);
	    for(int y=0;y<h;++y){
   		  for(int x=0;x<w;++x){
          int i = x+w*y;

          float ds = depth_to_distance_mm(d[i]) * norms[i] * depthScaling;
          m_vertices[i] = rays[i](ds); 

          if(d[i] == 2047){
            m_vertexColors[i] = GeomColor(0,0,0,0);
          }else{
            Vec pWN = T2*(Vec&)pNormals[i];
            pWNormals[i].x=-pWN[0];
            pWNormals[i].y=-pWN[1];
            pWNormals[i].z=-pWN[2];
            pWNormals[i].w=1.;
            
            float cx=fabs(pWN[0]);
            float cy=fabs(pWN[1]);
            float cz=fabs(pWN[2]);
	          m_vertexColors[i] = GeomColor(cx, cy, cz,1.);			
	        }	
        }
      }
    }
    if(useNormalLines==true){
      drawNormalLines(pWNormals);
    }
  } 


  void PointcloudSceneObject::setDepthScaling(float scaling){
    depthScaling=scaling;
  }
  
  
  float PointcloudSceneObject::depth_to_distance_mm(int d){
    return 1.046 * (d==2047 ? 0 : 1000. / (d * -0.0030711016 + 3.3309495161));
    /*  what is this?
        const float k1 = 1.1863;
        const float k2 = 2842.5;
        const float k3 = 0.1236;
        return k3 * tanf(d/k2 + k1);
    */
    
  }
  
  
  float PointcloudSceneObject::sprod(const Vec &a,const Vec &b){
    return a[0]*b[0] +  a[1]*b[1] +  a[2]*b[2];
  }
  
  
  float PointcloudSceneObject::getNormFactor(const ViewRay &a, const ViewRay &b){
    return sprod(a.direction,b.direction)/(norm3(a.direction)*norm3(b.direction));
  }
  
  
  void PointcloudSceneObject::create_view_rays(int w, int h, const Camera &cam){
    rays.resize(w*h);
    Mat T = cam.getCSTransformationMatrix();
    Mat P = cam.getProjectionMatrix();
    Mat M = P*T;
    
    FixedMatrix<icl32f,4,3> Q;
    Q.row(0) = M.row(0); Q.row(1) = M.row(1); Q.row(2) = M.row(3);
    FixedMatrix<icl32f,3,4> Qpinv = Q.pinv();
    const Vec pos = cam.getPosition();
    int i=0;
    if((pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]) < 1e-20){
      for(int y=0;y<h;++y){
        for(int x=0;x<w;++x,++i){
          ViewRay &v = rays[i];
          v.offset = pos;
          v.direction = Qpinv*FixedColVector<icl32f,3>(x, y, 1);
          v.direction[3] = 0; 
          v.direction.normalize(); 
          v.direction *= -1;
          v.direction[3] = 1;
        }
      }
    }else{    
      for(int y=0;y<h;++y){
        for(int x=0;x<w;++x,++i){
          ViewRay &v = rays[i];
          v.offset = pos;
          v.direction = Qpinv*FixedColVector<icl32f,3>(x, y, 1);
          v.direction = pos - homogenize(v.direction);
          v.direction[3] = 0; 
          v.direction.normalize(); 
          v.direction *= -1;
          v.direction[3] = 1;
        }
      }
    }
  }
  
  
  void PointcloudSceneObject::setUseCL(bool use){
    useCL=use;
  }
  
  
  bool PointcloudSceneObject::isCLReady(){
    return clReady;
  }
  
  
	bool PointcloudSceneObject::isCLActive(){
	  return useCL;
	}
	
	
	void PointcloudSceneObject::setUseDrawNormalLines(bool use){
	  useNormalLines=use;
	}
	
	
	void PointcloudSceneObject::drawNormalLines(PointNormalEstimation::Vec4* pNormals){
	  for(int y=0; y<h; y+=normalLinesGranularity){
	    for(int x=0; x<w; x+=normalLinesGranularity){
	      int i=x+w*y;
	      if(m_vertexColors.at(i)[3]!=0){ 
	        Vec point(m_vertices.at(i)[0]+normalLinesLength*pNormals[i].x, m_vertices.at(i)[1]+normalLinesLength*pNormals[i].y, m_vertices.at(i)[2]+normalLinesLength*pNormals[i].z, 1);
	        addVertex(point, geom_white());
	        addLine(m_vertices.size()-1, i, geom_white());
	      }
	    }
	  }
	  normalLinesSet=true;
	}
	
	
	void PointcloudSceneObject::clearNormalLines(){
	  m_vertices.resize(w*h);
	  m_vertexColors.resize(w*h);
		clearAllPrimitives();
	  normalLinesSet=false;
	}
	
	
	void PointcloudSceneObject::setNormalLinesLength(float length){
	  normalLinesLength=length;
	}
	
	
	void PointcloudSceneObject::setNormalLinesGranularity(int granularity){
	  normalLinesGranularity=granularity;
	}
	
	
	PointNormalEstimation::Vec4* PointcloudSceneObject::getWorldNormals(){
	  return pWNormals;
	}

}

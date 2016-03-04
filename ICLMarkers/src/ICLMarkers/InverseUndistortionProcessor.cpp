/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/                             **
**          InverseUndistortionProcessor.cpp                       **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/InverseUndistortionProcessor.h>

#ifdef ICL_HAVE_OPENCL
#define USE_OPENCL
#include <ICLUtils/CLProgram.h>
#endif


namespace icl{
  using namespace utils;
  
  struct InverseUndistortionProcessor::Data{
#ifdef USE_OPENCL
    CLProgram program;          // main class
    CLBuffer input,output,k;   // buffers for image input/output and the 3x3-convolution mask
    CLKernel kernel;
#endif
    bool preferOpenCL;
    std::vector<Point32f> outBuf;
    
    typedef Point32f float2;
    
    inline float abs_diff(float a, float b){
      return fabs(a-b);
    }
    
    inline float2 undistort(float2 pd, float *k){             
      const float rx = (pd.x - k[5])/k[5], ry = (pd.y - k[6])/k[6];
      const float r2 = rx*rx + ry*ry;
      const float cr = 1.0f + k[0]*r2 + k[1]*r2*r2 + k[4]*r2*r2*r2;   
      const float dx = 2*k[2]*rx*ry + k[3] *(r2 + 2*rx*rx);           
      const float dy = 2*k[3]*rx*ry + k[2] *(r2 + 2*ry*ry);           
      const float rxu = rx * cr + dx;                                 
      const float ryu = ry * cr + dy;                                 
      return float2(rxu*k[5] + k[5],  ryu*k[6] + k[6]);               
    }                                                                 
    
    inline float2 abs_diff_comp(float2 a, float2 b){                  
      return float2(abs_diff(a.x,b.x), abs_diff(a.y,b.y));            
    }                                                                 
    
    float2 undistort_inverse_point(float2 s, float *k){       
      float2 p=s;                                                     
      const float lambda = 0.5;                                       
      const float h = 0.01;                                           
      const float use_lambda = lambda/h;                              
      const float t = 0.05; // desired pixel distance!                
      int nn = 0; // step counter                                     
      const float2 hh(h,h);                                           
      while(1){                                                       
        float2 fx = abs_diff_comp(undistort(p,k), s);                 
        if (fx.x < t && fx.y < t) break;                              
        float2 fxh = abs_diff_comp(undistort(p+hh,k), s);             
        float2 grad = (fxh - fx) * use_lambda;                        
        grad.x = grad.x * fx.x;  // maybe element-wise product?       
        grad.y = grad.y * fx.y;  // is this maybe a*b                 
        p = p - grad;                                                 
        if(++nn > 100) break;                                          
      }                                                               
      return p;                                                       
    }                                                                 
    
    void undistort_inverse(const float2 *in,        
                           float2 *out,             
                           const float *k,          
                           int id){
      float kLocal[7]= {0};
      for(int i=0;i<7;++i) kLocal[i] = k[i];                        
      out[id] = undistort_inverse_point(in[id], kLocal);        
    }                                                                 
    
    Data(bool preferOpenCL):preferOpenCL(preferOpenCL){
#ifdef USE_OPENCL
      if(preferOpenCL){
        static const char *k = ("inline float2 undistort(float2 pd, __local float *k){             \n"
                                "  const float rx = (pd.x - k[5])/k[5], ry = (pd.y - k[6])/k[6];   \n"
                                "  const float r2 = rx*rx + ry*ry;                                 \n" 
                                "  const float cr = 1.0f + k[0]*r2 + k[1]*r2*r2 + k[4]*r2*r2*r2;   \n"
                                "  const float dx = 2*k[2]*rx*ry + k[3] *(r2 + 2*rx*rx);           \n"
                                "  const float dy = 2*k[3]*rx*ry + k[2] *(r2 + 2*ry*ry);           \n"
                                "  const float rxu = rx * cr + dx;                                 \n"
                                "  const float ryu = ry * cr + dy;                                 \n"
                                "  return (float2)(rxu*k[5] + k[5],  ryu*k[6] + k[6]);             \n"
                                "}                                                                 \n"
                                "                                                                  \n"
                                "inline float2 abs_diff_comp(float2 a, float2 b){                  \n"
                                "  float dx = a.x - b.x;                                           \n"
                                "  float dy = a.y - b.y;                                           \n"
                                "  return (float2)(fabs(dx), fabs(dy));                            \n"
                                "}                                                                 \n"
                                "                                                                  \n"
                                "float2 undistort_inverse_point(float2 s, __local float *k){       \n"
                                "  float2 p=s;                                                     \n"
                                "  const float lambda = 0.5;                                       \n"
                                "  const float h = 0.01;                                           \n"
                                "  const float use_lambda = lambda/h;                              \n"
                                "  const float t = 0.05; // desired pixel distance!                \n"
                                "  int nn = 0; // step counter                                     \n"
                                "  const float2 hh = (float2)(h,h);                                \n"
                                "  while(1){                                                       \n"
                                "    float2 fx = abs_diff_comp(undistort(p,k), s);                 \n"
                                "    if (fx.x < t && fx.y < t) break;                              \n"
                                "    float2 fxh = abs_diff_comp(undistort(p+hh,k), s);             \n"
                                "    float2 grad = (fxh - fx) * use_lambda;                        \n"
                                "    grad.x = grad.x * fx.x;  // maybe element-wise product?       \n"
                                "    grad.y = grad.y * fx.y;  // is this maybe a*b                 \n"
                                "    p = p - grad;                                                 \n"
                                "    if(++nn > 100) break;                                         \n"
                                "  }                                                               \n"
                                "  return p;                                                       \n"
                                "}                                                                 \n"
                                "                                                                  \n"
                                "__kernel void undistort_inverse(const __global float2 *in,        \n"
                                "                                __global float2 *out,             \n"
                                "                                const __global float *k,          \n"
                                "                                __local float *kLocal){           \n"
                                "    for(int i=0;i<7;++i) kLocal[i] = k[i];                        \n"
                                "    const int id = get_global_id(0);                              \n"
                                "    out[id] = undistort_inverse_point(in[id], kLocal);            \n"
                                "}                                                                 \n");
        program = CLProgram("gpu",k);                      
        program.listSelectedDevice();
        
        this->k = program.createBuffer("r",7*sizeof(float));
        kernel = program.createKernel("undistort_inverse");
      }
#endif
    }
  };
  
  InverseUndistortionProcessor::InverseUndistortionProcessor(bool preferOpenCL){
    m_data = new Data(preferOpenCL);
  }
  InverseUndistortionProcessor::~InverseUndistortionProcessor(){
    delete m_data;
  }
 
  void InverseUndistortionProcessor::setPreferOpenCL(bool preferOpenCL){
    m_data->preferOpenCL = preferOpenCL;
  }
  
  const std::vector<Point32f> &InverseUndistortionProcessor::run(const std::vector<Point32f> &p,
                                                                 const float kf[7]){
    m_data->outBuf.resize(p.size());
    bool done = false;
#ifdef USE_OPENCL
    if(m_data->preferOpenCL){
      int needed = p.size()*2*sizeof(float);   
      
      m_data->input = m_data->program.createBuffer("r",needed);
      m_data->output = m_data->program.createBuffer("w",needed);      

      m_data->input.write(p.data(), needed);
      m_data->k.write(kf,7*sizeof(float));
      CLKernel::LocalMemory l(7*sizeof(float));
      m_data->kernel.setArgs(m_data->input, m_data->output, m_data->k, l);
      m_data->kernel.apply(p.size(), 1, 0); 
      m_data->output.read(m_data->outBuf.data(), needed);
      done = true;
    }
#endif
    if(!done){
      //#pragma openmp parallel for
      for(size_t i=0;i<p.size();++i){
        m_data->undistort_inverse(p.data(), m_data->outBuf.data(), kf, i);
      }
    }
    return m_data->outBuf;
  }

}

  

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CLProgram.h                      **
** Module : ICLUtils                                               **
** Authors: Viktor Losing                                          **
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

#pragma once

#ifdef HAVE_OPENCL

#include <string.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#include <ICLUtils/CLImage2D.h>
#include <fstream>
#include <ICLUtils/CLException.h>

namespace icl {
  namespace utils {
    
    /// Main class for OpenCL based accelleration
    /** The CLProgram is the based class for ICL's OpenCL support
        framework. A Program instance can be used to create all other
        neccessary OpenCL support types. In contrast to OpenCL's C++ framework,
        ICL's framework is even settled on a higher level providing easier access
        to the relavant functionality.
        
        \section NOM Nomenclature
        
        A <b>CLProgram</b> is -- as presented above -- the main class
        for OpenCL acellerated implementations. It allows for selected
        a particular device, i.e. "cpu" or "gpu" devices and it
        automatically compiles the given OpenCL source code (either
        passed as string or as an input stream)
        
        A <b>CLBuffer</b> is a memory segment located in the
        graphics-card's memory. Buffers are associated with a certain
        CLProgram and they can only be created by that
        program. Buffers are either read-only, write-only or
        read-write. This access mode always refers to how the buffer
        can be accessed by the OpenCL source code, i.e. a "read-only"
        buffer becomes a "const" data pointer in the corresponding
        OpenCL kernel interface. And a "write only" buffer cannot be
        written but not read by the OpenCL source code (?).
        
        TODO: what are the differences ? Why can i use "r" and "w" without
              producing error messages? (seems to be an OpenCL bug/feature)
        
        A <b>CLKernel</b> is a callable OpenCL function. CLKernel instances
        are also created by a program (see \ref EX) and each kernel
        refers to a single function in the OpenCL source code that is 
        declared as "__kernel".\n
        Before a kernel is called, its arguments are given by using the
        overloaded CLKernel::setArgs method.
        
        \section EX Example (Image Convolution)
        
        #include <ICLUtils/CLProgram.h>
        #include <ICLQt/Common.h>
        
        
        \code
        struct Conv{
          CLProgram program;          // main class
          CLBuffer input,output,mask; // buffers for image input/output and the 3x3-convolution mask
          CLKernel kernel;            // the OpenCL function
          Size size;                  // ensure correct buffer sizes
          Conv(const float m[9], const Size &s):size(s){
            // source code
            static const char *k = ("__kernel void convolve(constant float *m,                                \n"
                                    "                        const __global unsigned char *in,                \n"
                                    "                        __global unsigned char *out){                    \n"
                                    "    const int w = get_global_size(0);                                    \n"
                                    "    const int h = get_global_size(1);                                    \n"
                                    "    const int x = get_global_id(0);                                      \n"
                                    "    const int y = get_global_id(1);                                      \n"
                                    "    if(x && y && x<w-1 && y<h-1){                                        \n"
                                    "      const int idx = x+w*y;                                             \n"
                                    "      out[idx] =  m[0]*in[idx-1-w] + m[1]*in[idx-w] + m[2]*in[idx-w+1]   \n"
                                    "                + m[3]*in[idx-1]   + m[4]*in[idx]   + m[5]*in[idx+1]     \n"
                                    "                + m[6]*in[idx-1+w] + m[7]*in[idx+w] + m[8]*in[idx+w+1];  \n"
                                    "    }                                                                    \n"
                                    "}                                                                        \n");

            program = CLProgram("gpu",k);                          // create program running on CPU-device
            program.listSelectedDevice();                          // show device seledted
            const int dim = s.getDim();                            // get image dimension
            input = program.createBuffer("w",dim);                 // create input image buffer
            output = program.createBuffer("r",dim);                // create output image buffer
            mask = program.createBuffer("w",9*sizeof(float),m);    // create buffer for the 3x3 conv. mask
            kernel = program.createKernel("convolve");             // create the OpenCL kernel
          }
          
          void apply(const Img8u &src, Img8u &dst){
            ICLASSERT_THROW(src.getSize() == size && dst.getSize() == size, ICLException("wrong size"));
            input.write(src.begin(0),src.getDim());           // write input image to graphics memory
            kernel.setArgs(mask, input, output);              // set kernel arguments
            kernel.apply(src.getWidth(), src.getHeight(), 0); // apply the kernel (using WxH threads max.
                                                              // i.e. one per pixel)
            output.read(dst.begin(0),dst.getDim());           // read output buffer to destination image
          }
        } *conv = 0;


        GUI gui;
        GenericGrabber grabber;
        
        void init(){
          grabber.init(pa("-i"));
        
          gui << Image().handle("image") << Show();
          const ImgBase &image = *grabber.grab();
          const float mask[] = { 0.25, 0, -0.25,
                                 0.50, 0, -0.50,
                                 0.25, 0, -0.25 };
          grabber.useDesired(image.getSize());
          grabber.useDesired(depth8u);
          grabber.useDesired(formatGray);
        
          conv = new Conv(mask,image.getSize());
        }

        void run(){
          const Img8u &image = *grabber.grab()->as8u();
          static Img8u res(image.getParams());
        
          conv->apply(image,res);
          gui["image"] = res;
        }

        int main(int n, char **args){
          return ICLApp(n,args,"-input|-i(2)",init,run).exec();
        }

        \endcode
        **/
    class CLProgram {
      struct Impl;
      Impl *impl;
      
      public:

      /// Default constructor (creates dummy instance)
      CLProgram();
      
      /// create CLProgram with given device type (either "gpu" or "cpu") and souce-code
      CLProgram(const string deviceType, const string &sourceCode) throw(CLInitException, CLBuildException);

      /// create CLProgram with given device type (either "gpu" or "cpu") and souce-code file
      CLProgram(const string deviceType, ifstream &fileStream) throw(CLInitException, CLBuildException);

      /// copy constructor (creating shallow copy)
      CLProgram(const CLProgram& other);
      
      /// assignment operator (perorming shallow copy)
      CLProgram const& operator=(CLProgram const& other);
      
      /// Destructor
      ~CLProgram();
      
      
      //accessMode = "r" only read
      //accessMode = "w" only write
      //accessMode = "rw" read and write
      /// creates a buffer object for memory exchange with graphics card memory
      /** acessMode can either be "r", "w" or "rw", which refers to the readibility of the data
          by the OpenCL source code (actually this seems to be not relevant since all buffers
          can be read and written).\n
          Each buffer has a fixed size (given in bytes). Optionally an initial source
          pointer can be passed that is then automatically uploaded to the buffer exisiting
          in the graphics memory.*/
      CLBuffer createBuffer(const string &accessMode, size_t size,const void *src=0) throw(CLBufferException);
      

      /// creates a image2D object for memory exchange with graphics card memory
      /** acessMode can either be "r", "w" or "rw", which refers to the readibility of the data
          by the OpenCL source code (actually this seems to be not relevant since all images
          can be read and written).\n
          Optionally an initial source pointer can be passed that is then automatically uploaded to the image exisiting
          in the graphics memory.

          various image depths can be used \n
            depth8u  = 0, < 8Bit unsigned integer values range {0,1,...255} \n
            depth16s = 1, < 16Bit signed integer values \n
            depth32s = 2, < 32Bit signed integer values \n
            depth32f = 3, < 32Bit floating point values \n
            depth64f = 4, < 64Bit floating point values \n
          */
      CLImage2D createImage2D(const string &accessMode, const size_t width, const size_t height, int depth, const void *src=0) throw(CLBufferException);

      /// extract a kernel from the program
      /** Kernels in the CLProgram's source code have to be qualified with the
          __kernel qualifier. The kernel (aka function) name in the OpenCL source code
          is used as id. */
      CLKernel createKernel(const string &id) throw (CLKernelException);
      
      /// lists various properties of the selected platform
      void listSelectedPlatform();
      
      /// lists various properties of the selected device
      void listSelectedDevice();
      /// lists various properties of all platforms and their devices

      static void listAllPlatformsAndDevices();

      /// lists various properties of all platforms
      static void listAllPlatforms();
    };
  }
}
#endif

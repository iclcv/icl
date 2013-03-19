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
#include <ICLQt/Quick.h>

#include <CL/cl.h>
#include "cstdio"
#include <unistd.h>
#include <sys/time.h>


#define DESC_SIZE 64
      
#define MAX_ERR_VAL 64

#define NUM_PROGRAMS 7

#define NUM_KERNELS 13
#define KERNEL_INIT_DET 0 
#define KERNEL_BUILD_DET 1 
#define KERNEL_SURF_DESC 2
#define KERNEL_NORM_DESC 3
#define KERNEL_NON_MAX_SUP 4
#define KERNEL_GET_ORIENT1 5
#define KERNEL_GET_ORIENT2 6
#define KERNEL_NN 7
#define KERNEL_SCAN 8
#define KERNEL_SCAN4 9
#define KERNEL_TRANSPOSE 10
#define KERNEL_SCANIMAGE 11
#define KERNEL_TRANSPOSEIMAGE 12

// Uncomment the following define to use optimized data transfers
// when possible.  Note that AMD's use of memory mapping is 
// different than NVIDIA, so it will crash on NVIDIA's devices

//#define OPTIMIZED_TRANSFERS

namespace icl{
  namespace cv{
    namespace clsurf{

      typedef struct{
        int x;
        int y;
      } int2;
      
      typedef struct{
        float x;
        float y;
      }float2;
      
      typedef struct{
        float x;
        float y;
        float z;
        float w;
      }float4;

      class ResponseLayer;
      class FastHessian;
      
      struct Surf::Data{
        IpVec m_outputBuffer;
        
        icl::core::Img32f m_grayBuffer;

        // The actual number of ipoints for this image
        int numIpts; 

        //! The amount of ipoints we have allocated space for
        int maxIpts;

        //! A fast hessian object that will be used for detecting ipoints
        FastHessian* fh;

        //! The integral image
        cl_mem d_intImage;
        cl_mem d_tmpIntImage;   // orig orientation
        cl_mem d_tmpIntImageT1; // transposed
        cl_mem d_tmpIntImageT2; // transposed

        //! Number of surf descriptors
        cl_mem d_length;

        //! List of precompiled kernels
        cl_kernel* kernel_list;

        //! Array of Descriptors for each Ipoint
        cl_mem d_desc;

        //! Orientation of each Ipoint an array of float
        cl_mem d_orientation;
    
        cl_mem d_gauss25;
    
        cl_mem d_id;	

        cl_mem d_i;

        cl_mem d_j;

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
        cl_mem d_pixPos;

        //! Scale buffer on the device
        cl_mem d_scale;

        //! Laplacian buffer on the device
        cl_mem d_laplacian;

        //! Res buffer on the device
        cl_mem d_res;

#ifdef OPTIMIZED_TRANSFERS
        // If we are using pinned memory, we need additional
        // buffers on the host

        //! Position buffer on the host
        cl_mem h_pixPos;

        //! Scale buffer on the host
        cl_mem h_scale;

        //! Laplacian buffer on the host
        cl_mem h_laplacian;

        //! Descriptor buffer on the host
        cl_mem h_desc;

        //! Orientation buffer on the host
        cl_mem h_orientation;
#endif

        const static int j[16];
    
        const static int i[16];

        const static unsigned int id[13];

        const static float gauss25[49];
      };

      class EventList
      {
        typedef std::pair<cl_event, char*> event_tuple;
        
        typedef std::pair<double, char*> time_tuple;
        
      public:
        
        EventList();
        
        ~EventList();
        
        void dumpCSV(char* path);
        
        void dumpTraceCSV(char* path);
        
        void newCompileEvent(double time, char* desc);
        
        void newIOEvent(cl_event event, char* desc);
        
        void newKernelEvent(cl_event event, char* desc);
        
        void newUserEvent(double time, char* desc);
        
        void printAllEvents();
        
        void printCompileEvents();
        
        void printIOEvents();
        
        void printKernelEvents();
        
        void printUserEvents();
        
        void printAllExecTimes();
        
        void printCompileExecTimes();
        
        void printIOExecTimes();
        
        void printKernelExecTimes();
        
        void printUserExecTimes();
        
      private:
        
        char* createFilenameWithTimestamp();
        
        //! A list of events for program compilation
        std::vector<time_tuple> compile_events;
        std::vector<time_tuple>::iterator compile_events_iterator;
        
        //! A list of Events that are provided to external functions
        std::vector<event_tuple> kernel_events;
        std::vector<event_tuple>::iterator kernel_events_iterator;
        
        //! A list of events for IO Actions
        std::vector<event_tuple> io_events;
        std::vector<event_tuple>::iterator io_events_iterator;
        
        //! A list of user provided events
        std::vector<time_tuple> user_events;
        std::vector<time_tuple>::iterator user_events_iterator;
      };
      
      
      static bool usingImages = true;
      
      typedef double cl_time;      

      class ResponseLayer {

      public:
    
        ResponseLayer(int width, int height, int step, int filter);

        ~ResponseLayer();

        int getWidth();

        int getHeight();

        int getStep();
    
        int getFilter();

        cl_mem getResponses(); 

        cl_mem getLaplacian();


      private:
    
        int width;

        int height;  

        int step;

        int filter;

        cl_mem d_responses;

        cl_mem d_laplacian;
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
        FastHessian(int i_height, 
                    int i_width,
                    const int octaves = OCTAVES, 
                    const int intervals = INTERVALS, 
                    const int sample_step = SAMPLE_STEP, 
                    const float thres = THRES,
                    cl_kernel* kernel_list = NULL);
        
        // TODO Fix this name
        void selectIpoints(cl_mem d_laplacian, cl_mem d_pixPos, cl_mem d_scale,
                           cl_kernel* kernel_list, int maxPoints);
        
        // TODO Fix this name
        void computeHessianDet(cl_mem d_intImage, int i_width, int i_height, 
                               cl_kernel* kernel_list);
        
        //! Find the image features and write into vector of features
        int getIpoints(const icl::core::Img32f &image, cl_mem d_intImage, cl_mem d_laplacian,
                       cl_mem d_pixPos, cl_mem d_scale, int maxIpts);
        
        //! Resets the information required for the next frame to compute
        void reset();
        
      private:

        void createResponseMap(int octaves, int imgWidth, int 
                               imgHeight, int sample_step);

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

        cl_kernel* kernel_list;

        std::vector<ResponseLayer*> responseMap;

        //! Number of Ipoints on GPU 
        cl_mem d_ipt_count;
      };


      // Rounds up size to the nearest multiple of multiple
      unsigned int roundUp(unsigned int value, unsigned int multiple){
        unsigned int remainder = value % multiple;
        // Make the value a multiple of multiple
        if(remainder != 0) {
          value += (multiple-remainder);
        }
        return value;
      }
      
      //////////////////////////////////////////////////////////////////////////////
      // utils functions ///////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////

      // Set the value of using images to true if they are being
      // used, or false if they are not
      void setUsingImages(bool val) 
      {
        usingImages = val;
      }
      
      
      // Return whether or not images are being used
      bool isUsingImages() 
      {
        return usingImages;
      }

      // Wrapper for malloc
      void* alloc(size_t size){
        void* ptr = NULL;
        ptr = malloc(size);
        if(ptr == NULL) {
          perror("malloc");
          exit(-1);
        }
        return ptr;
      }

      char* smartStrcat(char* str1, char* str2) 
      {
        char* newStr = NULL;
        
        newStr = (char*)alloc((strlen(str1)+strlen(str2)+1)*sizeof(char));
        
        strcpy(newStr, str1);
        strcat(newStr, str2);
        
        return newStr;
      }
      
      //-------------------------------------------------------
      // Initialization and Cleanup
      //-------------------------------------------------------

      // Detects platforms and devices, creates context and command queue
      cl_context cl_init(char devicePreference='\0');

      // Releases resources used by clutils
      void    cl_cleanup();

      // Releases a kernel object
      void    cl_freeKernel(cl_kernel kernel);

      // Releases a memory object
      void    cl_freeMem(cl_mem mem);

      // Releases a program object
      void    cl_freeProgram(cl_program program);


      //-------------------------------------------------------
      // Synchronization functions
      //-------------------------------------------------------

      // Performs a clFinish on the command queue
      void    cl_sync();


      //-------------------------------------------------------
      // Memory allocation
      //-------------------------------------------------------

      // Allocates a regular buffer on the device
      cl_mem  cl_allocBuffer(size_t mem_size, 
                             cl_mem_flags flags = CL_MEM_READ_WRITE);

      // XXX I don't think this does exactly what we want it to do
      // Allocates a read-only buffer and transfers the data
      cl_mem  cl_allocBufferConst(size_t mem_size, void* host_ptr);

      // Allocates pinned memory on the host
      cl_mem  cl_allocBufferPinned(size_t mem_size);

      // Allocates an image on the device
      cl_mem  cl_allocImage(size_t height, size_t width, char type, 
                            cl_mem_flags flags = CL_MEM_READ_WRITE);



      //-------------------------------------------------------
      // Data transfers
      //-------------------------------------------------------

      // Copies a buffer from the device to pinned memory on the host and 
      // maps it so it can be read
      void*   cl_copyAndMapBuffer(cl_mem dst, cl_mem src, size_t size); 

      // Copies from one buffer to another
      void    cl_copyBufferToBuffer(cl_mem dst, cl_mem src, size_t size);

      // Copies data to a buffer on the device
      void    cl_copyBufferToDevice(cl_mem dst, void *src, size_t mem_size, 
                                    cl_bool blocking = CL_TRUE);

      // Copies data to an image on the device
      void cl_copyImageToDevice(cl_mem dst, void* src, size_t height, size_t width);

      // Copies an image from the device to the host
      void    cl_copyImageToHost(void* dst, cl_mem src, size_t height, size_t width);

      // Copies data from a device buffer to the host
      void    cl_copyBufferToHost(void *dst, cl_mem src, size_t mem_size, 
                                  cl_bool blocking = CL_TRUE);

      // Copies data from a buffer on the device to an image on the device
      void    cl_copyBufferToImage(cl_mem src, cl_mem dst, int height, int width);

      // Maps a buffer
      void*   cl_mapBuffer(cl_mem mem, size_t mem_size, cl_mem_flags flags);

      // Unmaps a buffer
      void    cl_unmapBuffer(cl_mem mem, void *ptr);

      // Writes data to a zero-copy buffer on the device
      void    cl_writeToZCBuffer(cl_mem mem, void* data, size_t size);

      //-------------------------------------------------------
      // Program and kernels
      //-------------------------------------------------------

      // Compiles a program
      cl_program  cl_compileProgram(char* kernelPath, char* compileoptions, 
                                    bool verboseoptions = 0);

      // Creates a kernel
      cl_kernel   cl_createKernel(cl_program program, const char* kernelName);

      // Executes a kernel 
      void        cl_executeKernel(cl_kernel kernel, cl_uint work_dim, const size_t* 
                                   global_work_size, const size_t* local_work_size, 
                                   const char* description, int identifier = 0);

      // Precompiles the kernels for SURF
      cl_kernel*  cl_precompileKernels(char* buildOptions);

      // Sets a kernel argument
      void        cl_setKernelArg(cl_kernel kernel, unsigned int index, size_t size, 
                                  void* data);


      //-------------------------------------------------------
      // Profiling/events
      //-------------------------------------------------------

      // Computes the execution time (start to end) for an event
      double  cl_computeExecTime(cl_event);

      // Compute the elapsed time between two CPU timer values
      double  cl_computeTime(cl_time start, cl_time end); 

      // Creates an event from CPU timers
      void    cl_createUserEvent(cl_time start, cl_time end, char* desc);

      // Disable logging of events
      void    cl_disableEvents();

      // Enable logging of events
      void    cl_enableEvents();

      // Query the current system time
      void    cl_getTime(cl_time* time); 

      // Calls a function which prints events to the terminal
      void    cl_printEvents();

      // Calls a function which writes the events to a file
      void    cl_writeEventsToFile(char* path);


      //-------------------------------------------------------
      // Error handling
      //-------------------------------------------------------

      // Compare a status value to CL_SUCCESS and optionally exit on error
      int     cl_errChk(const cl_int status, const char *msg, bool exitOnErr);

      // Queries the supported image formats for the device and prints
      // them to the screen
      void    printSupportedImageFormats();

      //-------------------------------------------------------
      // Platform and device information
      //-------------------------------------------------------

      bool    cl_deviceIsAMD(cl_device_id dev=NULL);
      bool    cl_deviceIsNVIDIA(cl_device_id dev=NULL);
      bool    cl_platformIsNVIDIA(cl_platform_id plat=NULL);
      char*   cl_getDeviceDriverVersion(cl_device_id dev=NULL);
      char*   cl_getDeviceName(cl_device_id dev=NULL);
      char*   cl_getDeviceVendor(cl_device_id dev=NULL);
      char*   cl_getDeviceVersion(cl_device_id dev=NULL);
      char*   cl_getPlatformName(cl_platform_id platform);
      char*   cl_getPlatformVendor(cl_platform_id platform);

      //-------------------------------------------------------
      // Utility functions
      //-------------------------------------------------------

      char* catStringWithInt(const char* str, int integer);

      char* itoa_portable(int value, char* result, int base);


      
      //////////////////////////////////////////////////////////////////////////////
      // cl_utils functions ////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////

      // The following variables have file scope to simplify
      // the utility functions

      //! All discoverable OpenCL platforms
      static cl_platform_id* platforms = NULL;
      static cl_uint numPlatforms;

      //! All discoverable OpenCL devices (one pointer per platform)
      static cl_device_id** devices = NULL;
      static cl_uint* numDevices;

      //! The chosen OpenCL platform
      static cl_platform_id platform = NULL;

      //! The chosen OpenCL device
      static cl_device_id device = NULL;

      //! OpenCL context
      static cl_context context = NULL;

      //! OpenCL command queue
      static cl_command_queue commandQueue = NULL;
      static cl_command_queue commandQueueProf = NULL;
      static cl_command_queue commandQueueNoProf = NULL;

      //! List of precompiled kernels
      static cl_kernel kernel_list[NUM_KERNELS];

      //! List of program objects
      static cl_program program_list[NUM_PROGRAMS];

      //! Globally visible event table
      static EventList* events = NULL;

      //! Global status of events
      static bool eventsEnabled = false;


      //-------------------------------------------------------
      //          Initialization and Cleanup
      //-------------------------------------------------------

      /*!

          \brief Initialize OpenCl environment on one device

          Init function for one device. Looks for supported devices and creates a context
          \return returns a context initialized
          */
      cl_context cl_init(char devicePreference)
      {
        cl_int status;

        // Allocate the event table
        events = new EventList();

        // Discover and populate the platforms
        status = clGetPlatformIDs(0, NULL, &numPlatforms);
        cl_errChk(status, "Getting platform IDs", true);
        if (numPlatforms > 0)
          {
            // Get all the platforms
            platforms = (cl_platform_id*)alloc(numPlatforms *
                                               sizeof(cl_platform_id));

            status = clGetPlatformIDs(numPlatforms, platforms, NULL);
            cl_errChk(status, "Getting platform IDs", true);
          }
        else
          {
            // If no platforms are available, we shouldn't continue
            printf("No OpenCL platforms found\n");
            exit(-1);
          }

        // Allocate space for the device lists and lengths
        numDevices = (cl_uint*)alloc(sizeof(cl_uint)*numPlatforms);
        devices = (cl_device_id**)alloc(sizeof(cl_device_id*)*numPlatforms);

        // If a device preference was supplied, we'll limit the search of devices
        // based on type
        cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
        if(devicePreference == 'c') {
          deviceType = CL_DEVICE_TYPE_CPU;
        }
        if(devicePreference == 'g') {
          deviceType = CL_DEVICE_TYPE_GPU;
        }

        // Traverse the platforms array printing information and
        // populating devices
        for(unsigned int i = 0; i < numPlatforms ; i++)
          {
            // Print out some basic info about the platform
            char* platformName = NULL;
            char* platformVendor = NULL;

            platformName = cl_getPlatformName(platforms[i]);
            platformVendor = cl_getPlatformVendor(platforms[i]);

            status = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices[i]);
            cl_errChk(status, "Getting device IDs", false);
            if(status != CL_SUCCESS) {
              printf("This is a known NVIDIA bug (if platform == AMD then die)\n");
              printf("Setting number of devices to 0 and continuing\n");
              numDevices[i] = 0;
            }

            printf("Platform %d (%d devices):\n", i, numDevices[i]);
            printf("\tName: %s\n", platformName);
            printf("\tVendor: %s\n", platformVendor);

            free(platformName);
            free(platformVendor);

            // Populate OpenCL devices if any exist
            if(numDevices[i] != 0)
              {
                // Allocate an array of devices of size "numDevices"
                devices[i] = (cl_device_id*)alloc(sizeof(cl_device_id)*numDevices[i]);

                // Populate Arrray with devices
                status = clGetDeviceIDs(platforms[i], deviceType, numDevices[i],
                                        devices[i], NULL);
                cl_errChk(status, "Getting device IDs", true);
              }

            // Print some information about each device
            for( unsigned int j = 0; j < numDevices[i]; j++)
              {
                char* deviceName = NULL;
                char* deviceVendor = NULL;

                printf("\tDevice %d:\n", j);

                deviceName = cl_getDeviceName(devices[i][j]);
                deviceVendor = cl_getDeviceVendor(devices[i][j]);

                printf("\t\tName: %s\n", deviceName);
                printf("\t\tVendor: %s\n", deviceVendor);

                free(deviceName);
                free(deviceVendor);
              }
          }

        // Hard-code in the platform/device to use, or uncomment 'scanf'
        // to decide at runtime
        cl_uint chosen_platform, chosen_device;
        // UNCOMMENT the following two lines to manually select device each time
        //printf("Enter Platform and Device No (Seperated by Space) \n");
        //scanf("%d %d", &chosen_platform, &chosen_device);
        chosen_platform = 0;
        chosen_device = 0;
        printf("Using Platform %d, Device %d \n", chosen_platform, chosen_device);

        // Do a sanity check of platform/device selection
        if(chosen_platform >= numPlatforms ||
           chosen_device >= numDevices[chosen_platform]) {
          printf("Invalid platform/device combination\n");
          exit(-1);
        }

        // Set the selected platform and device
        platform = platforms[chosen_platform];
        device = devices[chosen_platform][chosen_device];

        // Create the context
        cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)(platform), 0};
        context = clCreateContext(cps, 1, &device, NULL, NULL, &status);
        cl_errChk(status, "Creating context", true);

        // Create the command queue
        commandQueueProf = clCreateCommandQueue(context, device,
                                                CL_QUEUE_PROFILING_ENABLE, &status);
        cl_errChk(status, "creating command queue", true);

        commandQueueNoProf = clCreateCommandQueue(context, device, 0, &status);
        cl_errChk(status, "creating command queue", true);

        if(eventsEnabled) {
          printf("Profiling enabled\n");
          commandQueue = commandQueueProf;
        }
        else {
          printf("Profiling disabled\n");
          commandQueue = commandQueueNoProf;
        }

        return context;
      }

      /*!
          Release all resources that the user doesn't have access to.
          */
      void  cl_cleanup()
      {
        // Free the events (this frees the OpenCL events as well)
        delete events;

        // Free the command queue
        if(commandQueue) {
          clReleaseCommandQueue(commandQueue);
        }

        // Free the context
        if(context) {
          clReleaseContext(context);
        }

        // Free the kernel objects
        for(int i = 0; i < NUM_KERNELS; i++) {
          clReleaseKernel(kernel_list[i]);
        }

        // Free the program objects
        for(int i = 0; i < NUM_PROGRAMS; i++) {
          clReleaseProgram(program_list[i]);
        }

        // Free the devices
        for(int i = 0; i < (int)numPlatforms; i++) {
          free(devices[i]);
        }
        free(devices);
        free(numDevices);

        // Free the platforms
        free(platforms);
      }

      //! Release a kernel object
      /*!
          \param mem The kernel object to release
          */
      void cl_freeKernel(cl_kernel kernel)
      {
        cl_int status;

        if(kernel != NULL) {
          status = clReleaseKernel(kernel);
          cl_errChk(status, "Releasing kernel object", true);
        }
      }

      //! Release memory allocated on the device
      /*!
          \param mem The device pointer to release
          */
      void cl_freeMem(cl_mem mem)
      {
        cl_int status;

        if(mem != NULL) {
          status = clReleaseMemObject(mem);
          cl_errChk(status, "Releasing mem object", true);
        }
      }

      //! Release a program object
      /*!
          \param mem The program object to release
          */
      void cl_freeProgram(cl_program program)
      {
        cl_int status;

        if(program != NULL) {
          status = clReleaseProgram(program);
          cl_errChk(status, "Releasing program object", true);
        }
      }


      //-------------------------------------------------------
      //          Synchronization functions
      //-------------------------------------------------------

      /*!
          Wait till all pending commands in queue are finished
          */
      void cl_sync()
      {
        clFinish(commandQueue);
      }


      //-------------------------------------------------------
      //          Memory allocation
      //-------------------------------------------------------

      //! Allocate a buffer on a device
      /*!
          \param mem_size Size of memory in bytes
          \param flags Optional cl_mem_flags
          \return Returns a cl_mem object that points to device memory
          */
      cl_mem cl_allocBuffer(size_t mem_size, cl_mem_flags flags)
      {
        cl_mem mem;
        cl_int status;

        /*!
            Logging information for keeping track of device memory
            */
        static int allocationCount = 1;
        static size_t allocationSize = 0;

        allocationCount++;
        allocationSize += mem_size;

        mem = clCreateBuffer(context, flags, mem_size, NULL, &status);

        cl_errChk(status, "creating buffer", true);

        return mem;
      }

      //! Allocate constant memory on device
      /*!
          \param mem_size Size of memory in bytes
          \param host_ptr Host pointer that contains the data
          \return Returns a cl_mem object that points to device memory
          */
      cl_mem cl_allocBufferConst(size_t mem_size, void* host_ptr)
      {
        cl_mem mem;
        cl_int status;

        mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             mem_size, host_ptr, &status);
        cl_errChk(status, "Error creating const mem buffer", true);

        return mem;
      }

      //! Allocate a buffer on device pinning the host memory at host_ptr
      /*!
          \param mem_size Size of memory in bytes
          \return Returns a cl_mem object that points to pinned memory on the host
          */
      cl_mem cl_allocBufferPinned(size_t mem_size)
      {
        cl_mem mem;
        cl_int status;

        mem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                             mem_size, NULL, &status);
        cl_errChk(status, "Error allocating pinned memory", true);

        return mem;
      }

      //! Allocate an image on a device
      /*!
          \param height Number of rows in the image
          \param width Number of columns in the image
          \param elemSize Size of the elements in the image
          \param flags Optional cl_mem_flags
          \return Returns a cl_mem object that points to device memory
          */
      cl_mem cl_allocImage(size_t height, size_t width, char type, cl_mem_flags flags)
      {
        cl_mem mem;
        cl_int status;

        size_t elemSize = 0;

        cl_image_format format;
        format.image_channel_order = CL_R;

        switch(type) {
          case 'f':
            elemSize = sizeof(float);
            format.image_channel_data_type = CL_FLOAT;
            break;
          case 'i':
            elemSize = sizeof(int);
            format.image_channel_data_type = CL_SIGNED_INT32;
            break;
          default:
            printf("Error creating image: Unsupported image type.\n");
            exit(-1);
        }

        /*!
            Logging information for keeping track of device memory
            */
        static int allocationCount = 1;
        static size_t allocationSize = 0;

        allocationCount++;
        allocationSize += height*width*elemSize;

        // Create the image
        mem = clCreateImage2D(context, flags, &format, width, height, 0, NULL, &status);

        //cl_errChk(status, "creating image", true);
        if(status != CL_SUCCESS) {
          printf("Error creating image: Images may not be supported for this device.\n");
          printSupportedImageFormats();
          getchar();
          exit(-1);
        }

        return mem;
      }


      //-------------------------------------------------------
      //          Data transfers
      //-------------------------------------------------------


      // Copy and map a buffer
      void* cl_copyAndMapBuffer(cl_mem dst, cl_mem src, size_t size) {

        void* ptr;  // Pointer to the pinned memory that will be returned

        cl_copyBufferToBuffer(dst, src, size);

        ptr = cl_mapBuffer(dst, size, CL_MAP_READ);

        return ptr;
      }

      // Copy a buffer
      void cl_copyBufferToBuffer(cl_mem dst, cl_mem src, size_t size)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        cl_int status;
        status = clEnqueueCopyBuffer(commandQueue, src, dst, 0, 0, size, 0, NULL,
                                     eventPtr);
        cl_errChk(status, "Copying buffer", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyBuffer", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      //! Copy a buffer to the device
      /*!
          \param dst Valid device pointer
          \param src Host pointer that contains the data
          \param mem_size Size of data to copy
          \param blocking Blocking or non-blocking operation
          */
      void cl_copyBufferToDevice(cl_mem dst, void* src, size_t mem_size, cl_bool blocking)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        cl_int status;
        status = clEnqueueWriteBuffer(commandQueue, dst, blocking, 0,
                                      mem_size, src, 0, NULL, eventPtr);
        cl_errChk(status, "Writing buffer", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyBufferToDevice", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      //! Copy a buffer to the host
      /*!
          \param dst Valid host pointer
          \param src Device pointer that contains the data
          \param mem_size Size of data to copy
          \param blocking Blocking or non-blocking operation
          */
      void cl_copyBufferToHost(void* dst, cl_mem src, size_t mem_size, cl_bool blocking)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        cl_int status;
        status = clEnqueueReadBuffer(commandQueue, src, blocking, 0,
                                     mem_size, dst, 0, NULL, eventPtr);
        cl_errChk(status, "Reading buffer", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyBufferToHost", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      //! Copy a buffer to a 2D image
      /*!
          \param src Valid device buffer
          \param dst Empty device image
          \param mem_size Size of data to copy
          */
      void cl_copyBufferToImage(cl_mem buffer, cl_mem image, int height, int width)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        size_t origin[3] = {0, 0, 0};
        size_t region[3] = {(size_t)width, (size_t)height, (size_t)1};

        cl_int status;
        status = clEnqueueCopyBufferToImage(commandQueue, buffer, image, 0,
                                            origin, region, 0, NULL, eventPtr);
        cl_errChk(status, "Copying buffer to image", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyBufferToImage", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      // Copy data to an image on the device
      /*!
          \param dst Valid device pointer
          \param src Host pointer that contains the data
          \param height Height of the image
          \param width Width of the image
          */
      void cl_copyImageToDevice(cl_mem dst, void* src, size_t height, size_t width)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        cl_int status;
        size_t origin[3] = {0, 0, 0};
        size_t region[3] = {width, height, 1};

        status = clEnqueueWriteImage(commandQueue, dst, CL_TRUE, origin,
                                     region, 0, 0, src, 0, NULL, eventPtr);
        cl_errChk(status, "Writing image", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyImageToDevice", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      //! Copy an image to the host
      /*!
          \param dst Valid host pointer
          \param src Device pointer that contains the data
          \param height Height of the image
          \param width Width of the image
          */
      void cl_copyImageToHost(void* dst, cl_mem src, size_t height, size_t width)
      {
        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        cl_int status;
        size_t origin[3] = {0, 0, 0};
        size_t region[3] = {width, height, 1};

        status = clEnqueueReadImage(commandQueue, src, CL_TRUE, origin,
                                    region, 0, 0, dst, 0, NULL, eventPtr);
        cl_errChk(status, "Reading image", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("copyImageToHost", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }
      }

      //! Map a buffer into a host address
      /*!
          \param mem cl_mem object
          \param mem_size Size of memory in bytes
          \param flags Optional cl_mem_flags
          \return Returns a host pointer that points to the mapped region
          */
      void *cl_mapBuffer(cl_mem mem, size_t mem_size, cl_mem_flags flags)
      {
        cl_int status;
        void *ptr;

        static int eventCnt = 0;

        cl_event* eventPtr = NULL, event;

        if(eventsEnabled) {
          eventPtr = &event;
        }

        ptr = (void *)clEnqueueMapBuffer(commandQueue, mem, CL_TRUE, flags,
                                         0, mem_size, 0, NULL, eventPtr, &status);

        cl_errChk(status, "Error mapping a buffer", true);

        if(eventsEnabled) {
          char* eventStr = catStringWithInt("MapBuffer", eventCnt++);
          events->newIOEvent(*eventPtr, eventStr);
        }

        return ptr;
      }

      //! Unmap a buffer or image
      /*!
          \param mem cl_mem object
          \param ptr A host pointer that points to the mapped region
          */
      void cl_unmapBuffer(cl_mem mem, void *ptr)
      {

        // TODO It looks like AMD doesn't support profiling unmapping yet. Leaving the
        //      commented code here until it's supported

        cl_int status;

        status = clEnqueueUnmapMemObject(commandQueue, mem, ptr, 0, NULL, NULL);

        cl_errChk(status, "Error unmapping a buffer or image", true);
      }

      void cl_writeToZCBuffer(cl_mem mem, void* data, size_t size)
      {

        void* ptr;

        ptr = cl_mapBuffer(mem, size, CL_MAP_WRITE);

        memcpy(ptr, data, size);

        cl_unmapBuffer(mem, ptr);
      }

      //-------------------------------------------------------
      //          Program and kernels
      //-------------------------------------------------------

      //! Convert source code file into cl_program
      /*!
          Compile Opencl source file into a cl_program. The cl_program will be made into a kernel in PrecompileKernels()

          \param kernelPath  Filename of OpenCl code
          \param compileoptions Compilation options
          \param verbosebuild Switch to enable verbose Output
          */
      cl_program cl_compileProgram(char* kernelPath, char* compileoptions, bool verbosebuild )
      {
        cl_int status;
        FILE *fp = NULL;
        char *source = NULL;
        long int size;

        printf("\t%s\n", kernelPath);

        // Determine the size of the source file
#ifdef _WIN32
        fopen_s(&fp, kernelPath, "rb");
#else
        fp = fopen(kernelPath, "rb");
#endif
        if(!fp) {
          printf("Could not open kernel file\n");
          exit(-1);
        }
        status = fseek(fp, 0, SEEK_END);
        if(status != 0) {
          printf("Error seeking to end of file\n");
          exit(-1);
        }
        size = ftell(fp);
        if(size < 0) {
          printf("Error getting file position\n");
          exit(-1);
        }
        rewind(fp);

        // Allocate enough space for the source code
        source = (char *)alloc(size + 1);

        // fill with NULLs (just for fun)
        for (int i = 0; i < size+1; i++)  {
          source[i] = '\0';
        }

        // Read in the source code
        int _ = fread(source, 1, size, fp);
        (void)_;
        source[size] = '\0';

        // Create the program object
        cl_program clProgramReturn = clCreateProgramWithSource(context, 1,
                                                               (const char **)&source, NULL, &status);
        cl_errChk(status, "Creating program", true);

        free(source);
        fclose(fp);

        // Try to compile the program
        status = clBuildProgram(clProgramReturn, 0, NULL, compileoptions, NULL, NULL);
        if(cl_errChk(status, "Building program", false) || verbosebuild == 1)
          {

            cl_build_status build_status;

            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_STATUS,
                                  sizeof(cl_build_status), &build_status, NULL);

            if(build_status == CL_SUCCESS && verbosebuild == 0) {
              return clProgramReturn;
            }

            //char *build_log;
            size_t ret_val_size;
            printf("Device: %p",device);
            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_LOG, 0,
                                  NULL, &ret_val_size);

            char *build_log = (char*)alloc(ret_val_size+1);

            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_LOG,
                                  ret_val_size+1, build_log, NULL);

            // to be careful, terminate with \0
            // there's no information in the reference whether the string is 0
            // terminated or not
            build_log[ret_val_size] = '\0';

            printf("Build log:\n %s...\n", build_log);
            if(build_status != CL_SUCCESS) {
              getchar();
              exit(-1);
            }
            else
              return clProgramReturn;
          }

        // print the ptx information
        // printBinaries(clProgram);

        return clProgramReturn;
      }


  cl_program cl_compileProgramFromGivenString(const char* programString, char* compileoptions, bool verbosebuild )
      {
        cl_int status;

        // Create the program object
        cl_program clProgramReturn = clCreateProgramWithSource(context, 1,
                                                               &programString, NULL, &status);
        cl_errChk(status, "Creating program", true);

        // Try to compile the program
        status = clBuildProgram(clProgramReturn, 0, NULL, compileoptions, NULL, NULL);
        if(cl_errChk(status, "Building program", false) || verbosebuild == 1)
          {

            cl_build_status build_status;

            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_STATUS,
                                  sizeof(cl_build_status), &build_status, NULL);

            if(build_status == CL_SUCCESS && verbosebuild == 0) {
              return clProgramReturn;
            }

            //char *build_log;
            size_t ret_val_size;
            printf("Device: %p",device);
            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_LOG, 0,
                                  NULL, &ret_val_size);

            char *build_log = (char*)alloc(ret_val_size+1);

            clGetProgramBuildInfo(clProgramReturn, device, CL_PROGRAM_BUILD_LOG,
                                  ret_val_size+1, build_log, NULL);

            // to be careful, terminate with \0
            // there's no information in the reference whether the string is 0
            // terminated or not
            build_log[ret_val_size] = '\0';

            printf("Build log:\n %s...\n", build_log);
            if(build_status != CL_SUCCESS) {
              getchar();
              exit(-1);
            }
            else
              return clProgramReturn;
          }

        // print the ptx information
        // printBinaries(clProgram);

        return clProgramReturn;
      }


      //! Create a kernel from compiled source
      /*!
          Create a kernel from compiled source

          \param program  Compiled OpenCL program
          \param kernel_name  Name of the kernel in the program
          \return Returns a cl_kernel object for the specified kernel
          */
      cl_kernel cl_createKernel(cl_program program, const char* kernel_name) {

        cl_kernel kernel;
        cl_int status;

        kernel = clCreateKernel(program, kernel_name, &status);
        cl_errChk(status, "Creating kernel", true);

        return kernel;
      }

      //! Enqueue and NDRange kernel on a device
      /*!
          \param kernel The kernel to execute
          \param work_dim  The number of dimensions that define the thread structure
          \param global_work_size  Array of size 'work_dim' that defines the total threads in each dimension
          \param local_work_size  Array of size 'work_dim' that defines the size of each work group
          \param description String describing the kernel
          \param identifier A number unique number identifying the kernel
          */
      int global_event_ctr = 0;

      void cl_executeKernel(cl_kernel kernel, cl_uint work_dim,
                            const size_t* global_work_size, const size_t* local_work_size,
                            const char* description, int identifier)
      {


        cl_int status;

        cl_event* eventPtr = NULL, event;

        //    eventsEnabled =  phasechecker(description, identifier, granularity);

        if(eventsEnabled) {
          eventPtr = &event;
        }

        status = clEnqueueNDRangeKernel(commandQueue, kernel, work_dim, NULL,
                                        global_work_size, local_work_size, 0, NULL, eventPtr);
        cl_errChk(status, "Executing kernel", true);


        if(eventsEnabled) {
          char* eventString = catStringWithInt(description, identifier);
          events->newKernelEvent(*eventPtr, eventString);
        }
      }

      //! SURF specific kernel precompilation call
      /*!
          */
      cl_kernel* cl_precompileKernels(char* buildOptions)
      {
        // Compile each program and create the kernel objects

        printf("Precompiling kernels...\n");

        cl_time totalstart, totalend;
        cl_time start, end;

        cl_getTime(&totalstart);

        // Creating descriptors kernel
        cl_getTime(&start);
        //program_list[1]  = cl_compileProgram("CLSource/createDescriptors_kernel.cl",
        program_list[1]  = cl_compileProgramFromGivenString(createDescriptors_kernel,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"createDescriptors");
        kernel_list[KERNEL_SURF_DESC] = cl_createKernel(program_list[1],
                                                        (char*)"createDescriptors_kernel");

        // Get orientation kernels
        cl_getTime(&start);
        //program_list[4]  = cl_compileProgram("CLSource/getOrientation_kernels.cl",
        program_list[4]  = cl_compileProgramFromGivenString(getOrientation_kernels,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"Orientation");
        kernel_list[KERNEL_GET_ORIENT1] = cl_createKernel(program_list[4],
                                                          (char*)"getOrientationStep1");
        kernel_list[KERNEL_GET_ORIENT2] = cl_createKernel(program_list[4],
                                                          (char*)"getOrientationStep2");

        // Hessian determinant kernel
        cl_getTime(&start);
        //program_list[0]  = cl_compileProgram("CLSource/hessianDet_kernel.cl",
        program_list[0]  = cl_compileProgramFromGivenString(hessianDet_kernel,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"hessian_det");
        kernel_list[KERNEL_BUILD_DET] = cl_createKernel(program_list[0],
                                                        (char*)"hessian_det");

        // Integral image kernels
        cl_getTime(&start);
        program_list[6] = cl_compileProgramFromGivenString(integralImage_kernels,
                                            buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"IntegralImage");
        kernel_list[KERNEL_SCAN] = cl_createKernel(program_list[6], (char*)"scan");
        kernel_list[KERNEL_SCAN4] = cl_createKernel(program_list[6], (char*)"scan4");
        kernel_list[KERNEL_SCANIMAGE] = cl_createKernel(program_list[6],
                                                        (char*)"scanImage");
        kernel_list[KERNEL_TRANSPOSE] = cl_createKernel(program_list[6],
                                                        (char*)"transpose");
        kernel_list[KERNEL_TRANSPOSEIMAGE] = cl_createKernel(program_list[6],
                                                             (char*)"transposeImage");

        // Nearest neighbor kernels
        cl_getTime(&start);
        program_list[5]  = cl_compileProgramFromGivenString(nearestNeighbor_kernel,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"NearestNeighbor");
        kernel_list[KERNEL_NN] = cl_createKernel(program_list[5],
                                                 (char*)"NearestNeighbor");

        // Non-maximum suppression kernel
        cl_getTime(&start);
        program_list[3]  = cl_compileProgramFromGivenString(nonMaxSuppression_kernel,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"NonMaxSuppression");
        kernel_list[KERNEL_NON_MAX_SUP] = cl_createKernel(program_list[3],
                                                          (char*)"non_max_supression_kernel");

        // Normalization of descriptors kernel
        cl_getTime(&start);
        program_list[2]  = cl_compileProgramFromGivenString(normalizeDescriptors_kernel,
                                             buildOptions, false);
        cl_getTime(&end);
        events->newCompileEvent(cl_computeTime(start, end), (char*)"normalize");
        kernel_list[KERNEL_NORM_DESC] = cl_createKernel(program_list[2],
                                                        (char*)"normalizeDescriptors");

        cl_getTime(&totalend);

        printf("\tTime for Off-Critical Path Compilation: %.3f milliseconds\n\n",
               cl_computeTime(totalstart, totalend));

        return kernel_list;
      }

      //! Set an argument for a OpenCL kernel
      /*!
          Set an argument for a OpenCL kernel

          \param kernel The kernel for which the argument is being set
          \param index The argument index
          \param size The size of the argument
          \param data A pointer to the argument
          */
      void cl_setKernelArg(cl_kernel kernel, unsigned int index, size_t size,
                           void* data)
      {
        cl_int status;
        status = clSetKernelArg(kernel, index, size, data);

        cl_errChk(status, "Setting kernel arg", true);
      }


      //-------------------------------------------------------
      //          Profiling/events
      //-------------------------------------------------------


      //! Time kernel execution using cl_event
      /*!
          Prints out the time taken between the start and end of an event
          \param event_time
          */
      double cl_computeExecTime(cl_event event_time)
      {
        cl_int status;
        cl_ulong starttime;
        cl_ulong endtime;

        double elapsed;

        status = clGetEventProfilingInfo(event_time, CL_PROFILING_COMMAND_START,
                                         sizeof(cl_ulong), &starttime, NULL);
        cl_errChk(status, "profiling start", true);

        status = clGetEventProfilingInfo(event_time, CL_PROFILING_COMMAND_END,
                                         sizeof(cl_ulong), &endtime, NULL);
        cl_errChk(status, "profiling end", true);

        // Convert to ms
        elapsed = (double)(endtime-starttime)/1000000.0;

        return elapsed;
      }

      //! Compute the elapsed time between two timer values
      double cl_computeTime(cl_time start, cl_time end)
      {
#ifdef _WIN32
        __int64 freq;
        int status;

        status = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        if(status == 0) {
          perror("QueryPerformanceFrequency");
          exit(-1);
        }

        // Return time in ms
        return double(end-start)/(double(freq)/1000.0);
#else

        return end-start;
#endif
      }

      //! Create a new user event
      void cl_createUserEvent(cl_time start, cl_time end, char* desc) {

        if(!eventsEnabled) {
          return;
        }

        events->newUserEvent(cl_computeTime(start, end), desc);
      }

      //! Disables events
      void cl_disableEvents() {

        commandQueue = commandQueueNoProf;

        eventsEnabled = false;

        printf("Profiling disabled\n");
      }

      //! Enables events
      void cl_enableEvents() {

        commandQueue = commandQueueProf;

        eventsEnabled = true;

        printf("Profiling enabled\n");
      }

      //! Grab the current time using a system-specific timer
      void cl_getTime(cl_time* time)
      {

#ifdef _WIN32
        int status = QueryPerformanceCounter((LARGE_INTEGER*)time);
        if(status == 0) {
          perror("QueryPerformanceCounter");
          exit(-1);
        }
#else
        // Use gettimeofday to get the current time
        struct timeval curTime;
        gettimeofday(&curTime, NULL);

        // Convert timeval into double
        *time = curTime.tv_sec * 1000 + (double)curTime.tv_usec/1000;
#endif
      }

      //! Print out the OpenCL events
      void cl_printEvents() {

        events->printAllExecTimes();
      }

      //! Write out all current events to a file
      void cl_writeEventsToFile(char* path) {

        events->dumpCSV(path);
        //events->dumpTraceCSV(path);

      }


      //-------------------------------------------------------
      //          Error handling
      //-------------------------------------------------------

      //! OpenCl error code list
      /*!
          An array of character strings used to give the error corresponding to the error code \n

          The error code is the index within this array
          */
      const char *cl_errs[MAX_ERR_VAL] = {
        "CL_SUCCESS",                         // 0
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
        "CL_INVALID_GLOBAL_WORK_SIZE"};       //-63

      //! OpenCl Error checker
      /*!
          Checks for error code as per cl_int returned by OpenCl
          \param status Error value as cl_int
          \param msg User provided error message
          \return True if Error Seen, False if no error
          */
      int cl_errChk(const cl_int status, const char * msg, bool exitOnErr)
      {

        if(status != CL_SUCCESS) {
          printf("OpenCL Error: %d %s %s\n", status, cl_errs[-status], msg);

          if(exitOnErr) {
            exit(-1);
          }

          return true;
        }
        return false;
      }

      // Queries the supported image formats for the device and prints
      // them to the screen
      void printSupportedImageFormats()
      {
        cl_uint numFormats;
        cl_int status;

        status = clGetSupportedImageFormats(context, 0, CL_MEM_OBJECT_IMAGE2D,
                                            0, NULL, &numFormats);
        cl_errChk(status, "getting supported image formats", true);

        cl_image_format* imageFormats = NULL;
        imageFormats = (cl_image_format*)alloc(sizeof(cl_image_format)*numFormats);

        status = clGetSupportedImageFormats(context, 0, CL_MEM_OBJECT_IMAGE2D,
                                            numFormats, imageFormats, NULL);

        printf("There are %d supported image formats\n", numFormats);

        cl_uint orders[]={CL_R,  CL_A, CL_INTENSITY, CL_LUMINANCE, CL_RG,
                          CL_RA, CL_RGB, CL_RGBA, CL_ARGB, CL_BGRA};
        const char  *orderstr[]={"CL_R", "CL_A","CL_INTENSITY", "CL_LUMINANCE", "CL_RG",
                           "CL_RA", "CL_RGB", "CL_RGBA", "CL_ARGB", "CL_BGRA"};

        cl_uint types[]={
          CL_SNORM_INT8 , CL_SNORM_INT16, CL_UNORM_INT8, CL_UNORM_INT16,
          CL_UNORM_SHORT_565, CL_UNORM_SHORT_555, CL_UNORM_INT_101010,CL_SIGNED_INT8,
          CL_SIGNED_INT16,  CL_SIGNED_INT32, CL_UNSIGNED_INT8, CL_UNSIGNED_INT16,
          CL_UNSIGNED_INT32, CL_HALF_FLOAT, CL_FLOAT};

        const char * typesstr[]={
          "CL_SNORM_INT8" ,"CL_SNORM_INT16","CL_UNORM_INT8","CL_UNORM_INT16",
          "CL_UNORM_SHORT_565","CL_UNORM_SHORT_555","CL_UNORM_INT_101010",
          "CL_SIGNED_INT8","CL_SIGNED_INT16","CL_SIGNED_INT32","CL_UNSIGNED_INT8",
          "CL_UNSIGNED_INT16","CL_UNSIGNED_INT32","CL_HALF_FLOAT","CL_FLOAT"};

        printf("Supported Formats:\n");
        for(int i = 0; i < (int)numFormats; i++) {
          printf("\tFormat %d: ", i);

          for(int j = 0; j < (int)(sizeof(orders)/sizeof(cl_int)); j++) {
            if(imageFormats[i].image_channel_order == orders[j]) {
              printf("%s, ", orderstr[j]);
            }
          }
          for(int j = 0; j < (int)(sizeof(types)/sizeof(cl_int)); j++) {
            if(imageFormats[i].image_channel_data_type == types[j]) {
              printf("%s, ", typesstr[j]);
            }
          }
          printf("\n");
        }

        free(imageFormats);
      }


      //-------------------------------------------------------
      //          Platform and device information
      //-------------------------------------------------------

      //! Returns true if AMD is the device vendor
      bool cl_deviceIsAMD(cl_device_id dev) {

        bool retval = false;

        char* vendor = cl_getDeviceVendor(dev);

        if(strncmp(vendor, "Advanced", 8) == 0) {
          retval = true;
        }

        free(vendor);

        return retval;
      }

      //! Returns true if NVIDIA is the device vendor
      bool cl_deviceIsNVIDIA(cl_device_id dev) {

        bool retval = false;

        char* vendor = cl_getDeviceVendor(dev);

        if(strncmp(vendor, "NVIDIA", 6) == 0) {
          retval = true;
        }

        free(vendor);

        return retval;
      }

      //! Returns true if NVIDIA is the device vendor
      bool cl_platformIsNVIDIA(cl_platform_id plat) {

        bool retval = false;

        char* vendor = cl_getPlatformVendor(plat);

        if(strncmp(vendor, "NVIDIA", 6) == 0) {
          retval = true;
        }

        free(vendor);

        return retval;
      }

      //! Get the name of the vendor for a device
      char* cl_getDeviceDriverVersion(cl_device_id dev)
      {
        cl_int status;
        size_t devInfoSize;
        char* devInfoStr = NULL;

        // If dev is NULL, set it to the default device
        if(dev == NULL) {
          dev = device;
        }

        // Print the vendor
        status = clGetDeviceInfo(dev, CL_DRIVER_VERSION, 0,
                                 NULL, &devInfoSize);
        cl_errChk(status, "Getting vendor name", true);

        devInfoStr = (char*)alloc(devInfoSize);

        status = clGetDeviceInfo(dev, CL_DRIVER_VERSION, devInfoSize,
                                 devInfoStr, NULL);
        cl_errChk(status, "Getting vendor name", true);

        return devInfoStr;
      }

      //! The the name of the device as supplied by the OpenCL implementation
      char* cl_getDeviceName(cl_device_id dev)
      {
        cl_int status;
        size_t devInfoSize;
        char* devInfoStr = NULL;

        // If dev is NULL, set it to the default device
        if(dev == NULL) {
          dev = device;
        }

        // Print the name
        status = clGetDeviceInfo(dev, CL_DEVICE_NAME, 0,
                                 NULL, &devInfoSize);
        cl_errChk(status, "Getting device name", true);

        devInfoStr = (char*)alloc(devInfoSize);

        status = clGetDeviceInfo(dev, CL_DEVICE_NAME, devInfoSize,
                                 devInfoStr, NULL);
        cl_errChk(status, "Getting device name", true);

        return(devInfoStr);
      }

      //! Get the name of the vendor for a device
      char* cl_getDeviceVendor(cl_device_id dev)
      {
        cl_int status;
        size_t devInfoSize;
        char* devInfoStr = NULL;

        // If dev is NULL, set it to the default device
        if(dev == NULL) {
          dev = device;
        }

        // Print the vendor
        status = clGetDeviceInfo(dev, CL_DEVICE_VENDOR, 0,
                                 NULL, &devInfoSize);
        cl_errChk(status, "Getting vendor name", true);

        devInfoStr = (char*)alloc(devInfoSize);

        status = clGetDeviceInfo(dev, CL_DEVICE_VENDOR, devInfoSize,
                                 devInfoStr, NULL);
        cl_errChk(status, "Getting vendor name", true);

        return devInfoStr;
      }

      //! Get the name of the vendor for a device
      char* cl_getDeviceVersion(cl_device_id dev)
      {
        cl_int status;
        size_t devInfoSize;
        char* devInfoStr = NULL;

        // If dev is NULL, set it to the default device
        if(dev == NULL) {
          dev = device;
        }

        // Print the vendor
        status = clGetDeviceInfo(dev, CL_DEVICE_VERSION, 0,
                                 NULL, &devInfoSize);
        cl_errChk(status, "Getting vendor name", true);

        devInfoStr = (char*)alloc(devInfoSize);

        status = clGetDeviceInfo(dev, CL_DEVICE_VERSION, devInfoSize,
                                 devInfoStr, NULL);
        cl_errChk(status, "Getting vendor name", true);

        return devInfoStr;
      }

      //! The the name of the device as supplied by the OpenCL implementation
      char* cl_getPlatformName(cl_platform_id platform)
      {
        cl_int status;
        size_t platformInfoSize;
        char* platformInfoStr = NULL;

        // Print the name
        status = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0,
                                   NULL, &platformInfoSize);
        cl_errChk(status, "Getting platform name", true);

        platformInfoStr = (char*)alloc(platformInfoSize);

        status = clGetPlatformInfo(platform, CL_PLATFORM_NAME, platformInfoSize,
                                   platformInfoStr, NULL);
        cl_errChk(status, "Getting platform name", true);

        return(platformInfoStr);
      }

      //! The the name of the device as supplied by the OpenCL implementation
      char* cl_getPlatformVendor(cl_platform_id platform)
      {
        cl_int status;
        size_t platformInfoSize;
        char* platformInfoStr = NULL;

        // Print the name
        status = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0,
                                   NULL, &platformInfoSize);
        cl_errChk(status, "Getting platform name", true);

        platformInfoStr = (char*)alloc(platformInfoSize);

        status = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, platformInfoSize,
                                   platformInfoStr, NULL);
        cl_errChk(status, "Getting platform name", true);

        return(platformInfoStr);
      }

      //-------------------------------------------------------
      //          Utility functions
      //-------------------------------------------------------

      //! Take a string and an int, and return a string
      char* catStringWithInt(const char* string, int integer) {

        if(integer > 99999) {
          printf("Can't handle event identifiers with 6 digits\n");
          exit(-1);
        }

        // 5 characters for the identifier, 1 for the null terminator
        int strLen = strlen(string)+5+1;
        char* eventStr = (char*)alloc(sizeof(char)*strLen);

        char tmp[6];

        strcpy(eventStr, string);
        strcat(eventStr, ",");
        strncat(eventStr, itoa_portable(integer, tmp, 10), 5);

        return eventStr;
      }

      /**
          ** C++ version 0.4 char* style "itoa":
          ** Written by Lukás Chmela
          ** Released under GPLv3.
          **/
      //portable itoa function
      char* itoa_portable(int value, char* result, int base) {
        // check that the base if valid
        if (base < 2 || base > 36) { *result = '\0'; return result; }

        char* ptr = result, *ptr1 = result, tmp_char;
        int tmp_value;

        do {
          tmp_value = value;
          value /= base;
          *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        } while ( value );

        //Apply negative sign
        if (tmp_value < 0) *ptr++ = '-';
        *ptr-- = '\0';

        while(ptr1 < ptr) {
          tmp_char = *ptr;
          *ptr--= *ptr1;
          *ptr1++ = tmp_char;
        }

        return result;
      }



      //////////////////////////////////////////////////////////////////////////////
      // ResponseLayer Hessian class implementation ////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////

      ResponseLayer::ResponseLayer(int width, int height, int step, int filter)
      {
        this->width = width;
        this->height = height; 
        this->step = step;
        this->filter = filter;

        if(isUsingImages()) {
          this->d_laplacian = cl_allocImage(height, width, 'i');
          this->d_responses = cl_allocImage(height, width, 'f');
        }
        else {
          this->d_laplacian = cl_allocBuffer(sizeof(int)*width*height);
          this->d_responses = cl_allocBuffer(sizeof(float)*width*height);
        }
      }

      ResponseLayer::~ResponseLayer() 
      {
        cl_freeMem(this->d_responses);
        cl_freeMem(this->d_laplacian);

      }

      int ResponseLayer::getWidth() 
      {

        return this->width;
      }

      int ResponseLayer::getHeight() 
      {

        return this->height;
      }

      int ResponseLayer::getStep() 
      {

        return this->step;
      }

      int ResponseLayer::getFilter() 
      {

        return this->filter;
      }

      cl_mem ResponseLayer::getLaplacian() 
      {

        return this->d_laplacian;
      }

      cl_mem ResponseLayer::getResponses() 
      {

        return this->d_responses;
      }

      //////////////////////////////////////////////////////////////////////////////
      // Fast Hessian class implementation /////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////


      // Based on the octave (row) and interval (column), this lookup table
      // identifies the appropriate determinant layer
      const int filter_map[OCTAVES][INTERVALS] = {{0, 1, 2, 3},
                                                  {1, 3, 4, 5},
                                                  {3, 5, 6, 7},
                                                  {5, 7, 8, 9},
                                                  {7, 9,10,11}};

      //-------------------------------------------------------

      //! Constructor
      FastHessian::FastHessian(int i_height, int i_width, const int octaves,
                               const int intervals, const int sample_step,
                               const float thres, cl_kernel* kernel_list)
        :kernel_list(kernel_list)
      {
        // Initialise variables with bounds-checked values
        this->octaves = (octaves > 0 && octaves <= 4 ? octaves : OCTAVES);
        this->intervals = (intervals > 0 && intervals <= 4 ? intervals : INTERVALS);
        this->sample_step = (sample_step > 0 && sample_step <= 6 ? sample_step : SAMPLE_STEP);
        this->thres = (thres >= 0 ? thres : THRES);

        this->num_ipts = 0;

        // TODO implement this as device zero-copy memory
        this->d_ipt_count = cl_allocBuffer(sizeof(int));
        cl_copyBufferToDevice(this->d_ipt_count, &this->num_ipts, sizeof(int));

        // Create the hessian response map objects
        this->createResponseMap(octaves, i_width, i_height, sample_step);
      }


      //! Destructor
      FastHessian::~FastHessian()
      {
        cl_freeMem(this->d_ipt_count);

        for(unsigned int i = 0; i < this->responseMap.size(); i++) {
          delete responseMap.at(i);
        }
      }

      void FastHessian::createResponseMap(int octaves, int imgWidth, int imgHeight, int sample_step)
      {

        int w = (imgWidth / sample_step);
        int h = (imgHeight / sample_step);
        int s = (sample_step);

        // Calculate approximated determinant of hessian values
        if (octaves >= 1)
          {
            this->responseMap.push_back(new ResponseLayer(w,   h,   s,   9));
            this->responseMap.push_back(new ResponseLayer(w,   h,   s,   15));
            this->responseMap.push_back(new ResponseLayer(w,   h,   s,   21));
            this->responseMap.push_back(new ResponseLayer(w,   h,   s,   27));
          }

        if (octaves >= 2)
          {
            this->responseMap.push_back(new ResponseLayer(w/2, h/2, s*2, 39));
            this->responseMap.push_back(new ResponseLayer(w/2, h/2, s*2, 51));
          }

        if (octaves >= 3)
          {
            this->responseMap.push_back(new ResponseLayer(w/4, h/4, s*4, 75));
            this->responseMap.push_back(new ResponseLayer(w/4, h/4, s*4, 99));
          }

        if (octaves >= 4)
          {
            this->responseMap.push_back(new ResponseLayer(w/8, h/8, s*8, 147));
            this->responseMap.push_back(new ResponseLayer(w/8, h/8, s*8, 195));
          }

        if (octaves >= 5)
          {
            this->responseMap.push_back(new ResponseLayer(w/16, h/16, s*16, 291));
            this->responseMap.push_back(new ResponseLayer(w/16, h/16, s*16, 387));
          }
      }


      //! Hessian determinant for the image using approximated box filters
      /*!
          \param d_intImage Integral Image
          \param surfipt Pointer to pre-allocated temp data structures
          \param i_width Image Width
          \param i_height Image Height
          \param octaves Octaves for SURF
          \param intervals Number of Intervals
          \param kernel_list pointer to precompiled kernels
          */
      void FastHessian::computeHessianDet(cl_mem d_intImage,
                                          int i_width, int i_height,
                                          cl_kernel* kernel_list)
      {
        // set matrix size and x,y threads per block
        const int BLOCK_DIM = 16;

        cl_kernel hessian_det =  kernel_list[KERNEL_BUILD_DET];

        size_t localWorkSize[2] = {BLOCK_DIM,BLOCK_DIM};
        size_t globalWorkSize[2];

        cl_setKernelArg(hessian_det, 0, sizeof(cl_mem), (void *)&d_intImage);
        cl_setKernelArg(hessian_det, 1, sizeof(cl_int), (void *)&i_width);
        cl_setKernelArg(hessian_det, 2, sizeof(cl_int), (void *)&i_height);

        for(unsigned int i = 0; i < this->responseMap.size(); i++) {

          cl_mem responses = this->responseMap.at(i)->getResponses();
          cl_mem laplacian = this->responseMap.at(i)->getLaplacian();
          int step = this->responseMap.at(i)->getStep();
          int filter = this->responseMap.at(i)->getFilter();
          int layerWidth = this->responseMap.at(i)->getWidth();
          int layerHeight = this->responseMap.at(i)->getHeight();

          globalWorkSize[0] = roundUp(layerWidth, localWorkSize[0]);
          globalWorkSize[1] = roundUp(layerHeight, localWorkSize[1]);

          cl_setKernelArg(hessian_det, 3, sizeof(cl_mem), (void*)&responses);
          cl_setKernelArg(hessian_det, 4, sizeof(cl_mem), (void*)&laplacian);
          cl_setKernelArg(hessian_det, 5, sizeof(int),    (void*)&layerWidth);
          cl_setKernelArg(hessian_det, 6, sizeof(int),    (void*)&layerHeight);
          cl_setKernelArg(hessian_det, 7, sizeof(int),    (void*)&step);
          cl_setKernelArg(hessian_det, 8, sizeof(int),    (void*)&filter);

          cl_executeKernel(hessian_det, 2, globalWorkSize, localWorkSize,
                           "BuildHessianDet", i);

          // TODO Verify that a clFinish is not required (setting an argument
          //      to the loop counter without it may be problematic, but it
          //      really kills performance on AMD parts)
          //cl_sync();
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
      int FastHessian::getIpoints(const icl::core::Img32f &image, 
                                  cl_mem d_intImage, cl_mem d_laplacian,
                                  cl_mem d_pixPos, cl_mem d_scale, int maxIpts)
      {

        // Compute the hessian determinants
        // GPU kernels: init_det and build_det kernels
        //    this->computeHessianDet(d_intImage, img->width, img->height, kernel_list);
        this->computeHessianDet(d_intImage, image.getWidth(), image.getHeight(), kernel_list);
  
        // Determine which points are interesting
        // GPU kernels: non_max_suppression kernel
        this->selectIpoints(d_laplacian, d_pixPos, d_scale, kernel_list, maxIpts);
  
        // Copy the number of interesting points back to the host
        cl_copyBufferToHost(&this->num_ipts, this->d_ipt_count, sizeof(int));
  
        // Sanity check
        if(this->num_ipts < 0) {
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
          \param kernel_list Precompiled Kernels
          */
      void FastHessian::selectIpoints(cl_mem d_laplacian, cl_mem d_pixPos,
                                      cl_mem d_scale, cl_kernel* kernel_list,
                                      int maxPoints)
      {

        // The search for exterema (the most interesting point in a neighborhood)
        // is done by non-maximal suppression

        cl_kernel non_max_supression = kernel_list[KERNEL_NON_MAX_SUP];

        int BLOCK_W=16;
        int BLOCK_H=16;

        cl_setKernelArg(non_max_supression, 14, sizeof(cl_mem), (void*)&(this->d_ipt_count));
        cl_setKernelArg(non_max_supression, 15, sizeof(cl_mem), (void*)&d_pixPos);
        cl_setKernelArg(non_max_supression, 16, sizeof(cl_mem), (void*)&d_scale);
        cl_setKernelArg(non_max_supression, 17, sizeof(cl_mem), (void*)&d_laplacian);
        cl_setKernelArg(non_max_supression, 18, sizeof(int),    (void*)&maxPoints);
        cl_setKernelArg(non_max_supression, 19, sizeof(float),  (void*)&(this->thres));

        // Run the kernel for each octave
        for(int o = 0; o < octaves; o++)
          {
            for(int i = 0; i <= 1; i++) {

              cl_mem bResponse = this->responseMap.at(filter_map[o][i])->getResponses();
              int bWidth = this->responseMap.at(filter_map[o][i])->getWidth();
              int bHeight = this->responseMap.at(filter_map[o][i])->getHeight();
              int bFilter = this->responseMap.at(filter_map[o][i])->getFilter();

              cl_mem mResponse = this->responseMap.at(filter_map[o][i+1])->getResponses();
              int mWidth = this->responseMap.at(filter_map[o][i+1])->getWidth();
              int mHeight = this->responseMap.at(filter_map[o][i+1])->getHeight();
              int mFilter = this->responseMap.at(filter_map[o][i+1])->getFilter();
              cl_mem mLaplacian = this->responseMap.at(filter_map[o][i+1])->getLaplacian();

              cl_mem tResponse = this->responseMap.at(filter_map[o][i+2])->getResponses();
              int tWidth = this->responseMap.at(filter_map[o][i+2])->getWidth();
              int tHeight = this->responseMap.at(filter_map[o][i+2])->getHeight();
              int tFilter = this->responseMap.at(filter_map[o][i+2])->getFilter();
              int tStep = this->responseMap.at(filter_map[o][i+2])->getStep();

              size_t localWorkSize[2] = {(size_t)BLOCK_W, (size_t)BLOCK_H};
              size_t globalWorkSize[2] = {(size_t)roundUp(mWidth, BLOCK_W),
                                          (size_t)roundUp(mHeight, BLOCK_H)};

              cl_setKernelArg(non_max_supression,  0, sizeof(cl_mem), (void*)&tResponse);
              cl_setKernelArg(non_max_supression,  1, sizeof(int),    (void*)&tWidth);
              cl_setKernelArg(non_max_supression,  2, sizeof(int),    (void*)&tHeight);
              cl_setKernelArg(non_max_supression,  3, sizeof(int),    (void*)&tFilter);
              cl_setKernelArg(non_max_supression,  4, sizeof(int),    (void*)&tStep);
              cl_setKernelArg(non_max_supression,  5, sizeof(cl_mem), (void*)&mResponse);
              cl_setKernelArg(non_max_supression,  6, sizeof(cl_mem), (void*)&mLaplacian);
              cl_setKernelArg(non_max_supression,  7, sizeof(int),    (void*)&mWidth);
              cl_setKernelArg(non_max_supression,  8, sizeof(int),    (void*)&mHeight);
              cl_setKernelArg(non_max_supression,  9, sizeof(int),    (void*)&mFilter);
              cl_setKernelArg(non_max_supression, 10, sizeof(cl_mem), (void*)&bResponse);
              cl_setKernelArg(non_max_supression, 11, sizeof(int),    (void*)&bWidth);
              cl_setKernelArg(non_max_supression, 12, sizeof(int),    (void*)&bHeight);
              cl_setKernelArg(non_max_supression, 13, sizeof(int),    (void*)&bFilter);

              // Call non-max supression kernel
              cl_executeKernel(non_max_supression, 2, globalWorkSize, localWorkSize,
                               "NonMaxSupression", o*2+i);

              // TODO Verify that a clFinish is not required (setting an argument
              //      to the loop counter without it may be problematic, but it
              //      really kills performance on AMD parts)
              //cl_sync();
            }
          }
      }


      //! Reset the state of the data
      void FastHessian::reset()
      {
        int numIpts = 0;
        cl_copyBufferToDevice(this->d_ipt_count, &numIpts, sizeof(int));
      }



      //////////////////////////////////////////////////////////////////////////////
      // Surf class implementation /////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////

      // TODO Get rid of these arrays (i and j).  Have the values computed 
      //      dynamically within the kernel
      const int Surf::Data::j[] = {-12, -7, -2, 3,
                             -12, -7, -2, 3,
                             -12, -7, -2, 3,
                             -12, -7, -2, 3};

      const int Surf::Data::i[] = {-12,-12,-12,-12,
                             -7, -7, -7, -7,
                             -2, -2, -2, -2,
                             3,  3,  3,  3};

      const unsigned int Surf::Data::id[] = {6,5,4,3,2,1,0,1,2,3,4,5,6};

      const float Surf::Data::gauss25[] = {
        0.02350693969273f, 0.01849121369071f, 0.01239503121241f, 0.00708015417522f, 0.00344628101733f, 0.00142945847484f, 0.00050524879060f,
        0.02169964028389f, 0.01706954162243f, 0.01144205592615f, 0.00653580605408f, 0.00318131834134f, 0.00131955648461f, 0.00046640341759f,
        0.01706954162243f, 0.01342737701584f, 0.00900063997939f, 0.00514124713667f, 0.00250251364222f, 0.00103799989504f, 0.00036688592278f,
        0.01144205592615f, 0.00900063997939f, 0.00603330940534f, 0.00344628101733f, 0.00167748505986f, 0.00069579213743f, 0.00024593098864f,
        0.00653580605408f, 0.00514124713667f, 0.00344628101733f, 0.00196854695367f, 0.00095819467066f, 0.00039744277546f, 0.00014047800980f,
        0.00318131834134f, 0.00250251364222f, 0.00167748505986f, 0.00095819467066f, 0.00046640341759f, 0.00019345616757f, 0.00006837798818f,
        0.00131955648461f, 0.00103799989504f, 0.00069579213743f, 0.00039744277546f, 0.00019345616757f, 0.00008024231247f, 0.00002836202103f};


      //! Constructor
      Surf::Surf(int initialPoints, int i_height, int i_width, int octaves, 
                 int intervals, int sample_step, float threshold):m_data(new Data){

        static cl_kernel *kernel_list = 0;
        if(!kernel_list){
          char  devicePref = '\0';     // Device preference
          cl_init(devicePref);
          // NVIDIA's OpenCL cuurently doesn't support single-channel images
          if(cl_deviceIsNVIDIA()) setUsingImages(false);
          
          // Print a message saying whether or not images are being used
          if(isUsingImages()){
            printf("Using OpenCL images\n\n");
          }else{
            printf("Not using OpenCL images\n\n");
          }
          kernel_list = cl_precompileKernels(isUsingImages() ? 
                                             (char*)"-DIMAGES_SUPPORTED" : 
                                             (char*)0);
        }
        this->m_data->kernel_list = kernel_list;
        
        this->m_data->fh = new FastHessian(i_height, i_width, octaves, 
                                           intervals, sample_step, threshold, kernel_list);
        
        // Once we know the size of the image, successive frames should stay
        // the same size, so we can just allocate the space once for the integral
        // image and intermediate data
        if(isUsingImages()) 
          {   
            this->m_data->d_intImage = cl_allocImage(i_height, i_width, 'f');
            this->m_data->d_tmpIntImage = cl_allocImage(i_height, i_width, 'f');
            this->m_data->d_tmpIntImageT1 = cl_allocImage(i_width, i_height, 'f');
            this->m_data->d_tmpIntImageT2 = cl_allocImage(i_width, i_height, 'f');
          }
        else {
          this->m_data->d_intImage = cl_allocBuffer(sizeof(float)*i_width*i_height);
          this->m_data->d_tmpIntImage = cl_allocBuffer(sizeof(float)*i_height*i_width);
          // These two are unnecessary for buffers, but required for images, so
          // we'll use them for buffers as well to keep the code clean
          this->m_data->d_tmpIntImageT1 = cl_allocBuffer(sizeof(float)*i_height*i_width);
          this->m_data->d_tmpIntImageT2 = cl_allocBuffer(sizeof(float)*i_height*i_width);
        }

        // Allocate constant data on device
        this->m_data->d_gauss25 = cl_allocBufferConst(sizeof(float)*49,(void*)Surf::Data::gauss25);
        this->m_data->d_id = cl_allocBufferConst(sizeof(unsigned int)*13,(void*)Surf::Data::id);
        this->m_data->d_i = cl_allocBufferConst(sizeof(int)*16,(void*)Surf::Data::i);
        this->m_data->d_j = cl_allocBufferConst(sizeof(int)*16,(void*)Surf::Data::j);

        // Allocate buffers for each of the interesting points.  We don't know
        // how many there are initially, so must allocate more than enough space
        this->m_data->d_scale = cl_allocBuffer(initialPoints * sizeof(float));
        this->m_data->d_pixPos = cl_allocBuffer(initialPoints * sizeof(float2));
        this->m_data->d_laplacian = cl_allocBuffer(initialPoints * sizeof(int));
    
        // These buffers used to wait for the number of actual ipts to be known
        // before being allocated, instead now we'll only allocate them once
        // so that we can take advantage of optimized data transfers and reallocate
        // them if there's not enough space available
        this->m_data->d_length = cl_allocBuffer(initialPoints * DESC_SIZE * sizeof(float));
        this->m_data->d_desc = cl_allocBuffer(initialPoints * DESC_SIZE * sizeof(float));
        this->m_data->d_res = cl_allocBuffer(initialPoints * 109 * sizeof(float4));
        this->m_data->d_orientation = cl_allocBuffer(initialPoints * sizeof(float));

        // Allocate buffers to store the output data (descriptor information)
        // on the host
#ifdef OPTIMIZED_TRANSFERS
        this->m_data->h_scale = cl_allocBufferPinned(initialPoints * sizeof(float));
        this->m_data->h_pixPos = cl_allocBufferPinned(initialPoints * sizeof(float2));
        this->m_data->h_laplacian = cl_allocBufferPinned(initialPoints * sizeof(int));
        this->m_data->h_desc = cl_allocBufferPinned(initialPoints * DESC_SIZE * sizeof(float));
        this->m_data->h_orientation = cl_allocBufferPinned(initialPoints * sizeof(float));
#else
        this->m_data->scale = (float*)alloc(initialPoints * sizeof(float));
        this->m_data->pixPos = (float2*)alloc(initialPoints * sizeof(float2));
        this->m_data->laplacian = (int*)alloc(initialPoints * sizeof(int));
        this->m_data->desc = (float*)alloc(initialPoints * DESC_SIZE * sizeof(float));
        this->m_data->orientation = (float*)alloc(initialPoints * sizeof(float));
#endif
        // This is how much space is available for Ipts
        this->m_data->maxIpts = initialPoints;

        this->m_data->m_grayBuffer = icl::core::Img32f(icl::utils::Size(1,1),icl::core::formatGray);
      }


      //! Destructor
      Surf::~Surf() {

        cl_freeMem(this->m_data->d_intImage);
        cl_freeMem(this->m_data->d_tmpIntImage);
        cl_freeMem(this->m_data->d_tmpIntImageT1);
        cl_freeMem(this->m_data->d_tmpIntImageT2);
        cl_freeMem(this->m_data->d_desc);
        cl_freeMem(this->m_data->d_orientation);
        cl_freeMem(this->m_data->d_gauss25);
        cl_freeMem(this->m_data->d_id);
        cl_freeMem(this->m_data->d_i);
        cl_freeMem(this->m_data->d_j);
        cl_freeMem(this->m_data->d_laplacian);
        cl_freeMem(this->m_data->d_pixPos);
        cl_freeMem(this->m_data->d_scale);
        cl_freeMem(this->m_data->d_res);
        cl_freeMem(this->m_data->d_length);

#ifdef OPTIMIZED_TRANSFERS
        cl_freeMem(this->m_data->h_orientation);
        cl_freeMem(this->m_data->h_scale);
        cl_freeMem(this->m_data->h_laplacian);
        cl_freeMem(this->m_data->h_desc);
        cl_freeMem(this->m_data->h_pixPos);
#else
        free(this->m_data->orientation);
        free(this->m_data->scale);
        free(this->m_data->laplacian);
        free(this->m_data->desc);
        free(this->m_data->pixPos);
#endif

        delete this->m_data->fh;
        
        delete m_data;
      }

      //! Computes the integral image of image img.
      //! Assumes source image to be a  32-bit floating point.
      /*!
          Saves integral Image in d_intImage on the GPU
          \param source Input Image as grabbed by OpenCv
          */

      inline void scale_to_01(float &f){
        static const float s = 1.0f/255.0f;
        f *= s;
      }

      void Surf::computeIntegralImage(const icl::core::Img32f &image){
        //! convert the image to single channel 32f

        // TODO This call takes about 4ms (is there any way to speed it up?)
        //IplImage *img = getGray(source);
        //cc(&image,&this->m_data->m_grayBuffer);
  

        // set up variables for data access
        int height = image.getHeight(); //img->height;
        int width = image.getWidth(); //img->width;
        float *data = (float*)image.begin(0); //img->imageData;

        //m_grayBuffer = m_grayBuffer/(1.0f/255.0f);
  
        //        this->m_data->m_grayBuffer.forEach(scale_to_01);

        cl_kernel scan_kernel;
        cl_kernel transpose_kernel;

        if(isUsingImages()) {
          // Copy the data to the GPU
          cl_copyImageToDevice(this->m_data->d_intImage, data, height, width);

          scan_kernel = this->m_data->kernel_list[KERNEL_SCANIMAGE];
          transpose_kernel = this->m_data->kernel_list[KERNEL_TRANSPOSEIMAGE];
        }
        else {
          // Copy the data to the GPU
          cl_copyBufferToDevice(this->m_data->d_intImage, data, sizeof(float)*width*height);

          // If it is possible to use the vector scan (scan4) use
          // it, otherwise, use the regular scan
          if(cl_deviceIsAMD() && width % 4 == 0 && height % 4 == 0) 
            {
              // NOTE Change this to KERNEL_SCAN when running verification code.
              //      The reference code doesn't use a vector type and
              //      scan4 produces a slightly different integral image
              scan_kernel = this->m_data->kernel_list[KERNEL_SCAN4];
            }
          else 
            {
              scan_kernel = this->m_data->kernel_list[KERNEL_SCAN];
            }
          transpose_kernel = this->m_data->kernel_list[KERNEL_TRANSPOSE];
        }
    

        // -----------------------------------------------------------------
        // Step 1: Perform integral summation on the rows
        // -----------------------------------------------------------------

        size_t localWorkSize1[2]={64, 1};
        size_t globalWorkSize1[2]={(size_t)64, (size_t)height};

        cl_setKernelArg(scan_kernel, 0, sizeof(cl_mem), (void *)&(this->m_data->d_intImage));
        cl_setKernelArg(scan_kernel, 1, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImage)); 
        cl_setKernelArg(scan_kernel, 2, sizeof(int), (void *)&height);
        cl_setKernelArg(scan_kernel, 3, sizeof(int), (void *)&width);

        cl_executeKernel(scan_kernel, 2, globalWorkSize1, localWorkSize1, "Scan", 0);

        // -----------------------------------------------------------------
        // Step 2: Transpose
        // -----------------------------------------------------------------

        size_t localWorkSize2[]={16, 16};
        size_t globalWorkSize2[]={roundUp(width,16), roundUp(height,16)};

        cl_setKernelArg(transpose_kernel, 0, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImage));  
        cl_setKernelArg(transpose_kernel, 1, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImageT1)); 
        cl_setKernelArg(transpose_kernel, 2, sizeof(int), (void *)&height);
        cl_setKernelArg(transpose_kernel, 3, sizeof(int), (void *)&width);

        cl_executeKernel(transpose_kernel, 2, globalWorkSize2, localWorkSize2, "Transpose", 0);

        // -----------------------------------------------------------------
        // Step 3: Run integral summation on the rows again (same as columns
        //         integral since we've transposed). 
        // -----------------------------------------------------------------

        int heightT = width;
        int widthT = height;

        size_t localWorkSize3[2]={64, 1};
        size_t globalWorkSize3[2]={(size_t)64, (size_t)heightT};

        cl_setKernelArg(scan_kernel, 0, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImageT1));
        cl_setKernelArg(scan_kernel, 1, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImageT2)); 
        cl_setKernelArg(scan_kernel, 2, sizeof(int), (void *)&heightT);
        cl_setKernelArg(scan_kernel, 3, sizeof(int), (void *)&widthT);

        cl_executeKernel(scan_kernel, 2, globalWorkSize3, localWorkSize3, "Scan", 1);

        // -----------------------------------------------------------------
        // Step 4: Transpose back
        // -----------------------------------------------------------------

        size_t localWorkSize4[]={16, 16};
        size_t globalWorkSize4[]={roundUp(widthT,16), roundUp(heightT,16)};

        cl_setKernelArg(transpose_kernel, 0, sizeof(cl_mem), (void *)&(this->m_data->d_tmpIntImageT2)); 
        cl_setKernelArg(transpose_kernel, 1, sizeof(cl_mem), (void *)&(this->m_data->d_intImage));
        cl_setKernelArg(transpose_kernel, 2, sizeof(int), (void *)&heightT);
        cl_setKernelArg(transpose_kernel, 3, sizeof(int), (void *)&widthT);

        cl_executeKernel(transpose_kernel, 2, globalWorkSize4, localWorkSize4, "Transpose", 1);

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
      void Surf::createDescriptors(int i_width, int i_height)
      {

        const size_t threadsPerWG = 81;
        const size_t wgsPerIpt = 16;

        cl_kernel surf64Descriptor_kernel = this->m_data->kernel_list[KERNEL_SURF_DESC];

        size_t localWorkSizeSurf64[2] = {threadsPerWG,1};
        size_t globalWorkSizeSurf64[2] = {(wgsPerIpt*threadsPerWG),(size_t)this->m_data->numIpts};

        cl_setKernelArg(surf64Descriptor_kernel, 0, sizeof(cl_mem), (void*)&(this->m_data->d_intImage));
        cl_setKernelArg(surf64Descriptor_kernel, 1, sizeof(int),    (void*)&i_width);
        cl_setKernelArg(surf64Descriptor_kernel, 2, sizeof(int),    (void*)&i_height);
        cl_setKernelArg(surf64Descriptor_kernel, 3, sizeof(cl_mem), (void*)&(this->m_data->d_scale));
        cl_setKernelArg(surf64Descriptor_kernel, 4, sizeof(cl_mem), (void*)&(this->m_data->d_desc));
        cl_setKernelArg(surf64Descriptor_kernel, 5, sizeof(cl_mem), (void*)&(this->m_data->d_pixPos));
        cl_setKernelArg(surf64Descriptor_kernel, 6, sizeof(cl_mem), (void*)&(this->m_data->d_orientation));
        cl_setKernelArg(surf64Descriptor_kernel, 7, sizeof(cl_mem), (void*)&(this->m_data->d_length));
        cl_setKernelArg(surf64Descriptor_kernel, 8, sizeof(cl_mem), (void*)&(this->m_data->d_j));
        cl_setKernelArg(surf64Descriptor_kernel, 9, sizeof(cl_mem), (void*)&(this->m_data->d_i));

        cl_executeKernel(surf64Descriptor_kernel, 2, globalWorkSizeSurf64,
                         localWorkSizeSurf64, "CreateDescriptors"); 

        cl_kernel normSurf64_kernel = kernel_list[KERNEL_NORM_DESC];

        size_t localWorkSizeNorm64[] = {DESC_SIZE};
        size_t globallWorkSizeNorm64[] =  {(size_t)this->m_data->numIpts*DESC_SIZE};

        cl_setKernelArg(normSurf64_kernel, 0, sizeof(cl_mem), (void*)&(this->m_data->d_desc));
        cl_setKernelArg(normSurf64_kernel, 1, sizeof(cl_mem), (void*)&(this->m_data->d_length));

        // Execute the descriptor normalization kernel
        cl_executeKernel(normSurf64_kernel, 1, globallWorkSizeNorm64, localWorkSizeNorm64,
                         "NormalizeDescriptors"); 

      } 


      //! Calculate orientation for all ipoints
      /*!
          Calculate orientation for all ipoints using the
          sliding window technique from OpenSurf
          \param i_width The image width
          \param i_height The image height
          */
      void Surf::getOrientations(int i_width, int i_height)
      {

        cl_kernel getOrientation = this->m_data->kernel_list[KERNEL_GET_ORIENT1];
        cl_kernel getOrientation2 = this->m_data->kernel_list[KERNEL_GET_ORIENT2];  

        size_t localWorkSize1[] = {169};
        size_t globalWorkSize1[] = {(size_t)this->m_data->numIpts*169};

        /*!
            Assign the supplied Ipoint an orientation
            */

        cl_setKernelArg(getOrientation, 0, sizeof(cl_mem), (void *)&(this->m_data->d_intImage));
        cl_setKernelArg(getOrientation, 1, sizeof(cl_mem), (void *)&(this->m_data->d_scale));
        cl_setKernelArg(getOrientation, 2, sizeof(cl_mem), (void *)&(this->m_data->d_pixPos));
        cl_setKernelArg(getOrientation, 3, sizeof(cl_mem), (void *)&(this->m_data->d_gauss25));
        cl_setKernelArg(getOrientation, 4, sizeof(cl_mem), (void *)&(this->m_data->d_id));
        cl_setKernelArg(getOrientation, 5, sizeof(int),    (void *)&i_width);
        cl_setKernelArg(getOrientation, 6, sizeof(int),    (void *)&i_height);
        cl_setKernelArg(getOrientation, 7, sizeof(cl_mem), (void *)&(this->m_data->d_res));

        // Execute the kernel
        cl_executeKernel(getOrientation, 1, globalWorkSize1, localWorkSize1, 
                         "GetOrientations");

        cl_setKernelArg(getOrientation2, 0, sizeof(cl_mem), (void *)&(this->m_data->d_orientation));
        cl_setKernelArg(getOrientation2, 1, sizeof(cl_mem), (void *)&(this->m_data->d_res));

        size_t localWorkSize2[] = {42};
        size_t globalWorkSize2[] = {(size_t)this->m_data->numIpts*42};

        // Execute the kernel
        cl_executeKernel(getOrientation2, 1, globalWorkSize2, localWorkSize2,
                         "GetOrientations2");
      }

      //! Allocates the memory objects requried for the ipt descriptor information
      void Surf::reallocateIptBuffers() {

        // Release the old memory objects (that were too small)
        cl_freeMem(this->m_data->d_scale);
        cl_freeMem(this->m_data->d_pixPos);
        cl_freeMem(this->m_data->d_laplacian);
        cl_freeMem(this->m_data->d_length);
        cl_freeMem(this->m_data->d_desc);
        cl_freeMem(this->m_data->d_res);
        cl_freeMem(this->m_data->d_orientation);

        free(this->m_data->orientation);
        free(this->m_data->scale);
        free(this->m_data->laplacian);
        free(this->m_data->desc);
        free(this->m_data->pixPos);

        int newSize = this->m_data->maxIpts;

        // Allocate new memory objects based on the new size
        this->m_data->d_scale = cl_allocBuffer(newSize * sizeof(float));
        this->m_data->d_pixPos = cl_allocBuffer(newSize * sizeof(float2));
        this->m_data->d_laplacian = cl_allocBuffer(newSize * sizeof(int));
        this->m_data->d_length = cl_allocBuffer(newSize * DESC_SIZE*sizeof(float));
        this->m_data->d_desc = cl_allocBuffer(newSize * DESC_SIZE * sizeof(float));
        this->m_data->d_res = cl_allocBuffer(newSize * 121 * sizeof(float4));
        this->m_data->d_orientation = cl_allocBuffer(newSize * sizeof(float));

#ifdef OPTIMIZED_TRANSFERS
        this->m_data->h_scale = cl_allocBufferPinned(newSize * sizeof(float));
        this->m_data->h_pixPos = cl_allocBufferPinned(newSize * sizeof(float2));
        this->m_data->h_laplacian = cl_allocBufferPinned(newSize * sizeof(int));
        this->m_data->h_desc = cl_allocBufferPinned(newSize * DESC_SIZE * sizeof(float));
        this->m_data->h_orientation = cl_allocBufferPinned(newSize * sizeof(float));
#else
        this->m_data->scale = (float*)alloc(newSize * sizeof(float));
        this->m_data->pixPos = (float2*)alloc(newSize * sizeof(float2));
        this->m_data->laplacian = (int*)alloc(newSize * sizeof(int));
        this->m_data->desc = (float*)alloc(newSize * DESC_SIZE * sizeof(float));
        this->m_data->orientation = (float*)alloc(newSize * sizeof(float));
#endif
      }


      //! This function gets called each time SURF is run on a new frame.  It prevents
      //! having to create and destroy the object each time (lots of OpenCL overhead)
      void Surf::reset() 
      {
        this->m_data->fh->reset();
      }


      //! Retreive the descriptors from the GPU
      /*!
          Copy data back from the GPU into an IpVec structure on the host
          */
      const IpVec &Surf::retrieveDescriptors()
      {
        m_data->m_outputBuffer.resize(m_data->numIpts);
        if(!m_data->m_outputBuffer.size()) {
          return m_data->m_outputBuffer;
        }

        // Copy back the output data

#ifdef OPTIMIZED_TRANSFERS
        // We're using pinned memory for the transfers.  The data is 
        // copied back to pinned memory and then must be mapped before
        // it's usable on the host

        // Copy back Laplacian data
        this->m_data->laplacian = (int*)cl_copyAndMapBuffer(this->m_data->h_laplacian, 
                                                    this->m_data->d_laplacian, this->m_data->numIpts * sizeof(int));

        // Copy back scale data
        this->m_data->scale = (float*)cl_copyAndMapBuffer(this->m_data->h_scale, 
                                                  this->m_data->d_scale, this->m_data->numIpts * sizeof(float));
    
        // Copy back pixel positions
        this->m_data->pixPos = (float2*)cl_copyAndMapBuffer(this->m_data->h_pixPos, 
                                                    this->m_data->d_pixPos, this->m_data->numIpts * sizeof(float2));

        // Copy back descriptors
        this->m_data->desc = (float*)cl_copyAndMapBuffer(this->m_data->h_desc, 
                                                 this->m_data->d_desc, this->m_data->numIpts * DESC_SIZE* sizeof(float));

        // Copy back orientation data
        this->m_data->orientation = (float*)cl_copyAndMapBuffer(this->m_data->h_orientation, 
                                                        this->m_data->d_orientation, this->m_data->numIpts * sizeof(float));
#else
        // Copy back Laplacian information
        cl_copyBufferToHost(this->m_data->laplacian, this->m_data->d_laplacian, 
                            (this->m_data->numIpts) * sizeof(int), CL_FALSE);

        // Copy back scale data
        cl_copyBufferToHost(this->m_data->scale, this->m_data->d_scale,
                            (this->m_data->numIpts)*sizeof(float), CL_FALSE);

        // Copy back pixel positions
        cl_copyBufferToHost(this->m_data->pixPos, this->m_data->d_pixPos, 
                            (this->m_data->numIpts) * sizeof(float2), CL_FALSE);   

        // Copy back descriptors
        cl_copyBufferToHost(this->m_data->desc, this->m_data->d_desc, 
                            (this->m_data->numIpts)*DESC_SIZE*sizeof(float), CL_FALSE);
    
        // Copy back orientation data
        cl_copyBufferToHost(this->m_data->orientation, this->m_data->d_orientation, 
                            (this->m_data->numIpts)*sizeof(float), CL_TRUE);
#endif  

        // Parse the data into Ipoint structures
        for(int i= 0;i<(this->m_data->numIpts);i++)
          {		
            Ipoint &ipt = m_data->m_outputBuffer[i];
            ipt.x = this->m_data->pixPos[i].x;
            ipt.y = this->m_data->pixPos[i].y;
            ipt.scale = this->m_data->scale[i];
            ipt.laplacian = this->m_data->laplacian[i];
            ipt.orientation = this->m_data->orientation[i];
            memcpy(ipt.descriptor, &this->m_data->desc[i*64], sizeof(float)*64);
            //            m_data->m_outputBuffer[i] = ipts->push_back(ipt);
          }

#ifdef OPTIMIZED_TRANSFERS
        // We're done reading from the buffers, so we unmap
        // them so they can be used again by the device
        cl_unmapBuffer(this->m_data->h_laplacian, this->m_data->laplacian);
        cl_unmapBuffer(this->m_data->h_scale, this->m_data->scale);
        cl_unmapBuffer(this->m_data->h_pixPos, this->m_data->pixPos);
        cl_unmapBuffer(this->m_data->h_desc, this->m_data->desc);
        cl_unmapBuffer(this->m_data->h_orientation, this->m_data->orientation);
#endif

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
      void Surf::run(const icl::core::Img32f &image, bool upright) 
      {

        if (upright)
          {
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
        this->m_data->numIpts = this->m_data->fh->getIpoints(image, this->m_data->d_intImage, 
                                                             this->m_data->d_laplacian,
                                                             this->m_data->d_pixPos, 
                                                             this->m_data->d_scale, 
                                                             this->m_data->maxIpts);
    
        // Verify that there was enough space allocated for the number of
        // Ipoints found
        if(this->m_data->numIpts >= this->m_data->maxIpts) {
          // If not enough space existed, we need to reallocate space and
          // run the kernels again

          printf("Not enough space for Ipoints, reallocating and running again\n");
          this->m_data->maxIpts = this->m_data->numIpts * 2;
          this->reallocateIptBuffers();
          // XXX This was breaking sometimes
          this->m_data->fh->reset();
          this->m_data->numIpts = this->m_data->fh->getIpoints(image, this->m_data->d_intImage, 
                                         this->m_data->d_laplacian, this->m_data->d_pixPos, this->m_data->d_scale, this->m_data->maxIpts);
        }

        // printf("There were %d interest points\n", this->m_data->numIpts);    

        // Main SURF-64 loop assigns orientations and gets descriptors    
        if(this->m_data->numIpts==0) return;

        // GPU kernel: getOrientation1 (1x), getOrientation2 (1x)
        this->getOrientations(image.getWidth(), image.getHeight());

        // GPU kernel: surf64descriptor (1x), norm64descriptor (1x)
        this->createDescriptors(image.getWidth(), image.getHeight());
      }

      
      const IpVec &Surf::detect(const ImgBase *image){
        ICLASSERT_THROW(image,ICLException("CLSurfLib::Surf::detect: given image was null"));
        // TODO This call takes about 4ms (is there any way to speed it up?)
        //IplImage *img = getGray(source);
        cc(image,&this->m_data->m_grayBuffer);
  
        // set up variables for data access

        //m_grayBuffer = m_grayBuffer/(1.0f/255.0f);
  
        this->m_data->m_grayBuffer.forEach(scale_to_01);
        
        run(m_data->m_grayBuffer,false);
        
        const IpVec &ipts =  retrieveDescriptors();

        reset();

        return ipts;
      }

      
      //////////////////////////////////////////////////////////////////////////////
      // EventList class implementation ////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////

      //! Constructor
      EventList::EventList()
      {

      }


      //! Destructor
      EventList::~EventList()
      {
        // TODO Changes these loops to use iterators

        // Release kernel events
        for(int i = 0; i < (int)kernel_events.size(); i++) {
          clReleaseEvent(this->kernel_events[i].first);
          free(this->kernel_events[i].second);
        }

        // Release IO events
        for(int i = 0; i < (int)io_events.size(); i++) {
          clReleaseEvent(this->io_events[i].first);
          free(this->io_events[i].second);
        }

        // Compile events and User events use static char*s, no need to free them

        // Free the event lists
        this->kernel_events.clear();
        this->io_events.clear();
      }


      char* EventList::createFilenameWithTimestamp()
      {
        // TODO Make this nicer
        int maxStringLen = 100;
        char* timeStr = NULL;
        timeStr = (char*)alloc(sizeof(char)*maxStringLen);

        time_t rawtime;
        struct tm* timeStruct;

        time(&rawtime);
        timeStruct = localtime(&rawtime);

        strftime(timeStr, maxStringLen, "/Events_%Y_%m_%d_%H_%M_%S.surflog", timeStruct);

        return timeStr;
      }

      //! Dump a CSV file with event information
      void EventList::dumpCSV(char* path)
      {

        char* fullpath = NULL;
        FILE* fp =  NULL;

        // Construct a filename based on the current time
        char* filename = this->createFilenameWithTimestamp();
        fullpath = smartStrcat(path, filename);

        // Try to open the file for writing
        fp = fopen(fullpath, "w");
        if(fp == NULL) {
          printf("Error opening %s\n", fullpath);
          exit(-1);
        }

        // Write some information out about the environment

        char* tmp;
        char* tmp2;

        // Write the device name
        tmp = cl_getDeviceName();
        if(isUsingImages()) {
          tmp2 = smartStrcat(tmp, (char*)" (images)");
        }
        else {
          tmp2 = smartStrcat(tmp, (char*)" (buffers)");
        }
        fprintf(fp, "Info;\t%s\n", tmp2);
        free(tmp);
        free(tmp2);

        // Write the vendor name
        tmp = cl_getDeviceVendor();
        fprintf(fp, "Info;\t%s\n", tmp);
        free(tmp);

        // Write the driver version
        tmp = cl_getDeviceDriverVersion();
        fprintf(fp, "Info;\tDriver version %s\n", tmp);
        free(tmp);

        // Write the device version
        tmp = cl_getDeviceVersion();
        fprintf(fp, "Info;\t%s\n", tmp);
        free(tmp);

        // Write the hostname
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
        char hostname[50];
        if(gethostname(hostname, 50) != 0) {
          printf("Error getting hostname\n");
        }
        else {
          fprintf(fp, "Info;\tHost %s\n", hostname);
        }

        int kernelEventSize = this->kernel_events.size();
        for(int i = 0; i < kernelEventSize; i++) {
          fprintf(fp, "Kernel;\t%s;\t%3.3f\n", this->kernel_events[i].second,
                  cl_computeExecTime(this->kernel_events[i].first));
        }
        int ioEventSize = this->io_events.size();
        for(int i = 0; i < ioEventSize; i++) {
          fprintf(fp, "IO;\t%s;\t%3.3f\n", this->io_events[i].second,
                  cl_computeExecTime(this->io_events[i].first));
        }
        int compileEventSize = this->compile_events.size();
        for(int i = 0; i < compileEventSize; i++) {
          fprintf(fp, "Compile;\t%s;\t%3.3f\n", this->compile_events[i].second,
                  compile_events[i].first);
        }
        int userEventSize = this->user_events.size();
        for(int i = 0; i < userEventSize; i++) {
          fprintf(fp, "User;\t%s;\t%3.3f\n", this->user_events[i].second,
                  user_events[i].first);
        }

        fclose(fp);

        free(filename);
        free(fullpath);
      }

      //! Add a new compile event
      void EventList::newCompileEvent(double time, char* desc)
      {
        time_tuple tuple;

        tuple.first = time;
        tuple.second = desc;

        this->compile_events.push_back(tuple);
      }


      //! Add a new kernel event
      void EventList::newKernelEvent(cl_event event, char* desc)
      {
        event_tuple tuple;

        tuple.first = event;
        tuple.second = desc;

        this->kernel_events.push_back(tuple);
      }


      //! Add a new IO event
      void EventList::newIOEvent(cl_event event, char* desc)
      {
        event_tuple tuple;

        tuple.first = event;
        tuple.second = desc;

        this->io_events.push_back(tuple);
      }

      //! Add a new user event
      void EventList::newUserEvent(double time, char* desc)
      {
        time_tuple tuple;

        tuple.first = time;
        tuple.second = desc;

        this->user_events.push_back(tuple);
      }

      //! Print event information for all events
      void EventList::printAllEvents()
      {
        this->printCompileEvents();
        this->printKernelEvents();
        this->printIOEvents();
        this->printUserEvents();
      }


      //! Print event information for all entries in compile_events vector
      void EventList::printCompileEvents()
      {

        int numEvents = this->compile_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("Compile Event %d: %s\n", i, this->compile_events[i].second);
            printf("\tDURATION: %f\n", this->compile_events[i].first);
          }
      }


      //! Print event information for all entries in io_events vector
      void EventList::printIOEvents()
      {

        int numEvents = this->io_events.size();
        cl_int status;
        cl_ulong timer;

        for(int i = 0; i < numEvents; i++)
          {
            printf("Kernel Event %d: %s\n", i, this->io_events[i].second);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tENQUEUE: %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tSUBMIT:  %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tSTART:   %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tEND:     %llu\n", (unsigned long long int)timer);
          }
      }


      //! Print event information for all entries in kernel_events vector
      void EventList::printKernelEvents()
      {

        int numEvents = this->kernel_events.size();
        cl_int status;
        cl_ulong timer;

        for(int i = 0; i < numEvents; i++)
          {

            printf("Kernel event %d: %s\n", i, kernel_events[i].second);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tENQUEUE: %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tSUBMIT:  %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tSTART:   %llu\n", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            printf("\tEND:     %llu\n", (unsigned long long int)timer);
          }
      }

      //! Print event information for all entries in user_events vector
      void EventList::printUserEvents()
      {

        int numEvents = this->user_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("User Event %d: %s\n", i, this->user_events[i].second);
            printf("\tDURATION: %f\n", this->user_events[i].first);
          }
      }

      //! Print execution times of all events
      void EventList::printAllExecTimes()
      {
        this->printCompileExecTimes();
        this->printKernelExecTimes();
        this->printIOExecTimes();
        this->printUserExecTimes();
      }

      //! Print execution times for all entries in compile_events vector
      void EventList::printCompileExecTimes()
      {

        int numEvents = this->compile_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("Compile: %f: %s\n", this->compile_events[i].first,
                   this->compile_events[i].second);
          }
      }


      //! Print execution times for all entries in io_events vector
      void EventList::printIOExecTimes()
      {

        int numEvents = this->io_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("IO:      %3.3f: %s\n", cl_computeExecTime(this->io_events[i].first),
                   this->io_events[i].second);
          }
      }

      //! Print execution times for all entries in kernel_events vector
      void EventList::printKernelExecTimes()
      {

        int numEvents = this->kernel_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("Kernel:  %3.3f: %s\n", cl_computeExecTime(this->kernel_events[i].first),
                   this->kernel_events[i].second);
          }
      }

      //! Print execution times for all entries in user_events vector
      void EventList::printUserExecTimes()
      {

        int numEvents = this->user_events.size();

        for(int i = 0; i < numEvents; i++)
          {
            printf("User: %f: %s\n", this->user_events[i].first,
                   this->user_events[i].second);
          }
      }






      //! Dump a CSV file with event information
      void EventList::dumpTraceCSV(char* path)
      {

        char* fullpath = NULL;
        FILE* fp =  NULL;

        // Construct a filename based on the current time
        char* filename = this->createFilenameWithTimestamp();
        fullpath = smartStrcat(path, filename);

        // Try to open the file for writing
        fp = fopen(fullpath, "w");
        if(fp == NULL) {
          printf("Error opening %s\n", fullpath);
          exit(-1);
        }

        // Write some information out about the environment

        char* tmp;
        char* tmp2;

        // Write the device name
        tmp = cl_getDeviceName();
        if(isUsingImages()) {
          tmp2 = smartStrcat(tmp, (char*)" (images)");
        }
        else {
          tmp2 = smartStrcat(tmp, (char*)" (buffers)");
        }
        fprintf(fp, "Info;\t%s\n", tmp2);
        free(tmp);
        free(tmp2);

        // Write the vendor name
        tmp = cl_getDeviceVendor();
        fprintf(fp, "Info;\t%s\n", tmp);
        free(tmp);

        // Write the driver version
        tmp = cl_getDeviceDriverVersion();
        fprintf(fp, "Info;\tDriver version %s\n", tmp);
        free(tmp);

        // Write the device version
        tmp = cl_getDeviceVersion();
        fprintf(fp, "Info;\t%s\n", tmp);
        free(tmp);

        // Write the hostname
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
        char hostname[50];
        if(gethostname(hostname, 50) != 0) {
          printf("Error getting hostname\n");
        }
        else {
          fprintf(fp, "Info;\tHost %s\n", hostname);
        }

        int kernelEventSize = this->kernel_events.size();
        cl_ulong timer;
        cl_int status;

        for(int i = 0; i < kernelEventSize; i++)
          {

            fprintf(fp, "Kernel\t%s", this->kernel_events[i].second);
            //    cl_computeExecTime(this->kernel_events[i].first));

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->kernel_events[i].first,
                                             CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu\n", (unsigned long long int)timer);


          }
        int ioEventSize = this->io_events.size();
        for(int i = 0; i < ioEventSize; i++)
          {
            fprintf(fp, "IO\t%s", this->io_events[i].second);
            //,cl_computeExecTime(this->io_events[i].first));

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu", (unsigned long long int)timer);

            status = clGetEventProfilingInfo(this->io_events[i].first,
                                             CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &timer, NULL);
            cl_errChk(status, "profiling", true);
            fprintf(fp, "\t%llu\n", (unsigned long long int)timer);

          }

        /*
            int compileEventSize = this->compile_events.size();
            for(int i = 0; i < compileEventSize; i++)
            {
            fprintf(fp, "Compile;\t%s;\t%3.3f\n", this->compile_events[i].second,
            compile_events[i].first);
            }
            int userEventSize = this->user_events.size();
            for(int i = 0; i < userEventSize; i++) {
            fprintf(fp, "User;\t%s;\t%3.3f\n", this->user_events[i].second,
            user_events[i].first);
            }
            */
        fclose(fp);

        free(filename);
        free(fullpath);
      }

    }
  }
}

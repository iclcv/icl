/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonUtils.h                           **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#include <ICLIO/PylonIncludes.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>

namespace icl {
  namespace io{
    namespace pylon {
  
      /// Buffer, registered to the Pylon-drivers StreamGrabber \ingroup GIGE_G
      template <typename T>
      class PylonGrabberBuffer {
        private:
          T *m_pBuffer;
          Pylon::StreamBufferHandle m_hBuffer;
  
        public:
          PylonGrabberBuffer(size_t size) : m_pBuffer(NULL) {
            m_pBuffer = new T[size];
            if (!m_pBuffer)
              throw utils::ICLException("Not enough memory to allocate image buffer");
          }
          ~PylonGrabberBuffer(){
            if (m_pBuffer)
              delete[] m_pBuffer;
            }
          T* getBufferPointer(void) {
            return m_pBuffer;
          }
          Pylon::StreamBufferHandle getBufferHandle(void) {
            return m_hBuffer;
          }
          void setBufferHandle(Pylon::StreamBufferHandle hBuffer) {
            m_hBuffer = hBuffer;
          }
      };
  
      /// A buffer holding image information and timestamp \ingroup GIGE_G
      template <typename T>
      class TsBuffer {
        public:
          /// Buffer for image information
          T* m_Buffer;
          /// Buffer for image-timestamp
          uint64_t m_Timestamp;
          /// holdss the size of m_Buffer
          size_t m_Size;
  
          /// Constructor allocates required memory
          TsBuffer(size_t size)  : m_Timestamp(0), m_Size(size){
            m_Buffer = new T[m_Size];
            if (!m_Buffer)
              throw utils::ICLException("Not enough memory to allocate image buffer");
          }
          /// Frees allocated memory
          ~TsBuffer(){
            if (m_Buffer){
              delete[] m_Buffer;
            }
          }
          /// uses icl::io::pylon::TsBuffer::copy to write buffer-data to m_Buffer
          /**
          * casts buffer to internal type and copies m_Size blocks
          * to m_Buffer.
          */
          void copy(void* buffer){
            T* tmp = (T*) buffer;
            core::copy(tmp, tmp + m_Size, m_Buffer);
          }
      };
  
      /// This class holds all buffers needed for ColorConversion
      class ConvBuffers {
        public:
          /// Constructor sets all pointers to NULL
          ConvBuffers();
          /// calls free
          ~ConvBuffers();
          /// deletes Objects pointed at, when pointer != NULL
          void free();
          /// To this ImageBase the converted image is written.
          icl::core::ImgBase* m_Image;
          /// Buffer für color conversion
          icl8u* m_ImageBuff;
          /// Buffer für 16 bit mono copy
          icl16s* m_ImageBuff16;
          /// Buffer for interlieved-to-planar conversion
          icl::core::Img8u* m_ImageRGBA;
          /// Vector for channels
          std::vector<icl8u*>* m_Channels;
          /// Vector for 16bit mono-channel
          std::vector<icl16s*>* m_Channels16;
          /// boolean showing whether this buffers need a reset
          bool m_Reset;
      };
  
      /// This is used for concurrent writing and reading of ConvBuffers
      /**
          This class holds three pointers to ConvBuffers of which one is the
          currently read and the other two are alternately written to.
      **/
      class ConcGrabberBuffer {
        public:
          /// Constructor creates and initializes resources.
          ConcGrabberBuffer();
          /// Destructor frees allocated memory.
          ~ConcGrabberBuffer();
  
          /// returns a pointer to the most recent actualized ConvBuffers.
          /**
              ConvBuffers will then be marked and not overwritten till the
              next call to getNextImage()
          **/
          ConvBuffers* getNextReadBuffer();
          /// returns a pointer to the next write ConvBuffers.
          /**
              sets the returned ConvBuffers as current writeable and marks
              the old writeable as new.
          **/
          ConvBuffers* getNextWriteBuffer();
          /// mark ConvBuffers to be reset on next write-access.
          void setReset();
          /// tells whether a new ConvBuffers is available
          bool newAvailable();
  
        private:
          /// current objects which alternately are read and written.
          ConvBuffers*  m_Buffers[3];
          /// the Mutex is used for concurrent reading and writing.
          utils::Mutex m_Mutex;
          /// The object currently written to.
          int m_Write;
          /// The write object currently not written to.
          int m_Next;
          /// The object currently read from.
          int m_Read;
          /// tells whether an actualized object was written.
          bool m_Avail;
      };
  
      /// Utility Structure \ingroup GIGE_G
      /**
       * This struct is used to initialize and terminate the pylon environment.
       * It intializes Pylon on creation and terminates it on destruction.
       * Uses a static counter to ensure initialization only on first and
       * termination on last call.
       */
      struct PylonAutoEnv{
        public:
          /// Initializes Pylon environment if not already done.
          PylonAutoEnv();
          /// Terminates Pylon environment when (calls to term) == (calls to init).
          ~PylonAutoEnv();
          /// Initializes the Pylon environment.
          /** @return whether Pylon::PylonInitialize() actually was called. */
          static bool initPylonEnv();
          /// terminates the Pylon environment.
          /** @return whether Pylon::PylontTerminate() actually was called. */
          static bool termPylonEnv();
      };
  
      /// Utility Structure \ingroup GIGE_G
      /**
      * This struct is used to realize easy interruprion of Pylon grabbing
      * processes.
      */
      struct Interruptable{
          /// Virtual destructor
          virtual ~Interruptable() {}
          /// starts the acquisition
          virtual void acquisitionStart() = 0;
          /// stops the acquisition
          virtual void acquisitionStop() = 0;
          /// starts grabbing
          virtual void grabbingStart() = 0;
          /// stops grabbing
          virtual void grabbingStop() = 0;
      };
  
      /// Utility Structure \ingroup GIGE_G
      /**
      * This struct is used to stop the acquisition and restart it
      * on destruction. It also locks the mutex lock so this can't be
      * created twice at the same time.
      */
      struct AcquisitionInterruptor{
        private:
          /// A pointer to the PylonGrabberImpl that is to be stopped.
          Interruptable* m_Interu;
  
        public:
          /// stops the acquisiton
          /**
          * @param i The Interruptable to stop.
          * @param mock Whether to mock the stopping. This can be used to
          *        create 'shallow' AcquisitionInterruptors on conditions.
          */
          AcquisitionInterruptor(Interruptable* i, bool mock=false);
          /// Starts acquisition if stopped before.
          ~AcquisitionInterruptor();
      };
  
      /// Utility Structure \ingroup GIGE_G
      /**
      * This struct is used to stop grabbing and restart it on destruction.
      * This should not be created while image acquisition is still active.
      */
      struct GrabbingInterruptor{
      private:
        /// A pointer to the Interruptable that needs to be stopped.
        Interruptable* m_Interu;
  
      public:
        /// Constructor calls grabbingStop().
        /**
        * @param i The Interruptable to stop.
        * @param mock Whether to mock the stopping. This can be used to
        *        create 'shallow' GrabbingInterruptors on conditions.
        */
        GrabbingInterruptor(Interruptable* i, bool mock=false);
        /// Destructor calls grabbingStart().
        ~GrabbingInterruptor();
      };
  
      /// Prints help-information to std::cout
      void printHelp();
      /// Uses args to find demanded device
      Pylon::CDeviceInfo getDeviceFromArgs(std::string args) throw(utils::ICLException);
      /// Uses args to find out which BufferChannel to use.
      int channelFromArgs(std::string args);
      /// Returns a list of available Pylon devices.
      /** @param filter if provided will be used to filter available devices */
      Pylon::DeviceInfoList_t
      getPylonDeviceList(Pylon::DeviceInfoList_t* filter=NULL);
  
    } //namespace pylon
  } // namespace io
} //namespace icl


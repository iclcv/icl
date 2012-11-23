/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/OpenNIUtils.h                            **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#pragma once

#include <ICLCore/ImgBase.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Configurable.h>

#include <XnOS.h>
#include <XnCppWrapper.h>

#include <map>
#include <limits>

namespace icl {
  namespace io{

    namespace icl_openni {

      /// fills an core::core::Img<T> from OpenNI DepthMetaData
      template<class T>
      core::Img<T>* convertDepthImg(xn::DepthMetaData* src, core::Img<T>* dst){
        float max = 0;
        if (std::numeric_limits<T>::max() < src -> ZRes()){
          max = ((float) std::numeric_limits<T>::max()) / ((float) src -> ZRes());
        }

        dst -> setSize(utils::Size(src -> XRes(), src -> YRes()));
        T* data = dst -> getData(0);
        // draw DEPTH image
        const XnDepthPixel* pDepthRow = src -> Data();
        if(!max){
          for (unsigned int y = 0; y < src -> YRes(); ++y){
            for (unsigned int x = 0; x < src -> XRes(); ++x, ++pDepthRow, ++data){
              *data = *pDepthRow;
            }
          }
        } else {
          for (unsigned int y = 0; y < src -> YRes(); ++y){
            for (unsigned int x = 0; x < src -> XRes(); ++x, ++pDepthRow, ++data){
              *data = *pDepthRow * max;
            }
          }
        }
        return dst;
      }

      /// fills an core::Img16s from OpenNI IRMetaData
      inline core::Img16s* convertIRImg(xn::IRMetaData* src, core::Img16s* dst){
        dst -> setSize(utils::Size(src -> XRes(), src -> YRes()));
        icl16s* data = dst -> getData(0);
        const XnIRPixel* pIRRow = src -> Data();
        // draw grayscale image
        for (unsigned int y = 0; y < src -> YRes(); ++y){
          for (unsigned int x = 0; x < src -> XRes(); ++x, ++pIRRow, ++data){
            *data = *pIRRow;
          }
        }
        return dst;
      }

      /**
        fills a three channel core::Img8u from OpenNI ImageMetaData expecting
        the Generator to generate RGB24 Data.
      **/
      inline core::Img8u* convertRGBImg(xn::ImageMetaData* src, core::Img8u* dst){
        dst -> setSize(utils::Size(src -> XRes(), src -> YRes()));
        // draw RGB image
        icl8u* rChannel = dst -> getData(0);
        icl8u* gChannel = dst -> getData(1);
        icl8u* bChannel = dst -> getData(2);
        const XnRGB24Pixel* rgbPixel = src -> RGB24Data();
        for (unsigned int y = 0; y < src -> YRes(); ++y){
          for (unsigned int x = 0; x < src -> XRes(); ++x, ++rgbPixel, ++rChannel,
               ++gChannel, ++bChannel)
          {
            *rChannel = rgbPixel -> nRed;
            *gChannel = rgbPixel -> nGreen;
            *bChannel = rgbPixel -> nBlue;
          }
        }
        return dst;
      }

      /// A BufferHandlers only task is to create T's.
      template<typename T>
      class ReadWriteBufferHandler {
        public:
          /// creates an instance of T and returns a pointer. passes ownership.
          virtual T* initBuffer() = 0;
      };

      /// This is used for concurrent writing and reading of Buffers
      /**
        This class holds three pointers to T of which one is the
        currently read and the other two are alternately written to.
      **/
      template<typename T>
      class ReadWriteBuffer {
        public:
          /// Constructor creates and initializes resources.
          ReadWriteBuffer(ReadWriteBufferHandler<T>* buffer_handler)
            : m_Mutex(), m_Write(0), m_Next(1), m_Read(2)
          {
            utils::Mutex::Locker l(m_Mutex);
            m_BufferHandler = buffer_handler;
            m_Buffers[0] = m_BufferHandler -> initBuffer();
            m_Buffers[1] = m_BufferHandler -> initBuffer();
            m_Buffers[2] = m_BufferHandler -> initBuffer();
            m_ResetBuffers[0] = false;
            m_ResetBuffers[1] = false;
            m_ResetBuffers[2] = false;
          }

          /// Destructor frees allocated memory.
          ~ReadWriteBuffer(){
            utils::Mutex::Locker l(m_Mutex);
            ICL_DELETE(m_Buffers[0]);
            ICL_DELETE(m_Buffers[1]);
            ICL_DELETE(m_Buffers[2]);
          }

          /// returns a pointer to the most recent actualized buffer.
          /**
            Buffer will then be marked and not overwritten till the
            next call to getNextReadBuffer()
        **/
          T* getNextReadBuffer(){
            utils::Mutex::Locker l(m_Mutex);
            if(m_Avail){
              // new buffer is available.
              std::swap(m_Next, m_Read);
              m_Avail = false;
            }
            return m_Buffers[m_Read];
          }

          /// returns pointer to most recent buffer.
          /**
            if omit_double_frames is true, this function will call sleep for
            omit_sleep_millis and retry until a new buffer is available or
            omit_max_wait_millis is reached. when no new buffer could be returned
            NULL will be returned.

            @param omit_double_frames whether double frames should be omitted
                   default value is false.
            @param omit_max_wait_millis how long to wait for a new image before
                   returning null.
            @param omit_sleep_micros how long to sleep between checking for new
                   buffer (in microseconds).
            will return null when no new ReadBuffer available.
        **/
          T* getNextReadBuffer(bool omit_double_frames=false,
                               int omit_max_wait_millis=1000,
                               int omit_sleep_micros=1000){
            T* tmp = NULL;
            utils::Time t = utils::Time::now();
            while (true){
              m_Mutex.lock();
              if(m_Avail){
                // new buffer is available.
                std::swap(m_Next, m_Read);
                m_Avail = false;
                tmp = m_Buffers[m_Read];
                m_Mutex.unlock();
                break;
              } else if(!omit_double_frames){
                tmp = m_Buffers[m_Read];
                m_Mutex.unlock();
                break;
              }
              m_Mutex.unlock();
              if(t.age().toMilliSeconds() > omit_max_wait_millis){
                break;
              }
              utils::Thread::usleep(omit_sleep_micros);
            }
            return tmp;
          }

          /// returns a pointer to the next write Buffer.
          /**
            sets the returned Buffer as current writeable and marks
            the old writeable as new.
        **/
          T* getNextWriteBuffer(){
            utils::Mutex::Locker l(m_Mutex);
            // swap write buffer and next buffer.
            std::swap(m_Next, m_Write);
            // new buffer is available for reading.
            m_Avail = true;
            // reset buffer when needed
            if(m_ResetBuffers[m_Write]){
              ICL_DELETE(m_Buffers[m_Write]);
              m_Buffers[m_Write] = m_BufferHandler -> initBuffer();
              m_ResetBuffers[m_Write] = false;
            }
            // return new write buffer.
            return m_Buffers[m_Write];
          }

          /// mark buffers to be reset on next write-access.
          void setReset(){
            utils::Mutex::Locker l(m_Mutex);
            m_ResetBuffers[0] = true;
            m_ResetBuffers[1] = true;
            m_ResetBuffers[2] = true;
          }

          /// switches the handler
          void switchHandler(ReadWriteBufferHandler<T>* new_handler){
            utils::Mutex::Locker l(m_Mutex);
            m_BufferHandler = new_handler;
            m_ResetBuffers[0] = true;
            m_ResetBuffers[1] = true;
            m_ResetBuffers[2] = true;
          }

          /// tells whether a new ConvBuffers is available
          bool newAvailable(){
            utils::Mutex::Locker l(m_Mutex);
            return m_Avail;
          }

        private:
          /// the handler used to create new buffers
          ReadWriteBufferHandler<T>* m_BufferHandler;
          /// current objects which alternately are read and written.
          T*  m_Buffers[3];
          /// a bool for every buffer telling whether it needs a reset
          bool  m_ResetBuffers[3];
          /// the mutex is used for concurrent reading and writing.
          utils::Mutex m_Mutex;
          /// the object currently written to.
          int m_Write;
          /// the write object currently not written to.
          int m_Next;
          /// the object currently read from.
          int m_Read;
          /// tells whether an actualized object was written.
          bool m_Avail;
      };

      /// this class interprets and sets Properties of OpenNI MapGenerators
      class MapGeneratorOptions : public utils::Configurable {
        public:
          /// constructor
          MapGeneratorOptions(xn::MapGenerator* generator);

          /// callback for changed configurable properties
          void processPropertyChange(const utils::Configurable::Property &prop);

          /// adds a general int capability as property
          void addGeneralIntProperty(const std::string name);

        private:
          /// the used MapGenerator
          xn::MapGenerator* m_Generator;
          /// A vector holding all capabilities of the MapGenerator
          std::vector<std::string> m_Capabilities;
          /// A Map Holding all used ProductionNodes
          std::map<std::string, xn::ProductionNode> m_ProductionNodeMap;
      };

      /// this class interprets and sets Properties of OpenNI DepthGenerators
      class DepthGeneratorOptions : public MapGeneratorOptions {
        public:
          /// constructor
          DepthGeneratorOptions(xn::DepthGenerator* generator);

          /// interface for the setter function for video device properties
          void setProperty(const std::string &property, const std::string &value);
          /// adds properties to propertylist
          void addPropertiesToList(std::vector<std::string> &properties);
          /// checks if property is supported
          bool supportsProperty(const std::string &property);
          /// get type of property
          std::string getType(const std::string &name);
          /// get information of a properties valid values
          std::string getInfo(const std::string &name);
          /// returns the current value of a property or a parameter
          std::string getValue(const std::string &name);
          /// Returns whether this property may be changed internally.
          int isVolatile(const std::string &propertyName);

        private:
          /// the used DepthGenerator
          xn::DepthGenerator* m_DepthGenerator;
      };

      /// this class interprets and sets Properties of OpenNI ImageGenerators
      class ImageGeneratorOptions : public MapGeneratorOptions {
        public:
          /// constructor
          ImageGeneratorOptions(xn::ImageGenerator* generator);

          /// interface for the setter function for video device properties
          void setProperty(const std::string &property, const std::string &value);
          /// adds properties to propertylist
          void addPropertiesToList(std::vector<std::string> &properties);
          /// checks if property is supported
          bool supportsProperty(const std::string &property);
          /// get type of property
          std::string getType(const std::string &name);
          /// get information of a properties valid values
          std::string getInfo(const std::string &name);
          /// returns the current value of a property or a parameter
          std::string getValue(const std::string &name);
          /// Returns whether this property may be changed internally.
          int isVolatile(const std::string &propertyName);

          /// callback for changed configurable properties
          void processPropertyChange(const utils::Configurable::Property &prop);

        private:
          /// the used ImageGenerator
          xn::ImageGenerator* m_ImageGenerator;
      };

      /// abstract super-class of all Image generators
      class OpenNIMapGenerator : public ReadWriteBufferHandler<core::ImgBase> {
        public:

          /// an enum listing all supported data generators
          enum Generators {
            RGB,
            DEPTH,
            IR,
            NOT_SPECIFIED = -1
          };

          /// grab function grabs an image returns whether grabbing worked
          virtual bool acquireImage(core::ImgBase* dest) = 0;
          /// tells the type of the Generator
          virtual Generators getGeneratorType() = 0;
          /// returns underlying xn::MapGenerator instance
          virtual xn::MapGenerator* getMapGenerator() = 0;
          /// Creates an core::ImgBase for ReadWriteBuffer
          virtual core::ImgBase* initBuffer() = 0;
          /// getter for MapGeneratorOptions
          virtual MapGeneratorOptions* getMapGeneratorOptions() = 0;


          ///  Creates the corresponding Generator.
          static OpenNIMapGenerator* createGenerator(xn::Context* context,
                                                     std::string id);
      };

      /// Depth Image Generator
      class OpenNIDepthGenerator : public OpenNIMapGenerator {
        public:
          /// Creates DepthGenerator number num from Context
          OpenNIDepthGenerator(xn::Context* context, int num);
          /// Destructor frees all resouurces
          ~OpenNIDepthGenerator();

          /// grab function grabs an image returns whether grabbing worked
          bool acquireImage(core::ImgBase* dest);
          /// tells the type of the Generator
          Generators getGeneratorType();
          /// returns underlying xn::MapGenerator instance
          xn::MapGenerator* getMapGenerator();
          /// Creates an core::Img16s for ReadWriteBuffer
          core::ImgBase* initBuffer();
          /// getter for MapGeneratorOptions
          MapGeneratorOptions* getMapGeneratorOptions();

        private:
          /// the OpenNI context
          xn::Context* m_Context;
          /// the underlying core::depth generator
          xn::DepthGenerator* m_DepthGenerator;
          /// a DepthMetaData object holding image information
          xn::DepthMetaData m_DepthMD;
          /// pointer to internally used MapGeneratorOptions
          MapGeneratorOptions* m_Options;
      };

      /// RGB Image Generator
      class OpenNIRgbGenerator : public OpenNIMapGenerator {
        public:
          /// Creates RgbGenerator number num from Context
          OpenNIRgbGenerator(xn::Context* context, int num);
          /// Destructor frees all resouurces
          ~OpenNIRgbGenerator();

          /// grab function grabs an image returns whether grabbing worked
          bool acquireImage(core::ImgBase* dest);
          /// tells the type of the Generator
          Generators getGeneratorType();
          /// returns underlying xn::MapGenerator instance
          xn::MapGenerator* getMapGenerator();
          /// Creates an core::Img8u for ReadWriteBuffer
          core::Img8u* initBuffer();
          /// getter for MapGeneratorOptions
          MapGeneratorOptions* getMapGeneratorOptions();

        private:
          /// the OpenNI context
          xn::Context* m_Context;
          /// A NodeInfo for the used device
          xn::NodeInfo* m_DeviceInfo;
          /// the underlying rgb-image generator
          xn::ImageGenerator* m_RgbGenerator;
          /// the underlying core::depth generator
          /**
            The Xtion cam does not provide rgb images without depthGenerator
            being created.
        **/
          xn::DepthGenerator* m_DepthGenerator;
          xn::IRGenerator* m_IrGenerator;
          /// a ImagehMetaData object holding image information
          xn::ImageMetaData m_RgbMD;
          /// pointer to internally used MapGeneratorOptions
          MapGeneratorOptions* m_Options;
      };

      /// IR Image Generator
      class OpenNIIRGenerator : public OpenNIMapGenerator {
        public:
          /// Creates IRGenerator number num from Context
          OpenNIIRGenerator(xn::Context* context, int num);
          /// Destructor frees all resouurces
          ~OpenNIIRGenerator();

          /// grab function grabs an image returns whether grabbing worked
          bool acquireImage(core::ImgBase* dest);
          /// tells the type of the Generator
          Generators getGeneratorType();
          /// returns underlying xn::MapGenerator instance
          xn::MapGenerator* getMapGenerator();
          /// Creates an core::Img8u for ReadWriteBuffer
          core::Img16s* initBuffer();
          /// getter for MapGeneratorOptions
          MapGeneratorOptions* getMapGeneratorOptions();

        private:
          /// the OpenNI context
          xn::Context* m_Context;
          /// A NodeInfo for the used device
          xn::NodeInfo* m_DeviceInfo;
          /// the underlying it-image generator
          xn::IRGenerator* m_IrGenerator;
          /// a ImagehMetaData object holding image information
          xn::IRMetaData m_IrMD;
          /// pointer to internally used MapGeneratorOptions
          MapGeneratorOptions* m_Options;
      };

    } // namespace icl_openni

  } // namespace io
} // namespace icl


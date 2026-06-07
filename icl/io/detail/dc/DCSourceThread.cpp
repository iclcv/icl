// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/dc/DCSourceThread.h>
#include <icl/io/detail/dc/DCFrameQueue.h>
#include <icl/utils/Macros.h>
#include <icl/utils/SignalHandler.h>
#include <icl/io/detail/dc/DCDevice.h>
#include <algorithm>
#include <vector>
#include <icl/io/detail/dc/DCSource.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;
namespace icl::io {
  namespace dc{

    /// mutex protected list of all currently running source threads
    std::recursive_mutex g_oSourceThreadMutex;
    vector<DCSourceThread*> g_vecAllThreads;
    bool g_bStopAllSourceThreadsCalled = false;

    DCSourceThread::~DCSourceThread(){
      stop();
      ICL_DELETE(m_poFrameQueue);

      if(!g_bStopAllSourceThreadsCalled){
        /// remove from the source thread list to
        g_oSourceThreadMutex.lock();
        vector<DCSourceThread*>::iterator it = find(g_vecAllThreads.begin(),g_vecAllThreads.end(),this);
        if(it != g_vecAllThreads.end()){
          g_vecAllThreads.erase(it);
        }
        g_oSourceThreadMutex.unlock();
      }

    }
    void DCSourceThread::resetBus(){
      dc1394_reset_bus(m_poCam);
    }

    void DCSourceThread::stopAllSourceThreads(){

      /* Why ???
          for(unsigned int i=0; i<g_vecAllThreads.size();++i){
          std::cout << "[unclean break detected] resetting bus for camera " << i << std::endl;
          g_vecAllThreads[i]->resetBus();
          }
      */

      g_oSourceThreadMutex.lock();
      g_bStopAllSourceThreadsCalled = true;

      for(unsigned int i=0;i<g_vecAllThreads.size();i++){
        std::cout << "> stopping source thread for camera " << i << std::endl;
        g_vecAllThreads[i]->stop();
      }
      g_vecAllThreads.clear();
      g_bStopAllSourceThreadsCalled = false;
      g_oSourceThreadMutex.unlock();

      std::cout << "> resetting firewire bus" << std::endl;
      DCSource::dc1394_reset_bus(false);
    }


    DCSourceThread::DCSourceThread(dc1394camera_t* c,
                                     DCDeviceOptions *options):

      m_poFrameQueue(0),m_poCam(c),m_poOptions(options),
      m_lastFramesTimeStamp(0){
      g_oSourceThreadMutex.lock();
      g_vecAllThreads.push_back(this);
      g_oSourceThreadMutex.unlock();

      m_poFrameQueue = new DCFrameQueue(c,options);
    }


    void DCSourceThread::run(){

      // I moved this to the constructor (why was it placed here?)
      //if(!m_poFrameQueue){
      // m_poFrameQueue = new DCFrameQueue(m_poCam, m_poOptions);
      //}
      while(running()){
        if(!trylock()){
          m_poFrameQueue->step();
          unlock();
          msleep(1);
        } else {
          msleep(5);
        }
      }
    }



    dc1394video_frame_t *DCSourceThread::waitForNextImageFrame(){
      Time &lastTime = m_lastFramesTimeStamp;
      dc1394video_frame_t *frame = m_poFrameQueue->back();

      if(m_poOptions->suppressDoubledImages && lastTime != Time(0)){
        while(Time(frame->timestamp) <= lastTime && running()){
          m_poFrameQueue->unlock();
          usleep(100);
          m_poFrameQueue->lock();
          frame = m_poFrameQueue->back();
        }
      }
      lastTime = Time(frame->timestamp);
      return frame;
    }

    /// returns the current image directly (if no desried parameters are set)
    void DCSourceThread::getCurrentImage(ImgBase **ppoDst,
                                          dc1394color_filter_t bayerLayout,
                                          dc1394bayer_method_t bayerMethod){

      while(!m_poFrameQueue) Thread::msleep(10);

      m_poFrameQueue->lock();

      extract_image_to_2(waitForNextImageFrame(),bayerLayout,ppoDst,m_oRGBInterleavedBuffer,bayerMethod);

      m_poFrameQueue->unlock();


    }


    void DCSourceThread::getCurrentImage(ImgBase **ppoDst,
                                          ImgBase **ppoDstTmp,
                                          bool &desiredParamsFullfilled,
                                          const Size &desiredSizeHint,
                                          format desiredFormatHint,
                                          depth desiredDepthHint,
                                          dc1394color_filter_t bayerLayout,
                                          dc1394bayer_method_t bayerMethod){

      while(!m_poFrameQueue) Thread::msleep(10);


      m_poFrameQueue->lock();

      dc1394video_frame_t *frame = waitForNextImageFrame();

      desiredParamsFullfilled = can_extract_image_to(frame,desiredSizeHint,desiredFormatHint,desiredDepthHint);

      extract_image_to(frame,
                       bayerLayout,
                       desiredParamsFullfilled ? ppoDst : ppoDstTmp,
                       desiredSizeHint,
                       desiredFormatHint,
                       desiredDepthHint,
                       m_oRGBInterleavedBuffer,
                       bayerMethod);

      m_poFrameQueue->unlock();

    }

  }
  } // namespace icl::io
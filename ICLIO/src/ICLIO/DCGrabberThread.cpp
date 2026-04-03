// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/DCGrabberThread.h>
#include <ICLIO/DCFrameQueue.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/SignalHandler.h>
#include <ICLIO/DCDevice.h>
#include <algorithm>
#include <vector>
#include <ICLIO/DCGrabber.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;
namespace icl::io {
  namespace dc{

    /// mutex protected list of all currently running grabber threads
    std::recursive_mutex g_oGrabberThreadMutex;
    vector<DCGrabberThread*> g_vecAllThreads;
    bool g_bStopAllGrabberThreadsCalled = false;

    DCGrabberThread::~DCGrabberThread(){
      stop();
      ICL_DELETE(m_poFrameQueue);

      if(!g_bStopAllGrabberThreadsCalled){
        /// remove from the grabber thread list to
        g_oGrabberThreadMutex.lock();
        vector<DCGrabberThread*>::iterator it = find(g_vecAllThreads.begin(),g_vecAllThreads.end(),this);
        if(it != g_vecAllThreads.end()){
          g_vecAllThreads.erase(it);
        }
        g_oGrabberThreadMutex.unlock();
      }

    }
    void DCGrabberThread::resetBus(){
      dc1394_reset_bus(m_poCam);
    }

    void DCGrabberThread::stopAllGrabberThreads(){

      /* Why ???
          for(unsigned int i=0; i<g_vecAllThreads.size();++i){
          std::cout << "[unclean break detected] resetting bus for camera " << i << std::endl;
          g_vecAllThreads[i]->resetBus();
          }
      */

      g_oGrabberThreadMutex.lock();
      g_bStopAllGrabberThreadsCalled = true;

      for(unsigned int i=0;i<g_vecAllThreads.size();i++){
        std::cout << "> stopping grabber thread for camera " << i << std::endl;
        g_vecAllThreads[i]->stop();
      }
      g_vecAllThreads.clear();
      g_bStopAllGrabberThreadsCalled = false;
      g_oGrabberThreadMutex.unlock();

      std::cout << "> resetting firewire bus" << std::endl;
      DCGrabber::dc1394_reset_bus(false);
    }


    DCGrabberThread::DCGrabberThread(dc1394camera_t* c,
                                     DCDeviceOptions *options):

      m_poFrameQueue(0),m_poCam(c),m_poOptions(options),
      m_lastFramesTimeStamp(0){
      g_oGrabberThreadMutex.lock();
      g_vecAllThreads.push_back(this);
      g_oGrabberThreadMutex.unlock();

      m_poFrameQueue = new DCFrameQueue(c,options);
    }


    void DCGrabberThread::run(){

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



    dc1394video_frame_t *DCGrabberThread::waitForNextImageFrame(){
      Time &lastTime = m_lastFramesTimeStamp;
      dc1394video_frame_t *frame = m_poFrameQueue->back();

      if(m_poOptions->suppressDoubledImages && lastTime != Time(0)){
        while(Time(frame->timestamp) <= lastTime && runningNoLock()){
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
    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst,
                                          dc1394color_filter_t bayerLayout,
                                          dc1394bayer_method_t bayerMethod){

      while(!m_poFrameQueue) Thread::msleep(10);

      m_poFrameQueue->lock();

      extract_image_to_2(waitForNextImageFrame(),bayerLayout,ppoDst,m_oRGBInterleavedBuffer,bayerMethod);

      m_poFrameQueue->unlock();


    }


    void DCGrabberThread::getCurrentImage(ImgBase **ppoDst,
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
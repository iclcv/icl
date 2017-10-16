/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCGrabberThread.cpp                    **
** Module : ICLIO                                                  **
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

#include <ICLIO/DCGrabberThread.h>
#include <ICLIO/DCFrameQueue.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/SignalHandler.h>
#include <ICLIO/DCDevice.h>
#include <algorithm>
#include <vector>
#include <ICLIO/DCGrabber.h>

using namespace icl::utils;
using namespace icl::core;

using namespace std;
namespace icl{
  namespace io{
    namespace dc{

      /// mutex protected list of all currently running grabber threads
      Mutex g_oGrabberThreadMutex;
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
        // {{{ open

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

      // }}}

      DCGrabberThread::DCGrabberThread(dc1394camera_t* c,
                                       DCDeviceOptions *options):
        // {{{ open

        m_poFrameQueue(0),m_poCam(c),m_poOptions(options),
        m_lastFramesTimeStamp(0){
        g_oGrabberThreadMutex.lock();
        g_vecAllThreads.push_back(this);
        g_oGrabberThreadMutex.unlock();

        m_poFrameQueue = new DCFrameQueue(c,options);
      }

      // }}}

      void DCGrabberThread::run(){
        // {{{ open

        // I moved this to the constructor (why was it placed here?)
        //if(!m_poFrameQueue){
        // m_poFrameQueue = new DCFrameQueue(m_poCam, m_poOptions);
        //}
        while(true){
          if(!trylock()){
            m_poFrameQueue->step();
            unlock();
            msleep(1);
          } else {
            msleep(5);
          }
        }
      }

      // }}}


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

        // {{{ open
        while(!m_poFrameQueue) Thread::msleep(10);

        m_poFrameQueue->lock();

        extract_image_to_2(waitForNextImageFrame(),bayerLayout,ppoDst,m_oRGBInterleavedBuffer,bayerMethod);

        m_poFrameQueue->unlock();


      }
      // }}}


      void DCGrabberThread::getCurrentImage(ImgBase **ppoDst,
                                            ImgBase **ppoDstTmp,
                                            bool &desiredParamsFullfilled,
                                            const Size &desiredSizeHint,
                                            format desiredFormatHint,
                                            depth desiredDepthHint,
                                            dc1394color_filter_t bayerLayout,
                                            dc1394bayer_method_t bayerMethod){
        // {{{ open

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

      // }}}
    }
  } // namespace io
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/PylonGrabberThread.cpp                       **
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

#include <pylon/PylonIncludes.h>

#include <ICLIO/PylonGrabberThread.h>

using namespace icl;
using namespace icl::pylon;

PylonGrabberThread::PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                              Mutex* camMutex, int bufferCount, int bufferSize)
  : m_Grabber(grabber), m_CamMutex(camMutex), m_BufferMutex(),
    m_BufferSize(bufferSize), m_BufferCount(bufferCount), m_BufferQueue(),
    m_Error(0), m_Timeout(0), m_Acquired(0), m_NewAvail(false)
{  
  // init Buffers
  pos=-1;
  runb = true;
  icl::Mutex::Locker l(m_BufferMutex);
  initBuffer();
  DEBUG_LOG("buffermutex = " << &m_BufferMutex)
  DEBUG_LOG("this pointer " << this)
}

PylonGrabberThread::~PylonGrabberThread(){
  // free all allocated memory
  icl::Mutex::Locker l(m_BufferMutex);
  clearBuffer();

  DEBUG_LOG("pictures aquired: " << m_Acquired << " errors: " << m_Error
            << " timesouts: " << m_Timeout)
}

void PylonGrabberThread::resetBuffer(int bufferSize, int bufferCount){
  DEBUG_LOG("Reset Buffer " << bufferSize << " , " << bufferCount)
  // set members to new values
  m_BufferSize = bufferSize;
  m_BufferCount = bufferCount;
  // reallocate buffers
  clearBuffer();
  initBuffer();
}

void PylonGrabberThread::initBuffer(){
  DEBUG_LOG("initBuffer. n=" << m_BufferCount)
  if(!m_BufferQueue.empty()){
    throw ICLException("m_BufferQueue must be empty when calling init");
  }
  // fill buffer-queue with new buffers.
  for (int i = 0; i < m_BufferCount; ++i){
    TsBuffer<int16_t>* buffer = new TsBuffer<int16_t>(m_BufferSize);
    DEBUG_LOG("BUFFER: " << buffer);
    m_BufferQueue.push(buffer);
  }
  DEBUG_LOG("buffer size" << m_BufferQueue.size())
  m_NewAvail = false;
}

void PylonGrabberThread::clearBuffer(){
  // pop and delete all buffers from queue.
  while (!m_BufferQueue.empty()){
    TsBuffer<int16_t>* tmp = m_BufferQueue.front();
    delete tmp;
    m_BufferQueue.pop();
  }
}

void PylonGrabberThread::run(){
  // non-stop-loop
  while(1){
    pos = 1;
    // to prevent deadlocks always get the camera mutex before locking thread
    int count = 0;
    while(m_CamMutex -> trylock()){
      ++count;
      msleep(1);
    }
    //if(count > 0){
    //    DEBUG_LOG("count = " << count)
    //}
    pos = 2;
    int trycount = 0;
    while(trylock() && trycount < 1000){
        ++trycount;
    }
    if(trycount>=1000){
        m_CamMutex->unlock();
        msleep(1);
    } else {
        pos = 3;
        // grab image.
        grab();
        pos = 4;
        // release camera lock before unlocking thread.
        pos = 5;
        // allow thread-stop.
        m_CamMutex -> unlock();
        unlock();
        //std::cout << "unlocked" << std::endl;
        //std::cerr << "unlocked" << std::endl;
        pos = 6;
        //std::cout << "unl2" << std::endl;
        //std::cerr << "unl2" << std::endl;
        // short sleep so thread can be stopped.
        //std::cout << "sleep " << this << std::endl;
        //std::cerr << "sleep " << this << std::endl;
        msleep(1);
        pos = 7;
        }
  }
}

void PylonGrabberThread::grab(){
  // Wait for the grabbed image with timeout of 2 seconds
  pos = 8;
  if (!m_Grabber -> GetWaitObject().Wait(2000)){
  pos = 9;
    // Timeout
    //DEBUG_LOG("Timeout occurred!")
    ++m_Timeout;
    return;
  }
  pos = 10;
  // Get the grab result from the grabber's result queue
  Pylon::GrabResult result;
  pos = 11;
  if(!m_Grabber -> RetrieveResult(result)){
      //This should not happen, but seems to do on camemu.
      //DEBUG_LOG("Wait object came back but no result available.")
      ++m_Error;
      return;
  }
  pos = 12;
  if (result.Succeeded()){
    pos = 13;
    ++m_Acquired;
    pos = 14;
    // Grabbing was successful, save the buffer content to queue
    m_BufferMutex.lock();
    pos = 15;
    TsBuffer<int16_t>* tmp = m_BufferQueue.front();
    pos = 16;
    m_BufferQueue.pop();
    pos = 17;
    tmp -> copy(result.Buffer());
    pos = 18;
    tmp -> m_Timestamp = result.GetTimeStamp();
    pos = 19;
    m_BufferQueue.push(tmp);
    pos = 20;
    m_NewAvail = true;
    pos = 21;
    // Reuse buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
    pos = 22;
    m_BufferMutex.unlock();
    pos = 13;
  } else {
    ++m_Error;
    // Error handling
    //DEBUG_LOG("No image acquired!" << "Error description : "
    //<< result.GetErrorDescription())
    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
  }
}

TsBuffer<int16_t>* PylonGrabberThread::getCurrentImage(){
  for (int n = 0; n < 20; ++n){
    // lock buffer-queue
    m_BufferMutex.lock();
    if(m_NewAvail){
      TsBuffer<int16_t>* tmp = m_BufferQueue.back();
      m_BufferMutex.unlock();
      m_NewAvail = false;;
      return tmp;
    }
    m_BufferMutex.unlock();
    //DEBUG_LOG("wait for new image")
    msleep(10);
  }
  //DEBUG_LOG("could not get new image, return null");
  return NULL;
}

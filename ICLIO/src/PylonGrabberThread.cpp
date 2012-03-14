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
  : m_BufferMutex(), m_Grabber(grabber), m_CamMutex(camMutex),
    m_BufferSize(bufferSize), m_BufferCount(bufferCount),
    m_Error(0), m_Timeout(0), m_Acquired(0), m_BufferQueue(),
    m_NewAvail(false)
{  
  // init Buffers
  icl::Mutex::Locker l(m_BufferMutex);
  initBuffer();
}

PylonGrabberThread::~PylonGrabberThread(){
  // free all allocated memory
  icl::Mutex::Locker l(m_BufferMutex);
  clearBuffer();
  DEBUG_LOG("pictures aquired: " << m_Acquired << " errors: " << m_Error
            << " timesouts: " << m_Timeout)
}

void PylonGrabberThread::resetBuffer(int bufferSize, int bufferCount){
  FUNCTION_LOG("Reset Buffer " << bufferSize << " , " << bufferCount)
  // set members to new values
  m_BufferSize = bufferSize;
  m_BufferCount = bufferCount;
  // reallocate buffers
  clearBuffer();
  initBuffer();
}

void PylonGrabberThread::initBuffer(){
  if(!m_BufferQueue.empty()){
    throw ICLException("m_BufferQueue must be empty when calling init");
  }
  // fill buffer-queue with new buffers.
  for (int i = 0; i < m_BufferCount; ++i){
    TsBuffer<int16_t>* buffer = new TsBuffer<int16_t>(m_BufferSize);
    m_BufferQueue.push(buffer);
  }
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
    msleep(1);
    // to prevent deadlocks always get the camera mutex before locking thread
    if(m_CamMutex -> trylock()){
      // did not get lock
      continue;
    }
    // camera-lock acquired. locking thread
    if(trylock()){
      // did not get thread lock. release cam.
      m_CamMutex -> unlock();
      continue;
    }
    // here we have both locks (cam and thread)
    // grab image.
    grab();
    // release camera lock before unlocking thread.
    m_CamMutex -> unlock();
    // allow thread-stop.
    unlock();
  }
}

void PylonGrabberThread::grab(){
  // Wait for the grabbed image with timeout of 2 seconds
  if (!m_Grabber -> GetWaitObject().Wait(2000)){
    // Timeout
    DEBUG_LOG("Timeout occurred!")
    ++m_Timeout;
    return;
  }
  // Get the grab result from the grabber's result queue
  Pylon::GrabResult result;
  if(!m_Grabber -> RetrieveResult(result)){
      //This should not happen, but seems to do on camemu.
      DEBUG_LOG("Wait object came back but no result available.")
      ++m_Error;
      return;
  }
  if (result.Succeeded()){
    ++m_Acquired;
    // Grabbing was successful, save the buffer content to queue
    m_BufferMutex.lock();
    TsBuffer<int16_t>* tmp = m_BufferQueue.front();
    m_BufferQueue.pop();
    tmp -> copy(result.Buffer());
    tmp -> m_Timestamp = result.GetTimeStamp();
    m_BufferQueue.push(tmp);
    m_NewAvail = true;
    // Reuse buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
    m_BufferMutex.unlock();
  } else {
    ++m_Error;
    // Error handling
    DEBUG_LOG("No image acquired!" << "Error description : "
    << result.GetErrorDescription())
    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
  }
}

TsBuffer<int16_t>* PylonGrabberThread::getCurrentImage(){
  for (int n = 0; n < 100; ++n){
    // lock buffer-queue
    m_BufferMutex.lock();
    if(m_NewAvail){
      TsBuffer<int16_t>* tmp = m_BufferQueue.back();
      m_BufferMutex.unlock();
      m_NewAvail = false;;
      return tmp;
    }
    m_BufferMutex.unlock();
    msleep(10);
  }
  return NULL;
}

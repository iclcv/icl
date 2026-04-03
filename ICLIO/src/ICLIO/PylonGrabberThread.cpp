// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <ICLIO/PylonGrabberThread.h>
#include <ICLUtils/Time.h>

using namespace icl;
using namespace icl::io::pylon;

// Constructor sets all internal fields and allocates memory
PylonGrabberThread::PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                                PylonColorConverter* converter,
                                PylonCameraOptions* options) :
m_Grabber(grabber), m_Buffers(),
    m_Error(0), m_Timeout(0), m_Acquired(0)
{
  m_Converter = converter;
  m_Options = options;
}

PylonGrabberThread::~PylonGrabberThread(){
  // free all allocated memory
  DEBUG_LOG("Images aquired: " << m_Acquired << " Errors: " << m_Error
            << " Timesouts: " << m_Timeout)
}

void PylonGrabberThread::resetBuffer(){
  m_Buffers.setReset();
}

void PylonGrabberThread::run(){
  while(running()){
    msleep(1);
    // locking thread
    if(trylock()) {
      DEBUG_LOG("threadlock returned error. sleep and retry.");
      continue;
    }
    //thread locked grab image.
    grab();
    // allow thread-stop.
    unlock();
  }
}

void PylonGrabberThread::grab(){
  // Wait for the grabbed image with timeout of 2 seconds
  if (!m_Grabber -> GetWaitObject().Wait(1000)){
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
    // Grabbing was successful, convert and save
    ConvBuffers* write = m_Buffers.getNextWriteBuffer();
    m_Converter -> convert(result.Buffer(), write);
    if(result.GetTimeStamp()){
      write -> m_Image -> setTime(result.GetTimeStamp());
    } else {
      write -> m_Image -> setTime();
    }
    // Reuse buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), nullptr);
  } else {
    ++m_Error;
    // Error handling
    DEBUG_LOG("No image acquired! Error description : "
              << result.GetErrorDescription())

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), nullptr);
  }
}

core::ImgBase* PylonGrabberThread::getCurrentDisplay(){
  // just return the buffered readimage.
  return m_Buffers.getNextReadBuffer() -> m_Image;
}

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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
  // non-stop-loop
  while(1){
    msleep(1);
    // locking thread
    if(trylock()) {
      DEBUG_LOG2("threadlock returned error. sleep and retry.");
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
    DEBUG_LOG2("Timeout occurred!")
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
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
  } else {
    ++m_Error;
    // Error handling
    DEBUG_LOG("No image acquired! Error description : "
              << result.GetErrorDescription())

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
  }
}

core::ImgBase* PylonGrabberThread::getCurrentImage(){
  // just return the buffered readimage.
  return m_Buffers.getNextReadBuffer() -> m_Image;
}

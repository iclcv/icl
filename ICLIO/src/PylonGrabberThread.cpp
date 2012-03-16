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

PylonGrabberThread::PylonGrabberThread(
Pylon::IStreamGrabber* grabber, int bufferSize) :
m_Grabber(grabber), m_Buffers(bufferSize), m_Error(0), m_Timeout(0),
    m_Acquired(0)
{  
  // nothing to do.
}

PylonGrabberThread::~PylonGrabberThread(){
  // free all allocated memory
  DEBUG_LOG("pictures aquired: " << m_Acquired << " errors: " << m_Error
            << " timesouts: " << m_Timeout)
}

void PylonGrabberThread::resetBuffer(int bufferSize){
  FUNCTION_LOG("Reset Buffer " << bufferSize)
  // reset buffer size
  m_Buffers.reset(bufferSize);
}

void PylonGrabberThread::run(){
  // non-stop-loop
  while(1){
    msleep(1);
    // locking thread
    if(trylock()) continue;
    //thread locked grab image.
    grab();
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
    TsBuffer<int16_t>* write = m_Buffers.getNextBuffer();
    write -> copy(result.Buffer());
    write -> m_Timestamp = result.GetTimeStamp();
    // Reuse buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
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
  // just return the buffered readimage.
  return m_Buffers.getNextImage();
}

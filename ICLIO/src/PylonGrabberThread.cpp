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

#include <pylon/PylonIncludes.h>

#include <ICLIO/PylonGrabberThread.h>
#include <ICLUtils/Time.h>

int iter = 0;

using namespace icl;
using namespace icl::pylon;

// Constructor sets all internal fields and allocates memory
PylonGrabberThread::PylonGrabberThread(Pylon::IStreamGrabber* grabber,
                                PylonColorConverter* converter,
                                PylonCameraOptions* options) :
m_Grabber(grabber), m_Buffers(),
    m_Error(0), m_Timeout(0), m_Acquired(0), m_ResultingFramerate(0.0)
{  
  m_Converter = converter;
  m_Options = options;
}

PylonGrabberThread::~PylonGrabberThread(){
  // free all allocated memory
  DEBUG_LOG("pictures aquired: " << m_Acquired << " errors: " << m_Error
            << " timesouts: " << m_Timeout)
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

      std::cout << "\nthreadlock returned error\n" << std::endl;
      continue;
    }
    //thread locked grab image.
    grab();
    // allow thread-stop.
    unlock();
  }
}

void PylonGrabberThread::grab(){
  if(m_ResultingFramerate == 0.0){
  msleep(2000);
  }
  Time t = Time::now();
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
    //std::cout << "waited for buffer: " << t.age() << std::endl;
    ++m_Acquired;
    // Grabbing was successful, convert and save
    ConvBuffers* write = m_Buffers.getNextWriteBuffer();
    //Time ct = Time::now();
    m_Converter -> convert(result.Buffer(), write);
    //++iter;
    //std::cout << iter << "\t" << ct.age() << std::endl;
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
    std::cout << "No image acquired! (waited " << t.age() << ")"
    << "Error description : " << result.GetErrorDescription() << std::endl;

    // Reuse the buffer for grabbing the next image
    m_Grabber -> QueueBuffer(result.Handle(), NULL);
    // make grabber reset framerate.
    m_ResultingFramerate = 0.0;
  }
  if(m_ResultingFramerate == 0.0){
    m_ResultingFramerate = m_Options -> getResultingFrameRateAbs();
    //DEBUG_LOG("FramerateAbs updated to: " << m_ResultingFramerate)
  }

  double looptime = 1000.0 / m_ResultingFramerate;
  double sleeptime = looptime - (t.age()).toMilliSecondsDouble();
  if(sleeptime > 0) {
    //std::cout << "age ~ " << t.age().toMilliSecondsDouble() << " sleeping: " << sleeptime << "(" << (int) (sleeptime + 1.0)<< ")" << " looptime: " << looptime << std::endl;
    msleep((int) (sleeptime + 1));
  }
}

ImgBase* PylonGrabberThread::getCurrentImage(){
  // just return the buffered readimage.
  return m_Buffers.getNextReadBuffer() -> m_Image;
}

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLIO/DefaultGrabEngine.h>
#include <ICLIO/UnicapDevice.h>

namespace icl{
  DefaultGrabEngine::DefaultGrabEngine(UnicapDevice *device, bool useDMA, bool progressiveGrabMode):
    m_poDevice(device), m_iCurrBuf(0), m_bUseDMA(useDMA), m_bStarted(false), m_bProgressiveGrabMode(progressiveGrabMode){
    
    UnicapFormat fmt = m_poDevice->getCurrentUnicapFormat();
    int bufferSize = fmt.getBufferSize() ? fmt.getBufferSize() : fmt.getSize().getDim()*4; // just something large here
    for(int i=0;i<NBUFS;i++){
      m_oBuf[i].buffer_size = useDMA ? 0 : bufferSize;
      m_oBuf[i].data = useDMA ? 0 :new unsigned char[bufferSize];
      if(m_oBuf[i].data) *m_oBuf[i].data = 0; // hack for detection of unfinished motion-jpeg stream
      m_oBuf[i].type = useDMA ? UNICAP_BUFFER_TYPE_SYSTEM : UNICAP_BUFFER_TYPE_USER;
      fmt.getUnicapFormat()->buffer_type = useDMA ? UNICAP_BUFFER_TYPE_SYSTEM : UNICAP_BUFFER_TYPE_USER;
    }
    device->setFormat(fmt);
  }
  DefaultGrabEngine::~DefaultGrabEngine(){
    unicap_stop_capture (m_poDevice->getUnicapHandle()); 
    for(int i=0;i<NBUFS;i++){
      if(m_oBuf[i].type == UNICAP_BUFFER_TYPE_USER && m_oBuf[i].data){
        delete [] m_oBuf[i].data;
      }
    }    
  }
  
  void DefaultGrabEngine::lockGrabber(){ 
    // no longer needed because of using double buffering
  }
  void DefaultGrabEngine::unlockGrabber(){ 
    // no longer needed because of using double buffering
  }
  
  const icl8u *DefaultGrabEngine::getCurrentFrameUnconverted(){
    
    if(m_bProgressiveGrabMode){
      if(!m_bStarted){

        unicap_start_capture(m_poDevice->getUnicapHandle());

        m_bStarted = true;

        unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuf[NEXT_IDX()]);
      }

      unicap_data_buffer_t *returned_buffer;
      static const int MAX_TRYS = 10;
      static const long WAIT_TIME = 100000;

      int i=0;

      int success = 0;
      while(!success){
        if( !SUCCESS (unicap_poll_buffer(m_poDevice->getUnicapHandle(),&success))){
          ERROR_LOG("Failed to wait for the buffer to be filled! ( POLL Section)<<");
        }
        usleep(1000);
      }
      
      for(;i<MAX_TRYS;i++){
        usleep(1000);
        if( SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer))){
          break;
        }else{
          usleep(WAIT_TIME);
        }

      }if(i==MAX_TRYS){
        ERROR_LOG("Failed to wait for the buffer to be filled! ( tried "<<MAX_TRYS<<" times)");        
      }

      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuf[NEXT_IDX()]);

      return returned_buffer->data;
    }else{
      if(!m_bStarted){
        unicap_start_capture(m_poDevice->getUnicapHandle());
        m_bStarted = true;
      }
      
      unicap_data_buffer_t *returned_buffer;
      static const int MAX_TRYS = 10;
      static const long WAIT_TIME = 100000;
      int i=0;
      for(;i<MAX_TRYS;i++){
        unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuf[NEXT_IDX()]);
        usleep(1000);
        if( SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer))){
          break;
        }else{
          usleep(WAIT_TIME);
        }
        
      }if(i==MAX_TRYS){
        ERROR_LOG("Failed to wait for the buffer to be filled! ( tried "<<MAX_TRYS<<" times)");        
      }
      
      return returned_buffer->data;
    }
  }
}

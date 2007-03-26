#include <iclUnicapGrabEngine.h>
#include <iclUnicapDevice.h>
#include <iclStackTimer.h>
#include "iclUnicapBuffer.h"

namespace icl{
  UnicapGrabEngine::UnicapGrabEngine(UnicapDevice *device, bool useDMA):
    m_poDevice(device), m_iCurrBuf(0), m_bUseDMA(useDMA), m_bStarted(false){
    
    UnicapFormat fmt = m_poDevice->getCurrentUnicapFormat();
    for(int i=0;i<NBUFS;i++){
      m_oBuf[i].buffer_size = useDMA ? 0 : fmt.getBufferSize();
      m_oBuf[i].data = useDMA ? 0 :new unsigned char[fmt.getBufferSize()];
      m_oBuf[i].type = useDMA ? UNICAP_BUFFER_TYPE_SYSTEM : UNICAP_BUFFER_TYPE_USER;
      fmt.getUnicapFormat()->buffer_type = useDMA ? UNICAP_BUFFER_TYPE_SYSTEM : UNICAP_BUFFER_TYPE_USER;
    }
    device->setFormat(fmt);
  }
  UnicapGrabEngine::~UnicapGrabEngine(){
    unicap_stop_capture (m_poDevice->getUnicapHandle()); 
    for(int i=0;i<NBUFS;i++){
      if(m_oBuf[i].type == UNICAP_BUFFER_TYPE_USER && m_oBuf[i].data){
        delete [] m_oBuf[i].data;
      }
    }    
  }
  
  void UnicapGrabEngine::lockGrabber(){ 
    // no longer needed because of using double buffering
  }
  void UnicapGrabEngine::unlockGrabber(){ 
    // no longer needed because of using double buffering
  }
  
  /*
      if(!m_bStarted){
      unicap_start_capture(m_poDevice->getUnicapHandle());
        m_bStarted = true;
      }
      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);    
      unicap_data_buffer_t *returned_buffer;

      if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
        ERROR_LOG("Failed to wait for the buffer to be filled!");
      }
      printf("bufa=%p bufb_%p \n",(void*)&m_oBuffer,(void*)returned_buffer);
      return m_oBuffer.data;
      */

  const icl8u *UnicapGrabEngine::getCurrentFrameUnconverted(){
    BENCHMARK_THIS_FUNCTION;
    
    if(!m_bStarted){
      unicap_start_capture(m_poDevice->getUnicapHandle());
      m_bStarted = true;
      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuf[NEXT_IDX()]);
    }
    
    unicap_data_buffer_t *returned_buffer;
    if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
      ERROR_LOG("Failed to wait for the buffer to be filled!");
    }
    printf("in UnicapGrabEngine::getCurrentFrameUnconverted(): buffer filled : idx=%d %p \n",m_iCurrBuf,(void*)returned_buffer);
    
    unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuf[NEXT_IDX()]);
        
    return returned_buffer->data;
  }
}

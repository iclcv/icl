#include <iclUnicapGrabEngine.h>
#include <iclUnicapDevice.h>
#include <iclStackTimer.h>
#include "iclUnicapBuffer.h"

namespace icl{
  UnicapGrabEngine::UnicapGrabEngine(UnicapDevice *device, bool useDMA):m_poDevice(device),m_bUseDMA(useDMA), m_bStarted(false){
    m_oBuffer.buffer_size = 0;
    m_oBuffer.data = 0;
    m_oBuffer.type = UNICAP_BUFFER_TYPE_USER;
    setupUseDMA(useDMA);
    
    UnicapFormat UF = m_poDevice->getCurrentUnicapFormat();
    if(m_oBuffer.buffer_size != UF.getBufferSize()){
      if(m_oBuffer.data) delete m_oBuffer.data;
      m_oBuffer.data = new unsigned char[UF.getBufferSize()];
      m_oBuffer.buffer_size = UF.getBufferSize();
    }

    m_poDMABuffer = new UnicapBuffer();
  }
  UnicapGrabEngine::~UnicapGrabEngine(){
    unicap_stop_capture (m_poDevice->getUnicapHandle()); 
    /// free the buffer e.t.c
    /// usw ...

    if(m_poDMABuffer) delete m_poDMABuffer;
  }
  
  void UnicapGrabEngine::setupUseDMA(bool useDMA){
    if(useDMA){
      m_bUseDMA = true;
      UnicapFormat UF = m_poDevice->getCurrentUnicapFormat();
      unicap_format_t *uf = UF.getUnicapFormat();
      uf->buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
      m_poDevice->setFormat(UF);
    }else{
      m_bUseDMA = false;
      UnicapFormat UF = m_poDevice->getCurrentUnicapFormat();
      unicap_format_t *uf = UF.getUnicapFormat();
      uf->buffer_type = UNICAP_BUFFER_TYPE_USER;
      m_poDevice->setFormat(UF);
    }
  }
  
  void UnicapGrabEngine::setGrabbingParameters(const std::string &params){
  }
  
  void UnicapGrabEngine::lockGrabber(){
    if(m_bUseDMA){
      m_poDMABuffer->lock();
    } 
  }
  void UnicapGrabEngine::unlockGrabber(){
    if(m_bUseDMA){
      m_poDMABuffer->unlock();
    }    
  }
  namespace{
    void dma_callback(unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t *buffer, void *usr_data){
      printf("callback was called \n");
      UnicapBuffer* b = (UnicapBuffer*)usr_data;
      b->lock();
      b->resize(buffer->buffer_size);
      b->fillFrom(buffer->data);
      b->unlock();
    }    
  }

  const icl8u *UnicapGrabEngine::getCurrentFrameUnconverted(){
    BENCHMARK_THIS_FUNCTION;
    if(!m_bUseDMA){      
      if(!m_bStarted){
        unicap_start_capture(m_poDevice->getUnicapHandle());
        m_bStarted = true;
      }
      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);    
      unicap_data_buffer_t *returned_buffer;
      if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
        ERROR_LOG("Failed to wait for the buffer to be filled!");
      }
      return m_oBuffer.data;
    }else{
      printf("grab was called \n");
      if(!m_bStarted){
        printf("registering callback! \n");
        unicap_register_callback(m_poDevice->getUnicapHandle(), 
                                 UNICAP_EVENT_NEW_FRAME, 
                                 (unicap_callback_t)dma_callback,
                                 (void*)m_poDMABuffer); 
        printf("starting to capture  \n");
        unicap_start_capture (m_poDevice->getUnicapHandle());   
        
        while(m_poDMABuffer->size() != m_oBuffer.buffer_size){
          printf("waiting ... \n");
          m_poDMABuffer->unlock();
          usleep(1000000);
          m_poDMABuffer->lock();
        }
        
        printf("end \n");
      }
      printf("hallo buffer is %p \n",m_poDMABuffer);
      return m_poDMABuffer->data();
      
    }
  }
}

#include <iclUnicapGrabEngine.h>
#include <iclUnicapDevice.h>
#include <iclStackTimer.h>
#include "iclUnicapBuffer.h"

namespace icl{
  UnicapGrabEngine::UnicapGrabEngine(UnicapDevice *device, bool useDMA):m_poDevice(device),m_bUseDMA(useDMA), m_bStarted(false){
    m_oBuffer.buffer_size = 0;
    m_oBuffer.data = 0;

    //!!!!!
    //useDMA = 1;    

    m_oBuffer.type = useDMA ? UNICAP_BUFFER_TYPE_SYSTEM : UNICAP_BUFFER_TYPE_USER;
    
    // if(useDMA){
    //  ERROR_LOG("DMA is not yet supported and will be deactivated!");
    //  useDMA = false;
    //}
    

    setupUseDMA(useDMA);
    
    UnicapFormat UF = m_poDevice->getCurrentUnicapFormat();

    if(!useDMA){
      if(m_oBuffer.buffer_size != UF.getBufferSize()){
        if(m_oBuffer.data) delete m_oBuffer.data;
        m_oBuffer.data = new unsigned char[UF.getBufferSize()];
        m_oBuffer.buffer_size = UF.getBufferSize();
      }
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
  
  void UnicapGrabEngine::lockGrabber(){
    if(m_bUseDMA){
      usleep(100000);
      //m_poDMABuffer->lock();
    } else{
      // unnecessary lock();
    }
  }
  void UnicapGrabEngine::unlockGrabber(){
    if(m_bUseDMA){
      //m_poDMABuffer->unlock();
    }else{
      // unnecessary unlock();
    }
  }
  namespace{
    void dma_callback(unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t *buffer, void *usr_data){
      BENCHMARK_THIS_FUNCTION;
      printf("callback!\n");
      UnicapBuffer* b = (UnicapBuffer*)usr_data;
      b->lock();
      b->resize(buffer->buffer_size);
      b->fillFrom(buffer->data);
      b->unlock();
    }    
  }

  void UnicapGrabEngine::run(){
    while(1){
      /*********************
      BENCHMARK_THIS_FUNCTION;
      printf("in run!");
      lock();
      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);    
      unicap_data_buffer_t *returned_buffer;
      if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
        ERROR_LOG("Failed to wait for the buffer to be filled!");
      }
      unlock();
      msleep(10);
      ***********************/
    }  
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
    //if(!m_bUseDMA){    
      /************************************ this works! **/
      if(!m_bStarted){
        unicap_start_capture(m_poDevice->getUnicapHandle());
        m_bStarted = true;
        unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);
      }
      //   unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);    
      unicap_data_buffer_t *returned_buffer;

      if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
        ERROR_LOG("Failed to wait for the buffer to be filled!");
      }
      unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);
      printf("bufa=%p bufb_%p \n",(void*)&m_oBuffer,(void*)returned_buffer);
      return m_oBuffer.data;
      //}else{
      //if(!m_bStarted){
        

        /******************************************************
            unicap_register_callback(m_poDevice->getUnicapHandle(), 
            UNICAP_EVENT_NEW_FRAME, 
            (unicap_callback_t)dma_callback,
            (void*)m_poDMABuffer); 
            unicap_start_capture (m_poDevice->getUnicapHandle());   
            while(m_poDMABuffer->size() != m_oBuffer.buffer_size){
            m_poDMABuffer->unlock();
            usleep(1000000);
            m_poDMABuffer->lock();
            }
       **************************************************/
      //}
      //return m_poDMABuffer->data();
      //}
  }
}

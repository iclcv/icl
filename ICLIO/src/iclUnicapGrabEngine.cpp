#include <iclUnicapGrabEngine.h>
#include <iclUnicapDevice.h>

namespace icl{
  UnicapGrabEngine::UnicapGrabEngine(UnicapDevice *device):m_poDevice(device),m_bUseDMA(true){
    m_oBuffer.buffer_size = 0;
    m_oBuffer.data = 0;
    m_oBuffer.type = UNICAP_BUFFER_TYPE_USER;
  }
  
  void UnicapGrabEngine::setGrabbingParameters(const std::string &params){
    //    m_bUseDMA = true;
  }
  
  void UnicapGrabEngine::lockGrabber(){
  
  }
  void UnicapGrabEngine::unlockGrabber(){
  
  }
  const icl8u *UnicapGrabEngine::getCurrentFrameUnconverted(){
    //    vector<ImgBase*> vec;
    // unicap_format_t format;
    //unicap_data_buffer_t buffer;
    //int cframe = 0;
    printf("1 device is %p \n",(void*)m_poDevice);
    printf("device: %s \n",m_poDevice->toString().c_str());
    UnicapFormat UF = m_poDevice->getCurrentUnicapFormat();
    printf("2 \n");
    
    unicap_format_t *uf = UF.getUnicapFormat();
    uf->buffer_type = UNICAP_BUFFER_TYPE_USER;
    printf("3 \n");
    
    m_poDevice->setFormat(UF);
    printf("4 \n");
    
    /**
        if (!SUCCESS (unicap_get_format (handle, &format))){
        fprintf (stderr, "Failed to get video format!\n");
        exit (-1);
        }
  
        format.buffer_type = UNICAP_BUFFER_TYPE_USER; // (1)
        
        if (!SUCCESS (unicap_set_format (handle, &format))) {
        fprintf (stderr, "Failed to set video format!\n");
        exit (-1);
        }
    **/
    if(m_oBuffer.buffer_size != UF.getBufferSize()){
      if(m_oBuffer.data) delete m_oBuffer.data;
      m_oBuffer.data = new unsigned char[UF.getBufferSize()];
      m_oBuffer.buffer_size = UF.getBufferSize();
    }
        
    unicap_start_capture(m_poDevice->getUnicapHandle());
    unicap_queue_buffer(m_poDevice->getUnicapHandle(),&m_oBuffer);
    //    unicap_start_capture (handle);        // (4)
    //unicap_queue_buffer (handle, &buffer);        // (3)
    
    unicap_data_buffer_t *returned_buffer;
    if( !SUCCESS (unicap_wait_buffer (m_poDevice->getUnicapHandle(), &returned_buffer)))  {
      ERROR_LOG("Failed to wait for the buffer to be filled!");
    }
    unicap_stop_capture (m_poDevice->getUnicapHandle()); // (6)    
    return m_oBuffer.data;

    /*
        while (cframe < nframes) {
        unicap_data_buffer_t *returned_buffer;
        if (!SUCCESS (unicap_wait_buffer (handle, &returned_buffer)))   {
        fprintf (stderr, "Failed to wait for buffer!\n");
        exit (-1);
        }
    */
    /*****************************
        Img8u *imRGB = new Img8u(Size(640,480),formatGray);
        copy(buffer.data,buffer.data+imRGB->getDim(),imRGB->getData(0));
        vec.push_back(imRGB);
    *******************************/
    
  }
}

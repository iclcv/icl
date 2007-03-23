#ifndef ICL_UNICAP_BUFFER
#define ICL_UNICAP_BUFFER

#include <pthread.h>
#include <iclTypes.h>
#include <string>

namespace icl{
  struct UnicapBuffer{
    UnicapBuffer(unsigned int size=1) : m_uiSize(size){
      if(!size) ERROR_LOG("Buffer size must be > 0 !!");
      pthread_mutex_init(&m_oMutex,0);
      m_pucData = new icl8u[size];
    }
    ~UnicapBuffer(){
      if(m_pucData) delete m_pucData;
    }
    void resize(unsigned int size){
      if(m_uiSize != size){
        delete m_pucData;
        m_pucData = new icl8u[size];
        m_uiSize = size;
      }
      printf("ICLUNICAPBUFFER data ptr = %p \n",m_pucData);
    }
    
    void fillFrom(void *data) { memcpy(m_pucData,data,m_uiSize); }
    icl8u *data() { return m_pucData; }
    unsigned int size() { return m_uiSize; }
    inline void lock(){ pthread_mutex_lock(&m_oMutex); }
    inline void unlock(){ pthread_mutex_unlock(&m_oMutex); }
    private:
    pthread_mutex_t m_oMutex;
    unsigned int m_uiSize;
    icl8u *m_pucData;
  };
}
  
#endif

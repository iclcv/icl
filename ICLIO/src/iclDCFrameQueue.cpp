#include "iclDCFrameQueue.h"
#include "iclDC.h"
namespace icl{
  namespace dc{
    
    DCFrameQueue::DCFrameQueue(dc1394camera_t* c, DCDeviceOptions *options,int nDMABuffers, int nQueuedBuffers):
      m_poCam(c),
      m_iBuffers(nDMABuffers),
      m_iQueuedBuffers(nQueuedBuffers)
    {

      initialize_dc_cam(c,nDMABuffers, options);
      
      /// dequeu all frames once 
#ifdef SYSTEM_APPLE
      for(int i=0;i<m_iBuffers-1;i++){
#else
      for(int i=0;i<m_iBuffers;i++){
#endif
        dc1394video_frame_t *frame;
        dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
        push(frame);
      }
      for(int i=0;i<m_iQueuedBuffers;i++){
        dc1394video_frame_t *frame = pop();
        dc1394_capture_enqueue(m_poCam,frame); 
      }    
    }
    DCFrameQueue::~DCFrameQueue(){
      //      release_dc_cam(m_poCam);
    }
    
    void DCFrameQueue::step(){
      dc1394video_frame_t *frame = pop();
      dc1394_capture_enqueue(m_poCam,frame); 
      
      frame=0;
      // OLD      dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
      dc1394error_t err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_POLL,&frame);
      // OLD RC7:     while(err !=  DC1394_SUCCESS ){ // sometimes frame was NULL but err was SUCCESS
      while(err !=  DC1394_SUCCESS || !frame){
        usleep(0);
        err = dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_POLL,&frame);
      }
      push(frame);
    }    
    
    void DCFrameQueue::push(dc1394video_frame_t *f){
      lock();
      q.push(f);
      unlock();
    }
    
    dc1394video_frame_t *DCFrameQueue::pop(){
      lock();
        dc1394video_frame_t *f = q.front();
        q.pop();
        unlock();
        return f;
      }
      
      
      void DCFrameQueue::showDetails() const{
        dc1394video_mode_t vm;
        dc1394_video_get_mode(m_poCam,&vm);
        printf("using video mode %s \n",to_string(vm).c_str());
        
        dc1394framerate_t fr;
        dc1394_video_get_framerate(m_poCam,&fr);
        printf("using framerate %s \n",to_string(fr).c_str());
      }


  }
}

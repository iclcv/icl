#include "iclDCFrameQueue.h"

namespace icl{
  namespace dc{
    
    DCFrameQueue::DCFrameQueue(dc1394camera_t* c,int nDMABuffers, int nQueuedBuffers):
      m_poCam(c),
      m_iBuffers(nDMABuffers),
      m_iQueuedBuffers(nQueuedBuffers)
    {
      
      dc1394_capture_stop(m_poCam);
      set_streaming(m_poCam,false);
      dc1394_cleanup_iso_channels_and_bandwidth(m_poCam);
      
      dc1394_video_set_iso_speed(m_poCam,DC1394_ISO_SPEED_400);
      dc1394_video_set_mode(m_poCam,DC1394_VIDEO_MODE_640x480_MONO8);
      dc1394_video_set_framerate(m_poCam, DC1394_FRAMERATE_60);
      
      dc1394_capture_setup(m_poCam,m_iBuffers,DC1394_CAPTURE_FLAGS_DEFAULT);
      set_streaming(m_poCam,true);
      
      /// dequeu all frames once 
      for(int i=0;i<m_iBuffers;i++){
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
      set_streaming(m_poCam,false);
      dc1394_capture_stop(m_poCam);
      dc1394_cleanup_iso_channels_and_bandwidth(m_poCam);
    }
    
    void DCFrameQueue::step(){
      dc1394video_frame_t *frame = pop();
      dc1394_capture_enqueue(m_poCam,frame); 
      
      frame=0;
      dc1394_capture_dequeue(m_poCam,DC1394_CAPTURE_POLICY_WAIT,&frame);
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

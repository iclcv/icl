#include <dc1394/control.h>
// #include <dc1394/conversions.h>

#include <iclTypes.h>
#include <string>
#include <vector>


namespace icl{
  
  class ImgBase;
  
  namespace dc{
    std::string to_string(dc1394video_mode_t vm);
    std::string to_string(dc1394framerate_t fr);
    std::string to_string(dc1394color_filter_t f);
    
    
    bool is_firefly_color(dc1394camera_t* c);
    bool is_firefly_mono(dc1394camera_t* c);
    void configure_cam(dc1394camera_t* c);
    
    
    void set_streaming(dc1394camera_t* c, bool on);
    
    dc1394video_frame_t *get_newest_frame(dc1394camera_t* c);
    dc1394video_frame_t *get_a_frame(dc1394camera_t* c);
    
    void extract_image_to(dc1394video_frame_t *f, ImgBase **ppoDst, std::vector<icl8u> &rgbInterleavedBuffer);
    void grab_frame(dc1394camera_t* c, ImgBase **image);
  }
}
    

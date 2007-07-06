#include <dc1394/control.h>
#include <iclTypes.h>
#include <string>
#include <vector>


namespace icl{
  /** \cond */
  class ImgBase;
  /** \endcond */
  
  /// internal used namespace for libdc1394 dependent help functions and classes
  namespace dc{
    
    /// translate a dc1394video_mode_t into a string representation
    std::string to_string(dc1394video_mode_t vm);

    /// translate a dc1394framerate_t into a string representation
    std::string to_string(dc1394framerate_t fr);

    /// translate a dc1394color_filter_t into a string representation
    std::string to_string(dc1394color_filter_t f);
    
    /// translate a dc1394video_mode_t from a string representation
    dc1394video_mode_t videomode_from_string(const std::string &s);
    
    /// translate a dc1394framerate_t from a string representation
    dc1394framerate_t framerate_from_string(const std::string &s);

    /// translate a dc1394color_filter_t into a string representation
    std::string to_string(dc1394color_filter_t f);

    /// checks if a camera has the firefly mv color model id
    /** @param c camera to check 
        @return string(c->model) == "Firefly MV FFMV-03MTC"
    */
    bool is_firefly_color(dc1394camera_t* c);

    /// checks if a camera has the firefly mv mono model id
    /** @param c camera to check 
        @return string(c->model) == "Firefly MV FFMV-03MTM"
    */
    bool is_firefly_mono(dc1394camera_t* c);
    
    /// (TODO) sets up a camera to some useful defaults dependent on its model id    
    void initialize_dc_cam(dc1394camera_t *c, int nDMABuffers);

    /// stops the capturing and streaming process
    void release_dc_cam(dc1394camera_t *c);
    
    /// ensures iso streaming of the given camera to have the given state (on or off)
    void set_streaming(dc1394camera_t* c, bool on);
    
    /// gets the newest frame from a given camera
    /** Internally this function flushes the ring buffer, queues a brand new
        frame, and then waits for it to be filled with the newest frame.
        <b>NOTE:</b>This is not the procedure that is used by the DCGrabber!
    */
    dc1394video_frame_t *get_newest_frame(dc1394camera_t* c);
    
    /// gets a new frame from the given camera
    /** The camera MUST be initialized, running and streaming */
    dc1394video_frame_t *get_a_frame(dc1394camera_t* c);
    
    /// extracts a dc1394video_frame_t struct into a given ImgBase*
    /** Yet, the given frame must be grabbed from a firefly camera.

        <b>TODO:</b> Adapt this function to be more flexible: 
        - additional parameters could be:
          - a destination format (e.g. to force gray even when grabbing from an RGB device)
          - bayer interpolation mode (nearest, simple, bilin, hq, and <b>subsample</b>
          - ...
        - additional features should be:
          - better handling of different source cameras (f->cam)
          - handling of the above parameters
          - ...
        
        
        @param f given video frame to convert
        @param ppoDst destination images (adapted to icl8u,size of the given frame, and 
                      color format, which depend on the dc1394camera_t, which is associated
                      with it)
        @param rgbInterleavedBuffer internal used static data buffer, which is resized if
                                    necessary and only for rgb images. 

        */
    void extract_image_to(dc1394video_frame_t *f, ImgBase **ppoDst, std::vector<icl8u> &rgbInterleavedBuffer);
    
    /// utility function to grab a single frame from a camera into a given ImgBase**
    /** <b>DEPRECATED</b>*/
    void grab_frame(dc1394camera_t* c, ImgBase **image);

    /// creates a signal handler for the SIGINT signal
    /** The signal handler will enshure, that the SIGINT-signal,
        which is send when CTRL+C is pressed to kill a the programm,
        is caught. The new signal handler function ensures, that
        all camera threads are stopped before the programm exits */
    void install_signal_handler();
  }
}
    

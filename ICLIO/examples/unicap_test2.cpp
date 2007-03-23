#include <unicap.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iclImg.h>
#include <iclCC.h>

#define MAX_FORMATS 64
#define MAX_PROPERTIES 64
#define MAX_DEVICES 64

using namespace icl;
using namespace std;

void set_format (unicap_handle_t handle){
  // {{{ open

  unicap_format_t formats[MAX_FORMATS];
  int format_count;
  unicap_status_t status = STATUS_SUCCESS;
  int f = -1;
   
  for (format_count = 0; SUCCESS (status) && (format_count < MAX_FORMATS); format_count++){
    status = unicap_enumerate_formats (handle, NULL, &formats[format_count], format_count);
    if (SUCCESS (status)){
      printf ("%d: %s\n", format_count, formats[format_count].identifier);
    }else{
      break;
    }
  }
  if(!format_count){
    return;
  }
  
  while ((f < 0) || (f >= format_count)){
    printf ("Use Format:  using 3 \n");
    scanf ("%d", &f);
    //f=3;
  }
  
  if (formats[f].size_count){
    int i;
    int s = -1;
    
    for (i = 0; i < formats[f].size_count; i++){
      printf ("%d: %dx%d\n", i, formats[f].sizes[i].width,
              formats[f].sizes[i].height);
    }
    
    while ((s < 0) || (s >= formats[f].size_count)){
      printf ("Select Size: using 3! \n");
      scanf ("%d", &s);
      //s=3;
    }
    formats[f].size.width = formats[f].sizes[s].width;
    formats[f].size.height = formats[f].sizes[s].height;
  }
  
  if (!SUCCESS (unicap_set_format (handle, &formats[f]))){
    fprintf (stderr, "Failed to set the format!\n");
    exit (-1);
  }
}

// }}}

unicap_handle_t open_device (){
  // {{{ open

  int dev_count;
  int status = STATUS_SUCCESS;
  unicap_device_t devices[MAX_DEVICES];
  unicap_handle_t handle;
  int d = -1;
  
  for (dev_count = 0; SUCCESS (status) && (dev_count < MAX_DEVICES);dev_count++){
    status = unicap_enumerate_devices (NULL, &devices[dev_count], dev_count); // (1)
    if (SUCCESS (status)){
      printf ("%d: %s\n", dev_count, devices[dev_count].identifier);
    }else{
      break;
    }
  }
  
  if (dev_count == 0){
    return NULL;  // no device selected
  }
  while ((d < 0) || (d >= dev_count)){
    printf ("Open Device: \n");
    //    d=0;
    scanf ("%d", &d);
  }
  unicap_open (&handle, &devices[d]);   // (2)
  return handle;
}

// }}}


static void new_frame_cb (unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t * buffer, void *usr_data){
  // {{{ open
  volatile int *frame_count = (volatile int *) usr_data;
  *frame_count = *frame_count - 1;
  printf("callback was called %d frames left\n",*frame_count);
}

// }}}

void capture_frames_dma (unicap_handle_t handle, int nframes){
  // {{{ open

  unicap_format_t format;
  volatile int frame_count;
   
  if (!SUCCESS (unicap_get_format (handle, &format))){
    fprintf (stderr, "Failed to get video format!\n");
    exit (-1);
  }
  
  format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;   
  
  if (!SUCCESS (unicap_set_format (handle, &format))){
    fprintf (stderr, "Failed to set video format!\n");
    exit (-1);
  }
  
  printf("registering callback \n");
  frame_count = nframes;
  unicap_status_t st = unicap_register_callback (handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t) new_frame_cb, (void *) &frame_count);   // (2)
  printf("success for register = %s \n",SUCCESS(st) ? "success" : "failed");
  
  printf("starting to capture \n");
  st = unicap_start_capture (handle); 

  printf("success for start capture = %s \n",SUCCESS(st) ? "success" : "failed");
  
  printf("entering loop");
  while (frame_count > 0){
    printf("wait loop: framecount is %d \n",frame_count);
    usleep (1000000);
  }
  printf ("Captured %d frames!\n", nframes);
  
  unicap_stop_capture (handle); // (5)
  
}

// }}}

vector<ImgBase*>  capture_frames (unicap_handle_t handle, int nframes){
  // {{{ open
  vector<ImgBase*> vec;
  unicap_format_t format;
  unicap_data_buffer_t buffer;
  int cframe = 0;
  
  if (!SUCCESS (unicap_get_format (handle, &format))){
    fprintf (stderr, "Failed to get video format!\n");
    exit (-1);
  }
  
  format.buffer_type = UNICAP_BUFFER_TYPE_USER; // (1)
  
  if (!SUCCESS (unicap_set_format (handle, &format))) {
    fprintf (stderr, "Failed to set video format!\n");
    exit (-1);
  }
  
  buffer.data = new unsigned char[format.buffer_size];
  buffer.buffer_size = format.buffer_size;
  
  unicap_start_capture (handle);        // (4)
  unicap_queue_buffer (handle, &buffer);        // (3)
  
  while (cframe < nframes) {
     
    unicap_data_buffer_t *returned_buffer;
    if (!SUCCESS (unicap_wait_buffer (handle, &returned_buffer)))   {
      fprintf (stderr, "Failed to wait for buffer!\n");
      exit (-1);
    }
    /*****************************
    Img8u *imRGB = new Img8u(Size(640,480),formatGray);
    copy(buffer.data,buffer.data+imRGB->getDim(),imRGB->getData(0));
    vec.push_back(imRGB);
    *******************************/
    
    
    
    Img8u *imRGB = new Img8u(Size(640,480),formatRGB);
    icl8u *rgbBuf = new icl8u[imRGB->getDim()*3];
    ippiYUV422ToRGB_8u_C2C3R(buffer.data,
                             imRGB->getWidth()*2,
                             rgbBuf,
                             imRGB->getWidth()*3,
                             imRGB->getROISize()    );
    
    interleavedToPlanar(rgbBuf,imRGB->getSize(),3, imRGB);
    delete [] rgbBuf;
    
    vec.push_back(imRGB);
    
    if (!SUCCESS (unicap_queue_buffer (handle, returned_buffer)))  {
      fprintf (stderr, "Failed to queue buffer!\n");
      exit (-1);
    }
    
    cframe++;
  }
  printf ("Captured %d frames!\n", nframes);
  unicap_stop_capture (handle); // (6)
  delete [] buffer.data;
  return vec;
}

// }}}


int main(){
  unicap_handle_t handle = open_device();
  if(!handle){
    printf("error: got no handle !");
    return -1;
  }
  
  set_format(handle);
  
  capture_frames_dma(handle,10);

  unicap_close(handle);

  return 0;
}

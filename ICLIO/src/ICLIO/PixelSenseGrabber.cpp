/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PixelSenseGrabber.cpp                  **
** Module : ICLIO                                                  **
** Authors: Eckard Riedenklau, Christof Elbrechter                 **
**          (based on code by Florian Echtler)                     **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/PixelSenseGrabber.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/StringUtils.h>
#ifdef WIN32
  #include <lusb0_usb.h>
#else
  #include <usb.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    // local definitions
#define ID_MICROSOFT 0x045e
#define ID_SURFACE   0x0775
#define VIDEO_RES_X 960
#define VIDEO_RES_Y 540
#define VIDEO_BUFFER_SIZE  VIDEO_RES_X * VIDEO_RES_Y

#define ENDPOINT_VIDEO 0x82
#define ENDPOINT_BLOBS 0x86
#define VIDEO_HEADER_MAGIC 0x46425553
#define VIDEO_PACKET_SIZE  16384

#define PS_GET_VERSION 0xb0 // 12 bytes string
#define PS_UNKNOWN1    0xb3 //  5 bytes
#define PS_UNKNOWN2    0xc1 // 24 bytes

#define PS_GET_STATUS  0xc5 //  4 bytes state (?)
#define PS_GET_SENSORS 0xb1 //  8 bytes sensors

#define TIMEOUT 1000

    namespace{
      // read 512 bytes from endpoint 0x86 -> get header + blobs
      struct ps_header {
        uint16_t type;  // always 0x0001
        uint16_t count; // count of blobs (if == 0: continue prev. packet ID)
        uint32_t packet_id;
        uint32_t timestamp; // milliseconds (increases by 16 or 17 each frame)
        uint32_t unknown;   // "epoch?" always 02/03 00 00 00
      };
      struct ps_blob {
        uint16_t blob_id;
        uint8_t action;     // 0x02 = enter/exit, 0x03 = update (?)
        uint8_t unknown;    // always 0x01 or 0x02 (no idea what this is?)
        uint16_t bb_pos_x;  // upper left corner of bounding box
        uint16_t bb_pos_y;
        uint16_t bb_size_x; // size of bounding box
        uint16_t bb_size_y;
        uint16_t pos_x;     // finger tip position
        uint16_t pos_y;
        uint16_t ctr_x;     // centroid position
        uint16_t ctr_y;
        uint16_t axis_x;    // somehow related to major/minor axis, mostly:
        uint16_t axis_y;    // axis_x == bb_size_y && axis_y == bb_size_x
        float    angle;     // orientation in radians relative to x axis
        uint32_t area;      // size in pixels/pressure (?)
        uint8_t padding[32];
      };
      
      static std::ostream &operator<<(std::ostream &str, const ps_blob &b){
        return str << b.blob_id << ',' //<< b.action << ','
                   << b.bb_pos_x << ',' << b.bb_pos_y << ',' << b.bb_size_x << ',' << b.bb_size_y << ','
                   << b.pos_x << ',' << b.pos_y << ',' << b.ctr_x << ',' << b.ctr_y << ','
                   << b.axis_x << ',' << b.axis_y << ',' << b.angle << ',' << b.area;
      }
      
      static std::istream &operator>>(std::istream &str, ps_blob &b){
        return str >> b.blob_id //>> b.action
                   >> b.bb_pos_x >> b.bb_pos_y >> b.bb_size_x >> b.bb_size_y
                   >> b.pos_x >> b.pos_y >> b.ctr_x >> b.ctr_y
                   >> b.axis_x >> b.axis_y >> b.angle >> b.area;
      }
      
      // read 512 bytes from endpoint 0x82 -> get header below
      // continue reading 16k blocks until header.size bytes read
      struct ps_image {
        uint32_t magic;     // "SUBF"
        uint32_t packet_id;
        uint32_t size;      // always 0x0007e900 = 960x540
        uint32_t timestamp; // milliseconds (increases by 16 or 17 each frame)
      uint32_t unknown;   // "epoch?" always 02/03 00 00 00
      };
      
      // read 8 bytes using control message 0xc0,0xb1,0x00,0x00
      struct ps_sensors {
        uint16_t temp;
        uint16_t acc_x;
        uint16_t acc_y;
        uint16_t acc_z;
      };
      
      static void hline(Channel8u c, int x, int y, int len){
        std::fill(&c(x,y), &c(x,y)+len, 255);
      }
      static void vline(Channel8u c, int x, int y, int len){
        for(int cy = y; cy <= y+len; ++cy){
          if(cy < c.getHeight()){
            c(x,cy) = 255;
          }
        }
      }
    
    static void vis_bounding_box(Channel8u c, int x, int y, int w, int h){
      hline(c,x,y,w);
      hline(c,x,y+h,w);
      vline(c,x,y,h);
      vline(c,x+w,y,h);

    }
    }

    std::ostream &operator<<(std::ostream &str, const PixelSenseGrabber::Blob &b){
      return str << ((const ps_blob&)b);
    }
    
    std::istream &operator>>(std::istream &str, PixelSenseGrabber::Blob &b){
      return str >> ((ps_blob&)b);
    }


    // local helper methods
    static usb_dev_handle* usb_get_device_handle( int vendor, int product );
    static int ps_get_status( usb_dev_handle* handle );
    static void ps_get_sensors( usb_dev_handle* handle );
    static void ps_command( usb_dev_handle* handle, uint16_t cmd, uint16_t index, uint16_t len );
    static void ps_init( usb_dev_handle* handle );
    static int ps_get_image(usb_dev_handle* handle, uint8_t* image );
    static int ps_get_blobs(usb_dev_handle* handle, ps_blob* blob );
    static void ps_get_version(usb_dev_handle* handle, uint16_t index);
    
    struct PixelSenseGrabber::Data{
      usb_dev_handle * s40;
      Img8u image;
      std::vector<ps_blob> blobs;
      Mutex mutex;
      Data():mutex(Mutex::mutexTypeRecursive){}
    };

    
    PixelSenseGrabber::PixelSenseGrabber(float maxFPS):m_data(new Data){
      m_data->s40 = usb_get_device_handle( ID_MICROSOFT, ID_SURFACE );
      
      if(!m_data->s40) throw ICLException("unable to initializte Surface Grabber (the crazy one)");
      ps_init( m_data->s40 );
      
      m_data->image = Img8u(Size(VIDEO_RES_X,VIDEO_RES_Y),1);
      m_data->blobs.resize(256);
      
      addProperty("format", "menu","formatGray-depth8u","formatGray-depth8u",0,"image format can't be changed");
      addProperty("size", "menu","QHD","QHD",0,"image size can't be changed");
      addProperty("blobs found", "info","",0,0,"number of blobs, found in the current frame");
      addProperty("visualize blobs","flag","",false,0,"if true, blobs are visualized in the output image");
    }
    
    PixelSenseGrabber::~PixelSenseGrabber(){
      delete m_data;
    }
    

   
    
    const ImgBase* PixelSenseGrabber::acquireImage(){
      Mutex::Locker __lock(m_data->mutex);

      ps_get_image( m_data->s40, m_data->image.begin(0) );
      int bc = ps_get_blobs( m_data->s40, m_data->blobs.data() );
      setPropertyValue("blobs found",bc);
      if(getPropertyValue("visualize blobs")){
	Channel8u c = m_data->image[0];
	for(int i=0;i<bc;++i){
	  vis_bounding_box(c, m_data->blobs[i].bb_pos_x/2, m_data->blobs[i].bb_pos_y/2, 
			   m_data->blobs[i].bb_size_x/2, m_data->blobs[i].bb_size_y/2);
	}
      }
      
      std::string meta = cat( std::vector<ps_blob>(m_data->blobs.begin(), m_data->blobs.begin()+bc), ",");
      m_data->image.setMetaData(meta);
      return &m_data->image;
    }


    std::vector<PixelSenseGrabber::Blob> 
    PixelSenseGrabber::extractBlobMetaData(const ImgBase *image){
      ICLASSERT_THROW(image, ICLException("PixelSenseGrabber::extractBlobMetaData: image is null"));
      std::istringstream istr(image->getMetaData());
      std::vector<Blob> ps;
      std::copy(std::istream_iterator<Blob>(istr), std::istream_iterator<Blob>(),
                std::back_inserter(ps));
      return ps;
                
    }

    usb_dev_handle* usb_get_device_handle( int vendor, int product ) {

      usb_init();
      usb_find_busses();
      usb_find_devices();

      struct usb_bus* busses = usb_get_busses();

      for (struct usb_bus* bus = busses; bus; bus = bus->next) {
        for (struct usb_device* dev = bus->devices; dev; dev = dev->next) {
          if ((dev->descriptor.idVendor == vendor) && (dev->descriptor.idProduct == product)) {
            usb_dev_handle* handle = usb_open(dev);
            if (!handle) return 0;
            if (usb_claim_interface( handle, 0 ) < 0) return 0;
            return handle;
          }
        }
      }
      return 0;
    }

    void ps_get_version( usb_dev_handle* handle, uint16_t index ) {
      uint8_t buf[13]; buf[12] = 0;
      usb_control_msg( handle, 0xC0, PS_GET_VERSION, 0x00, index, (char*)buf, 12, TIMEOUT );
      //printf("version string 0x%02x: %s\n", index, buf);
    }

    // get device status word
    int ps_get_status( usb_dev_handle* handle ) {
      uint8_t buf[4];
      usb_control_msg( handle, 0xC0, PS_GET_STATUS, 0x00, 0x00, (char*)buf, 4, TIMEOUT );
      return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    }

    // get sensor status
    void ps_get_sensors( usb_dev_handle* handle ) {
      ps_sensors sensors;
      usb_control_msg( handle, 0xC0, PS_GET_SENSORS, 0x00, 0x00, (char*)(&sensors), 8, TIMEOUT );
      //printf("temp: %d x: %d y: %d z: %d\n",sensors.temp,sensors.acc_x,sensors.acc_y,sensors.acc_z);
    }

    // other commands
    void ps_command( usb_dev_handle* handle, uint16_t cmd, uint16_t index, uint16_t len ) {
      uint8_t buf[24];
      usb_control_msg( handle, 0xC0, cmd, 0x00, index, (char*)buf, len, TIMEOUT );
      /*
      printf("command 0x%02x,0x%02x: ", cmd, index );
      for (int i = 0; i < len; i++) printf("0x%02x ", buf[i]);
      printf("\n");
      */
    }

    // mindless repetition of the microsoft driver's init sequence.
    // quite probably unnecessary, but leave it like this for now.
    void ps_init( usb_dev_handle* handle ) {

      //printf("microsoft surface 2.0 open source driver 0.0.1\n");

      ps_get_version(handle, 0x00);
      ps_get_version(handle, 0x01);
      ps_get_version(handle, 0x02);

      ps_command(handle, PS_UNKNOWN2, 0x00, 24 );
      ps_command(handle, PS_UNKNOWN1, 0x00,  5 );

      ps_get_version(handle, 0x03);
    }


    /************************** IMAGE FUNCTIONS *************************/

    int ps_get_image( usb_dev_handle* handle, uint8_t* image ) {
      uint8_t buffer[512];
      int result, bufpos = 0;

      result = usb_bulk_read( handle, ENDPOINT_VIDEO, (char*)buffer, sizeof(buffer), TIMEOUT );
      if (result != sizeof(ps_image)) { printf("transfer size mismatch\n"); return -1; }

      ps_image* header = (ps_image*)buffer;
      if (header->magic != VIDEO_HEADER_MAGIC) { printf("image magic mismatch\n"); return -1; }
      if (header->size  != VIDEO_BUFFER_SIZE ) { printf("image size  mismatch\n"); return -1; }

      while (bufpos < VIDEO_BUFFER_SIZE) {
        result = usb_bulk_read( handle, ENDPOINT_VIDEO, (char*)(image+bufpos), VIDEO_PACKET_SIZE, TIMEOUT );
        if (result < 0) { printf("error in usb_bulk_read\n"); return result; }
        bufpos += result;
      }

      return header->timestamp;
    }


    /************************** BLOB FUNCTIONS **************************/

    int ps_get_blobs( usb_dev_handle* handle, ps_blob* outblob ) {

      uint8_t buffer[512];
      uint32_t packet_id;
      int result;

      int need_blobs = -1;
      int current = 0;

      ps_header* header = (ps_header*)buffer;
      ps_blob*   inblob = (ps_blob*)(buffer+sizeof(ps_header));

      do {

        result = usb_bulk_read( handle, ENDPOINT_BLOBS, (char*)(buffer), sizeof(buffer), TIMEOUT ) - sizeof(ps_header);
        if (result < 0) { printf("error in usb_bulk_read\n"); return result; }
        if (result % sizeof(ps_blob) != 0) { printf("transfer size mismatch\n"); return -1; }
        //printf("id: %x count: %d\n",header->packet_id,header->count);

        // first packet
        if (need_blobs == -1) {
          need_blobs = header->count;
          packet_id = header->packet_id;
        }

        // sanity check. when video data is also being retrieved, the packet ID
        // will usually increase in the middle of a series instead of at the end.
        if (packet_id != header->packet_id) { 
	  static bool first = true;
	  if(first){
	    printf("packet ID mismatch (this message will be suppressed in the future)\n"); 
	    first = false;
	  }
	}

        int packet_blobs = result / sizeof(ps_blob);

        for (int i = 0; i < packet_blobs; i++) outblob[current++] = inblob[i];
	
      }	while (current < need_blobs);

      return need_blobs;
    }


    REGISTER_CONFIGURABLE(PixelSenseGrabber, return new PixelSenseGrabber(30));

    Grabber* createPixelSenseGrabber(const std::string &param){
      return new PixelSenseGrabber(to32f(param));
    }

    const std::vector<GrabberDeviceDescription>& getPixelSenseDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
      deviceList.clear();
        if(hint.size()){
          deviceList.push_back(GrabberDeviceDescription("ps",
              hint,
              "a pixelsense image source"));
        } else {
          deviceList.push_back(GrabberDeviceDescription("ps",
              "0",
              "a pixelsense image source"));
        }
      }
      return deviceList;
    }

    REGISTER_GRABBER(ps,utils::function(createPixelSenseGrabber), utils::function(getPixelSenseDeviceList),"ps:0:pixelsense image source");

  } // namespace io
}


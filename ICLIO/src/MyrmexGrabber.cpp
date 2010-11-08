/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/MyrmexGrabber.cpp                            **
** Module : ICLIO                                                  **
** Authors: Carsten Schuermann, Christof Elbrechter                **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

/*
    MyrmexGrabber class:	
    Handles communication with the uvc videodriver, returns the imagedata from device
    Communication with the video driver is done by sending IO control commands according to the V4L2 API
*/


#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <sys/time.h>	//needed by linux/videodev
#include <linux/videodev.h>

#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/MyrmexGrabber.h>


#define AMOUNT_BUFFERS 2	//number of videobuffers we want to reserve from driver
#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define NOGATE 4
#define SIDE 1
#define TOP 0
//Special modes 
#define MODE_NORMAL 0
#define MODE_BOOK 1


#define ERROR_MARK  0xA000 //Top 4 Bits code on last pixel of last frame to mark that an error occurred during readout

namespace icl {


  //swap16: Returns swapped low and high byte of value a
  static unsigned short swap16(unsigned short a){
    return ((a & 0xff) << 8 ) | ((a & 0xff00) >> 8);  
    //unsigned char *b = (unsigned char *) &a;
    //return ( *(b+1) + (*b << 8) );
  }

  // seems not to be in use ??
  //int bigtargetBook[16*16];

  struct MyrmexGrabberImpl::Data{
    std::vector<unsigned int> flat;
    int bigtarget[16*16];
    
    int fd;						//filepointer to videodevice
    void *mem[AMOUNT_BUFFERS];	//maps to the buffers the driver gives us	
    int bufferSizes[AMOUNT_BUFFERS]; // needed for munmap 
    bool showDebug;	//flag for showing debug info
    unsigned int connections_size; //amount of connections between modules
    unsigned int image_width; //store converted width
   unsigned int image_height; //store converted height
    char attachedPosition; //store position of central unit
    Img16s outputImage;

    std::vector<char> conversionTable;  //table which maps usb input texel position to grabber output texel position
    bool isNull;
    
    int device;
    MyrmexGrabberImpl::Viewpoint viewpoint;
    int compression;
    int speedFactor;

    bool substractNoiseImage;
    int recordNoiseImageFrames;
    Img16s noiseImage;
    
    Data(){
      substractNoiseImage = false;
      recordNoiseImageFrames = -1;
    }
    
    void updateNoiseImage(){
      if(recordNoiseImageFrames == -1){
        if(!substractNoiseImage) return;
        noiseImage.setSize(outputImage.getSize());
        noiseImage.setChannels(1);
              
        icl16s *o = outputImage.begin(0);
        const icl16s *n = noiseImage.begin(0);
        const int dim = outputImage.getDim();

        for(int i=0;i<dim;++i){
          o[i] = iclMax(0,o[i]-n[i]);
        }
        return;
      }
      noiseImage.setSize(outputImage.getSize());
      noiseImage.setChannels(1);
      
      const icl16s *o = outputImage.begin(0);
      icl16s *n = noiseImage.begin(0);
      const int dim = outputImage.getDim();
      
      for(int i=0;i<dim;++i){
        if(o[i] > n[i]) n[i] = o[i];
      }
      
      --recordNoiseImageFrames;
    }
    
    
  };

  MyrmexGrabberImpl::MyrmexGrabberImpl():m_data(new Data){
    m_data->isNull = true;
    m_data->showDebug = false;
  }

  
  MyrmexGrabberImpl::MyrmexGrabberImpl(int device, MyrmexGrabberImpl::Viewpoint viewpoint, int compression, int speedFactor) throw (ICLException) :
    m_data(new Data){
    m_data->isNull = true;
    m_data->showDebug = false;
    init(device,viewpoint,compression,speedFactor);
  }

  const std::vector<GrabberDeviceDescription> &MyrmexGrabberImpl::getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> deviceList;
    if(rescan){
      deviceList.clear();
      for(int i=0;i<6;i++){
        try{
          MyrmexGrabberImpl g;
          g.initDevice(i,VIEW_W);
          deviceList.push_back(GrabberDeviceDescription("myr",str(i),"Myrmex Device at /dev/video"+str(i)));
        }catch(...){}
      }
    }
    return deviceList;
  }

  void MyrmexGrabberImpl::initDevice(int device, MyrmexGrabberImpl::Viewpoint viewpoint) throw (ICLException){
    if(!m_data->isNull) throw ICLException("MyrmexGrabberImpl device was tryed to initialize twice");
    
    m_data->device = device;
    m_data->viewpoint = viewpoint;
    m_data->showDebug = false;
    m_data->isNull = false;
    
    
    int ret,i;
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_requestbuffers requestbuffers;
    struct v4l2_buffer v4l2buffer;
    struct video_capability vcap;

    //Init requestbuffer structure to request buffers
    requestbuffers.count = AMOUNT_BUFFERS;
    requestbuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestbuffers.memory = V4L2_MEMORY_MMAP;

    //open device file
    std::string videodevice = "/dev/video"+str(device);
    if ((m_data->fd = open(videodevice.c_str(), O_RDWR)) == -1) {
      throw ICLException("V4L2 Error: Error opening videodevice for read/write");
    }

    if(ioctl(m_data->fd, VIDIOCGCAP, &vcap) == -1){
      throw ICLException("V4L2 Error: Error extracting vcap");
    }
    
    if(std::string("Myrmex") != vcap.name){
      throw ICLException("Device index " + str(device) + " does not reference a Myrmex device (found device name '"
                         + vcap.name + "' expected 'Mymrex')");
    }

    //send IO control to request buffers
    if ( (ret=ioctl(m_data->fd, VIDIOC_REQBUFS, &requestbuffers) < 0) ) {
      printf("Code: %i ", ret);
      throw ICLException("V4L2 Error: Unable to request buffers");
    }

    //query all buffers and map them
    for (i = 0; i < AMOUNT_BUFFERS; i++) {
      memset(&v4l2buffer, 0, sizeof(struct v4l2_buffer));	//reset all values
      v4l2buffer.index = i;
      v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      v4l2buffer.memory = V4L2_MEMORY_MMAP;
      ret = ioctl(m_data->fd, VIDIOC_QUERYBUF, &v4l2buffer);
      if (ret < 0) {
        throw ICLException("V4L2 Error: Unable to query buffer");
      }
      //each buffer gets mapped, the address is stored in mem[i]
      m_data->mem[i] = mmap(0,
                            v4l2buffer.length, PROT_READ, MAP_SHARED, m_data->fd,
                            v4l2buffer.m.offset);
      m_data->bufferSizes[i] = v4l2buffer.length;
      if (m_data->mem[i] == MAP_FAILED) {
        throw ICLException("V4L2 Error: Unable to map buffer");
      }
	
    }
    //queue all buffers so they can be filled with data
    for (i = 0; i < AMOUNT_BUFFERS; ++i) {
      memset(&v4l2buffer, 0, sizeof(struct v4l2_buffer));
      v4l2buffer.index = i;
      v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      v4l2buffer.memory = V4L2_MEMORY_MMAP;
		
      if (ioctl(m_data->fd, VIDIOC_QBUF, &v4l2buffer) < 0) {
        throw ICLException("V4L2 Error: Unable to queue buffer");
      }
    }

    //send streamon command
    if (ioctl(m_data->fd, VIDIOC_STREAMON, &type) < 0) {
      throw ICLException("V4L2 Error: Unable to start capture");
    }



    //Discover connections and setup conversion table
    std::vector<char> connections = getConnections(); //Get the module connection list from the avr32
    m_data->connections_size = connections.size();


    //Here we reorganize the data from 'flat' to real world positons
    int widthX,heightX;

    //Walk through the connections and determine attachment point and dimension of the module array
    parseConnections(connections, &m_data->attachedPosition, &widthX, &heightX );
    //Create a conversion table, which reorders the image data to match the real world setup
    m_data->conversionTable = createConversiontable( widthX, heightX, connections, m_data->attachedPosition, viewpoint );
    //printf("widthX:%d heightX:%d\n",widthX,heightX);

    m_data->image_width = widthX * 16; //enclosing rectangle
    m_data->image_height = heightX * 16; //enclosing rectangle
    if (viewpoint==VIEW_3 || viewpoint==VIEW_E){
      int a = m_data->image_width;
      m_data->image_width = m_data->image_height;
      m_data->image_height = a;
	
    }

    unsigned int p=0;
    unsigned int bcount=0;
    unsigned int ccount=0;
    unsigned int has=0;
    unsigned int hasbase=0;
    unsigned int inc =0;
    unsigned int ybase =0;

    //The data is transmitted per module, but if you have an array of modules this order doesnt reflect the
    //line-per-line data storage an image is used fpr, so we create a flat LUT which will be used by the grabFrame function
    //to compensate this
    m_data->flat.resize(m_data->image_width*m_data->image_height,0);

    for(unsigned int x=0;x<m_data->image_width;x++){
      for(unsigned int y=0;y<m_data->image_height;y++){

        m_data->flat[ has ] = p;
        //printf("%d : %d \n", p, has  );

        p++;
        bcount++;
			
        if(p%16==0){
          bcount=0;
				
				
          if (p%(m_data->image_width)==0){
            //zurück erstes modul
            ccount=0;
            hasbase = inc+16;
            inc = inc +16;
          } else {
            //zwischen zwei modulen
            ccount++;
            hasbase = 256*ccount + inc;
          }

        }


        if (p%(m_data->image_width*16)==0){
          ybase = ybase + m_data->image_width*16;
          inc = 0;
          hasbase =0;
          bcount =0;
        }

			
        has = ybase + hasbase + bcount;
			
      }	

    }
  }

  // Init function: Opens the video device, requests buffers and tests them, sends streamon command
  void MyrmexGrabberImpl::init(int device, MyrmexGrabberImpl::Viewpoint viewpoint, int compression, int speedFactor) throw (ICLException){

    initDevice(device,viewpoint);
    
    //it is required to configure the compression and Speed divider, otherwise the system will not begin to stream data
    m_data->compression = compression;
    m_data->speedFactor = speedFactor;

    setSpeedDevider(speedFactor);
    setCompression(compression);

    // This cannot happen since setSpeed would have thrown an exception ...
    //if( (getCompression()!=compression) || (getSpeed()!=speedFactor)){
    //  ERROR_LOG("MyrmexGrabberImpl: Compression and Speed setting can only be configured after startup.");
    //}
  }

  //Destructor
  MyrmexGrabberImpl::~MyrmexGrabberImpl(){
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(m_data->fd, VIDIOC_STREAMOFF, &type); // this is not initialized since capturing might not have been started yet
    close(m_data->fd);
    /*
        usbvflg_buf[m_iDevice]=(icl8u *)mmap(0,usbvflg_vmbuf[m_iDevice].size, 
					   PROT_READ|PROT_WRITE,
					   MAP_SHARED, usbvflg_fd[m_iDevice],0);

        here:
        m_data->mem[i] = mmap(0,
                              v4l2buffer.length, PROT_READ, MAP_SHARED, m_data->fd,
                               v4l2buffer.m.offset);
        */
    for(int i=0;i<AMOUNT_BUFFERS;++i){
      munmap(m_data->mem[i],m_data->bufferSizes[i]);
    }
    delete m_data;
  }


  //set compression for myrmex sensor system, improves speed when there is lots of unused tactile area
  //may reduce speed when used with few modules, because of overhead caused by compression
  //framerate will vary according to tactile area that is above threshold, values below threshold get converted to 0
  //auto threshold means that each module individually will determine its threshold to the unloaded max it encounters
  //so DONT place anything on the modules during startup while using auto threshold, or you will get wrong thresholds..
  //
  //i=0=no compression, 1=compression with auto threshold, i>1 compression with threshold i  (i=0..255)
  void MyrmexGrabberImpl::setCompression(unsigned int i) throw (ICLException){
    struct v4l2_control control_s;
    control_s.id =V4L2_CID_BRIGHTNESS;
    control_s.value = i;

    if (( ioctl(m_data->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
      WARNING_LOG("MyrmexGrabberImpl: Compression was already set. It can only be configured after startup.");
    }else{
      m_data->compression = i;
    }
  }

  //Setting of ADC sampling speed,  higher value means slower speed and more accuracy,
  //actually is used as divider for PIC32 SPI Clock, which is dervied from 80Mhz base clock, so 80/6 (default) = 13.3MHZ. Theo. max is 20 Mhz = div 4, div 6 is stable
  //80mhz/i = spi ADC clock,  i=4..255
  void MyrmexGrabberImpl::setSpeedDevider(unsigned int i) throw (ICLException){
    
    struct v4l2_control control_s;
    control_s.id =V4L2_CID_CONTRAST;
    if (i<6) i=6; //0 or 1 wont work...	
    control_s.value = i;
	
    if (( ioctl(m_data->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
      WARNING_LOG("MyrmexGrabberImpl: Speed was already set. It can only be configured after startup.");
    }else{
      m_data->speedFactor = i;
    }
  }

  //return compression setting
  int MyrmexGrabberImpl::getCompression() throw (ICLException){
    struct v4l2_control control_s;
    control_s.id =V4L2_CID_BRIGHTNESS;
    control_s.value = 0;

    if (( ioctl(m_data->fd, VIDIOC_G_CTRL, &control_s)) < 0) {
      throw ICLException("MyrmexGrabberImpl V4L2: ioctl got control Compression error");
    }
    return control_s.value;
  }
  //return speed setting
  int MyrmexGrabberImpl::getSpeed() throw (ICLException){
    struct v4l2_control control_s;
    control_s.id =V4L2_CID_CONTRAST;
    control_s.value = 0;
    if (( ioctl(m_data->fd, VIDIOC_G_CTRL, &control_s)) < 0) {
      throw ICLException("MyrmexGrabberImpl V4L2: ioctl get control Speed error");
    }
    return control_s.value;
  }

  //get m_data->connections between modules
  std::vector<char> MyrmexGrabberImpl::getConnections(){
    struct v4l2_frmsizeenum fsize;
    struct v4l2_fmtdesc fmt;
    int length=0;
	
    //first get the pixelformat, from frame format
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;	//myrmex has only one frame format
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_data->fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
    }
    //then we can query the framesize
    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 1;	
    fsize.pixel_format = fmt.pixelformat;
    if ((ioctl(m_data->fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
      //save amount of m_data->connections between modules
      length = fsize.discrete.width;
    }

    std::vector<char> connections(length);
    //printf("%d\n",length);


    //extract connection information (2bits per connection) from frame parameters
    //each frame setting has 16 m_data->connections (16bit width, 16 bit height parameter)

    int j=0;
    int frames = ceil (length/16.0);
    //printf("Frames %f \n", ceil (length/16.0) );
    for(int i=0; i< frames;i++){
      memset(&fsize, 0, sizeof(fsize));
      fsize.index = 2+i;	
      fsize.pixel_format = fmt.pixelformat;
      if ((ioctl(m_data->fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
        //save size to variables
        //printf("W %X H %X \n", fsize.discrete.width ,fsize.discrete.height);

        for (int k=14; k>=0;k-=2){
          if(j<length) connections[j++] = ((fsize.discrete.width & (3<<k))>>k);
        }			
        for (int k=14; k>=0;k-=2){
          if(j<length) connections[j++] = ((fsize.discrete.height & (3<<k))>>k);
        }	
      }


    }


    return connections;

  }

  //parseconnections:
  // find out at which corner the avr is attached (attached)
  // find out width & height of the module array (widthX / heightX) (rounded up if incomplete row)
  void MyrmexGrabberImpl::parseConnections(const std::vector<char> &connections, char* attached, int* widthX, int* heightX ){


    char smart_dir=0;// = corner attached, 0 = upper left, 1 = lower left side, 2 = lower left bottom, 3 = lower right bottom

    int top_counter=0;// = max in vert richtung
    int side_counter=0;// = max in hor richtung
    int max_top =0;
    int max_side =0;
    bool not_yet_side_corner = true;
    bool not_yet_up_corner = false;
    int dir=-1;

    //get first connection to smart detect attachment point
    if (connections[0] == NORTH){ 
      smart_dir = (smart_dir | 2);
      not_yet_up_corner = false; 
    } else if (connections[0] == WEST){
      smart_dir = (smart_dir | 0);
      not_yet_side_corner = false;
    }

    for(unsigned int i=1;i<connections.size();i++){

      //get first corner direction to clarify attachment point
      if ( not_yet_side_corner == true ){
        if (connections[i] == EAST ){
          smart_dir = (smart_dir | 1);
          not_yet_side_corner = false;
        } else if (connections[i] == WEST ){
          smart_dir = (smart_dir | 0);
          not_yet_side_corner = false;	
        }
      }


      if (not_yet_up_corner){
	
        if ( connections[i] == NORTH ){
          smart_dir = (smart_dir | 1);
          not_yet_up_corner = false;
        } else if (connections[i] == SOUTH ){
          smart_dir = (smart_dir | 0);
          not_yet_up_corner = false;	
        }
      }

      //count side and top steps to get dimensions
      //DEPENDS ON DISCOVER ALGO!!!  

      if (smart_dir==0){//upper left, so we dont get a 'maze'


	if ((connections[i] == NORTH) or (connections[i] == SOUTH)){
          if (dir == TOP){ //direction in which we are moving
            top_counter++;

          } else { //corner?
            top_counter++;
			
            if (side_counter > max_side) max_side = side_counter; //side finished
            side_counter = 0;
          }	
          dir = TOP;
          //printf("TOP\n");
	}

	if ((connections[i] == EAST) or (connections[i] == WEST)){
          if (dir == SIDE){
            side_counter++;

          } else { //corner?
            side_counter++;
            if (top_counter > max_top) max_top = top_counter; //top finished
            //top_counter=0; //because maze gives single style TOP moves
          }	
          dir = SIDE;
          //printf("SIDE\n");
	}



      } else { //we get a 'maze' type discovery which gives max width/height from longest walks
	if ((connections[i] == NORTH) or (connections[i] == SOUTH)){
          if (dir == TOP){ //direction in which we are moving
            top_counter++;

          } else { //corner?
            top_counter++;
			
            if (side_counter > max_side) max_side = side_counter; //side finished
            side_counter = 0;
          }	
          dir = TOP;
          //printf("TOP\n");
	}

	if ((connections[i] == EAST) or (connections[i] == WEST)){
          if (dir == SIDE){
            side_counter++;

          } else { //corner?
            side_counter++;
            if (top_counter > max_top) max_top = top_counter; //top finished
            top_counter=0;
          }	
          dir = SIDE;
          //printf("SIDE\n");
	}
	
      }
    }
    //If we have a single row
    if (top_counter > max_top) max_top = top_counter; //top finished
    if (side_counter > max_side) max_side = side_counter; //side finished

    *attached = smart_dir;
    *widthX = max_side+1;
    *heightX = max_top+1;
  }



  //create conversion table:
  //to place modules in right location, independent of routing algorithm, viewpoint be given as parameter
  //
  //create target array with module locations for given view
  //follow m_data->connections and create conversion array with module locations from target array
  //startpoint for m_data->connections is determined by attachment point
  //
  // bigtarget array is to reorderthe pixels inside a module to a new rotated position
  std::vector<char> MyrmexGrabberImpl::createConversiontable( int width, int height,const std::vector<char> &connections, 
                                                          char attached, Viewpoint viewpoint ){

    char actual[width][height];

    //conversion array
    std::vector<char> conversion(width*height);
    std::vector<char> conversion2(width*height);

    /*for(unsigned int i=1;i < m_data->connections.size();i++){
        conversion[i] =  -1;
	}*/

    //target array
    char target[width][height];
    int bigtargetB[16][16];


    int index = 0;


    //printf("widthX: %d, heightX: %d\n",m_data->connections.size(),width*height);


    //Create target depending on viewpoint

    //module targets for W

    switch(viewpoint){
      case VIEW_W:{
        for (int y=0;y<height;y++){
          for (int x=0;x<width;x++){
            target[x][y] =  index++;
          }
        }
        //W Pixel reorder
        index =0; 
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){
            bigtargetB[x][y] = index++;
          }
        }
        int findex =0;
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            m_data->bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
        break;
      }
      case VIEW_3:{
        if (viewpoint == VIEW_3 ){
          for (int x=width-1;x>=0;x--){
            for (int y=0;y<height;y++){
              target[x][y] =  index++;
            }
          }
          //3 Pixel reorder
          index =0; 
          for (int x=15;x>=0;x--){
            for (int y=0;y<16;y++){
              bigtargetB[x][y] = index++;
            }
          }
          int findex =0;
          for (int y=0;y<16;y++){
            for (int x=0;x<16;x++){	
              m_data->bigtarget[findex++] =  bigtargetB[x][y];
            }
          }
        }
        break;
      }
      case VIEW_M:{
        for (int y=height-1;y>=0;y--){
          for (int x=width-1;x>=0;x--){
            target[x][y] =  index++;		
        }
        }
        //M Pixel reorder
        index =0; 
        for (int y=15;y>=0;y--){
          for (int x=15;x>=0;x--){
            bigtargetB[x][y] = index++;
          }
        }
        int findex =0;
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            m_data->bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
        break;
      }
      case VIEW_E:{
        //module targets for E
        if (viewpoint == VIEW_E ){
          for (int x=0;x<width;x++){
            for (int y=height-1;y>=0;y--){
              target[x][y] =  index++;
            }
          }
          //E Pixel reodering
          index =0; 
          for (int x=0;x<16;x++){
            for (int y=15;y>=0;y--){
              bigtargetB[x][y] = index++;
            }
          }
          int findex =0;
          for (int y=0;y<16;y++){
            for (int x=0;x<16;x++){	
              m_data->bigtarget[findex++] =  bigtargetB[x][y];
            }
          }
        }
        break;
      }
    } // end switch

    //print target
    if(m_data->showDebug){
      printf("Target:\n");
      for (int y=0;y<height;y++){
        for (int x=0;x<width;x++){
          printf(" %d ", target[x][y] );
        }
        printf("\n");
      }
    }
    //start point at attachment
    static const char xs[] = {0,0,0,width-1};
    static const char ys[] = {0,height-1,height-1,height-1};
    const char startx = xs[(int)attached];
    const char starty = ys[(int)attached];

    if(m_data->showDebug){
      printf("Width / Height %d %d\n", width, height);
      printf("Start X/Y %d %d\n", startx, starty);
    }

    //create conversion	
    int x=startx;
    int y=starty;
    int ccounter=0;
    conversion[ccounter++] = target[x][y];
    actual[x][y]=0; 
    //We walk through the target array using the connection information
    //and create the actual module positions and the conversion ones
    for(unsigned int i=1;i<connections.size();i++){
      if (connections[i] == NORTH ) y-=1;
      if (connections[i] == SOUTH ) y+=1;
      if (connections[i] == EAST ) x-=1;
      if (connections[i] == WEST ) x+=1;
      conversion[ccounter++] = target[x][y];
      actual[x][y]=i;

    }
    
    if(m_data->showDebug){
      printf("Actual:\n");
      for (int y=0;y<height;y++){
        for (int x=0;x<width;x++){
          printf(" %d ", actual[x][y] );
        }
        printf("\n");
      }
      
      printf("Conversion:\n");
      for (int y=0;y<height*width;y++){
        printf(" %d ",conversion[y] );
      }
      printf("\n");
    }
    return conversion;
  }


  //GetSize: Gets the imagesize after conversion done
  Size MyrmexGrabberImpl::getSize() const{
    return Size(m_data->image_width,m_data->image_height);
  }




	
  //GetSizeDevice: Gets the imagesize from the usb video device
  //
  // unsigned int *x, unsigned int *y : pointers to variables which will hold the sizes
  Size MyrmexGrabberImpl::getSizeDevice() const{
    struct v4l2_frmsizeenum fsize;
    struct v4l2_fmtdesc fmt;
    //first get the pixelformat, from frame format
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;	//myrmex has only one frame format
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_data->fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
    }
		

    //then we can query the framesize
    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;	//myrmex has only one frame size
    fsize.pixel_format = fmt.pixelformat;
    if ((ioctl(m_data->fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
      //save size to variables
		
    }

    return Size(fsize.discrete.width,fsize.discrete.height);
  }


  const ImgBase* MyrmexGrabberImpl::grabUD(ImgBase **ppoDst){
    Img16s &outputImage = m_data->outputImage;
    outputImage.setChannels(1);
    Size s(m_data->image_width,m_data->image_height);	
    outputImage.setSize(s);	

    if(!grabFrame(outputImage.begin(0))){
      ERROR_LOG("MyrmexGrabberImpl Modules: Error while reading data from module");
    }

    outputImage.setTime(Time::now());
    m_data->updateNoiseImage();
    return &outputImage;
  }

  //GrabFrame: Read frame and save into a buffer
  //
  //int* inbuffer : pointer to buffer where the framedata will be stored
  //unsigned int bufflength : length of the buffer in words

  int MyrmexGrabberImpl::grabFrame(short* inbuffer) throw (ICLException) {
    static const unsigned int correctionLUT[] = {
      3, 7, 11, 15, 67, 71, 75, 79, 131, 135, 139, 143, 195, 199, 203, 207, 
      18, 22, 26, 30, 82, 86, 90, 94, 146, 150, 154, 158, 210, 214, 218, 222, 
      2, 6, 10, 14, 66, 70, 74, 78, 130, 134, 138, 142, 194, 198, 202, 206, 
      17, 21, 25, 29, 81, 85, 89, 93, 145, 149, 153, 157, 209, 213, 217, 221, 
      19, 23, 27, 31, 83, 87, 91, 95, 147, 151, 155, 159, 211, 215, 219, 223, 
      35, 39, 43, 47, 99, 103, 107, 111, 163, 167, 171, 175, 227, 231, 235, 239, 
      51, 55, 59, 63, 115, 119, 123, 127, 179, 183, 187, 191, 243, 247, 251, 255, 
      34, 38, 42, 46, 98, 102, 106, 110, 162, 166, 170, 174, 226, 230, 234, 238, 
      50, 54, 58, 62, 114, 118, 122, 126, 178, 182, 186, 190, 242, 246, 250, 254, 
      33, 37, 41, 45, 97, 101, 105, 109, 161, 165, 169, 173, 225, 229, 233, 237, 
      48, 52, 56, 60, 112, 116, 120, 124, 176, 180, 184, 188, 240, 244, 248, 252, 
      32, 36, 40, 44, 96, 100, 104, 108, 160, 164, 168, 172, 224, 228, 232, 236, 
      16, 20, 24, 28, 80, 84, 88, 92, 144, 148, 152, 156, 208, 212, 216, 220, 
      0, 4, 8, 12, 64, 68, 72, 76, 128, 132, 136, 140, 192, 196, 200, 204, 
      1, 5, 9, 13, 65, 69, 73, 77, 129, 133, 137, 141, 193, 197, 201, 205, 
      49, 53, 57, 61, 113, 117, 121, 125, 177, 181, 185, 189, 241, 245, 249, 253
    }; 
    unsigned int j;
    unsigned short* words;
    unsigned int module_count;
    struct v4l2_buffer v4l2buffer;

    float temp;
    int error_mark=0;
    bool result=true;


    //prepare a buffer request
    memset(&v4l2buffer, 0, sizeof(struct v4l2_buffer));
    v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2buffer.memory = V4L2_MEMORY_MMAP;

    //ask to dequeue a filled buffer   
    if (ioctl(m_data->fd, VIDIOC_DQBUF, &v4l2buffer) < 0) {
      throw ICLException("MyrmexGrabberImpl V4L2: Unable to dequeue buffer" );
      return false;
    }

    //access buffer wordwise
    words = (unsigned short*) m_data->mem[v4l2buffer.index]; 
	

    module_count = v4l2buffer.bytesused/512;
    //copy all image data from linear videostream buffer to linear image buffer:
    // - correct byteorder because USB transmission swapped high and low byte
    // - remove top 4 Bits (they contain ad channel address)
    // - invert values so high pressure = high values (optional)
    // - correct value positions by LUT
    j=0;
    for (unsigned int k=0;k<module_count;k++){
      //printf("%d %d \n", k, (m_data->conversionTable[k]));
      for (unsigned int i=0;i<256;i++){
        //correct byteorder and invert values, then save into inbuffer
        temp = (4095 -(0xFFF & swap16(words[j]))) ;

				
        //Check for error frame
        if( (k==module_count-1) && (i==255)) {
          error_mark = (0xF000 & swap16(words[j]));
        }
 

        //-correct pixel position
        //m_data->conversionTable sets converted Module position offset
        //bigtarget corrects pixel orientation based on module position
        //correctionLUT corrects pixel position based on AD channel nr


        if (module_count>1) {
          inbuffer[ m_data->flat[ (m_data->conversionTable[k]*256) + m_data->bigtarget[i]  ] ] = temp ;
        } else { //multi modules saved reordered in their memory
          inbuffer[ m_data->flat[ (m_data->conversionTable[k]*256) + m_data->bigtarget[correctionLUT[i]]  ] ] = temp ;
        }
        j+=1;
      }
    }
			
    //requeue the buffer to be used again
    if (ioctl(m_data->fd, VIDIOC_QBUF, &v4l2buffer)< 0) {
      throw ICLException("MyrmexGrabberImpl V4L2: Unable to requeue buffer " );
    }

	
    if (error_mark == ERROR_MARK){
      result = false;
	 
    }
    return result;
  }

  int MyrmexGrabberImpl::getConnectionCount() const{
    return m_data->connections_size;

  }
  char MyrmexGrabberImpl::getAttachedPosition() const{
    return m_data->attachedPosition;
  }
 
  
  void MyrmexGrabberImpl::setProperty(const std::string &property, const std::string &value){
    if(property == "substract-noise-image"){
      m_data->substractNoiseImage = (value == "on");
    }else if(property == "record-noise-image"){
      m_data->recordNoiseImageFrames = 10;
    }
  }

  std::vector<std::string> MyrmexGrabberImpl::getPropertyList(){
    return tok("record-noise-image,substract-noise-image,connection-count,format,size",",");
  }
  
  std::string MyrmexGrabberImpl::getType(const std::string &name){
    if(name == "record-noise-image") return "command";
    else if(name == "substract-noise-image" || name == "format" || name == "size") return "menu";
    else if(name == "connection-count") return "info";
    else return "undefined";
  }
  
  std::string MyrmexGrabberImpl::getInfo(const std::string &name){
    if(name == "record-noise-image") return "";
    else if(name == "substract-noise-image") return "{\"off\",\"on\"}";
    else if(name == "format") return "{\"16Bit Grayscale\"}";
    else if(name == "size") return "{\""+str(getSize())+"\"}";
    else return "undefined";
  }
  
  std::string MyrmexGrabberImpl::getValue(const std::string &name){
     if(name == "substract-noise-image") return m_data->substractNoiseImage ? "on": "off";
     else if(name == "connection-count") return str(m_data->connections_size);
     else if(name == "format") return "16Bit Grayscale";
     else if(name == "size") return str(getSize());
     else return "";
  }
  int MyrmexGrabberImpl::isVolatile(const std::string &propertyName){
    if(propertyName == "connection-count") return 1000000;
    return 0;
  }



}


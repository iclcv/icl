#include "iclUnicapGrabber.h"
#include <iclImg.h>
#include <string>
#include <unicap.h>
#include <iclConverter.h>
#include <iclCC.h>

using namespace icl;
using namespace std;

#define MAX_DEVICES 64
#define MAX_FORMATS 64
#define MAX_PROPERTIES 64

namespace icl{
  
  
  class UnicapFormat{
    // {{{ open

    static Rect cvt(const unicap_rect_t &r){
      return Rect(r.x,r.y,r.width, r.height);
    }
  public:
    UnicapFormat(){
      unicap_void_format(&m_oUnicapFormat);
    }
    UnicapFormat(unicap_handle_t handle):m_oUnicapHandle(handle){}

    enum buffertype{
      userBuffer = UNICAP_BUFFER_TYPE_USER ,
      systemBuffer = UNICAP_BUFFER_TYPE_SYSTEM 
    };

    string getID() const { return m_oUnicapFormat.identifier; }
    Rect getRect() const { return cvt(m_oUnicapFormat.size); }
    Size getSize() const { return getRect().size(); }

    Rect getMinRect() const { return cvt(m_oUnicapFormat.min_size); }  
    Rect getMaxRect() const { return cvt(m_oUnicapFormat.max_size); }  
    
    Size getMinSize() const { return getMinRect().size(); }
    Size getMaxSize() const { return getMaxRect().size(); }

    int getHStepping() const { return m_oUnicapFormat.h_stepping; }
    int getVStepping() const { return m_oUnicapFormat.v_stepping; }

    bool checkSize(const Size &size)const{
      vector<Size> v= getPossibleSizes();
      for(unsigned int i=0;i<v.size();++i){
        if(v[i] == size){ 
          return true; 
        }
      }
      return false;
    }
    
    vector<Rect> getPossibleRects() const{
      vector<Rect> v;
      for(int i=0;i< m_oUnicapFormat.size_count; v.push_back( cvt(m_oUnicapFormat.sizes[i])) );
      return v;
    }
    vector<Size> getPossibleSizes() const{
      vector<Size> v;
      for(int i=0;i< m_oUnicapFormat.size_count; v.push_back(cvt(m_oUnicapFormat.sizes[i]).size()));
      return v;
    }
    // bit per pixel ??
    int getBitsPerPixel() const { return m_oUnicapFormat.bpp; }
    unsigned int getFourCC() const { return m_oUnicapFormat.fourcc; }
    unsigned int getFlags() const { return m_oUnicapFormat.flags; }
    
    unsigned int getBufferTypes() const { return m_oUnicapFormat.buffer_types; }
    unsigned int getSystemBufferCount() const { return m_oUnicapFormat.system_buffer_count; }
    
    unsigned int getBufferSize() const { return m_oUnicapFormat.buffer_size; }
    
    buffertype getBufferType() const { return (buffertype)m_oUnicapFormat.buffer_type; }

    const unicap_format_t &getUnicapFormat() const { return m_oUnicapFormat; }
    unicap_format_t &getUnicapFormat(){ return m_oUnicapFormat; }

    const unicap_handle_t &getUnicapHandle() const { return m_oUnicapHandle; }
    unicap_handle_t &getUnicapHandle() { return m_oUnicapHandle; }
    
    string toString()const{
      // {{{ open

      char buf[10000];
      Rect r = getRect();
      Rect a = getMinRect();
      Rect b = getMaxRect();
      sprintf(buf,
              "ID               = %s\n"
              "Rect:     curr   = %d %d %d %d\n"
              "          min    = %d %d %d %d\n"
              "          max    = %d %d %d %d\n"
              "Stepping: h      = %d\n"
              "          v      = %d\n"
              "Misc:     bpp    = %d\n"
              "          fourcc = %d\n"
              "Buffers:  types  = %d\n"
              "          #sysbf = %d\n"
              "          size   = %d\n"
              "          type   = %s\n"
              ,getID().c_str(),r.x,r.y,r.width,r.height,a.x,a.y,a.width,a.height,
              b.x,b.y,b.width,b.height,getHStepping(),getVStepping(),getBitsPerPixel(),
              getFourCC(),getBufferTypes(),getSystemBufferCount(),getBufferSize(),
              getBufferType()==userBuffer ? "user" : "system" );
    
      string s = buf;
      s.append("PossibleSizes:\n");
      vector<Rect> v = getPossibleRects();
      for(unsigned int i=0;i<v.size();i++){
        sprintf(buf,"                   %d %d %d %d\n",v[i].x,v[i].y,v[i].width,v[i].height);
        s.append(buf);
      }
      return s;
    }

    // }}}

  private:
    unicap_format_t m_oUnicapFormat;
    unicap_handle_t m_oUnicapHandle;
  };

  // }}}

  class UnicapProperty{
    // {{{ open

  public:
    UnicapProperty(){
      unicap_void_property (&m_oUnicapProperty);  
    }
    UnicapProperty(unicap_handle_t handle):m_oUnicapHandle(handle){}
    enum type {
      range = UNICAP_PROPERTY_TYPE_RANGE,
      valueList = UNICAP_PROPERTY_TYPE_VALUE_LIST,
      menu = UNICAP_PROPERTY_TYPE_MENU,
      data = UNICAP_PROPERTY_TYPE_DATA,
      flags = UNICAP_PROPERTY_TYPE_FLAGS
    };
    struct Data{
      void *data;
      unsigned int size;
    };
    string getID() const{ return m_oUnicapProperty.identifier; }
    string getCategory() const{ return m_oUnicapProperty.category; }
    string getUnit() const { return m_oUnicapProperty.unit; }
  
    vector<string> getRelations() const { 
      vector<string> v;
      for(int i=0;i<m_oUnicapProperty.relations_count;v.push_back(m_oUnicapProperty.relations[i++]));
      return v;
    }

    type getType() const{
      return (type)m_oUnicapProperty.type;
    }
    
    double getValue() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_RANGE || 
                            m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_VALUE_LIST ,0 );
      return m_oUnicapProperty.value;
    }

    string getMenuItem() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_MENU, 0);
      return m_oUnicapProperty.menu_item;
    } 
    
    Range<double> getRange() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_RANGE, Range<double>());
      return Range<double>( m_oUnicapProperty.range.min, m_oUnicapProperty.range.max );
    }

    vector<double> getValueList() const{
       ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_VALUE_LIST ,vector<double>() );
       vector<double> v;
       for(int i=0;i<m_oUnicapProperty.value_list.value_count;v.push_back(m_oUnicapProperty.value_list.values[i++]));
       return v;
    }
    vector<string> getMenu() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_MENU , vector<string>());
      vector<string> v;
      for(int i=0;i<m_oUnicapProperty.menu.menu_item_count;v.push_back(m_oUnicapProperty.menu.menu_items[i++]));
      return v;
    }
    double getStepping() const {
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_RANGE, 0);
      return m_oUnicapProperty.stepping;
    }
    
    u_int64_t getFlags() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_FLAGS, 0);
      return m_oUnicapProperty.flags;
    }
    u_int64_t getFlagMask() const{
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_FLAGS, 0);
      return m_oUnicapProperty.flags_mask;
    }
    
    const Data getData() const{
      static const Data NULL_DATA = {0,0}; 
      ICLASSERT_RETURN_VAL( m_oUnicapProperty.type == UNICAP_PROPERTY_TYPE_DATA,NULL_DATA);
      Data d = { m_oUnicapProperty.property_data, m_oUnicapProperty.property_data_size };
      return d;
    }
    const unicap_property_t &getUnicapProperty() const{
      return m_oUnicapProperty;
    }
    unicap_property_t &getUnicapProperty(){
      return m_oUnicapProperty;
    }
    const unicap_handle_t &getUnicapHandle() const { return m_oUnicapHandle; }
    unicap_handle_t &getUnicapHandle() { return m_oUnicapHandle; }
 
    void setValue(double value){
      // {{{ open
      type t=getType();
      ICLASSERT_RETURN( t == range || t==valueList );
      if(t==range){
        if(getRange().in(value)){
          m_oUnicapProperty.value = value;
          unicap_set_property(m_oUnicapHandle,&m_oUnicapProperty);
        }else{
          ERROR_LOG("could not set up value to: " << value << " (outside range!)");
        }
      }else if(t==valueList){
        vector<double> v = getValueList();
        if(find(v.begin(),v.end(),value) != v.end()){
          m_oUnicapProperty.value = value;
          if(!SUCCESS (unicap_set_property(m_oUnicapHandle,&m_oUnicapProperty) )){
            ERROR_LOG("failed to set new property [code 0]");
          }
        }
      }
    }
    // }}}
    
    void setMenuItem(const string &item){
      // {{{ open

      ICLASSERT_RETURN( getType() == menu );
      vector<string> v = getMenu();
      if(find(v.begin(),v.end(),item) != v.end()){
        sprintf(m_oUnicapProperty.menu_item,item.c_str());
         if(!SUCCESS (unicap_set_property(m_oUnicapHandle,&m_oUnicapProperty) )){
           ERROR_LOG("failed to set new property [code 1]");
         }
      }else{
        ERROR_LOG("could not set up menu item to : " << item << "(item not allowed!)");
      }
    }

    // }}}
  
    static  const char *ftoa(double d){
      static char buf[30];
      sprintf(buf,"%f",d);
      return buf;
    }
    static  const char *itoa(int i){
      static char buf[30];
      sprintf(buf,"%d",i);
      return buf;
    }
    
    string toString(){
      string typeStr;
      switch(getType()){
        case  range: typeStr = "range"; break;
        case  valueList: typeStr = "value-list"; break;
        case  menu: typeStr = "menu"; break;
        case  data: typeStr = "data"; break;
        default: typeStr = "flags"; break;
      }
      
      char buf[10000];
      sprintf(buf,
              "ID       = %s\n"
              "Category = %s\n"
              "Unit     = %s\n"
              "Type     = %s\n"
              , getID().c_str(), getCategory().c_str(),getUnit().c_str(),typeStr.c_str());
      
      string s = buf;
      switch(getType()){
        case  range:
          sprintf(buf,"   min=%f\n   max=%f\n   curr=%f\n",getRange().minVal,getRange().maxVal,getValue());
          s.append(buf);
          break;
        case  valueList:{
          string l = "list     = {";
          vector<double> v = getValueList();
          for(unsigned int i=0;i<v.size();i++){
            l.append(ftoa(v[i]));
            if(i<v.size()-1){
              l.append(",");
            }
          }
          l.append("}\n");
          s.append(l);
          s.append("curr     = ");
          s.append(ftoa(getValue()));
          s.append("\n");
          break;
        }
        case  menu:{
          string l = "list     = {";
          vector<string> v = getMenu();
          for(unsigned int i=0;i<v.size();i++){
            l.append(v[i]);
            if(i<v.size()-1){
              l.append(",");
            }
          }
          l.append("}\n");
          s.append(l);
          s.append("curr     = ");
          s.append(getMenuItem());
          s.append("\n");
          break;
        }
        case flags:
          s.append("flag     = ");
          s.append(itoa(getFlags()));
          s.append("\n");
          s.append("mask     = ");
          s.append(itoa(getFlagMask()));
          s.append("\n");
          break;
        default: break;
      }
      return s;
    }
  private:
    
    unicap_property_t m_oUnicapProperty;
    unicap_handle_t m_oUnicapHandle;
  };

  // }}}

  class UnicapDevice{
    // {{{ open

  public:
    UnicapDevice(unicap_handle_t handle) : m_oUnicapHandle(handle){
      // {{{ open

      unicap_get_device(handle, &m_oUnicapDevice);
      
      // properties
      unicap_status_t status = STATUS_SUCCESS;
      for(int i=0;SUCCESS(status);i++){
        m_oProperties.push_back(UnicapProperty(handle));
        status = unicap_enumerate_properties(handle, NULL, &(m_oProperties[i].getUnicapProperty()),i);
        if (!SUCCESS (status)){
          m_oProperties.pop_back();
        }
      }
      
      // formats
      status = STATUS_SUCCESS;
      for(int i=0;SUCCESS(status);i++){ 
        m_oFormats.push_back(UnicapFormat());
        status = unicap_enumerate_formats (handle, NULL, &(m_oFormats[i].getUnicapFormat()), i);
        if (!SUCCESS (status)){
          m_oFormats.pop_back();
        }
      }
    }

    // }}}

    string getID()const { return m_oUnicapDevice.identifier; }
    string getModelName()const { return  m_oUnicapDevice.model_name;}
    string getVendorName()const { return  m_oUnicapDevice.vendor_name;}
    unsigned long long getModelID()const { return  m_oUnicapDevice.model_id; }
    unsigned int getVendorID()const { return  m_oUnicapDevice.vendor_id;}
    string getCPILayer()const { return  m_oUnicapDevice.cpi_layer;}
    string getDevice() const { return m_oUnicapDevice.device;}
    unsigned int getFlags() const { return m_oUnicapDevice.flags;}
    
    const vector<UnicapProperty> getProperties() const{ return m_oProperties; }
    vector<UnicapProperty> getProperties(){ return m_oProperties; }

    const vector<UnicapFormat> getFormats() const{ return m_oFormats; }
    vector<UnicapFormat> getFormats(){ return m_oFormats; }
    
    
    const unicap_handle_t &getUnicapHandle()const { return m_oUnicapHandle; }
    unicap_handle_t &getUnicapHandle() { return m_oUnicapHandle; }
    
    const unicap_device_t &getUnicapDevice()const {  return m_oUnicapDevice; }
    unicap_device_t &getUnicapDevice(){ return m_oUnicapDevice; }
    
    UnicapFormat getCurrentUnicapFormat(){
      // {{{ open

      UnicapFormat f;
      if(!SUCCESS( unicap_get_format(m_oUnicapHandle,&(f.getUnicapFormat())) )){
        ERROR_LOG("failed to get current unicap format!");
      }
      return f;
    }

    // }}}

    Size getCurrentSize(){
      // {{{ open

      return getCurrentUnicapFormat().getSize();
    }

    // }}}

    string getFormatID(){
      // {{{ open

      return getCurrentUnicapFormat().getID();
    }

    // }}}
    
    void setFormat(UnicapFormat &fmt){
      // {{{ open

      if(!SUCCESS(unicap_set_format(m_oUnicapHandle,&(fmt.getUnicapFormat())))){
        ERROR_LOG("failed to set up unicap format! \n");
      }
    }

    // }}}
    void setFormatID(const string &fmtID){
      // {{{ open

      // search the format from the formatlist by id
      for(unsigned int i=0;i<m_oFormats.size();i++){
        if(m_oFormats[i].getID() == fmtID ){
          if(m_oFormats[i].checkSize(getCurrentSize())){
            UnicapFormat f = getCurrentUnicapFormat();
            sprintf(f.getUnicapFormat().identifier,fmtID.c_str());
            setFormat(f);
          }else{
            ERROR_LOG("current size is supported for new format: " << fmtID << "!");          
          }
          break;
        }
      }
      ERROR_LOG("could not set unicap format to: " << fmtID << " (unknown format id)");
    }

    // }}}
    
    void setFormatSize(const Size &newSize){
      // {{{ open

      UnicapFormat f = getCurrentUnicapFormat();
      if(f.checkSize(newSize)){
        f.getUnicapFormat().size.width = newSize.width;
        f.getUnicapFormat().size.width = newSize.height;
        setFormat(f);
      }else{
        ERROR_LOG("nes size is supported for current format");          
      }
    }

    // }}}
    void setFormat(const string &fmtID, const Size &newSize){
      // {{{ open

      for(unsigned int i=0;i<m_oFormats.size();i++){
        if(m_oFormats[i].getID() == fmtID ){
          if(m_oFormats[i].checkSize(newSize)){
            UnicapFormat f = getCurrentUnicapFormat();
            sprintf(f.getUnicapFormat().identifier,fmtID.c_str());
            f.getUnicapFormat().size.width = newSize.width;
            f.getUnicapFormat().size.width = newSize.height; 
            setFormat(f);
          }else{
            ERROR_LOG("combination of format and size is not supported \n");
          }
        }
      }
      ERROR_LOG("could not set unicap format to: " << fmtID << " (unknown format id)");
    }

    // }}}
    
    void listProperties()const{
      vector<UnicapProperty> v = getProperties();
      for(unsigned int i=0;i<v.size();i++){
        printf("Property %d:\n%s\n",i,v[i].toString().c_str());
      }
    }
    
    void listFormats() const{
      vector<UnicapFormat> v = getFormats();
      for(unsigned int i=0;i<v.size();i++){
        printf("Format %d:\n%s\n",i,v[i].toString().c_str());
      }    
    }
    
  private:
    unicap_handle_t m_oUnicapHandle;
    unicap_device_t m_oUnicapDevice;
    
    vector<UnicapProperty> m_oProperties; 
    vector<UnicapFormat> m_oFormats; 
  };

  // }}}


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
    //    scanf ("%d", &f);
    f=3;
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
      //scanf ("%d", &s);
      s=3;
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
    d=0;
    
    //scanf ("%d", &d);
  }
  unicap_open (&handle, &devices[d]);   // (2)
  return handle;
}

// }}}

void  set_range_property (unicap_handle_t handle){
  // {{{ open

  unicap_property_t properties[MAX_PROPERTIES];
  int property_count;
  int range_ppty_count;
  unicap_status_t status = STATUS_SUCCESS;
  int p = -1;
  double new_value = 0.0;
  
  for (property_count = range_ppty_count = 0; SUCCESS (status) && (property_count < MAX_PROPERTIES); property_count++){
    status = unicap_enumerate_properties (handle, NULL, &properties[range_ppty_count], property_count);       // (1)
    if (SUCCESS (status)){
      if (properties[range_ppty_count].type == UNICAP_PROPERTY_TYPE_RANGE){
        printf ("%d: %s\n", range_ppty_count, properties[range_ppty_count].identifier);
        range_ppty_count++;
      }
    }else{
      break;
    }
  }
  if (range_ppty_count == 0){
    // no range properties
    return;
  }
  while ((p < 0) || (p > range_ppty_count)){
    printf ("Property: ");
    scanf ("%d", &p);
  }
  
  status = unicap_get_property (handle, &properties[p]); 
  if (!SUCCESS (status)){
    fprintf (stderr, "Failed to inquire property '%s'\n",properties[p].identifier);
    exit (-1);
  }
  printf ("Property '%s': Current = %f, Range = [%f..%f]\n",
          properties[p].identifier, properties[p].value,
          properties[p].range.min, properties[p].range.max);
  
  new_value = properties[p].range.min - 1.0f;
  while ((new_value < properties[p].range.min) || (new_value > properties[p].range.max)){
    printf ("New value for property: ");
    scanf ("%lf", &new_value);
  }
  properties[p].value = new_value;
  if (!SUCCESS (unicap_set_property (handle, &properties[p]))){
    fprintf (stderr, "Failed to set property!\n");
    exit (-1);
  }
}

// }}}

static void new_frame_cb (unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t * buffer, void *usr_data){
  // {{{ open
  printf("callback was called \n");
  volatile int *frame_count = (volatile int *) usr_data;
  *frame_count = *frame_count - 1;
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
  
  format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;       // (1)
   
  if (!SUCCESS (unicap_set_format (handle, &format))){
    fprintf (stderr, "Failed to set video format!\n");
    exit (-1);
  }
  
  printf("registering callback \n");
  frame_count = nframes;
  unicap_register_callback (handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t) new_frame_cb, (void *) &frame_count);   // (2)
  
  printf("starting to capture \n");
  unicap_start_capture (handle); 
  while (frame_count > 0){
    printf("framecount is %d \n",frame_count);
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

// {{{ unicap_device_t

/************************************
typedef struct { 	
    char identifier [128];
    char model_name [128] ;
    char vendor_name [128] ;
    unsigned long long model_id ;
    unsigned int vendor_id ;
    char cpi_layer [1024] ;
    char device [1024] ;
    unsigned int flags ;
}unicap_device_t;
*************************************/

// }}}

// {{{ unicap_format_t

/************************************
typedef struct{
  char identifier[128];
  // default
  unicap_rect_t size;
  
  // min and max extends
  unicap_rect_t min_size;
  unicap_rect_t max_size;
  
  // stepping:
  // 0 if no free scaling available
  int h_stepping;
  int v_stepping;
   
  // array of possible sizes
  unicap_rect_t *sizes;
  int size_count;
   
  int bpp;
  unsigned int fourcc;
  unsigned int flags;
  
  unsigned int buffer_types;    // available buffer types
  int system_buffer_count;
  
  size_t buffer_size;
  
  unicap_buffer_type_t buffer_type;
};
*************************************/

// }}}

// {{{ unicap_property_t
/************************************************************
struct unicap_property_t{
  char identifier[128];         //mandatory
  char category[128];
  char unit[128];               //
  
  // list of properties identifier which value / behaviour may change if this property changes
  char **relations;
  int relations_count;
  
  union
  {
    double value;               // default if enumerated
    char menu_item[128];
  };
    
  union
  {
    unicap_property_range_t range;      // if UNICAP_USE_RANGE is asserted
    unicap_property_value_list_t value_list;    // if UNICAP_USE_VALUE_LIST is asserted
      unicap_property_menu_t menu;
  };
  
  double stepping;
  
  unicap_property_type_enum_t type;
  u_int64_t flags;              // defaults if enumerated
  u_int64_t flags_mask;         // defines capabilities if enumerated
  
  // optional data
  void *property_data;
  size_t property_data_size;
};
*********************************************************************/
// }}}

  UnicapGrabber::UnicapGrabber():m_poImage(0){}
  
  const ImgBase* UnicapGrabber::grab(ImgBase *poDst){
    ensureCompatible(&m_poImage,depth8u,Size(640,480),formatRGB);

    unicap_handle_t handle;

    handle = open_device();
    if(!handle){ return 0; }
    
    
    UnicapDevice ud(handle);
    ud.listFormats();
    ud.listProperties();
   
    return 0;
    
    set_format(handle);
    
    //set_range_property(handle);

    vector<ImgBase*> v=capture_frames(handle,10);

    unicap_close(handle);

    v[0]->deepCopy(&m_poImage);    
    for(unsigned int i=0;i<v.size();i++){
      delete v[i];
    }

    return m_poImage;
  }

}

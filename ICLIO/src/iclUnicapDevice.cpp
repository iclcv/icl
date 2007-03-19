#include <iclUnicapDevice.h>

using namespace std;
namespace icl{

  UnicapDevice::UnicapDevice(int deviceIndex) :
    m_oUnicapDevicePtr((unicap_device_t*)malloc(sizeof(unicap_device_t))),m_oUnicapHandle(NULL), m_bOpen(false), m_bValid(false){
    // {{{ open
    if(deviceIndex == -1 ){
      unicap_void_device( m_oUnicapDevicePtr.get() );
      return;
    }
    unicap_status_t status = unicap_enumerate_devices (NULL,m_oUnicapDevicePtr.get(),deviceIndex);
    
    if(!SUCCESS(status)){
      unicap_void_device( m_oUnicapDevicePtr.get() );
      return;
    }
    
    if(open()){
      
      // properties
      unicap_status_t status = STATUS_SUCCESS;
      for(int i=0;SUCCESS(status);i++){
        m_oProperties.push_back(UnicapProperty(m_oUnicapHandle));
        status = unicap_enumerate_properties(m_oUnicapHandle, NULL, m_oProperties[i].getUnicapProperty(),i);
        if (!SUCCESS (status)){
          m_oProperties.pop_back();
        }
      }
      
      // formats
      status = STATUS_SUCCESS;
      for(int i=0;SUCCESS(status);i++){ 
        m_oFormats.push_back(UnicapFormat(m_oUnicapHandle));
        status = unicap_enumerate_formats (m_oUnicapHandle, NULL, m_oFormats[i].getUnicapFormat(), i);
        if (!SUCCESS (status)){
          m_oFormats.pop_back();
        }
      }
      m_bValid = true;
    }else{ 
      unicap_void_device( m_oUnicapDevicePtr.get() );
      ERROR_LOG("Unable to open Unicap Device");
      return;
    }
  }
  
  // }}}
  
  UnicapDevice::~UnicapDevice(){
    if(m_bOpen) close();
  }

  bool UnicapDevice::open(){
    // {{{ open
    if(m_bOpen){
      WARNING_LOG("Unicap Device already open (proceeding)");
      return true;
    }
    unicap_open(&m_oUnicapHandle,m_oUnicapDevicePtr.get());
    return true;
  }

  // }}}
  
  bool UnicapDevice::close(){
    // {{{ open
    if(!m_bOpen){
      WARNING_LOG("Unicap Device already closed (proceeding)");
      return true;
    }
    unicap_close(m_oUnicapHandle);
    return true;
  }

  // }}}
  
  bool UnicapDevice::isValid() const { return m_bValid; }



  string UnicapDevice::getID()const { return m_oUnicapDevicePtr->identifier; }
  string UnicapDevice::getModelName()const { return  m_oUnicapDevicePtr->model_name;}
  string UnicapDevice::getVendorName()const { return  m_oUnicapDevicePtr->vendor_name;}
  unsigned long long UnicapDevice::getModelID()const { return  m_oUnicapDevicePtr->model_id; }
  unsigned int UnicapDevice::getVendorID()const { return  m_oUnicapDevicePtr->vendor_id;}
  string UnicapDevice::getCPILayer()const { return  m_oUnicapDevicePtr->cpi_layer;}
  string UnicapDevice::getDevice() const { return m_oUnicapDevicePtr->device;}
  unsigned int UnicapDevice::getFlags() const { return m_oUnicapDevicePtr->flags;}
  
  const vector<UnicapProperty> UnicapDevice::getProperties() const{ return m_oProperties; }
  vector<UnicapProperty> UnicapDevice::getProperties(){ return m_oProperties; }
  
  const vector<UnicapFormat> UnicapDevice::getFormats() const{ return m_oFormats; }
  vector<UnicapFormat> UnicapDevice::getFormats(){ return m_oFormats; }
  
  
  const unicap_handle_t UnicapDevice::getUnicapHandle()const { return m_oUnicapHandle; }
  unicap_handle_t UnicapDevice::getUnicapHandle() { return m_oUnicapHandle; }
  
  const unicap_device_t *UnicapDevice::getUnicapDevice()const {  return m_oUnicapDevicePtr.get(); }
  unicap_device_t *UnicapDevice::getUnicapDevice(){ return m_oUnicapDevicePtr.get(); }
  
  const vector<UnicapFormat> UnicapDevice::getFilteredFormats(const Size &size) const{
    // {{{ open
    
    vector<UnicapFormat> v;
    const vector<UnicapFormat> v2=getFormats();
    for(unsigned int i=0;i<v2.size();i++){
      if(v2[i].checkSize(size)){
        v.push_back(v2[i]);
      }
    }
    return v2;
  }
  
  // }}}
  
  vector<UnicapFormat> UnicapDevice::getFilteredFormats(const Size &size){
    // {{{ open
    
    vector<UnicapFormat> v,v2=getFormats();
    for(unsigned int i=0;i<v2.size();i++){
      if(v2[i].checkSize(size)){
        v.push_back(v2[i]);
      }
    }
    return v2;
  }
  
  // }}}
  
  const vector<UnicapProperty> UnicapDevice::getFilteredProperties(UnicapProperty::type t, const string &category)const{
    // {{{ open
    
    const vector<UnicapProperty> v=getProperties();
    vector<UnicapProperty> v2;
    bool useType=t!=UnicapProperty::anytype;
    bool useCategory=category!="";
    for(unsigned int i=0;i<v.size();i++){
      if( (!useType || v[i].getType()==t) || (!useCategory || v[i].getCategory()==category) ){
        v2.push_back(v[i]);
      }
    }
    return v2;
    }
  
    // }}}
  
  vector<UnicapProperty> UnicapDevice::getFilteredProperties(UnicapProperty::type t, const string &category){
      // {{{ open
    
    const vector<UnicapProperty> v=getProperties();
    vector<UnicapProperty> v2;
    bool useType=t!=UnicapProperty::anytype;
    bool useCategory=category!="";
    for(unsigned int i=0;i<v.size();i++){
      if( (!useType || v[i].getType()==t) || (!useCategory || v[i].getCategory()==category) ){
        v2.push_back(v[i]);
      }
    }
    return v2;
  }
  
  // }}}
    
  UnicapFormat UnicapDevice::getCurrentUnicapFormat(){
    // {{{ open
    
    UnicapFormat f;
    if(!SUCCESS( unicap_get_format(m_oUnicapHandle,f.getUnicapFormat()) )){
      ERROR_LOG("failed to get current unicap format!");
    }
    return f;
  }
  
  // }}}
  
  Size UnicapDevice::getCurrentSize(){
    // {{{ open
    
    return getCurrentUnicapFormat().getSize();
  }
  
  // }}}
  
  string UnicapDevice::getFormatID(){
    // {{{ open
    
    return getCurrentUnicapFormat().getID();
  }
  
  // }}}
  
  void UnicapDevice::setFormat(UnicapFormat &fmt){
    // {{{ open
    
    if(!SUCCESS(unicap_set_format(m_oUnicapHandle,fmt.getUnicapFormat()))){
      ERROR_LOG("failed to set up unicap format! \n");
    }
  }
  
  // }}}
  void UnicapDevice::setFormatID(const string &fmtID){
    // {{{ open
    
    // search the format from the formatlist by id
    for(unsigned int i=0;i<m_oFormats.size();i++){
      if(m_oFormats[i].getID() == fmtID ){
        if(m_oFormats[i].checkSize(getCurrentSize())){
          UnicapFormat f = getCurrentUnicapFormat();
          sprintf(f.getUnicapFormat()->identifier,fmtID.c_str());
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
  
  void UnicapDevice::setFormatSize(const Size &newSize){
    // {{{ open
    
    UnicapFormat f = getCurrentUnicapFormat();
    if(f.checkSize(newSize)){
      f.getUnicapFormat()->size.width = newSize.width;
      f.getUnicapFormat()->size.width = newSize.height;
      setFormat(f);
    }else{
      ERROR_LOG("nes size is supported for current format");          
    }
  }
  
  // }}}
  
  void UnicapDevice::setFormat(const string &fmtID, const Size &newSize){
    // {{{ open
    
    for(unsigned int i=0;i<m_oFormats.size();i++){
      if(m_oFormats[i].getID() == fmtID ){
        if(m_oFormats[i].checkSize(newSize)){
          UnicapFormat f = getCurrentUnicapFormat();
          sprintf(f.getUnicapFormat()->identifier,fmtID.c_str());
          f.getUnicapFormat()->size.width = newSize.width;
          f.getUnicapFormat()->size.width = newSize.height; 
          setFormat(f);
        }else{
          ERROR_LOG("combination of format and size is not supported \n");
        }
      }
    }
    ERROR_LOG("could not set unicap format to: " << fmtID << " (unknown format id)");
  }
  
  // }}}
  
  void UnicapDevice::listProperties()const{
    // {{{ open
    vector<UnicapProperty> v = getProperties();
    for(unsigned int i=0;i<v.size();i++){
      printf("Property %d:\n%s\n",i,v[i].toString().c_str());
    }
  }

  // }}}
  
  void UnicapDevice::listFormats() const{
    // {{{ open
    vector<UnicapFormat> v = getFormats();
    for(unsigned int i=0;i<v.size();i++){
      printf("Format %d:\n%s\n",i,v[i].toString().c_str());
    }    
  }

  // }}}

} // end of namespace icl


#include <iclUnicapFormat.h>

namespace icl{
  using namespace std;

  Rect cvt(const unicap_rect_t &r){  return Rect(r.x,r.y,r.width, r.height); }

  UnicapFormat::UnicapFormat(): 
    m_oUnicapFormatPtr((unicap_format_t*)malloc(sizeof(unicap_format_t))),
    m_oUnicapHandle(NULL){
    unicap_void_format(m_oUnicapFormatPtr.get());
  }
  
  UnicapFormat::UnicapFormat(unicap_handle_t handle):
    m_oUnicapFormatPtr((unicap_format_t*)malloc(sizeof(unicap_format_t))),
    m_oUnicapHandle(handle){
    unicap_void_format(m_oUnicapFormatPtr.get());
  }
  string UnicapFormat::getID() const { return m_oUnicapFormatPtr->identifier; }
  Rect UnicapFormat::getRect() const { return cvt(m_oUnicapFormatPtr->size); }
  Size UnicapFormat::getSize() const { return getRect().size(); }
  
  Rect UnicapFormat::getMinRect() const { return cvt(m_oUnicapFormatPtr->min_size); }  
  Rect UnicapFormat::getMaxRect() const { return cvt(m_oUnicapFormatPtr->max_size); }  
  
  Size UnicapFormat::getMinSize() const { return getMinRect().size(); }
  Size UnicapFormat::getMaxSize() const { return getMaxRect().size(); }
  
  int UnicapFormat::getHStepping() const { return m_oUnicapFormatPtr->h_stepping; }
  int UnicapFormat::getVStepping() const { return m_oUnicapFormatPtr->v_stepping; }
  
  bool UnicapFormat::checkSize(const Size &size)const{
    vector<Size> v= getPossibleSizes();
    for(unsigned int i=0;i<v.size();++i){
      if(v[i] == size){ 
        return true; 
      }
    }
    return false;
  }
  
  vector<Rect> UnicapFormat::getPossibleRects() const{
    vector<Rect> v;
    for(int i=0;i< m_oUnicapFormatPtr->size_count; v.push_back( cvt(m_oUnicapFormatPtr->sizes[i])) );
    return v;
  }
  vector<Size> UnicapFormat::getPossibleSizes() const{
    vector<Size> v;
    for(int i=0;i< m_oUnicapFormatPtr->size_count; v.push_back(cvt(m_oUnicapFormatPtr->sizes[i]).size()));
    return v;
  }

  int UnicapFormat::getBitsPerPixel() const { return m_oUnicapFormatPtr->bpp; }
  
  string UnicapFormat::getFourCC() const { 
    unsigned int i = m_oUnicapFormatPtr->fourcc; 
    char buf[5] = {i,i>>8,i>>16,i>>24,0};
    return buf;
  }
  
  unsigned int UnicapFormat::getFlags() const { return m_oUnicapFormatPtr->flags; }
    
  unsigned int UnicapFormat::getBufferTypes() const { return m_oUnicapFormatPtr->buffer_types; }
  unsigned int UnicapFormat::getSystemBufferCount() const { return m_oUnicapFormatPtr->system_buffer_count; }
  
  unsigned int UnicapFormat::getBufferSize() const { return m_oUnicapFormatPtr->buffer_size; }
  
  UnicapFormat::buffertype UnicapFormat::getBufferType() const { return (buffertype)m_oUnicapFormatPtr->buffer_type; }
  
  const unicap_format_t *UnicapFormat::getUnicapFormat() const { return m_oUnicapFormatPtr.get(); }
  unicap_format_t *UnicapFormat::getUnicapFormat(){ return m_oUnicapFormatPtr.get(); }
  
  const unicap_handle_t UnicapFormat::getUnicapHandle() const { return m_oUnicapHandle; }
  unicap_handle_t UnicapFormat::getUnicapHandle() { return m_oUnicapHandle; }
  
  string UnicapFormat::toString()const{
    
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
            "          fourcc = %s\n"
            "Buffers:  types  = %d\n"
            "          #sysbf = %d\n"
            "          size   = %d\n"
            "          type   = %s\n"
            ,getID().c_str(),r.x,r.y,r.width,r.height,a.x,a.y,a.width,a.height,
            b.x,b.y,b.width,b.height,getHStepping(),getVStepping(),getBitsPerPixel(),
            getFourCC().c_str(),
            getBufferTypes(),getSystemBufferCount(),getBufferSize(),
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
  
} // end of namespace icl

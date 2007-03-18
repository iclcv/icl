#ifndef ICL_UNICAP_FORMAT_H
#define ICL_UNICAP_FORMAT_H

#include <unicap.h>
#include <iclRect.h>
#include <string>
#include <vector>
#include <stdio.h>

namespace icl{
  
  class UnicapFormat{
    public:
    UnicapFormat();
    UnicapFormat(unicap_handle_t handle);

    enum buffertype{
      userBuffer = UNICAP_BUFFER_TYPE_USER ,
      systemBuffer = UNICAP_BUFFER_TYPE_SYSTEM 
    };

    std::string getID() const;
    Rect getRect() const;
    Size getSize() const;

    Rect getMinRect() const;
    Rect getMaxRect() const;
    
    Size getMinSize() const;
    Size getMaxSize() const;

    int getHStepping() const;
    int getVStepping() const;

    bool checkSize(const Size &size)const;
    
    std::vector<Rect> getPossibleRects() const;
    std::vector<Size> getPossibleSizes() const;

    int getBitsPerPixel() const;
    unsigned int getFourCC() const;
    unsigned int getFlags() const;
    
    unsigned int getBufferTypes() const;
    unsigned int getSystemBufferCount() const;
    
    unsigned int getBufferSize() const;
    
    buffertype getBufferType() const;

    const unicap_format_t &getUnicapFormat() const;
    unicap_format_t &getUnicapFormat();

    const unicap_handle_t &getUnicapHandle() const;
    unicap_handle_t &getUnicapHandle();
    
    std::string toString()const;

  private:
    unicap_format_t m_oUnicapFormat;
    unicap_handle_t m_oUnicapHandle;
  };


} // end of namespace icl

#endif

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/src/File.cpp                                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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
*********************************************************************/

#include <ICLIO/File.h>
#include <stdio.h>
#include <ICLUtils/Macros.h>
#include <ICLIO/IOUtils.h>
#include <sys/stat.h>
#include <cstring>
using std::memcpy;

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

using namespace std;

namespace icl{
  namespace{
    static const char DIR_SEPERATOR = '/';
    static const char NEW_LINE = '\n';

    std::string toString(File::OpenMode om){
      switch(om){
        case File::writeBinary: return "Write Binary";
        case File::writeText: return "Write Text";
        case File::readBinary: return "Read Binary";
        case File::readText: return "Read Text";
        default: return "unknown mode";
      }
    }
    
    void break_apart(string s, string &dir, string &basename, string &suffix, string &filename){
      // {{{ open

      size_t p = s.rfind(DIR_SEPERATOR);
      
      /// split directory xxx/filename.suffix
      if(p==string::npos){
        dir = "";
        filename = s;
      }else{
        dir = s.substr(0,p);
        filename = s.substr(p+1);
      }
      
      // split suffix
      p = filename.rfind('.');
      if(p == string::npos){
        suffix = "";
        basename = filename;
      }else{
        suffix = filename.substr(p);
        basename = filename.substr(0,p);
        if(suffix == ".gz"){
          p = basename.rfind('.');                   ;
          if(p != string::npos){
            suffix = basename.substr(p)+suffix;
            basename = basename.substr(0,p-1);
          }
        }
      }
    }

    // }}}
    void buffer_file(FILE *fp, vector<unsigned char> &data){
      // {{{ open

      int len = 0;
      static const int LEN = 1024;
      vector<unsigned char *> buf(1,new unsigned char[LEN]);
      int curlen = fread(buf[0],1,LEN,fp);
      if(curlen == -1) ERROR_LOG("error reading file");
      while(curlen==LEN){
        len += curlen;
        buf.push_back(new unsigned char[LEN]);
        curlen = fread(buf[buf.size()-1],1,LEN,fp);
        if(curlen == -1) ERROR_LOG("error reading file");
      }
      data.resize(len+curlen);
      for(unsigned int i=0;i<buf.size();i++){
        memcpy(&data[0]+i*LEN,buf[i],i==buf.size()-1 ? curlen : LEN);
        delete [] buf[i];
      }
    } 

    // }}}

#ifdef HAVE_LIBZ
    void buffer_file_gz(gzFile fp, vector<unsigned char> &data){
      // {{{ open

      int len = 0;
      static const int LEN = 1024;
      vector<unsigned char *> buf(1,new unsigned char[LEN]);
      int curlen = gzread(fp,buf[0],LEN);
      if(curlen == -1) ERROR_LOG("error reading file");
      while(curlen==LEN){
        len += curlen;
        buf.push_back(new unsigned char[LEN]);
        curlen = gzread(fp,buf[buf.size()-1],LEN);
        if(curlen == -1) ERROR_LOG("error reading file");
      }
      data.resize(len+curlen);
      for(unsigned int i=0;i<buf.size();i++){
        memcpy(&data[0]+i*LEN,buf[i],i==buf.size()-1 ? curlen : LEN);
        delete [] buf[i];
      }
    }

    // }}}
#endif

    static const char *s_apcOpenModes[4] = { "rb","r","wb","w" };

    bool file_exists(const string &filename){
      struct stat stFileInfo;
      return stat(filename.c_str(),&stFileInfo)==0;
    }
  }

  class FileImpl{
  public:
    FileImpl(const std::string &name):
      // {{{ open

      name(name),handle(0),
#ifdef HAVE_LIBZ
      gzipped(ioutils::endsWith(name,".gz")),
#endif
      bufferoffset(0),
      binary(false),
      precision("%8f")
    {
      break_apart(name,dir,basename,suffix,filename);
    }

    // }}}
    void open(File::OpenMode openmode){
      // {{{ open

      ICLASSERT_RETURN(!handle);
      const char *pcOpenMode = s_apcOpenModes[openmode];
#ifdef HAVE_LIBZ
      if(gzipped){
        handle = gzopen(name.c_str(),pcOpenMode);
      }else{
        handle = fopen(name.c_str(),pcOpenMode);
      }
#else
      handle = fopen(name.c_str(),pcOpenMode);
#endif
      binary = openmode == File::readBinary || openmode == File::writeBinary;
      
      this->openmode = openmode;
    }
    
    // }}}
    
    void reopen(File::OpenMode openmode){
      // {{{ open

      if(!handle){
        open(openmode);
        return;
      }

      const char *pcOpenMode = s_apcOpenModes[openmode];
      
#ifdef HAVE_LIBZ
      if(gzipped){
        gzclose((gzFile)handle);
        handle = gzopen(name.c_str(),pcOpenMode);
      }else{
        handle = freopen(name.c_str(),pcOpenMode,(FILE*)handle);
      }
#else
      handle = freopen(name.c_str(),pcOpenMode,(FILE*)handle);
#endif
      binary = openmode == File::readBinary || openmode == File::writeBinary;
      
      this->openmode = openmode;
    }

    // }}}
    
    void close(){
      // {{{ open

      ICLASSERT_RETURN(handle);
#ifdef HAVE_LIBZ
      if(gzipped){
        gzclose((gzFile)handle);
      }else{
        fclose((FILE*)handle);
      }
#else
      fclose((FILE*)handle);
#endif
      handle = 0;
    }

    // }}}
    void bufferData() {
      // {{{ open

      ICLASSERT_RETURN(handle);
      if(buffer.size()) return;
#ifdef HAVE_LIBZ
      if(gzipped){
        buffer_file_gz((gzFile)handle,buffer);
      }else{
        buffer_file((FILE*)handle,buffer);
      }
#else
      buffer_file((FILE*)handle,buffer);
#endif
      bufferoffset = 0;
    }

    // }}}

    string name;
    string dir;
    string suffix;
    string basename;
    string filename;

    void *handle;
#ifdef HAVE_LIBZ
    bool gzipped;
#endif
    vector<icl8u> buffer;
    mutable int bufferoffset;
    bool binary;
    string precision;
    char writebuf[100];
    File::OpenMode openmode;
  };

  void FileImplDelOp::delete_func(FileImpl *i){
    // {{{ open

    ICL_DELETE( i );
  }

  // }}}


  
  File::File():
    // {{{ open
    ShallowCopyable<FileImpl,FileImplDelOp>(0){

  }

  // }}}
  File::File(const std::string &name):
    // {{{ open
    ShallowCopyable<FileImpl,FileImplDelOp>(new FileImpl(name)){
  }

  // }}}
  File::File(const std::string &name, File::OpenMode openmode):
    // {{{ open
    ShallowCopyable<FileImpl,FileImplDelOp>(new FileImpl(name)){
    open(openmode);
  }

  // }}}
  File::~File(){
    // {{{ open

    if(isOpen()) close();
  }

  // }}}
  
  bool File::isBinary() const{
    // {{{ open
    ICLASSERT_RETURN_VAL(!isNull(),false);
    return impl->binary;
  }

  // }}}

  bool File::exists() const{ 
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),false);
    return file_exists(impl->name);
  }

  // }}}
  bool File::isOpen() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),false);
    return impl->handle != 0;
  }

  // }}}

  string File::getDir() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->dir;
  }

  // }}}
  string File::getBaseName() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->basename;
  }

  // }}}
  string File::getSuffix() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->suffix;
  }

  // }}}
  string File::getName() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->name;
  }

  // }}}
  
  void File::write(const void *data, int len){
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    ICLASSERT_RETURN(isOpen());

    int bytesWritten = 0;
      
#ifdef HAVE_LIBZ
    if(impl->gzipped){
      while(bytesWritten < len){
        bytesWritten += gzwrite((gzFile)(impl->handle),data,len); 
      }
    }else{
      while(bytesWritten < len){
        bytesWritten += fwrite(data,1,len,(FILE*)(impl->handle));
      }
    }
#else
    while(bytesWritten < len){
      bytesWritten += fwrite(data,1,len,(FILE*)(impl->handle));
    }
#endif
  }

  // }}}
  void File::writeLine(const void *data, int len){
    // {{{ open
    ICLASSERT_RETURN(!isBinary());
    write(data,0);
    write(&NEW_LINE,1);
  }

  // }}}
  void File::write(const string &text){
    // {{{ open
    if(isBinary()){
      write(text.c_str(),text.length());
    }else{
      write(text.c_str(),text.length());
    }
  }

  // }}}
  
  void File::setPrecision(const std::string &p){
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    impl->precision = p;
  }

  // }}}
  
  File &File::operator<<(char c){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&c,sizeof(char));
    }else{
      sprintf(impl->writebuf,"%c",c);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(unsigned char uc){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&uc,sizeof(unsigned char));
    }else{
      sprintf(impl->writebuf,"%u",uc);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(int i){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&i,sizeof(int));
    }else{
      sprintf(impl->writebuf,"%d",i);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(unsigned int ui){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&ui,sizeof(unsigned int));
    }else{
      sprintf(impl->writebuf,"%u",ui);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(float f){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&f,sizeof(float));
    }else{
      sprintf(impl->writebuf,impl->precision.c_str(),f);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(double d){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),*this);
    if(isBinary()){
      write(&d,sizeof(double));
    }else{
      sprintf(impl->writebuf,impl->precision.c_str(),d);
      write(impl->writebuf);
    }
    return *this;
  }

  // }}}
  File &File::operator<<(const std::string &s){
    // {{{ open

    write(s);
    return *this;
  }

  // }}}

  bool File::canRead() const {
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    return getOpenMode() == readText || getOpenMode() == readBinary;
  }

  // }}}
  bool File::canWrite() const {
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    return getOpenMode() == writeText || getOpenMode() == writeBinary;
  }

  // }}}
  
  bool File::hasMoreLines() const{
    // {{{ open
    return bytesAvailable();
  }

  // }}}
  int File::bytesAvailable() const{
    // {{{ open
    ICLASSERT_RETURN_VAL(!isNull(),0);    
    ICLASSERT_RETURN_VAL(isOpen(),0);
    const vector<icl8u> &data = readAll();
    int offs = impl->bufferoffset;
    return std::max<unsigned int>(0,data.size()-offs);
  }

  // }}}
  int File::getFileSize() const{
    // {{{ open
    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    ICLASSERT_RETURN_VAL(canRead(),0);
    return readAll().size();
  }

  // }}}
  
  string File::readLine() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    ICLASSERT_RETURN_VAL(isOpen(),"");
    ICLASSERT_RETURN_VAL(hasMoreLines(),"");
    
    const vector<icl8u> &data = readAll();
    unsigned int i=impl->bufferoffset;
    while(i<data.size()&& data[i] != NEW_LINE){
      i++;
    }
    string s(data.begin()+impl->bufferoffset,data.begin()+i);
    if(isBinary()){
      i++;
    }else{
      while(i<data.size() && ( data[i]==NEW_LINE || data[i]==0 ) ){
        i++;
      }
    }
    impl->bufferoffset = i;
    return s;

  }

  // }}}
  string &File::readLine(string &dst) const{
    // {{{ open

    static string _null;
    ICLASSERT_RETURN_VAL(!isNull(),_null);
    ICLASSERT_RETURN_VAL(isOpen(),_null);
    ICLASSERT_RETURN_VAL(hasMoreLines(),_null);
    ICLASSERT_RETURN_VAL(!isBinary(),_null);

    const vector<icl8u> &data = readAll();
    unsigned int i=impl->bufferoffset;
    while(i<data.size()&& data[i] != NEW_LINE){
      i++;
    }
    dst.resize(i-impl->bufferoffset);
    std::copy(data.begin()+impl->bufferoffset,data.begin()+i,dst.begin());

    while(i<data.size() && ( data[i]==NEW_LINE || data[i]==0 ) ){
      i++;
    }
    impl->bufferoffset = i;
    return dst;

  }

  // }}}
  vector<icl8u> File::read(int len) const{
    // {{{ open

    static vector<icl8u> vec;
    ICLASSERT_RETURN_VAL(!isNull(),vec);
    ICLASSERT_RETURN_VAL(isOpen(),vec);
    ICLASSERT_RETURN_VAL(len <= bytesAvailable(),vec);
    vec.resize(len);
    const vector<icl8u> &data = readAll();
    std::copy(&data[0]+impl->bufferoffset,&data[0]+impl->bufferoffset+len,&vec[0]);
    impl->bufferoffset+=len;
    return data;
  }

  // }}}
  
  int File::read(int len, void *dst) const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);

    len = iclMin(len,bytesAvailable());
    
    const vector<icl8u> &data = readAll();
    std::copy(&data[0]+impl->bufferoffset,&data[0]+impl->bufferoffset+len,(icl8u*)dst);
    impl->bufferoffset+=len;
    
    return len;
  }

  // }}}
  
  const vector<icl8u> &File::readAll() const{
    // {{{ open
    static vector<icl8u> _null;
    ICLASSERT_RETURN_VAL(!isNull(),_null);
    ICLASSERT_RETURN_VAL(isOpen(),_null);
    const_cast<FileImpl*>(impl.get())->bufferData();
    return impl->buffer;
  }

  // }}}

  const icl8u* File::getCurrentDataPointer() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    return (&(readAll()[0]))+impl->bufferoffset;
  }

  // }}}
  
  const icl8u* File::getFileDataPointer() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    return &readAll()[0];
  }

  // }}}


  void File::open(File::OpenMode openmode){
    // {{{ open
    ICLASSERT_RETURN(!isNull());
    ICLASSERT_RETURN(!isOpen());
    ICLASSERT_RETURN(openmode != notOpen);
    
    impl->open(openmode);
    
    ICLASSERT_THROW(isOpen(),FileOpenException(getName()+"(OpenMode: "+toString(openmode)+")"));
  }

  // }}}
  void File::close(){
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    impl->close();
  }

  // }}}

  void File::reset(){
    // {{{ open
    ICLASSERT_RETURN(!isNull());
    if(getOpenMode() == readBinary || getOpenMode() == readText){
      impl->bufferoffset = 0;
    }else{
      ERROR_LOG("resetting file position is only allowed in read mode (which is buffered!)");
    }
  }

  // }}}
  
  void File::erase(){
    // {{{ open

    ICLASSERT_RETURN(!isNull() && exists());
    remove(getName().c_str());
  }

  // }}}
  
  void File::reopen(File::OpenMode om){
    // {{{ open
    ICLASSERT_RETURN(!isNull());
    impl->reopen(om);
    ICLASSERT_THROW(isOpen(),FileOpenException(getName()+"(OpenMode: "+toString(om)+")"));
  }

  // }}}
  
  File::OpenMode File::getOpenMode() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),notOpen);
    ICLASSERT_RETURN_VAL(isOpen(),notOpen);
    return impl->openmode;
  }

  // }}}
  void *File::getHandle() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(isOpen(),0);
    return impl->handle;
  }

  // }}}
}

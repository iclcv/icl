/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/File.cpp                         **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/File.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/StringUtils.h>
#include <sys/stat.h>
#include <cstring>
#include <stdio.h>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif


namespace icl{
  namespace utils{

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
      
      void break_apart(std::string s, std::string &dir, std::string &basename, std::string &suffix, std::string &filename){
        // {{{ open
  
        size_t p = s.rfind(DIR_SEPERATOR);
        
        /// split directory xxx/filename.suffix
        if(p==std::string::npos){
          dir = "";
          filename = s;
        }else{
          dir = s.substr(0,p);
          filename = s.substr(p+1);
        }
        
        // split suffix
        p = filename.rfind('.');
        if(p == std::string::npos){
          suffix = "";
          basename = filename;
        }else{
          suffix = filename.substr(p);
          basename = filename.substr(0,p);
          if(suffix == ".gz"){
            p = basename.rfind('.');                   ;
            if(p != std::string::npos){
              suffix = basename.substr(p)+suffix;
              basename = basename.substr(0,p-1);
            }
          }
        }
      }
  
      // }}}
      void buffer_file(FILE *fp, std::vector<unsigned char> &data){
        // {{{ open
  
        int len = 0;
        static const int LEN = 1024;
        std::vector<unsigned char *> buf(1,new unsigned char[LEN]);
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
      void buffer_file_gz(gzFile fp, std::vector<unsigned char> &data){
        // {{{ open
  
        int len = 0;
        static const int LEN = 1024;
        std::vector<unsigned char *> buf(1,new unsigned char[LEN]);
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
  
      bool file_exists(const std::string &filename){
        struct stat stFileInfo;
        return stat(filename.c_str(),&stFileInfo)==0;
      }
      
      bool file_is_dir(const std::string &filename){
        struct stat s;
        if( stat(filename.c_str(),&s) != 0 ) return false; // dos not exist
        else return s.st_mode & S_IFDIR;
      }
    }
  
    class FileImpl{
    public:
      FileImpl(const std::string &name):
        // {{{ open
  
        name(name),handle(0),
  #ifdef HAVE_LIBZ
        gzipped(endsWith(name,".gz")),
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
  
      std::string name;
      std::string dir;
      std::string suffix;
      std::string basename;
      std::string filename;
  
      void *handle;
  #ifdef HAVE_LIBZ
      bool gzipped;
  #endif
      std::vector<icl8u> buffer;
      mutable int bufferoffset;
      bool binary;
      std::string precision;
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

    bool File::isDirectory() const{
      // {{{ open
      ICLASSERT_RETURN_VAL(!isNull(),false);
      return file_is_dir(impl->name);
    }
    // }}}
  
    // }}}
    bool File::isOpen() const{
      // {{{ open
  
      ICLASSERT_RETURN_VAL(!isNull(),false);
      return impl->handle != 0;
    }
  
    // }}}
  
    std::string File::getDir() const{
      // {{{ open
  
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return impl->dir;
    }
  
    // }}}
    std::string File::getBaseName() const{
      // {{{ open
  
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return impl->basename;
    }
  
    // }}}
    std::string File::getSuffix() const{
      // {{{ open
  
      ICLASSERT_RETURN_VAL(!isNull(),"");
      return impl->suffix;
    }
  
    // }}}
    std::string File::getName() const{
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
    void File::write(const std::string &text){
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
      const std::vector<icl8u> &data = readAll();
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
    
    std::string File::readLine() const{
      // {{{ open
  
      ICLASSERT_RETURN_VAL(!isNull(),"");
      ICLASSERT_RETURN_VAL(isOpen(),"");
      ICLASSERT_RETURN_VAL(hasMoreLines(),"");
      
      const std::vector<icl8u> &data = readAll();
      unsigned int i=impl->bufferoffset;
      while(i<data.size()&& data[i] != NEW_LINE){
        i++;
      }
      std::string s(data.begin()+impl->bufferoffset,data.begin()+i);
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
    std::string &File::readLine(std::string &dst) const{
      // {{{ open
  
      static std::string _null;
      ICLASSERT_RETURN_VAL(!isNull(),_null);
      ICLASSERT_RETURN_VAL(isOpen(),_null);
      ICLASSERT_RETURN_VAL(hasMoreLines(),_null);
      ICLASSERT_RETURN_VAL(!isBinary(),_null);
  
      const std::vector<icl8u> &data = readAll();
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
    std::vector<icl8u> File::read(int len) const{
      // {{{ open
  
      static std::vector<icl8u> vec;
      ICLASSERT_RETURN_VAL(!isNull(),vec);
      ICLASSERT_RETURN_VAL(isOpen(),vec);
      ICLASSERT_RETURN_VAL(len <= bytesAvailable(),vec);
      vec.resize(len);
      const std::vector<icl8u> &data = readAll();
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
      
      const std::vector<icl8u> &data = readAll();
      std::copy(&data[0]+impl->bufferoffset,&data[0]+impl->bufferoffset+len,(icl8u*)dst);
      impl->bufferoffset+=len;
      
      return len;
    }
  
    // }}}
    
    const std::vector<icl8u> &File::readAll() const{
      // {{{ open
      static std::vector<icl8u> _null;
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

    std::string File::read_file(const std::string &filename, bool textMode){
      File f(filename,textMode ? File::readText:File::readBinary);
      const int len = f.getFileSize();
      std::string s(len+1,'\0');
      std::copy(f.getFileDataPointer(),f.getFileDataPointer()+len,&s[0]);
      return s;
    }
    
    std::vector<std::string> File::read_lines(const std::string &filename){
      return tok(read_file(filename,true),"\\n");
    }

    void File::write_file(const std::string &filename, const std::string &text, bool textMode){
      File f(filename,textMode ? File::writeText:File::writeBinary);
      f.write(text);
    }

    void File::write_lines(const std::string &filename, const std::vector<std::string> &lines){
      File f(filename,File::writeText);
      for(size_t i=0;i<lines.size();++i){
        f.writeLine(&lines[i],lines[i].length());
      }
    }

  } // namespace io
}

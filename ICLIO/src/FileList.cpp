/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/FileList.cpp                                 **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/FileList.h>

#ifdef ICL_SYSTEM_LINUX
#include <wordexp.h>
#endif
#ifdef ICL_SYSTEM_APPLE // wordexp not supported on osx
#include <glob.h>
#endif

#include <ICLIO/File.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>
#include <algorithm>
#include <sstream>
#include <set>
#include <ICLIO/IOUtils.h>

using namespace std;

namespace icl{

  namespace{
    inline void replace_newline (string::value_type& c) {
      // {{{ open

      if (c == '\n') c = ' ';
    }

    // }}}
  }
   
  class FileListImpl{
  public:
    FileListImpl(const std::string &pattern, bool omitDoubledFiles):
      // {{{ open
      m_bNoDoubledFiles(omitDoubledFiles){

      if(pattern == "") return;
      string sPattern = pattern;
      std::for_each (sPattern.begin(), sPattern.end(), replace_newline);

#ifndef ICL_SYSTEM_WINDOWS
#ifndef ICL_SYSTEM_APPLE
      wordexp_t match;
      
      // search for file matching the pattern(s)
      switch (wordexp (sPattern.c_str(), &match, WRDE_UNDEF)) {
        case 0: break;
        case WRDE_BADCHAR: 
          throw ICLException ("illegal chars in pattern (|, &, ;, <, >, (, ), {, }");
          break;
        case WRDE_BADVAL:
          throw ICLException ("encountered undefined shell variable");
          break;
        case WRDE_NOSPACE:
          throw ICLException ("out of memory");
          break;
        case WRDE_SYNTAX:
          throw ICLException ("syntax error, e.g. unbalanced parentheses or quotes");
          break;
      }
      
      char **ppcFiles = match.we_wordv;
      for (unsigned int i=0; i < match.we_wordc; ++i) {
        if(!strchr(ppcFiles[i],'*')){
          add(ppcFiles[i]);
        }
      }

      wordfree(&match);
#else /*__APPLE__*/
      glob_t pglob;

      int gflags = GLOB_MARK | GLOB_TILDE;
//#ifdef __APPLE__ // only supported on __APPLE__
      gflags |= GLOB_QUOTE;
//#endif

      int gerr = glob(sPattern.c_str(), gflags, NULL, &pglob);
      if (gerr) {
          // refine if necessary
          throw ICLException ("wrong use of glob");
          //  pglob.gl_pathc = 0;
      }

      for (unsigned int i=0; i<pglob.gl_pathc; ++i) {
        /// only add those patterns without remaining wildchards
        if(!strchr(pglob.gl_pathv[i],'*')){
          add(pglob.gl_pathv[i]);
        }
      }

      globfree(&pglob);
#endif
#else /* WIN32 */
      add(sPattern);
#endif /* WIN32 */
    }

    // }}}
    FileListImpl(const std::vector<std::string> &filenames)
      // {{{ open

      :m_vecFiles(filenames),m_bNoDoubledFiles(false){
    }

    // }}}
    const string &at(int i) const { 
      // {{{ open

      return m_vecFiles[i]; 
    }

    // }}}
    int size() const { 
      // {{{ open

      return (int)m_vecFiles.size(); 
    }

    // }}}
    void add(const string &filename){
      // {{{ open
      if(ioutils::endsWith(filename,".seq")){
        addSequence(filename);
      }else{
        if(m_bNoDoubledFiles){
          if(m_setFiles.find(filename) == m_setFiles.end()){
            m_setFiles.insert(filename);
            m_vecFiles.push_back(filename);
          }
        }else{
          m_vecFiles.push_back(filename);
        }
      }
    } 
    
    // }}}
  private:
    void addSequence(const string &filename){
      // {{{ open
      if(m_setSequenceFiles.find(filename) != m_setSequenceFiles.end()){
        return; // this sequnce was already added (abort to avoid infinite loops)
      }
      File seqFile(filename);
      ICLASSERT_RETURN(seqFile.exists());
      m_setSequenceFiles.insert(filename);
      
      seqFile.open(File::readText);
      while(seqFile.hasMoreLines()){
        add(seqFile.readLine());
      }            
    }
    // }}}

    vector<string> m_vecFiles;
    set<string> m_setFiles;
    set<string> m_setSequenceFiles;
    bool m_bNoDoubledFiles;
  };

  void FileListImplDelOp::delete_func(FileListImpl *i){
    // {{{ open

    ICL_DELETE( i );
  }

  // }}}
  
  FileList::FileList():
    // {{{ open
    ShallowCopyable<FileListImpl,FileListImplDelOp>(0){
  }

  // }}}
  
  FileList::FileList(const string &pattern, bool omitDoubledFiles):
    // {{{ open

    ShallowCopyable<FileListImpl,FileListImplDelOp>(new FileListImpl(pattern,omitDoubledFiles)){
  }

  // }}}
  
  FileList::FileList(const vector<string> &filenames):
    // {{{ open

    ShallowCopyable<FileListImpl,FileListImplDelOp>(new FileListImpl(filenames)){
  }

  // }}}
  
  FileList::~FileList(){
    // {{{ open

  }

  // }}}
  
  int FileList::size() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),0);
    return impl->size();
  }

  // }}}
  
  const string &FileList::operator[](int i) const{
    // {{{ open

    static string _null;
    ICLASSERT_RETURN_VAL( !isNull() ,_null);
    ICLASSERT_RETURN_VAL( i < size(),_null);
    return impl->at(i);
  }

  // }}}
  
  void FileList::join(const FileList &other){
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    for(int i=0;i<other.size();i++){
      impl->add(other[i]);
    }
  }

  // }}}

  void FileList::toSequenceFile(const std::string &seqFileName) const{
    // {{{ open

    ICLASSERT_RETURN(ioutils::endsWith(seqFileName,".seq"));
    File seqFile(seqFileName);
    ICLASSERT_RETURN(!seqFile.exists());
    seqFile.open(File::writeText);
    for(int i=0;i<size();i++){
      seqFile.write((*this)[i]);
    }
  }

  // }}}

  void FileList::show() const {
    // {{{ open

    if(isNull()){
      printf("FileList: NULL\n");
      return;
    }
    printf("FileList: %d files \n",size());
    for(int i=0;i<size();i++){
      printf("%d: %s\n",i,(*this)[i].c_str());
    }
  }

  // }}}

  string FileList::translateHashPattern(const std::string& sFileName) {
    // {{{ open

    std::string::size_type iSuffixPos=string::npos;
    unsigned int nHashes=0;
    
    // count number of hashes directly before file suffix
    ioutils::analyseHashes (sFileName, nHashes, iSuffixPos);
    if (nHashes) {
      // and replace them by [0-9] regular expressions
      ostringstream oss;
      for (unsigned int i=1; i <= nHashes; ++i) {
	oss << sFileName.substr(0, iSuffixPos-nHashes);
	for (unsigned int j=1; j <= i; ++j) oss << "[0-9]";
	oss << sFileName.substr(iSuffixPos) << " ";
      }
      return oss.str();
    }
    return sFileName;
  }

  // }}}
}

#include "iclFileList.h"
#ifndef WIN32
#ifndef __APPLE__
#include <wordexp.h>
#else // wordexp not supported on osx
#include <glob.h>
#endif
#endif
#include <iclFile.h>
#include <iclMacros.h>
#include <iclException.h>
#include <algorithm>
#include <sstream>
#include <set>
#include <iclIOUtils.h>

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

#ifndef WIN32
#ifndef __APPLE__
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
        add(ppcFiles[i]);
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
            add(pglob.gl_pathv[i]);
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

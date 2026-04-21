// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileList.h>

#include <filesystem>
#include <regex>

#include <icl/utils/Macros.h>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/File.h>

#include <algorithm>
#include <sstream>
#include <set>
using namespace icl::utils;

namespace icl::io {
  namespace{
    inline void replace_newline (std::string::value_type& c) {

      if (c == '\n') c = ' ';
    }

  }

  class FileListImpl{
  public:
    FileListImpl(const std::string &pattern, bool omitDoubledFiles):
      m_bNoDoubledFiles(omitDoubledFiles){

      if(pattern.empty()) return;
      std::string sPattern = pattern;
      std::for_each(sPattern.begin(), sPattern.end(), replace_newline);

      namespace fs = std::filesystem;
      fs::path patternPath(sPattern);
      fs::path dirPart = patternPath.parent_path();
      std::string globPart = patternPath.filename().string();

      // If no directory, use current directory
      if (dirPart.empty()) dirPart = ".";

      // Handle ~ (tilde) expansion for home directory
      if (dirPart.string().front() == '~') {
        const char *home = std::getenv("HOME");
        if (home) {
          std::string dirStr = dirPart.string();
          dirStr.replace(0, 1, home);
          dirPart = dirStr;
        }
      }

      // Check if the pattern has no wildcards — could be a direct file or .seq
      if (globPart.find('*') == std::string::npos &&
          globPart.find('?') == std::string::npos &&
          globPart.find('[') == std::string::npos) {
        // No wildcards — treat as literal filename
        std::string fullPath = (dirPart / globPart).string();
        // Normalize: if dirPart was ".", strip the "./" prefix for paths
        // that didn't have a directory component
        if (patternPath.parent_path().empty()) {
          add(globPart);
        } else {
          add(fullPath);
        }
        return;
      }

      // Convert glob to regex: escape special chars, then convert glob wildcards
      // Note: [ and ] pass through as-is (valid in both glob and regex)
      std::string regexStr;
      for (char c : globPart) {
        switch (c) {
          case '*': regexStr += ".*"; break;
          case '?': regexStr += "."; break;
          case '.': regexStr += "\\."; break;
          case '(': regexStr += "\\("; break;
          case ')': regexStr += "\\)"; break;
          case '+': regexStr += "\\+"; break;
          case '^': regexStr += "\\^"; break;
          case '$': regexStr += "\\$"; break;
          case '|': regexStr += "\\|"; break;
          case '{': regexStr += "\\{"; break;
          case '}': regexStr += "\\}"; break;
          default: regexStr += c; break;
        }
      }

      std::error_code ec;
      std::vector<std::string> matches;

      for (const auto &entry : fs::directory_iterator(dirPart, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().filename().string();
        if (std::regex_match(filename, std::regex(regexStr))) {
          matches.push_back(entry.path().string());
        }
      }

      // Sort for deterministic order (old implementations returned sorted results)
      std::sort(matches.begin(), matches.end());

      for (const auto &f : matches) {
        add(f);
      }
    }

    FileListImpl(const std::vector<std::string> &filenames)

      :m_vecFiles(filenames),m_bNoDoubledFiles(false){
    }

    const std::string &at(int i) const {

      return m_vecFiles[i];
    }

    int size() const {

      return static_cast<int>(m_vecFiles.size());
    }

    void add(const std::string &filename){
      if(endsWith(filename,".seq")){
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

  private:
    void addSequence(const std::string &filename){
      if(m_setSequenceFiles.find(filename) != m_setSequenceFiles.end()){
        return; // this sequnce was already added (abort to avoid infinite loops)
      }
      utils::File seqFile(filename);
      ICLASSERT_RETURN(seqFile.exists());
      m_setSequenceFiles.insert(filename);

      seqFile.open(File::readText);
      while(seqFile.hasMoreLines()){
        add(seqFile.readLine());
      }
    }

    std::vector<std::string> m_vecFiles;
    std::set<std::string> m_setFiles;
    std::set<std::string> m_setSequenceFiles;
    bool m_bNoDoubledFiles;
  };

  FileList::FileList() = default;

  FileList::FileList(const std::string &pattern, bool omitDoubledFiles):
    impl(std::make_shared<FileListImpl>(pattern,omitDoubledFiles)){
  }

  FileList::FileList(const std::vector<std::string> &filenames):
    impl(std::make_shared<FileListImpl>(filenames)){
  }


  FileList::~FileList(){

  }


  int FileList::size() const{

    ICLASSERT_RETURN_VAL(!isNull(),0);
    return impl->size();
  }


  const std::string &FileList::operator[](int i) const{

    static std::string _null;
    ICLASSERT_RETURN_VAL( !isNull() ,_null);
    ICLASSERT_RETURN_VAL( i < size(),_null);
    return impl->at(i);
  }


  void FileList::join(const FileList &other){

    ICLASSERT_RETURN(!isNull());
    for(int i=0;i<other.size();i++){
      impl->add(other[i]);
    }
  }


  void FileList::toSequenceFile(const std::string &seqFileName) const{

    ICLASSERT_RETURN(endsWith(seqFileName,".seq"));
    File seqFile(seqFileName);
    ICLASSERT_RETURN(!seqFile.exists());
    seqFile.open(File::writeText);
    for(int i=0;i<size();i++){
      seqFile.write((*this)[i]);
    }
  }


  void FileList::show() const {

    if(isNull()){
      std::cout << "FileList: NULL" << std::endl;
      return;
    }
    std::cout << "FileList: " << size() << " files" << std::endl;
    for(int i=0;i<size();i++){
      std::cout << i << ": " << (*this)[i] << std::endl;
    }
  }


  std::string FileList::translateHashPattern(const std::string& sFileName) {

    std::string::size_type iSuffixPos=std::string::npos;
    unsigned int nHashes=0;

    // count number of hashes directly before file suffix
    analyseHashes (sFileName, nHashes, iSuffixPos);
    if (nHashes) {
      // and replace them by [0-9] regular expressions
      std::ostringstream oss;
      for (unsigned int i=1; i <= nHashes; ++i) {
  oss << sFileName.substr(0, iSuffixPos-nHashes);
  for (unsigned int j=1; j <= i; ++j) oss << "[0-9]";
  oss << sFileName.substr(iSuffixPos) << " ";
      }
      return oss.str();
    }
    return sFileName;
  }

  } // namespace icl::io
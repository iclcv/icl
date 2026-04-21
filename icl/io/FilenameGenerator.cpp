// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FilenameGenerator.h>
#include <sstream>
#include <cstdio>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>

using namespace icl::utils;

namespace icl::io {
  namespace{
    std::string generate_filename(int digits, int counter, const std::string &prefix, const std::string &postfix){
      if (!digits) {
        return prefix+postfix;
      }
      std::ostringstream oss;
      oss << prefix;
      oss.fill('0');
      oss.width(digits);
      oss << counter << postfix;
      return oss.str ();
    }

    enum FilenameGeneratorMode { hashPatterns, objectAndImage };
  }

  class FilenameGeneratorImpl{
  public:
    FilenameGeneratorImpl(const std::string &patternIn, int maxFiles)

      :m_iMaxFiles(maxFiles),m_bInfinite(maxFiles<0){

      ICLASSERT(maxFiles != 0);

      std::string pattern = patternIn;
      bool startedWithHashHack = pattern.length() && pattern[0] == '#'; // here, we start with a hash
      if(startedWithHashHack){
        pattern = "_" + pattern;
      }

      m_eMode = hashPatterns;
      std::string::size_type postfixpos=0;
      analyseHashes(pattern,m_uiNumHashes,postfixpos);
      m_sPraefix = pattern.substr(0,postfixpos-m_uiNumHashes);
      m_sPostfix = pattern.substr(postfixpos);
      m_iCurrIdx = 0;
      m_iFilesLeft = m_bInfinite ? FilenameGenerator::INFINITE_FILE_COUNT : maxFiles;

      if(startedWithHashHack){
        m_sPraefix.clear();
      }
    }

    FilenameGeneratorImpl(const std::string& prefix, const std::string& postfix,
                          int ostart, int oend, int istart, int iend){
      m_eMode = objectAndImage;
      m_iCurrIdx = 0;
      m_iFilesLeft = 0;
      char *namebuf = new char[prefix.length()+postfix.length()+30];
      for (int i=ostart;i<=oend;++i){
        for (int j=istart;j<=iend;++j){
          snprintf(namebuf,prefix.length()+postfix.length()+30,"%s%d__%d%s",prefix.c_str(),i,j,postfix.c_str());
          m_vecFileNames.push_back(namebuf);
          m_iFilesLeft++;
        }
      }
      delete [] namebuf;
      ICLASSERT( m_iFilesLeft );
    }


    std::string next(){

      switch(m_eMode){
        case objectAndImage:{
          ICLASSERT_RETURN_VAL(m_iFilesLeft > 0, "");
          std::string nextname = m_vecFileNames[m_iCurrIdx++];
          m_iFilesLeft--;
          return nextname;
          break;
        }
        case hashPatterns:{
          ICLASSERT_RETURN_VAL( m_bInfinite || m_iCurrIdx < m_iMaxFiles,"");
          if(!m_bInfinite) m_iFilesLeft--;
          return generate_filename(m_uiNumHashes,m_iCurrIdx++,m_sPraefix,m_sPostfix);
          break;
        }
      }
      return "";
    }


    std::string showNext() const{

      switch(m_eMode){
        case objectAndImage:{
          ICLASSERT_RETURN_VAL(m_iFilesLeft > 0, "");
          return m_vecFileNames[m_iCurrIdx];
        }
        case hashPatterns:{
          ICLASSERT_RETURN_VAL( m_bInfinite || m_iCurrIdx < m_iMaxFiles,"");
          return generate_filename(m_uiNumHashes,m_iCurrIdx,m_sPraefix,m_sPostfix);
        }
      }
      return "";

    }

    int filesLeft() const{

      return m_iFilesLeft;
    }


    void reset(){

      switch(m_eMode){
        case objectAndImage:{
          m_iFilesLeft = m_vecFileNames.size();
          m_iCurrIdx = 0;
          break;
        }
        case hashPatterns:{
          m_iCurrIdx = 0;
          m_iFilesLeft = m_bInfinite ? FilenameGenerator::INFINITE_FILE_COUNT : m_iMaxFiles;
          break;
        }
      }
    }


    unsigned int m_uiNumHashes;
    std::string m_sPostfix;
    std::string m_sPraefix;
    int m_iMaxFiles;
    bool m_bInfinite;
    std::vector<std::string> m_vecFileNames;
    int m_iFilesLeft;
    int m_iCurrIdx;
    FilenameGeneratorMode m_eMode;
  };

  const int FilenameGenerator::INFINITE_FILE_COUNT = 2<<30;

  FilenameGenerator::FilenameGenerator() = default;

  FilenameGenerator::FilenameGenerator(const std::string &pattern, int maxFiles):
    impl(std::make_shared<FilenameGeneratorImpl>(pattern,maxFiles)){
  }

  FilenameGenerator::FilenameGenerator(const std::string& prefix, const std::string& postfix,
                                       int ostart, int oend, int istart, int iend):
    impl(std::make_shared<FilenameGeneratorImpl>(prefix,postfix,ostart,oend,istart,iend)){
  }


  FilenameGenerator::~FilenameGenerator(){

  }


  std::string FilenameGenerator::next(){

    ICLASSERT_RETURN_VAL(impl,0);
    return impl->next();
  }


  std::string FilenameGenerator::showNext() const{

    ICLASSERT_RETURN_VAL(impl,0);
    return impl->showNext();
  }


  int FilenameGenerator::filesLeft() const{

    ICLASSERT_RETURN_VAL(impl,0);
    return impl->filesLeft();
  }


  void FilenameGenerator::reset(){

    ICLASSERT_RETURN(impl);
    impl->reset();
  }


  std::vector<std::string> FilenameGenerator::getList(){

    ICLASSERT_RETURN_VAL(impl,std::vector<std::string>());
    ICLASSERT_RETURN_VAL(filesLeft() != FilenameGenerator::INFINITE_FILE_COUNT ,std::vector<std::string>());

    FilenameGeneratorImpl save = *impl.get();

    reset();
    std::vector<std::string> v;
    while(filesLeft()){
      v.push_back(next());
    }

    *impl.get() = save;
    return v;
  }


  void FilenameGenerator::show(){

    ICLASSERT_RETURN(impl);

    FilenameGeneratorImpl save = *impl.get();
    reset();

    int nFiles = filesLeft() == INFINITE_FILE_COUNT ? 10 : filesLeft();
    if( filesLeft() == INFINITE_FILE_COUNT ){
      printf("FilenameGenerator [file count: infinite]\n");
    }else{
      printf("FilenameGenerator [file count: %d]\n",nFiles);
    }
    for(int i=0;i<nFiles;i++){
      printf("%d: %s\n",i,next().c_str());
    }
    if( filesLeft() == INFINITE_FILE_COUNT ){
      printf("11: ...\n");
    }
    *impl.get() = save;

  }

  } // namespace icl::io
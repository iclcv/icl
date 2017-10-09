/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FilenameGenerator.cpp                  **
** Module : ICLIO                                                  **
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

#include <ICLIO/FilenameGenerator.h>
#include <sstream>
#include <cstdio>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace std;

namespace icl{
  namespace io{
    namespace{
      string generate_filename(int digits, int counter, const string &prefix, const string &postfix){
        // {{{ open
        if (!digits) {
          return prefix+postfix;
        }
        ostringstream oss;
        oss << prefix;
        oss.fill('0');
        oss.width(digits);
        oss << counter << postfix;
        return oss.str ();
      }
      // }}}

      enum FilenameGeneratorMode { hashPatterns, objectAndImage };
    }

    class FilenameGeneratorImpl{
    public:
      FilenameGeneratorImpl(const string &patternIn, int maxFiles)
        // {{{ open

        :m_iMaxFiles(maxFiles),m_bInfinite(maxFiles<0){

        ICLASSERT(maxFiles != 0);

        std::string pattern = patternIn;
        bool startedWithHashHack = pattern.length() && pattern[0] == '#'; // here, we start with a hash
        if(startedWithHashHack){
          pattern = "_" + pattern;
        }

        m_eMode = hashPatterns;
        string::size_type postfixpos=0;
        analyseHashes(pattern,m_uiNumHashes,postfixpos);
        m_sPraefix = pattern.substr(0,postfixpos-m_uiNumHashes);
        m_sPostfix = pattern.substr(postfixpos);
        m_iCurrIdx = 0;
        m_iFilesLeft = m_bInfinite ? FilenameGenerator::INFINITE_FILE_COUNT : maxFiles;

        if(startedWithHashHack){
          m_sPraefix.clear();
        }
      }

      // }}}
      FilenameGeneratorImpl(const string& prefix, const string& postfix,
                            int ostart, int oend, int istart, int iend){
        // {{{ open
        m_eMode = objectAndImage;
        m_iCurrIdx = 0;
        m_iFilesLeft = 0;
        char *namebuf = new char[prefix.length()+postfix.length()+30];
        for (int i=ostart;i<=oend;++i){
          for (int j=istart;j<=iend;++j){
            sprintf(namebuf,"%s%d__%d%s",prefix.c_str(),i,j,postfix.c_str());
            m_vecFileNames.push_back(namebuf);
            m_iFilesLeft++;
          }
        }
        delete [] namebuf;
        ICLASSERT( m_iFilesLeft );
      }

      // }}}

      string next(){
        // {{{ open

        switch(m_eMode){
          case objectAndImage:{
            ICLASSERT_RETURN_VAL(m_iFilesLeft > 0, "");
            string nextname = m_vecFileNames[m_iCurrIdx++];
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

      // }}}

      string showNext() const{

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
        // {{{ open

        return m_iFilesLeft;
      }

      // }}}

      void reset(){
        // {{{ open

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

      // }}}

      unsigned int m_uiNumHashes;
      string m_sPostfix;
      string m_sPraefix;
      int m_iMaxFiles;
      bool m_bInfinite;
      vector<string> m_vecFileNames;
      int m_iFilesLeft;
      int m_iCurrIdx;
      FilenameGeneratorMode m_eMode;
    };

    const int FilenameGenerator::INFINITE_FILE_COUNT = 2<<30;

    void FilenameGeneratorImplDelOp::delete_func(FilenameGeneratorImpl *i){
      // {{{ open

      ICL_DELETE( i );
    }

    // }}}

    FilenameGenerator::FilenameGenerator():
      // {{{ open
        ShallowCopyable<FilenameGeneratorImpl,FilenameGeneratorImplDelOp>(0){
    }

    // }}}

    FilenameGenerator::FilenameGenerator(const string &pattern, int maxFiles):
      // {{{ open

      ShallowCopyable<FilenameGeneratorImpl,FilenameGeneratorImplDelOp>(new FilenameGeneratorImpl(pattern,maxFiles)){
    }

    // }}}

    FilenameGenerator::FilenameGenerator(const string& prefix, const string& postfix,
                                         int ostart, int oend, int istart, int iend):
      // {{{ open

      ShallowCopyable<FilenameGeneratorImpl,FilenameGeneratorImplDelOp>(new FilenameGeneratorImpl(prefix,postfix,ostart,oend,istart,iend)){
    }

    // }}}

    FilenameGenerator::~FilenameGenerator(){
      // {{{ open

    }

    // }}}

    string FilenameGenerator::next(){
      // {{{ open

      ICLASSERT_RETURN_VAL(!isNull(),0);
      return impl->next();
    }

    // }}}

    string FilenameGenerator::showNext() const{
      // {{{ open

      ICLASSERT_RETURN_VAL(!isNull(),0);
      return impl->showNext();
    }

    // }}}

    int FilenameGenerator::filesLeft() const{
      // {{{ open

      ICLASSERT_RETURN_VAL(!isNull(),0);
      return impl->filesLeft();
    }

    // }}}

    void FilenameGenerator::reset(){
      // {{{ open

      ICLASSERT_RETURN(!isNull());
      impl->reset();
    }

    // }}}

    vector<string> FilenameGenerator::getList(){
      // {{{ open

      ICLASSERT_RETURN_VAL(!isNull(),vector<string>());
      ICLASSERT_RETURN_VAL(filesLeft() != FilenameGenerator::INFINITE_FILE_COUNT ,vector<string>());

      FilenameGeneratorImpl save = *impl.get();

      reset();
      vector<string> v;
      while(filesLeft()){
        v.push_back(next());
      }

      *impl.get() = save;
      return v;
    }

    // }}}

    void FilenameGenerator::show(){
      // {{{ open

      ICLASSERT_RETURN(!isNull());

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

    // }}}
  } // namespace io
}

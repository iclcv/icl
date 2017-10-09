#include "regex_icl.h"
#include <regex>
#include <memory>
#include <string>
#include <iostream>

namespace icl{
  namespace regex{

    struct regex_t::Impl{
		Impl() :foundAtAll(false){}
		bool foundAtAll;
		std::shared_ptr<std::regex> re;
		std::string error;
    };
    regex_t::regex_t():m_impl(0){
	}

    regex_t::~regex_t(){
      if(m_impl){
	     delete m_impl;
	     m_impl = 0;
      }
    }

    int regcomp(regex_t *re, const char *regex, int cflags){
	  //std::cout << "compiling regular expression -" << regex << "-" << std::endl;
      if(re->m_impl){
		delete re->m_impl;
		re->m_impl = 0;
      }
      re->m_impl = new regex_t::Impl;
	  re->m_impl->re.reset(new std::regex(regex,std::regex_constants::extended));
	  return 0;
    }

	void regerror(int status, regex_t *re, char *buf, int bufSize){
		std::string error = "";
		if (status == -1){
			error = "regex_t* was null";
		}else if (status == -2){
			error = "regex_t impl was null";
		}
		else if (status == -3){
			error = "regex was not complied before calling regexec";
		}else{
			error = re->m_impl->error;
		}
		// print error stored in re->impl into buf
		(void)status; // we use the workaround here, that the error is stored in the reg-ex type
		int i = 0;
		for (; i < (int)error.size() && (i < (bufSize - 1)); ++i){
			buf[i] = error[i];
		}
		if (bufSize){
			buf[i] = '\0';
		}
	}
    int regexec(regex_t *re, const char *text, int nSubMatches,
		regmatch_t *matches, int dummy){
		//std::cout << "matching regular expression against text -" << text << "-" << std::endl;
		if (!re) return -1;
		if (!re->m_impl) return -2;
		if (!re->m_impl->re) return -3;

		std::string s(text);
		std::smatch sm;
		std::regex_search(s, sm, *re->m_impl->re);
		re->m_impl->foundAtAll = !!sm.size();
		for (unsigned i = 0; i<sm.size(); ++i) {
			std::string::const_iterator begin = sm[i].first;
			std::string::const_iterator end = sm[i].second;
			std::string::const_iterator sBegin = s.begin();
			if ((int)i < nSubMatches){
				matches[i].rm_so = (int)(begin - sBegin);
				matches[i].rm_eo = (int)(end - sBegin);
			}
		}
		//std::cout << "returning from regmatch: -" << !sm.size() << "-" << std::endl;
		return !sm.size();
	}

    void regfree(regex_t *re){
		//std::cout << "regfree started" << std::endl;
      if(re->m_impl){
		delete re->m_impl;
		re->m_impl = 0;
      }
	 // std::cout << "regfree ended" << std::endl;
    }
  }
}



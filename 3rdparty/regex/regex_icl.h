#pragma once

namespace icl{
  namespace regex{
    // flags
    static const int REG_EXTENDED=1;
    static const int REG_NOSUB=2;

	struct regmatch_t{
		int rm_so; // start offset
		int rm_eo; // end offset
	};

    class regex_t{
      struct Impl;
      Impl *m_impl;
      friend int regcomp(regex_t *re, const char *regex, int cflags);
      friend void regerror(int status, regex_t *re, char *buf, int bufSize);
      friend int regexec(regex_t *re, const char *text, int nSubMatches,
		regmatch_t *matches, int dummy);
      friend void regfree(regex_t *re);
    public:
      regex_t();
      ~regex_t();
    };

    int regcomp(regex_t *re, const char *regex, int cflags);
    void regerror(int status, regex_t *re, char *buf, int bufSize);
    int regexec(regex_t *re, const char *text, int nSubMatches,
		regmatch_t *matches, int dummy);
    
    void regfree(regex_t *re);
    
  }
}


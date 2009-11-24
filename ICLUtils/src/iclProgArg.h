#ifndef PROG_ARG_H
#define PROG_ARG_H

#include <string>
#include <vector>
#include <iclStringUtils.h>
#include <iclException.h>

namespace icl{
  
  /// initialization of the ProgArg-Environment \ingroup PA
  /** The UCLUtils/ProgArg tool provides a more convenient access to
      executables arguments, given via "nArgs" and "ppcArg" in the main
      function.
      <H1>What?</H1>
      Consider a default main(int n, char **ppc) - application. If 
      the programs arguments have to be evaluated to affect the
      program flow, you have to take care about many things:
      - define what arguments you will need / which are allowed
      - parsing the list of arguments
      - parsing sub-arguments as "-grabber pwc"
      - converting the string-arguments in correct data types as in
        "-size 640 480"
      - printing error messages and usage if 
         - denied arguments were given the program
         - an argument go not the correct count of sub-arguments
         - list and explain possible arguments
         - ...
      <H1>ProgArg - a more convenient approach!</H1>
      The following example, also available in ICLUtils/examples/progargdemo,
      shows the advantages of using the ProgArg environment:
      \code

#include <iclProgArg.h>
using namespace icl;
  
int main(int n, char **ppc){

  pa_explain("-size",
             "image size\n"
             "first param = width (one of 160, 320 or 640)\n"
             "second param = height one of (120, 240 or 480)");
  pa_explain("-format","image format\none of:\n- formatRGB\n- formatGray\n- formatHLS");
  pa_explain("-channels","count of image channels\none of {1,2,3,4}");
  pa_explain("-fast","enables the \"fast\"-mode which does everything\nmuch faster!");

  pa_init(n,ppc,"-size(2) -format(1) -channels(1) -fast");
  
  printf("programs name is %s \n",pa_progname().c_str());
  printf("argcount is %d \n",pa_argcount());

  if(pa_defined("-size")){
    printf("given size was %d %d \n",pa_subarg<int>("-size",0),pa_subarg<int>("-size",1));  
  }
  if(pa_defined("-format")){
    printf("given format was %s \n",pa_subarg<char*>("-format",0));
  }
  if(pa_defined("-fast")){
    printf("enabling fast (whatever this will effect!?) \n");
  }
  
  for(unsigned int i=0;i<pa_argcount();i++){
    printf("arg %d was %s \n",i,pa_arg<char*>(i));
  }
  
  return 0;
}
  \endcode

  The following outputs are possible. 
  <pre>
  gordonfreeman\@blackmesa:~/projects/ICL/ICLUtils/examples> ./progargdemo -size 640 480 -channels 3 -format rgb -fast
  programs name is ./progargdemo
  argcount is 8
  given size was 640 480
  given format was rgb
  arg 0 was -size
  arg 1 was 640
  arg 2 was 480
  arg 3 was -channels
  arg 4 was 3
  arg 5 was -format
  arg 6 was rgb
  arg 7 was -fast

  but
 
  gordonfreeman\@blackmesa:~/projects/ICL/ICLUtils/examples> ./progargdemo -size 640 480 -slow
  error: nothing known about arg -slow [index=3]
  usage: progargdemo [ARGS] 
        allowed ARGS are:
        -channels(1) : count of image channels
                       one of {1,2,3,4}
        -fast : enables the "fast"-mode which does everything
                much faster!
        -format(1) : image format
                     one of:
                     - formatRGB
                     - formatGray
                     - formatHLS
        -size(2) : image size
                   first param = width (one of 160, 320 or 640)
                   second param = height one of (120, 240 or 480)
        --help : show this usage
  </pre>
  
      <H1>pa_init</H1>
      This function must be called before any other ProgArg function is available.
      The function knows two different modes:
      - if "allowedArgs" is given, actual program arguments are checked for
        being compatible to the argument definition (given with "allowedArgs").
      - if not, all params are valid, and the developer has to take care himself
        about tackling params.
      @param nArgs argument count received in the main function
      @param ppcArg argument vector received in the main function
      @param allowedArgs optional definition of the allowed arguments with the following
                         syntax: [ARG<(\#SUBARGS)>]. E.g. "-size(2) -input-file(1) -fast".
                         <em>More details in the example above!</em>  
      @param skipUnknownArgs if set to true, unknown args are just skipped, otherwise, the
             "usage" is shown, and the programm is aborted using exit(-1)
  */
  void pa_init(int nArgs, char **ppcArg, std::string allowedArgs="", bool skipUnknownArgs=false);
  
  /// returns the program name as it was written to start the program \ingroup PA
  /** e.g. "./myprogram" */
  const std::string &pa_progname();
  
  /// this function can be used to explain arguents  \ingroup PA
  /** e.g. if you call pa_init(n,ppc,"-s(1)"), the "-s" arg is not explained enogh
      propably. To do this, just call: 
      \code
      pa_explain("-s","sets up the current image size"); 
      \endcode
      and the following usage will help to understand programm args better:
      <pre>
      usage: 
      </pre>
      <b>NOTE: pa_explain(..) must be called BEFORE pa_init() is called !!</b>
      @param argname name of the argument to explain
      @param explanation explanation for argname
  */
  void pa_explain(const std::string &argname, const std::string &explanation);

  /// returns the count of parameters actually given \ingroup PA
  unsigned int pa_argcount();
  
  /// writes the error message followed by a usage definition \ingroup PA
  /** the usage is only defined, if "allowedArgs" was set in pa_init */
  void pa_usage(const std::string &errorMessage="");
  
  /// returns weather a certain argument was actually given \ingroup PA
  bool pa_defined(const std::string &param);

  /// returns list of args not defined in pa_defined and not subargs of those
  /** only available if pa_init was called with skipUnknownArgs*/
  const std::vector<std::string> &pa_dangling_args();
  
  /// internal utility function
  const std::string &pa_arg_internal(unsigned int index) throw (ICLException);

  /// internal utility function
  const std::string &pa_subarg_internal(const std::string &param, unsigned int idx) throw (ICLException);
  
  /// access to the actually given program arguments \ingroup PA
  /** <b>Note:</b> Arguments are received as double and than parsed using the 
      std::istream-operator-based icl::parse-template function.
  */
  template<class T> 
  inline T pa_arg(unsigned int index) throw (ICLException){
    return parse<T>(pa_arg_internal(index));
  }

  /// access to sub arguments with a given default value \ingroup PA
  /** If the given argument "param" was not actually given, the default argument
      is returned without an additional warning message.
      Possible types T are: 
      - string (std::string)
      - int 
      - uint 
      - bool 
      - char 
      - uchar 
      - float 
      - double
  */
  template<class T> 
  inline T pa_subarg(const std::string &param, unsigned int index, T defaultValue) throw (ICLException){
      try{
        return parse<T>(pa_subarg_internal(param,index));
      }catch(...){}
      return defaultValue;
   }
        
  

}

#endif

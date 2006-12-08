#ifndef PROG_ARG_H
#define PROG_ARG_H

#include <string>

namespace icl{
  
  /// initialization of the ProgArg-Environment
  /** The UCLUtils/ProgArg tool provides a more convenient access to
      executables arguments, given via "nArgs" and "ppcArg" in the main
      function.
      <H1>What?</H2>
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
         - ...
      <H1>ProgArg - a more convenient approach!</H1>
      The following example, also available in ICLUtils/examples/progargdemo,
      shows the advantages of using the ProgArg environment:
      <pre>
#include "ProgArg.h"
  
using namespace icl;
  
int main(int n, char **ppc){
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
  </pre>
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
  usage: ./progargdemo [ARGS]
         allowed ARGS are:
         -channels(1)
         -fast
         -format(1)
         -size(2)
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
                         syntax: [ARG<(#SUBARGS)>]. E.g. "-size(2) -input-file(1) -fast".
                         <em>More details in the example above!</em>  
  */
  void pa_init(int nArgs, char **ppcArg, std::string allowedArgs="");
  
  /// returns the program name as it was written to start the program
  /** e.g. "./myprogram" */
  const std::string &pa_progname();

  /// returns the count of parameters actually given
  unsigned int pa_argcount();
  
  /// writes the error message followed by a usage definition
  /** the usage is only defined, if "allowedArgs" was set in pa_init */
  void pa_usage(std::string errorMessage);
  
  /// returns weather a certain argument was actually given
  bool pa_defined(const std::string &param);
  
  /// access to the actually given program arguments
  /** <b>Note:</b> The index references all actually given args directly,
      sub args are not evaluated here.
      Possible types T are: 
      - char* 
      - int 
      - uint 
      - bool 
      - char 
      - uchar 
      - float 
      - double
  */
  template<class T> 
  T pa_arg(unsigned int index);

  /// access to sub arguments with a given default value
  /** If the given argument "param" was not actually given, the default argument
      is returned without an additional warning message.
      Possible types T are: 
      - char* 
      - int 
      - uint 
      - bool 
      - char 
      - uchar 
      - float 
      - double
  */
  template<class T> 
  T pa_subarg(std::string param, unsigned int index, T def=0);

}

#endif

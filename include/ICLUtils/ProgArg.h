#ifndef ICL_PROGARG_H
#define ICL_PROGARG_H

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Macros.h>

namespace icl{

  /// Programm argument environment exception type
  struct ProgArgException : public ICLException{
    ProgArgException(const std::string &func, const std::string &what):
    ICLException(func+":"+what){}
  };
#define THROW_ProgArgException(X) throw ProgArgException(__FUNCTION__,(X))
  
  /** \cond */
  // internally used programm argument data type
  class ProgArgData{
    protected:
    friend const std::string &pasubarg_internal(const ProgArgData &pa) throw (ProgArgException);
    std::string id;
    int subargidx;
    bool danglingOnly;
    inline ProgArgData(const std::string &id, int subargidx):
      id(id),subargidx((int)subargidx){
    }
    inline ProgArgData(unsigned int idx, bool danglingOnly):
      subargidx(idx),danglingOnly(danglingOnly){
    }
      
  };
  /** \endcond */
  
  /** \cond */
  // internal function for program argument explanation
  void paex_internal(const std::string &pa, const std::string &ex);
  
  // internal function that checks if a given arg was actually given
  bool padefined_internal(const std::string &pa);

  // internal sub-argument access function
  const std::string &pasubarg_internal(const ProgArgData &pa) throw (ProgArgException);
  /** \endcond */
  
  /// Programm argument utility class 
  /** @see icl::pa(const std::string&,unsigned int) */
  class ProgArg : public ProgArgData, public std::string{
    /// private constructor
    /** Use the functions icl::pa(const std::string&,unsigned int) and
        icl::pa(unsigned int,bool) to create an instance of this 
        class in order to access program arguments*/
    inline ProgArg(const std::string &id, unsigned int subargidx):
      ProgArgData(id,subargidx),std::string(pasubarg_internal(*this)){
    }
    
    inline ProgArg(unsigned int idx, bool danglingOnly):
      ProgArgData(idx,danglingOnly),std::string(pasubarg_internal(*this)){
    }
    
    public:
    /** \cond */
    // undocumented friend
    friend const ProgArg pa(const std::string &,unsigned int) throw (ProgArgException);
    // undocumented friend
    friend const ProgArg pa(unsigned int,bool);
    /** \endcond */
    
    /// returns the count of actually given sub arguments
    /** If this argument was not given, this function returns 0.*/
    int n() const;
    
    /// this is the main conversion function. It returns the associated sub argument as given T
    /** If T is bool, this operator returns whether the arg was given rather than
        its value */
    template<class T>
    inline operator T() const throw (ProgArgException){
      return parse<T>(*this);
    }
  };

  /// just puts the referenced argument value as string into the lvalue-stream
  //  std::ostream &operator<<(std::ostream &s,const ProgArg &pa){
  //  std::string t = pa;
  //  return s << t;
  //}
  
  /** \cond */
  // explicit specialization for bool types (returns whether the arg was actually given)
  template<>
  inline ProgArg::operator bool() const{
    return padefined_internal(id);
  }
  /** \endcond */
    

  /// returns given program argument 
  /** The pa-function is the main interface for extracting information 
      about given program arguments and/or their default values at run-time.
      
      The returned icl::ProgArg instance is always automatically
      parsed from its internal string representation into the expressions
      lvalue-type (this can easily be implemented with a <em>templated</em>
      version of the implicit cast operator of a class). Here are some 
      examples:
      
      \code
      
      painit(n,ppc,"-size|-s(Size=VGA) -index(int) -files(...)");
      
      // extract the first sub-argument of argument 
      // '-index' and convert it into an int value
      int i = pa("-index"); 

      // extract the first sub-argument of argument '-size'
      // if '-s' is the shortcut for '-size'
      Size s = pa("-s); 

      // extract the 2nd sub-argument of argument '-input'
      // if '-s' is the shortcut for '-size'
      int = pa("-input",1); 
      
      // check if argument -size was actually given
      if(pa("-size")){ ... }
      
      // read a list of files from the arbitrary sub-argument
      // arg '-files'. Note: it is not recommended to use 
      // pa("-files") within a loop, as internally the argument hash-
      // map must always be searched
      int nFiles = pa("-files").n();
      for(int i=0;i<nFiles;++i){
         std::cout << "file " << i << pa("-files",i) << std::endl;
      }

      // list all given arguments and subarguments
      std::cout << "all arguments " << std::endl;
      for(unsigned int i=0;i<pacount(false);++i){
        std::cout << pa(i,false) << std::endl;
      }

      // in case of having dangling arguments allowed in 
      // painit-call: list all dangling arguments
      std::cout << "all dangling arguments " << std::endl;
      for(unsigned int i=0;i<pacount();++i){
        std::cout << pa(i) << std::endl;
      }

      \endcode

      @see icl::painit(int,char**,const std::string&,bool)
  */
  inline const ProgArg pa(const std::string &id, unsigned int subargidx=0) throw (ProgArgException){
    if(!id.length()) THROW_ProgArgException("invalid programm argument id ''");
    return ProgArg(id,subargidx);
  }
  
  /// returns given program argument at given index
  /** @see icl::pa(const std::string&,unsigned int) */
  inline const ProgArg pa(unsigned int idx, bool danglingOnly=true){
    return ProgArg(idx,danglingOnly);
  }
  
  /// returns number of actually given args given
  /** @see icl::pa(const std::string&,unsigned int) */
  unsigned int pacount(bool danglingOnly=true);

  /// returns application name (full command line)
  /** @see icl::pa(const std::string&,unsigned int) */
  const std::string &paprogname();

  /// shows current available programm arguments
  void pausage(const std::string &msg="");
  
  /** \cond */
  // utility class which allows the user to call the paex-function in a 'stacked' manner
  struct PAEX{
    PAEX operator()(const std::string &pa, const std::string &ex);
  };
  /** \endcond */
  
  /// This function can be used to provide additional information for certain program arguments
  /** Due to the use of a special but hidden utility structure called icl::PAEX, this
      function can be called in a 'stacked' manner. This mean, that the function
      can be called several times without repeating the function name. Example:
      \code
      paex("-size","defines the input image size")
          ("-input","defines input device id and parameters")
          ("-format","define input image format");
      \endcode
      @see icl::pa(const std::string&,unsigned int) */
  inline PAEX paex(const std::string &pa, const std::string &ex){
    paex_internal(pa,ex);
    return PAEX();
  }

  /** \cond */
  // deferred implementation of stacking operator
  inline PAEX PAEX::operator()(const std::string &pa, const std::string &ex){
    return paex(pa,ex);
  }
  /** \endcond */


  /// initialization function for ICL's program argument evaluation framework
  /** painit always receives your program's <tt>main</tt>-functions arguments
      as <tt>n</tt> and <tt>ppc</tt>. The actual definition of the underlying
      program argument evaluation framework is given by the <tt>init</tt>-
      argument.\n
      The following rules define the syntax for the <tt>init</tt>-string:\n
      - The init string consists of space-separated tokens of single program
        argument definitions. Necessary space-characters within these tokens
        must be escaped using a back-slash character.
      - Each single argument definition token consists of a pipe-separated
        list of argument name alternatives (e.g. <tt>-size|-s</tt> followed
        by an optional parameter list in round braces.
      - Each argument is allowed to have 0, N or unlimited sub-arguments. 
      - If the argument has no sub arguments it is a flag, which implicitly
        has the value true if it was given and false otherwise. Zero sub-
        arguments can be forced by using no parameter list, i.e., no round
        braces, an empty parameter list <tt>()</tt> or a parameter list with a zero
        argument count <tt>(0)</tt>.
      - An arbitrary sub-argument count can be reached by using the special
        parameter list <tt>(...)</tt>.
      - Otherwise, the parameter-list is a comma separated list of type,
        type=default-value, or argument-count tokens. Examples:
        - <tt>(int,int)</tt> defines two integer sub-arguments
        - <tt>(5)</tt> defines 5 sub-arguments with no type information
        - <tt>(float,2,int) defines 4 sub-arguments of type float, any, any and int
        - <tt>(int=4,Size=VGA) defines two sub-arguments of type int and Size
          with given default values 4 and VGA
        Note: if an int-type is used to define several arguments together,
        no defaults can be given. If you don't want to define the type, but
        only the default values, the special type string <tt>*</tt> should
        be used e.g., <tt>(*=foo,*=bar)</tt>
      
      Furthermore, it is worth to mention, that the defined types are always 
      just hints for the user. Internally all parameters are treated as strings.
      
      Here are some further complete example calls for painit.
      \code
      int main(int n, char **ppc){
        painit(n,ppc,"-size|-s(Size=VGA) -format|-f(format-string)");
      }
      \endcode
      \code
      int main(int n, char **ppc){
        painit(n,ppc,"-input|-i(device-type=dc,device-params=0)");
      }
      \endcode

      
      \section Dangling Arguments
      Sometimes, certain arguments shall be used directly without defining
      arguments and sub-arguments. E.g. if you have a converter application
      that gets an input and an output file only (e.g., program call: 
      <tt>myConvert image.ppm converted-image.ppm</tt> rather than something
      like <tt>myConvert -i image.ppm -o converted-image.ppm</tt>).
      Dangling arguments are these arguments that do not match defined
      arguments or sub-arguments of these. Usually, given arguments that do not
      match primary arguments or sub-arguments lead to a ProgArgException. 
      You can explicitly allow these <em>dangling</em> arguments by setting
      allowDanglingArgs to 'true'. \n
      Dangling arguments can then be obtained simply using the functions
      icl::pacount and icl::pa(unsigned int,bool).

      To list all dangling arguments, the followed code can be used:
      \code
      for(unsigned int i=0;i<pacount();++i){
        std::cout << pa(i) << std::endl;
      }
      \endcode

      \section Optional and Mandatory Arguments

      As default, all arguments are optional. If you want
      to define an argument to be mandatory, simply prepend
      a '[m]'-prefix to the argument name alternative list.
      Example:
      \code
      // -size and -format are optional; -input or -i is mandatory
      painit(n,ppc,"-size(Size=VGA) -format(format=RGB) "
                  "[m]-input|-i(string,string)");
      \endcode
      
      If mandatory arguments are not actually given to the program,
      a ProgArgException is thrown. As it does not make sense to
      define default values for mandatory arguments, a
      ProgArgException is thrown in this case as well. Furthermore,
      mandatory arguments must have sub-arguments (which is also
      sensible if you think about it). 
  */
  void painit(int n,char **ppc,const std::string &init, bool allowDanglingArgs=false);


  /// shows all given program arguments 
  void pashow();
  
}

#endif

/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/ProgArg.h                        **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Macros.h>

namespace icl{
  namespace utils{

    /// Programm argument environment exception type \ingroup PA \ingroup EXCEPT
    struct ProgArgException : public ICLException{
      ProgArgException(const std::string &func, const std::string &what):
      ICLException(func+":"+what){}
    };
  #define THROW_ProgArgException(X) throw ProgArgException(__FUNCTION__,(X))

    /** \cond */
    // internally used programm argument data type
    class ProgArgData{
    protected:
      friend ICLUtils_API const std::string &pa_subarg_internal(const ProgArgData &pa) throw (ProgArgException);
      friend ICLUtils_API bool pa_defined_internal(const ProgArgData &pa) throw (ProgArgException);
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
    ICLUtils_API void pa_explain_internal(const std::string &pa, const std::string &ex);

    // internal sub-argument access function
    ICLUtils_API const std::string &pa_subarg_internal(const ProgArgData &pa) throw (ProgArgException);

    // another internal helper function
    ICLUtils_API bool pa_defined_internal(const ProgArgData &pa) throw (ProgArgException);
    /** \endcond */

    /// Programm argument utility class \ingroup PA
    /** @see icl::pa(const std::string&,unsigned int) */
    class ProgArg : public ProgArgData{
      /// private constructor
      /** Use the functions icl::utils::pa(const std::string&,unsigned int) and
          icl::utils::pa(unsigned int,bool) to create an instance of this
          class in order to access program arguments*/
      inline ProgArg(const std::string &id, unsigned int subargidx):
        ProgArgData(id,subargidx){
      }

      inline ProgArg(unsigned int idx, bool danglingOnly):
        ProgArgData(idx,danglingOnly){
      }

      public:
      /** \cond */
      // undocumented friend
      friend ICLUtils_API const ProgArg pa(const std::string &,unsigned int) throw (ProgArgException);
      // undocumented friend
      friend ICLUtils_API const ProgArg pa(unsigned int, bool);
      // yet another one
      friend ICLUtils_API bool pa_defined_internal(const ProgArgData &pa) throw (ProgArgException);
      /** \endcond */

      /// returns the count of actually given sub arguments
      /** If this argument was not given, this function returns 0.*/
      ICLUtils_API int n() const throw (ProgArgException);

      /// returns the given sub-argument in shape of an utils::Any
      ICLUtils_API Any operator[](int subArgIdx) const throw (ProgArgException);

      /// this is the main conversion function. It returns the associated sub argument as given T
      /** If T is bool, this operator returns whether the arg was given rather than
          its value */
      template<class T>
      inline operator T() const throw (ProgArgException){
        return parse<T>(pa_subarg_internal(*this));
      }

      /// negation operator
      inline bool operator!() const throw (ProgArgException){
        return !pa_defined_internal(*this);
      }

      /// this template function can be used to explicitly cast a program argument into a given type
      template<class T>
      inline T as() const throw (ProgArgException){
        return parse<T>(pa_subarg_internal(*this));
      }

      /// important convenience operator for using a ProgArg instance as string
      /** <tt>*pa("-x")</tt> is the same as <tt>pa("-x").as<std::string>()</tt>.
          However, the first version is much shorter.*/
      inline std::string operator*() const throw (ProgArgException){
        return pa_subarg_internal(*this);
      }

      /// returns the prog-arg id
      inline const std::string &getID() const {
        return id;
      }
    };

    /// just puts the referenced argument value as string into the lvalue-stream
    inline std::ostream &operator<<(std::ostream &s, const ProgArg &pa){
      return s << pa.as<std::string>();
    }

    /** \cond */
    // explicit specialization for bool types (returns whether the arg was actually given)
    template<>
    inline ProgArg::operator bool() const throw(utils::ProgArgException){
      return pa_defined_internal(*this);
    }

    // explicit specialization for bool types (returns whether the arg was actually given)
    template<>
    inline bool ProgArg::as() const throw (ProgArgException){
      return pa_defined_internal(*this);
    }

    /** \endcond */

    /// this allows to check if two progargs are defined \ingroup PA
    /** this allows you to write:
        \code
        if(pa("-size") && pa("-scale")){
          ...
        }
        \endcode
    */
    inline bool operator&&(const ProgArg &a, const ProgArg &b){
      return a.as<bool>() && b.as<bool>();
    }

    /// allows to check more than two ProgArg instances at once \ingroup PA
    /** Example:
        \code
        if(pa("-size") && pa("-scale") && pa("-format")){ ... }
        \endcode
    */
    inline bool operator&&(const ProgArg &a, bool b){
      return b && a.as<bool>();
    }

    /// allows to check more than two ProgArg instances at once \ingroup PA
    /** Example:
        \code
        if(pa("-size") && pa("-scale") && pa("-format")){ ... }
        \endcode
    */
    inline bool operator&&(bool &b, const ProgArg &a){
      return b && a.as<bool>();
    }

    /// this allows to check if either of two progargs are defined \ingroup PA
    /** this allows you to write:
        \code
        if(pa("-size") || pa("-scale")){
          ...
        }
        \endcode
    */
    inline bool operator||(const ProgArg &a, const ProgArg &b){
      return a.as<bool>() || b.as<bool>();
    }
    /// allows to check if either of more than two ProgArg instances is defined \ingroup PA
    /** Example:
        \code
        if(pa("-size") || pa("-scale") || pa("-format")){ ... }
        \endcode
    */
    inline bool operator||(const ProgArg &a, bool b){
      return b || a.as<bool>();
    }

    /// allows to check if either of more than two ProgArg instances is defined \ingroup PA
    /** Example:
        \code
        if(pa("-size") || pa("-scale") || pa("-format")){ ... }
        \endcode
    */
    inline bool operator||(bool &b, const ProgArg &a){
      return b || a.as<bool>();
    }




    /// returns given program argument \ingroup PA
    /** The pa-function is the main interface for extracting information
        about given program arguments and/or their default values at run-time.

        The returned icl::utils::ProgArg instance is always automatically
        parsed from its internal string representation into the expressions
        lvalue-type (this can easily be implemented with a <em>templated</em>
        version of the implicit cast operator of a class). Here are some examples:

        \code

        pa_init(n,ppc,"-size|-s(Size=VGA) -index(int) -files(...)");

        // extract the first sub-argument of argument
        // '-index' and convert it into an int value
        int i = pa("-index");

        // extract the first sub-argument of argument '-size'
        // if '-s' is the shortcut for '-size'
        Size s = pa("-s");

        // extract the 2nd sub-argument of argument '-input'
        int = pa("-input",1);

        // check if argument -size was actually given
        if(pa("-size")){ ... }

        // read a list of files from the arbitrary sub-argument
        // arg '-files'. Note: it is not recommended to use
        // pa("-files") within a loop, as internally the argument hash-
        // map must always be searched
        int nFiles = pa("-files").n();
        for(int i=0;i<nFiles;++i){
           std::cout << "file " << i << " " << pa("-files",i) << std::endl;
        }

        // alternatively, the "-files" prog-arg can be extracted
        ProgArg f = pa("-files");
        for(int i=0;i<f.n();++i){
           std::cout << "file " << i << " " << f[i] << std::endl;
        }

        // list all given arguments and subarguments
        std::cout << "all arguments " << std::endl;
        for(unsigned int i=0;i<pa_get_count(false);++i){
          std::cout << pa(i,false) << std::endl;
        }

        // in case of having dangling arguments allowed in
        // painit-call: list all dangling arguments
        std::cout << "all dangling arguments " << std::endl;
        for(unsigned int i=0;i<pa_get_count();++i){
          std::cout << pa(i) << std::endl;
        }

        // using ProgArg instances as std::strings is sometimes
        // a bit complicated as conversion to std::string is
        // sometimes ambiguous
        struct Test{
           Test(int i){...}
           Test(const std::string &s){ .. }
        };
        ...
        Test t(pa("-x")); // ambiguous -> int | std::string
        Test t((std::string)pa("-x")); // also ambiguous
        // due to different available std::string constructors

        Test t(pa("-x").as<std::string>()); // works, but long

        Test t(*pa("-x")); // much shorter, but only for using
        // a program argument as std::string

        \endcode

        @see icl::utils::pa_init(int,char**,const std::string&,bool)*/

    inline const ProgArg pa(const std::string &id, unsigned int subargidx = 0) throw (ProgArgException){
      if(!id.length()) THROW_ProgArgException("invalid programm argument id ''");
      return ProgArg(id,subargidx);
    }

    /// returns given program argument at given index \ingroup PA
    /** @see icl::utils::pa(const std::string&,unsigned int) */
    inline const ProgArg pa(unsigned int idx, bool danglingOnly = true){
      return ProgArg(idx,danglingOnly);
    }


    /// utility function that allows to use a default value, if given argument was not defined \ingroup PA
    template<class T>
    inline const T pa_def(const std::string &id, unsigned int subargidx, const T &def) throw (ProgArgException){
      const ProgArg p = pa(id,subargidx);
      return p ? parse<T>(p) : def;
    }

    /// utility function that allows to use a default value, if given argument was not defined \ingroup PA
    template<class T>
    inline const T pa_def(const std::string &id, const T &def) throw (ProgArgException){
      return pa_def(id,0,def);
    }



    /// returns number of actually given args given \ingroup PA
    /** @see icl::utils::pa(const std::string&,unsigned int) */
    ICLUtils_API unsigned int pa_get_count(bool danglingOnly = true);

    /// returns application name (full command line)
    /** @param fullpath if this is set to true, the complete
               first argument of main is returned, which
               may be something like <tt>/usr/bin/icl-create</tt>.
               If fullpath is false, which is default,
               just the program name is returned.

        @see icl::utils::pa(const std::string&,unsigned int) */
    ICLUtils_API const std::string &pa_get_progname(bool fullpath = false);

    /// shows current available programm arguments \ingroup PA
    ICLUtils_API void pa_show_usage(const std::string &msg = "");

    /** \cond */
    // utility class which allows the user to call the paex-function in a 'stacked' manner
    struct ICLUtils_API PAEX{
      PAEX operator()(const std::string &pa, const std::string &ex);
    };
    /** \endcond */

    /// This function can be used to provide additional information for certain program arguments \ingroup PA
    /** Due to the use of a special but hidden utility structure called icl::PAEX, this
        function can be called in a 'stacked' manner. This mean, that the function
        can be called several times without repeating the function name. Example:
        \code
        paex("-size","defines the input image size")
            ("-input","defines input device id and parameters")
            ("-format","define input image format");
        \endcode
        @see icl::utils::pa(const std::string&,unsigned int) */
    inline PAEX pa_explain(const std::string &pa, const std::string &ex){
      pa_explain_internal(pa,ex);
      return PAEX();
    }

    /** \cond */
    // deferred implementation of stacking operator
    inline PAEX PAEX::operator()(const std::string &pa, const std::string &ex){
      return pa_explain(pa,ex);
    }
    /** \endcond */


    /// initialization function for ICL's program argument evaluation framework \ingroup PA
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
          icl::utils::pa_init(n,ppc,"-size|-s(Size=VGA) -format|-f(format-string)");
        }
        \endcode
        \code
        int main(int n, char **ppc){
          icl::utils::pa_init(n,ppc,"-input|-i(device-type=dc,device-params=0)");
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
        icl::utils::pa_get_count and icl::utils::pa(unsigned int,bool).

        To list all dangling arguments, the followed code can be used:
        \code
        for(unsigned int i=0;i<icl::utils::pa_get_count();++i){
          std::cout << icl::utils::pa(i) << std::endl;
        }
        \endcode

        \section Optional and Mandatory Arguments

        As default, all arguments are optional. If you want
        to define an argument to be mandatory, simply prepend
        a '[m]'-prefix to the argument name alternative list.
        Example:
        \code
        // -size and -format are optional; -input or -i is mandatory
        icl::utils::pa_init(n,ppc,"-size(Size=VGA) -format(format=RGB) "
                    "[m]-input|-i(string,string)");
        \endcode

        If mandatory arguments are not actually given to the program,
        a ProgArgException is thrown. As it does not make sense to
        define default values for mandatory arguments, a
        ProgArgException is thrown in this case as well. Furthermore,
        mandatory arguments must have sub-arguments (which is also
        sensible if you think about it).
    */
    ICLUtils_API void pa_init(int n, char **ppc, const std::string &init, bool allowDanglingArgs = false);


    /// shows all given program arguments \ingroup PA
    ICLUtils_API void pa_show();

    /// Sets a license text, that is used when applications are run with --version or -v \ingroup PA
    /** pasetlic has to be called before painit is called. Otherwise, only the default licese text is shown. */
    ICLUtils_API void pa_set_license(const std::string &newLicenseText);

    /// Sets a applications help text that is used when applications are run with --help or with unknown arguments \ingroup PA
    /** pasethelp has to be called before painit is called. */
    ICLUtils_API void pa_set_help_text(const std::string &newHelpText);

    /// returns the current license text \ingroup PA
    ICLUtils_API std::string pa_get_license();

    /// returns the current help text (which is empty, if it was not set) \ingroup PA
    ICLUtils_API std::string pa_get_help_text();
  } // namespace utils
}


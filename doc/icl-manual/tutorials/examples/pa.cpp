#include <ICLQt/Common.h>

int main(int n, char **args){
  bool allowDanglingArgs = true;
  pa_init(n,args,"-size|-s(Size=VGA) -index(int) -input|-i(2) -files(...)",
          allowDanglingArgs);

  // extract the first sub-argument of argument
  // '-index' and convert it into an int value
  int i = pa("-index");

  // extract the first sub-argument of argument '-size'
  // alternative names can be used as synonyms
  Size s = pa("-s");

  // extract the 2nd sub-argument of argument '-input'
  std::string text = pa("-input",1);

  // check if argument -size was actually given
  if(pa("-size")){

    // extract the sub-argument as a special type
    // (for the string to type logic, C++'s
    // istream-operator is used!!)
    Size size = pa("-s");
  }

  // read a list of files from the arbitrary sub-argument
  // here, we explicitly instantiate a ProgArg
  ProgArg a = pa("-files");
  for(int i=0;i<a.n();++i){
    std::cout << "file " << i << " " << a[i] << std::endl;
  }

  // list all given arguments and sub-arguments
  std::cout << "all arguments " << std::endl;
  for(unsigned int i=0;i<pa_get_count(false);++i){
    std::cout << pa(i,false) << std::endl;
  }

  // in case of having dangling arguments allowed in
  // pa_init-call: list all dangling arguments
  std::cout << "all dangling arguments " << std::endl;
  for(unsigned int i=0;i<pa_get_count();++i){
    std::cout << pa(i) << std::endl;
  }

  // using ProgArg instances as std::strings is sometimes
  // a bit complicated since the conversion to an
  // std::string is sometimes ambiguous
  struct Test{
    Test(int i){}
    Test(const std::string &s){}
  };

  // Test t(pa("-x")); // ambiguous -> int | std::string

  // Test t((std::string)pa("-x")); // also ambiguous
  // due to different available std::string constructors

  Test t(pa("-size").as<std::string>()); // works, but long code

  Test t2(*pa("-s")); // much shorter, but only for using
  // a program argument as std::string

  // Note:: *pa("-s") will always return the argument as string

}

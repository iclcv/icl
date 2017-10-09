#include <ICLQt/Common.h>

int main(int n, char **args){
  pa_set_help_text("This is an example program that has no function\n"
                   "but it demonstrates how to add some program\n"
                   "argument and program description");
  pa_explain("-s","image size use for something");
  pa_explain("-index","some index used for something else");
  pa_explain("-input","input definition, the first first arg defines\n"
                      "the input grabber backend, second arg selects\n"
                      "a certain device from this backend");

  pa_init(n,args,"-size|-s(Size=VGA) -index(int) -input|-i(2)");

}

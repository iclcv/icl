#include <ICLUtils/Function.h>
#include <iostream>
#include <algorithm>

using namespace icl;
void global_foo(){
  std::cout << "void global_foo()" << std::endl;
}
int global_foo2(){
  std::cout << "int global_foo() returning 5" << std::endl;
  return 5;
}

int global_add(int a, int b) { return a+b; }

struct Foo{
  int add(int a, int b){
    std::cout << "Foo.add(" << a << "," << b << ") = " << a+b << std::endl;
    return a+b;
  }
  int operator()(int a, int b){
    std::cout << "Foo(" << a << "," << b << ") = " << a+b << std::endl;
    return a+b;
  }
  static void show_int(int i){
    std::cout << i << std::endl; 
  }
};

int main(){
  /// simple parameterless global function
  Function<void> gfoo(global_foo); 
  gfoo();

  /// global function that returns an int
  Function<int> gfoo2(global_foo2); 
  std::cout << gfoo2() << std::endl;
  
  /// Implicit cast from function with return value to function without return value
  Function<void> gfoo3 = function(global_foo2); 
  gfoo3();

  /// Global function with parameters
  /// identical to function(global_add)(4,5)
  Function<int,int,int> gadd(global_add); 
  std::cout << "global_add(4,5)=" << gadd(4,5) << std::endl;

  /// Global function with parameters (ignoring the result of the function)
  /// Functions with non-void return type can always be casted into another
  /// Function type with return type (the return value is simply ignored then)
  Function<void,int,int> gadd_void = function(global_add); gadd_void(4,5);

  
  /// create an std::vector
  std::vector<int> v;
  
  /// void-Member function with one parameter
  /// preserve type-correctness (argument is not int, but const int&)
  Function<void,const int&> vpush = function(v,&std::vector<int>::push_back);
  vpush(1);  vpush(2);  vpush(3);

  /// access elements with this function
  Function<int&,unsigned int> vat = function(v,&std::vector<int>::at);
  std::cout << "elem 0: " << vat(0) << std::endl;
  std::cout << "elem 1: " << vat(1) << std::endl;
  std::cout << "elem 2: " << vat(2) << std::endl;

  /// create an instance of the foo class
  Foo f;
  
  /// creating a list of functions of same type
  std::vector<Function<int,int,int> > list;
  list.push_back(function(f,&Foo::add)); // member function
  list.push_back(function(f,SelectFunctor<int,int,int>())); // a functor
  list.push_back(global_add);  // a global function
  
  /// Finally, we are also able to implement the FunctionImpl-interface
  /// here, we have to implement the corresponding constructor 
  /// (which must const!!!)
  struct Impl : FunctionImpl<int,int,int>{
    virtual int operator()(int a, int b) const{ 
      std::cout << "custom impl:operator()(a,b) = " << a+b << std::endl;
      return a+b;
    }
  };
  // list.push_back(function(new Impl)); 
  // would also be possible, but implicit cast is possible
  list.push_back(new Impl);
  
  /// clear the vector of ints also by using a Function-instance:
  function(v,&std::vector<int>::clear)();

  /// create a function that wraps the index operator
  Function<int&,unsigned int> vidxop = function(v,&std::vector<int>::operator[]);

  /// push the results of the function in the vector
  for(unsigned int i=0;i<list.size();++i){
    vpush(list[i](i,i));
  }

  /// create a function for the vector size
  Function<size_t> vsize = function(v,&std::vector<int>::size);
  
  /// show the result of the vector-size function
  std::cout << vsize() << std::endl;
  
  
  for(unsigned int i=0;i<vsize();++i){
    std::cout << "v[" << i << "] = " << vidxop(i) << std::endl;
  }
  
  /// or use a function and std::for_each to print the results
  std::for_each(v.begin(),v.end(),function(Foo::show_int));
}

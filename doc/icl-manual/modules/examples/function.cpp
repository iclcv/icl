#include <functional>
#include <iostream>
#include <algorithm>
#include <vector>

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
  std::function<void()> gfoo(global_foo);
  gfoo();

  /// global function that returns an int
  std::function<int()> gfoo2(global_foo2);
  std::cout << gfoo2() << std::endl;

  /// wrapping and adapting via lambda
  std::function<void()> gfoo3 = []{ global_foo2(); };
  gfoo3();

  /// global function with parameters
  std::function<int(int,int)> gadd(global_add);
  std::cout << "global_add(4,5)=" << gadd(4,5) << std::endl;

  /// create an std::vector
  std::vector<int> v;

  /// member function wrapping via lambda
  std::function<void(const int&)> vpush = [&v](const int& val){ v.push_back(val); };
  vpush(1);  vpush(2);  vpush(3);

  /// access elements via lambda
  std::function<int&(size_t)> vat = [&v](size_t i) -> int& { return v.at(i); };
  std::cout << "elem 0: " << vat(0) << std::endl;
  std::cout << "elem 1: " << vat(1) << std::endl;
  std::cout << "elem 2: " << vat(2) << std::endl;

  /// create an instance of the foo class
  Foo f;

  /// creating a list of functions of same type
  std::vector<std::function<int(int,int)>> list;
  list.push_back([&f](int a, int b){ return f.add(a,b); }); // member function
  list.push_back([&f](int a, int b){ return f(a,b); }); // functor
  list.push_back(global_add);  // a global function

  /// custom callable struct
  struct Impl {
    int operator()(int a, int b) const{
      std::cout << "custom impl:operator()(a,b) = " << a+b << std::endl;
      return a+b;
    }
  };
  list.push_back(Impl{});

  /// clear the vector using a lambda
  [&v]{ v.clear(); }();

  /// create a function that wraps the index operator
  std::function<int&(size_t)> vidxop = [&v](size_t i) -> int& { return v[i]; };

  /// push the results of the function in the vector
  for(unsigned int i=0;i<list.size();++i){
    vpush(list[i](i,i));
  }

  /// create a function for the vector size
  std::function<size_t()> vsize = [&v]() -> size_t { return v.size(); };

  /// show the result of the vector-size function
  std::cout << vsize() << std::endl;

  for(unsigned int i=0;i<vsize();++i){
    std::cout << "v[" << i << "] = " << vidxop(i) << std::endl;
  }

  /// or use std::for_each with a function pointer
  std::for_each(v.begin(),v.end(),Foo::show_int);
}

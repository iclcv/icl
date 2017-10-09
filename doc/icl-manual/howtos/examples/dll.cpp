// Header file

#ifdef TESTLIBRARY_EXPORTS
#define TESTLIBRARY_API __declspec(dllexport)
#else
#define TESTLIBRARY_API __declspec(dllimport)
#endif

template<class T> class TESTLIBRARY_API TemplateClass {
public:
  class InClass {
  public:
    TESTLIBRARY_API InClass(void);
  };
  int var;
  T tVar;
  TemplateClass(T arg);
  int func(void);
  template<class A> TESTLIBRARY_API A templateFunc(A arg);
  int callGlobalFriendFunc(void);
  friend TESTLIBRARY_API int friendFunc(void);
protected:
  int protectedFunc(void);
private:
  int privateFunc(void);
  friend TESTLIBRARY_API int gloabalFriendFunc(void);
};

extern TESTLIBRARY_API int globalVar;
TESTLIBRARY_API int globalFunc(void);


// Source file

template<class T>
TemplateClass<T>::InClass::InClass() {
  // some code
}

template<class T>
TemplateClass<T>::TemplateClass(T arg) {
  var = protectedFunc();
  tVar = arg;
}

template<class T>
int TemplateClass<T>::func() {
  return privateFunc();
}

template<class T>
int TemplateClass<T>::callGlobalFriendFunc() {
  return globalFunc();
}

template<class T>
int TemplateClass<T>::protectedFunc() {
  return 10;
}

template<class T>
int TemplateClass<T>::privateFunc() {
  return 55;
}

template<class T> template<class A>
A TemplateClass<T>::templateFunc(A arg) {
  return arg + 3;
}

// Explicit instantiations
template TESTLIBRARY_API int   TemplateClass<int>::templateFunc(int   arg);
template TESTLIBRARY_API float TemplateClass<int>::templateFunc(float arg);

// Explicit instantiation of the class
template class TESTLIBRARY_API TemplateClass<int>;

int globalVar = 1337;

int globalFunc(void)
{
  return 42;
}

int friendFunc(void) {
  return 555;
}

#ifndef ICL_UNCOPYABLE_H
#define ICL_UNCOPYABLE_H

namespace icl{
  /// Class interface for un-copyable classes.
  /** In some cases, classes might not be copied e.g. if
      the implementation of this would be very complex and
      it provides no benefits to copy an instance of
      a particular class.\n
      To forbid, that instance of this class might be copied,
      you can either implement a private copy constructor and
      a private assignment operator, or you can inherit
      the Uncopyable class.\n
      The following example demonstrates how you can protect
      a picture from being copied:
      \code
      #include <ICLUtils/Uncopyable.h>
      
      class Picasso : public Uncopyable{
         ...
      }; 
      
      int main(){
        Picasso guernica;
        Picasso copyOfGuernica = guernica; // not allowed
        Picasso anotherTryToCopyGuernica( guernica ); // also not allowed
  
        return 0;
      }
      \endcode
  **/
  class Uncopyable{
    protected:
    /// Empty base constructor
    Uncopyable(){}

    private:
    /// forbidden copy constructor
    Uncopyable(const Uncopyable &other){
      (void)other;
    }
    /// forbidden assignment operator
    Uncopyable &operator=(const Uncopyable &other){
      (void)other;
      return *this;
    }
  };
}

#endif

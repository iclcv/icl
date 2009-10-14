#include <iclStateHandle.h>
#include <iclThreadedUpdatableTextView.h>

namespace icl{

  
  void StateHandle::append(const std::string &text){
    this->text()->appendTextFromOtherThread(text);
  }
  
  void StateHandle::clear(){
    text()->clearTextFromOtherThread();
  }
  
  void StateHandle::setMaxLen(int maxLen){
    this->maxLen = maxLen;
    removeOldLines();
  }
  
  int StateHandle::getMaxLen() const{
    return maxLen;
  }
  
  void StateHandle::removeOldLines(){
    // todo!
  }
 
}                               


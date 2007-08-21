#include <iclButtonGroupHandle.h>
#include <iclMacros.h>
#include <QRadioButton>

using namespace std;
namespace icl{
  void ButtonGroupHandle::select(int id){
    ICLASSERT_RETURN(valid(id));
    vec()[id]->setChecked(true);
  }
  int ButtonGroupHandle::getSelected() const{
    ICLASSERT_RETURN_VAL(n(),-1);
    for(int i=0;i<n();i++){
      if(vec()[i]->isChecked()){
        return i;
      }
    }
    return -1;
  }
  std::string ButtonGroupHandle::getSelectedText() const{
    ICLASSERT_RETURN_VAL(n(),"");
    int selectedIndex = getSelected();
    ICLASSERT_RETURN_VAL(selectedIndex,"");
    return vec()[selectedIndex]->text().toLatin1().data();
  }
  std::string ButtonGroupHandle::getText(int id) const{
    ICLASSERT_RETURN_VAL(valid(id),"");
    return vec()[id]->text().toLatin1().data();
  }
  void ButtonGroupHandle::setText(int id, const std::string &t) {
    ICLASSERT_RETURN(valid(id));
    vec()[id]->setText(t.c_str());
    
  }
}

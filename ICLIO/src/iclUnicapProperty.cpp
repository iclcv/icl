#include <iclUnicapProperty.h>
#include <math.h>
#include <cstring>
using namespace std;

namespace icl{
  
  UnicapProperty::UnicapProperty():
    m_oUnicapPropertyPtr((unicap_property_t*)malloc(sizeof(unicap_property_t))),
    m_oUnicapHandle(NULL){
    unicap_void_property (m_oUnicapPropertyPtr.get());  
  }
  UnicapProperty::UnicapProperty(unicap_handle_t handle): 
    m_oUnicapPropertyPtr((unicap_property_t*)malloc(sizeof(unicap_property_t))),
    m_oUnicapHandle(handle){
    unicap_void_property (m_oUnicapPropertyPtr.get());  
  }
  
  bool UnicapProperty::isValid() const{
    // {{{ open

    return m_oUnicapHandle != NULL;
  }

  // }}}
  
  string UnicapProperty::getID() const{
    // {{{ open

    return m_oUnicapPropertyPtr->identifier;
  }

  // }}}

  string UnicapProperty::getCategory() const{
    // {{{ open

    return m_oUnicapPropertyPtr->category;
  }

  // }}}

  string UnicapProperty::getUnit() const {
    // {{{ open

    return m_oUnicapPropertyPtr->unit;
  }

  // }}}
  
  vector<string> UnicapProperty::getRelations() const { 
    // {{{ open

    vector<string> v;
    for(int i=0;i<m_oUnicapPropertyPtr->relations_count;v.push_back(m_oUnicapPropertyPtr->relations[i++]))
      ;
    return v;
  }

  // }}}
  
  UnicapProperty::type UnicapProperty::getType() const{
    // {{{ open

    return (type)m_oUnicapPropertyPtr->type;
  }

  // }}}
  
  double UnicapProperty::getValue() const{
    // {{{ open
    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_RANGE || 
                          m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_VALUE_LIST ,0 );
    return m_oUnicapPropertyPtr->value;
  }

  // }}}

  string UnicapProperty::getMenuItem() const{
    // {{{ open

    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_MENU, 0);
    return m_oUnicapPropertyPtr->menu_item;
  } 

  // }}}
    
  Range<double> UnicapProperty::getRange() const{
    // {{{ open

    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_RANGE, Range<double>());
    return Range<double>( m_oUnicapPropertyPtr->range.min, m_oUnicapPropertyPtr->range.max );
  }

  // }}} 
  
  vector<double> UnicapProperty::getValueList() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_VALUE_LIST, vector<double>());
    vector<double> v;
    for(int i=0;i<m_oUnicapPropertyPtr->value_list.value_count;v.push_back(m_oUnicapPropertyPtr->value_list.values[i++]))
      ;
    return v;
  }

  // }}}

  vector<string> UnicapProperty::getMenu() const{
    // {{{ open

    ICLASSERT_RETURN_VAL(m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_MENU, vector<string>());
    vector<string> v;
    for(int i=0;i<m_oUnicapPropertyPtr->menu.menu_item_count;v.push_back(m_oUnicapPropertyPtr->menu.menu_items[i++]))
      ;
    return v;
  }

  // }}}

  double UnicapProperty::getStepping() const {
    // {{{ open

    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_RANGE, 0);
    return m_oUnicapPropertyPtr->stepping;
  }

  // }}}
  
  u_int64_t UnicapProperty::getFlags() const{
    // {{{ open

    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_FLAGS, 0);
      return m_oUnicapPropertyPtr->flags;
    }

  // }}}

  u_int64_t UnicapProperty::getFlagMask() const{
    // {{{ open

    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_FLAGS, 0);
    return m_oUnicapPropertyPtr->flags_mask;
  }

  // }}}
    
  const UnicapProperty::Data UnicapProperty::getData() const{
    // {{{ open

    static const Data NULL_DATA = {0,0}; 
    ICLASSERT_RETURN_VAL( m_oUnicapPropertyPtr->type == UNICAP_PROPERTY_TYPE_DATA,NULL_DATA);
    Data d = { m_oUnicapPropertyPtr->property_data, m_oUnicapPropertyPtr->property_data_size };
    return d;
  }

  // }}}

  const unicap_property_t *UnicapProperty::getUnicapProperty() const{
    // {{{ open

    return m_oUnicapPropertyPtr.get();
  }

  // }}}

  unicap_property_t *UnicapProperty::getUnicapProperty(){
    // {{{ open

    return m_oUnicapPropertyPtr.get();
  }

  // }}}
  
  const unicap_handle_t &UnicapProperty::getUnicapHandle() const {
    // {{{ open

    return m_oUnicapHandle;
  }

  // }}}
  
  unicap_handle_t &UnicapProperty::getUnicapHandle() {
    // {{{ opemn

    return m_oUnicapHandle;
  }

  // }}}
  

  /** range = {10,100}
      step = 5;
      value = 44;
      
      stepIdx = round[ (44-10)/step }
              = round[ 34/5 ]
              = 7
      value   = 10+7*5 
              = 45
  */
  
  double UnicapProperty::translateValue(double value){
    ICLASSERT_RETURN_VAL( getType() == range , value);
    Range<double> r = getRange();
    if(r.in(value)){
      double step = getStepping();
      if(step > 0){
        double minVal = r.minVal;
        int stepIdx = (int)round((value-minVal)/step);
        return minVal+stepIdx*step;
      }
      return value;
    }else{
      if(r.minVal > value) return r.minVal;
      else return r.maxVal;
    }
  }
  
  
  void UnicapProperty::setValue(double value){
    // {{{ open
      type t=getType();
      ICLASSERT_RETURN( t == range || t==valueList );
      if(t==range){
        if(getRange().in(value)){
          // find the next value with respect to current stepping or clip it
          value = translateValue(value);
          m_oUnicapPropertyPtr->value = value;
          unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get());
        }else{
          ERROR_LOG("could not set up value to: " << value << " (outside range!)");
        }
      }else if(t==valueList){
        vector<double> v = getValueList();
        if(find(v.begin(),v.end(),value) != v.end()){
          m_oUnicapPropertyPtr->value = value;
          if(!SUCCESS (unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get()) )){
            ERROR_LOG("failed to set new property [code 0]");
          }
        }
      }
    }
    // }}}
  
  void UnicapProperty::setMenuItem(const string &item){
    // {{{ open

      ICLASSERT_RETURN( getType() == menu );
      vector<string> v = getMenu();
      if(find(v.begin(),v.end(),item) != v.end()){
        sprintf(m_oUnicapPropertyPtr->menu_item,item.c_str());
         if(!SUCCESS (unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get()) )){
           ERROR_LOG("failed to set new property [code 1]");
         }
      }else{
        ERROR_LOG("could not set up menu item to : " << item << "(item not allowed!)");
      }
    }

    // }}}
  
  void UnicapProperty::setFlags(u_int64_t flags_val){
    // {{{ open

    ICLASSERT_RETURN(getType() == flags);
    m_oUnicapPropertyPtr->flags = flags_val;
    if(!SUCCESS (unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get()) )){
      ERROR_LOG("failed to set new property [code 2]");
    }
  }

  // }}}

  void UnicapProperty::setFlagsMask(u_int64_t flags_mask){
    // {{{ open

    ICLASSERT_RETURN(getType() == flags);
    m_oUnicapPropertyPtr->flags_mask = flags_mask;
    if(!SUCCESS (unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get()) )){
      ERROR_LOG("failed to set new property [code 3]");
    }
  }

  // }}}
  
  void UnicapProperty::setData(void *data,unsigned int nBytes){
    // {{{ open

   ICLASSERT_RETURN(getType() == UnicapProperty::data);
   if(m_oUnicapPropertyPtr->property_data_size == 0){
     ERROR_LOG("cannot set up data array of size 0 [code 4]");
     return;
   }
   if(nBytes != m_oUnicapPropertyPtr->property_data_size){
     ERROR_LOG("data array and new data have different sizes [code 5]");
     return;
   }
   if(!m_oUnicapPropertyPtr->property_data){
     m_oUnicapPropertyPtr->property_data = malloc(nBytes);
   }
   memcpy(m_oUnicapPropertyPtr->property_data, data, nBytes);
   
   if(!SUCCESS (unicap_set_property(m_oUnicapHandle,m_oUnicapPropertyPtr.get()) )){
     ERROR_LOG("failed to set new property [code 6]");
   }
  }

  // }}}

  namespace {
    const char *ftoa(double d){
      // {{{ open

      static char buf[30];
      sprintf(buf,"%f",d);
      return buf;
    }

    // }}}
    
    const char *itoa(int i){
      // {{{ open

      static char buf[30];
      sprintf(buf,"%d",i);
      return buf;
    }

    // }}}
  }
    
  string UnicapProperty::toString(){
    // {{{ open

    string typeStr;
    switch(getType()){
      case  range: typeStr = "range"; break;
      case  valueList: typeStr = "value-list"; break;
      case  menu: typeStr = "menu"; break;
      case  data: typeStr = "data"; break;
      default: typeStr = "flags"; break;
    }
      
    char buf[10000];
    sprintf(buf,
            "ID       = %s\n"
            "Category = %s\n"
            "Unit     = %s\n"
            "Type     = %s\n"
            , getID().c_str(), getCategory().c_str(),getUnit().c_str(),typeStr.c_str());
    
    string s = buf;
    switch(getType()){
      case  range:
        sprintf(buf,"   min=%f\n   max=%f\n  steps=%f\n  curr=%f\n",getRange().minVal,getRange().maxVal,getStepping(),getValue());
        s.append(buf);
        break;
      case  valueList:{
        string l = "list     = {";
        vector<double> v = getValueList();
        for(unsigned int i=0;i<v.size();i++){
          l.append(ftoa(v[i]));
          if(i<v.size()-1){
            l.append(",");
          }
        }
        l.append("}\n");
        s.append(l);
        s.append("curr     = ");
        s.append(ftoa(getValue()));
        s.append("\n");
        break;
      }
      case  menu:{
        string l = "list     = {";
        vector<string> v = getMenu();
        for(unsigned int i=0;i<v.size();i++){
          l.append(v[i]);
          if(i<v.size()-1){
            l.append(",");
          }
        }
        l.append("}\n");
        s.append(l);
        s.append("curr     = ");
        s.append(getMenuItem());
        s.append("\n");
        break;
      }
      case flags:
        s.append("flag     = ");
        s.append(itoa(getFlags()));
        s.append("\n");
        s.append("mask     = ");
        s.append(itoa(getFlagMask()));
        s.append("\n");
        break;
      default: break;
    }
    return s;
  }

  // }}}
    
} // end of namespace 



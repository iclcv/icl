#ifndef ICL_UNICAP_PROPERTY
#define ICL_UNICAP_PROPERTY

#include <unicap.h>
#include <vector>
#include <string>
#include <iclSmartPtr.h>
#include <iclRange.h>
 

namespace icl{
  class UnicapProperty{
    public:
    UnicapProperty();
    UnicapProperty(unicap_handle_t handle);
    enum type {
      range = UNICAP_PROPERTY_TYPE_RANGE,
      valueList = UNICAP_PROPERTY_TYPE_VALUE_LIST,
      menu = UNICAP_PROPERTY_TYPE_MENU,
      data = UNICAP_PROPERTY_TYPE_DATA,
      flags = UNICAP_PROPERTY_TYPE_FLAGS,
      anytype 
    };
    struct Data{
      void *data;
      unsigned int size;
    };
    std::string getID() const;
    std::string getCategory() const;
    std::string getUnit() const;
  
    std::vector<std::string> getRelations() const;
    type getType() const;
    double getValue() const;

    std::string getMenuItem() const;    
    Range<double> getRange() const;

    std::vector<double> getValueList() const;
    std::vector<std::string> getMenu() const;
    double getStepping() const;
    u_int64_t getFlags() const;
    u_int64_t getFlagMask() const;
    const Data getData() const;
    const unicap_property_t *getUnicapProperty() const;
    unicap_property_t *getUnicapProperty();
    const unicap_handle_t &getUnicapHandle() const;
    unicap_handle_t &getUnicapHandle();
 
    void setValue(double value);    
    void setMenuItem(const std::string &item);
    std::string toString();
    private:
    struct UnicapPropertyDelOp : public DelOpBase{
      static void delete_func(unicap_property_t* p){
        // other data may not be released here
        free(p);        
      }
    };
    SmartPtr<unicap_property_t,UnicapPropertyDelOp> m_oUnicapPropertyPtr;
    unicap_handle_t m_oUnicapHandle;
  };


} // end of namespace 
#endif

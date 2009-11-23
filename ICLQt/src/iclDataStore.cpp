#include <iclDataStore.h>

#include <iclRect.h>
#include <iclRect32f.h>
#include <iclSize.h>
#include <iclPoint.h>
#include <iclPoint32f.h>
#include <iclRange.h>
#include <iclImg.h>

#include <iclComboHandle.h>
#include <iclFloatHandle.h>
#include <iclImageHandle.h>
#include <iclSliderHandle.h>
#include <iclFPSHandle.h>
#include <iclIntHandle.h>
#include <iclSpinnerHandle.h>
#include <iclButtonGroupHandle.h>
#include <iclFSliderHandle.h>
#include <iclLabelHandle.h>
#include <iclButtonHandle.h>
#include <iclDrawHandle.h>
#include <iclDrawHandle3D.h>
#include <iclStringHandle.h>


#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>
#include <QSlider>
#include <iclCompabilityLabel.h>
#include <QPushButton>
#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclDrawWidget3D.h>

#include <iclStringUtils.h>

using namespace icl;

namespace{
  
  struct Assign{
    virtual bool operator()(void *src, void *dst){ return false; }
  };
  
  template<class S, class D>
  struct AssignSpecial : public Assign{
    bool apply(S &src, D &dst){ dst = static_cast<D>(src); return true; }
    virtual bool operator()(void *src, void *dst){
      return apply(*reinterpret_cast<S*>(src),*reinterpret_cast<D*>(dst));
    }
  };

#define INST_NUM_TYPES                           \
  INST_TYPE(bool)                                \
  INST_TYPE(char)                                \
  INST_TYPE(unsigned char)                       \
  INST_TYPE(short)                               \
  INST_TYPE(unsigned short)                      \
  INST_TYPE(int)                                 \
  INST_TYPE(unsigned int)                        \
  INST_TYPE(long)                                \
  INST_TYPE(unsigned long)                       \
  INST_TYPE(float)                               \
  INST_TYPE(double)                              


#define INST_OTHER_TYPES                         \
  INST_TYPE(Rect)                                \
  INST_TYPE(Rect32f)                             \
  INST_TYPE(Size)                                \
  INST_TYPE(Point)                               \
  INST_TYPE(Point32f)                            \
  INST_TYPE(Range32s)                            \
  INST_TYPE(Range32f)                            \
  INST_TYPE(Img8u)                               \
  INST_TYPE(Img16s)                              \
  INST_TYPE(Img32s)                              \
  INST_TYPE(Img32f)                              \
  INST_TYPE(Img64f)                              \
  INST_TYPE(std::string)                              
  

  // X = X for other types
#define INST_TYPE(T) template class AssignSpecial<T,T>;
INST_OTHER_TYPES
#undef INST_TYPE

#define ADD(X,Y) template class AssignSpecial<X,Y>;


   // X = Y for numerical types

#define INST_TYPE(T) ADD(bool,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(char,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned char,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(short,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned short,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(int,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned int,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(long,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned long,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(float,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(double,T)
    INST_NUM_TYPES
#undef INST_TYPE

#undef ADD
    
#define FROM_NUM(D,HOW)                          \
    FROM_TO(bool,D,HOW)                          \
    FROM_TO(char,D,HOW)                          \
    FROM_TO(unsigned char,D,HOW)                 \
    FROM_TO(short,D,HOW)                         \
    FROM_TO(unsigned short,D,HOW)                \
    FROM_TO(int,D,HOW)                           \
    FROM_TO(unsigned int,D,HOW)                  \
    FROM_TO(long,D,HOW)                          \
    FROM_TO(unsigned long,D,HOW)                 \
    FROM_TO(float,D,HOW)                         \
    FROM_TO(double,D,HOW)                          

#define TO_NUM(S,HOW)                            \
    FROM_TO(S,bool,HOW)                          \
    FROM_TO(S,char,HOW)                          \
    FROM_TO(S,unsigned char,HOW)                 \
    FROM_TO(S,short,HOW)                         \
    FROM_TO(S,unsigned short,HOW)                \
    FROM_TO(S,int,HOW)                           \
    FROM_TO(S,unsigned int,HOW)                  \
    FROM_TO(S,long,HOW)                          \
    FROM_TO(S,unsigned long,HOW)                 \
    FROM_TO(S,float,HOW)                         \
    FROM_TO(S,double,HOW)                          
    
#define FROM_IMG(D,HOW)                          \
    FROM_TO(Img8u,D,HOW)                         \
    FROM_TO(Img16s,D,HOW)                        \
    FROM_TO(Img32s,D,HOW)                        \
    FROM_TO(Img32f,D,HOW)                        \
    FROM_TO(Img64f,D,HOW)                        \
    FROM_TO(ImgBase,D,HOW)                        

#define FROM_IMG_PTR(D,HOW)                       \
    FROM_TO(Img8u*,D,HOW)                         \
    FROM_TO(Img16s*,D,HOW)                        \
    FROM_TO(Img32s*,D,HOW)                        \
    FROM_TO(Img32f*,D,HOW)                        \
    FROM_TO(Img64f*,D,HOW)                        \
    FROM_TO(ImgBase*,D,HOW)                       \
    FROM_TO(const Img8u*,D,HOW)                         \
    FROM_TO(const Img16s*,D,HOW)                        \
    FROM_TO(const Img32s*,D,HOW)                        \
    FROM_TO(const Img32f*,D,HOW)                        \
    FROM_TO(const Img64f*,D,HOW)                        \
    FROM_TO(const ImgBase*,D,HOW)                       \
  

#define FROM_TO_NUM(S,HOW_FROM,HOW_TO) \
    FROM_NUM(S,HOW_FROM)               \
    TO_NUM(S,HOW_TO)
    
#define FROM_TO_STR(S,HOW_FROM,HOW_TO)    \
    FROM_TO(S,std::string,HOW_FROM)       \
    FROM_TO(std::string,S,HOW_TO)       

#define FROM_TO(S,D,HOW)                                                         \
    template<> struct AssignSpecial<S,D> : public Assign{                        \
        bool apply(S &src, D &dst){                                              \
          HOW; return true;                                                      \
        }                                                                        \
        virtual bool operator()(void *src, void *dst){                           \
           return apply(*reinterpret_cast<S*>(src),*reinterpret_cast<D*>(dst));  \
        }                                                                        \
      };                                                                         \
      template class AssignSpecial<S,D>;

    // ComboHandle
    FROM_TO_NUM(ComboHandle,dst.setSelectedIndex((int)src),dst=src.getSelectedIndex());
    FROM_TO_STR(ComboHandle,dst=src.getSelectedItem(),dst.setSelectedItem(src));
    
    // FloatHandle
    FROM_TO_NUM(FloatHandle,dst=(float)src,dst=src.getValue());
    FROM_TO_STR(FloatHandle,dst=str(src.getValue()),dst=parse<float>(src));
    
    // ImageHandle
    FROM_IMG(ImageHandle,dst=src);
    FROM_IMG_PTR(ImageHandle,dst=src);
    /*
        template<> struct AssignSpecial<DataStore::Data::Event,ImageHandle> : public Assign{
      bool apply(DataStore::Data::Event &src, ImageHandle &dst){     
        if(src.message=="update"){
          dst.update();
        }
        else if(src.message=="install"){
          (*dst)->install((MouseHandler*)src.data);
        }
        return true;
      }
      virtual bool operator()(void *src, void *dst){
        return apply(*reinterpret_cast<DataStore::Data::Event*>(src),
                     *reinterpret_cast<ImageHandle*>(dst));
      }
    };
    template class AssignSpecial<DataStore::Data::Event,ImageHandle>;
        */


#define INST_REGISTER_EVENT_FOR_HANDLE(T)                               \
    FROM_TO(DataStore::Data::Event,T##Handle,                           \
            if(src.message=="register"){                                \
              dst.registerCallback((GUI::Callback*)src.data);           \
            }else{                                                      \
              ERROR_LOG("unable to apply function'" << src.message      \
                        << "' on " #T "Handle instances");              \
            });

    INST_REGISTER_EVENT_FOR_HANDLE(Button);
    INST_REGISTER_EVENT_FOR_HANDLE(ButtonGroup);
    INST_REGISTER_EVENT_FOR_HANDLE(Slider);
    INST_REGISTER_EVENT_FOR_HANDLE(FSlider);
    INST_REGISTER_EVENT_FOR_HANDLE(Combo);
    INST_REGISTER_EVENT_FOR_HANDLE(Spinner);
    // maybe more ...


    FROM_TO(DataStore::Data::Event,ImageHandle,
            if(src.message=="update"){
              dst.update();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });

    // DrawHandle
    FROM_IMG(DrawHandle,dst=src);
    FROM_IMG_PTR(DrawHandle,dst=src);
    FROM_TO(DataStore::Data::Event,DrawHandle,
            if(src.message=="update"){
              dst.update();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });
    //FROM_TO(DataStore::Data::Event,DrawHandle,if(src.message=="update")dst.update());

    // DrawHandle3D
    FROM_IMG(DrawHandle3D,(*dst)->setImage(&src));
    FROM_IMG_PTR(DrawHandle3D,dst=src);
    //FROM_TO(DataStore::Data::Event,DrawHandle,if(src.message=="update")dst.update());
    FROM_TO(DataStore::Data::Event,DrawHandle3D,
            if(src.message=="update"){
              dst.update();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });

    
    // FPSHandle
    FROM_TO(DataStore::Data::Event,FPSHandle,if(src.message=="update")dst.update());
    
    // SliderHandle
    FROM_TO_NUM(SliderHandle,dst.setValue((int)src),dst=src.getValue());
    FROM_TO_STR(SliderHandle,dst=str(src.getValue()),dst.setValue(parse<int>(src)));
    FROM_TO(Range8u,SliderHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32s,SliderHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32f,SliderHandle,dst.setRange((int)src.minVal,(int)src.maxVal));

    // IntHandle
    FROM_TO_NUM(IntHandle,dst=(int)src,dst=src.getValue());
    FROM_TO_STR(IntHandle,dst=str(src.getValue()),dst=parse<int>(src));
    
    // SpinnerHandle
    FROM_TO_NUM(SpinnerHandle,dst.setValue((int)src),dst=src.getValue());
    FROM_TO_STR(SpinnerHandle,dst=str(src.getValue()),dst.setValue(parse<int>(src)));
    FROM_TO(Range8u,SpinnerHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32s,SpinnerHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32f,SpinnerHandle,dst.setRange((int)src.minVal,(int)src.maxVal));
    
    // ButtonGroup
    TO_NUM(ButtonGroupHandle,dst=src.getSelected());
    FROM_TO(ButtonGroupHandle,std::string,dst=src.getSelectedText());

    // FSliderHandle
    FROM_TO_NUM(FSliderHandle,dst.setValue((float)src),dst=src.getValue());
    FROM_TO_STR(FSliderHandle,dst=str(src.getValue()),dst.setValue(parse<float>(src)));
    FROM_TO(Range8u,FSliderHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32s,FSliderHandle,dst.setRange(src.minVal,src.maxVal));
    FROM_TO(Range32f,FSliderHandle,dst.setRange(src.minVal,src.maxVal));
    
    // LabelHandle
    FROM_NUM(LabelHandle,dst=(double)src);
    FROM_TO(std::string,LabelHandle,dst=src);
    FROM_IMG(LabelHandle,dst=str(src));
    FROM_IMG_PTR(LabelHandle,dst=str(*src));

    // ButtonHandle
    TO_NUM(ButtonHandle,dst=(*src)->isChecked());
    
    // StringHandle
    FROM_TO_NUM(StringHandle,dst=str(src),dst=parse<double>(src.getCurrentText()));
    FROM_TO_STR(StringHandle,dst=src.getCurrentText(),dst=src);
    
}


typedef std::map<const std::string, std::map< const std::string, Assign*> > AssignMap;

namespace icl{
  
  AssignMap *create_assign_map(){
    AssignMap &m = *new AssignMap;
    
#define TYPE(T) (DataStore::get_type_name<T>())
#define ADD(X,Y) m[TYPE(X)][TYPE(Y)] = new AssignSpecial<X,Y>;

    // X = X
#define INST_TYPE(T) ADD(T,T)
    INST_OTHER_TYPES
#undef INST_TYPE
    
    // X = Y for numerical types
#define INST_TYPE(T) ADD(bool,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(char,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned char,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(short,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned short,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(int,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned int,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(long,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(unsigned long,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(float,T)
    INST_NUM_TYPES
#undef INST_TYPE

#define INST_TYPE(T) ADD(double,T)
    INST_NUM_TYPES
#undef INST_TYPE

    /// Other supported types ...


#define FROM_NUM_ADD(D)  \
    ADD(bool,D)          \
    ADD(char,D)          \
    ADD(unsigned char,D) \
    ADD(short,D)         \
    ADD(unsigned short,D)\
    ADD(int,D)           \
    ADD(unsigned int,D)  \
    ADD(long,D)          \
    ADD(unsigned long,D) \
    ADD(float,D)         \
    ADD(double,D)                          

#define TO_NUM_ADD(S)    \
    ADD(S,bool)          \
    ADD(S,char)          \
    ADD(S,unsigned char) \
    ADD(S,short)         \
    ADD(S,unsigned short)\
    ADD(S,int)           \
    ADD(S,unsigned int)  \
    ADD(S,long)          \
    ADD(S,unsigned long) \
    ADD(S,float)         \
    ADD(S,double)         
    
#define FROM_TO_NUM_ADD(X) \
    FROM_NUM_ADD(X)        \
    TO_NUM_ADD(X)          
    
#define FROM_TO_STR_ADD(X)  \
    ADD(X,std::string)      \
    ADD(std::string,X)
    
#define FROM_IMG_ADD(X) \
    ADD(Img8u,X)        \
    ADD(Img16s,X)       \
    ADD(Img32s,X)       \
    ADD(Img32f,X)       \
    ADD(Img64f,X)       \
    ADD(ImgBase,X)                   

#define FROM_IMG_PTR_ADD(X)    \
    ADD(Img8u*,X)              \
    ADD(Img16s*,X)             \
    ADD(Img32s*,X)             \
    ADD(Img32f*,X)             \
    ADD(Img64f*,X)             \
    ADD(ImgBase*,X)            \
    ADD(const Img8u*,X)        \
    ADD(const Img16s*,X)       \
    ADD(const Img32s*,X)       \
    ADD(const Img32f*,X)       \
    ADD(const Img64f*,X)       \
    ADD(const ImgBase*,X)                            

    
    // ComboHandle
    FROM_TO_NUM_ADD(ComboHandle);
    FROM_TO_STR_ADD(ComboHandle);
    
    // FloatHandle
    FROM_TO_NUM_ADD(FloatHandle);
    FROM_TO_STR_ADD(FloatHandle);
    
    // ImageHandle
    FROM_IMG_ADD(ImageHandle);
    FROM_IMG_PTR_ADD(ImageHandle);
    ADD(DataStore::Data::Event,ImageHandle);

    // DrawHandle
    FROM_IMG_ADD(DrawHandle);
    FROM_IMG_PTR_ADD(DrawHandle);
    ADD(DataStore::Data::Event,DrawHandle);

    // DrawHandle3D
    FROM_IMG_ADD(DrawHandle3D);
    FROM_IMG_PTR_ADD(DrawHandle3D);
    ADD(DataStore::Data::Event,DrawHandle3D);
    
    // FPSHandle
    ADD(DataStore::Data::Event,FPSHandle);
    
    
    // SliderHandle
    FROM_TO_NUM_ADD(SliderHandle);
    FROM_TO_STR_ADD(SliderHandle);
    ADD(Range8u,SliderHandle);
    ADD(Range32s,SliderHandle);
    ADD(Range32f,SliderHandle);
    ADD(DataStore::Data::Event,SliderHandle);

    // IntHandle
    FROM_TO_NUM_ADD(IntHandle);
    FROM_TO_STR_ADD(IntHandle);
    
    // SpinnerHandle
    FROM_TO_NUM_ADD(SpinnerHandle);
    FROM_TO_STR_ADD(SpinnerHandle);
    ADD(Range8u,SpinnerHandle);
    ADD(Range32s,SpinnerHandle);
    ADD(Range32f,SpinnerHandle);
    ADD(DataStore::Data::Event,SpinnerHandle);
    
    // ButtonGroup
    TO_NUM_ADD(ButtonGroupHandle);
    ADD(ButtonGroupHandle,std::string);
    ADD(DataStore::Data::Event,ButtonGroupHandle);


    // FSliderHandle
    FROM_TO_NUM_ADD(FSliderHandle);
    FROM_TO_STR_ADD(FSliderHandle);
    ADD(Range8u,FSliderHandle);
    ADD(Range32s,FSliderHandle);
    ADD(Range32f,FSliderHandle);
    ADD(DataStore::Data::Event,FSliderHandle);

    
    // LabelHandle
    FROM_NUM_ADD(LabelHandle);
    ADD(std::string,LabelHandle);
    FROM_IMG_ADD(LabelHandle);
    FROM_IMG_PTR_ADD(LabelHandle);

    // ButtonHandle
    TO_NUM_ADD(ButtonHandle);
    ADD(DataStore::Data::Event,ButtonHandle);
    
    // StringHandle
    FROM_TO_NUM_ADD(StringHandle);
    FROM_TO_STR_ADD(StringHandle);

    // ComboHandle
    ADD(DataStore::Data::Event,ComboHandle);

    return &m;
  }
  
  void DataStore::Data::assign(void *src, const std::string &srcType, void *dst, const std::string &dstType) throw (DataStore::UnassignableTypesException){
    static AssignMap *am = 0;
    if(!am) am = create_assign_map();
    AssignMap::iterator it1 = am->find(srcType);
    if(it1 == am->end()){
      throw DataStore::UnassignableTypesException(srcType,dstType); 
    }
    std::map< const std::string, Assign*>::iterator it2 = it1->second.find(dstType);
    if(it2 == it1->second.end()){
      throw DataStore::UnassignableTypesException(srcType,dstType); 
    }
    bool success = (*it2->second)(src, dst);
    if(!success){
      throw DataStore::UnassignableTypesException(srcType,dstType);
    }
  }


  DataStore::Data DataStore::operator[](const std::string &key) throw (KeyNotFoundException){
    DataMap::iterator it = m_oDataMapPtr->find(key);
    if(it == m_oDataMapPtr->end()) throw KeyNotFoundException(key);
    return Data(&it->second);
  }
}

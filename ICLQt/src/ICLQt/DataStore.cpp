/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DataStore.cpp                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/DataStore.h>

#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Range.h>
#include <ICLCore/Img.h>

#include <ICLQt/PlotHandle.h>
#include <ICLQt/TabHandle.h>
#include <ICLQt/StateHandle.h>
#include <ICLQt/SplitterHandle.h>
#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/DispHandle.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/FloatHandle.h>
#include <ICLQt/ImageHandle.h>
#include <ICLQt/SliderHandle.h>
#include <ICLQt/FPSHandle.h>
#include <ICLQt/IntHandle.h>
#include <ICLQt/SpinnerHandle.h>
#include <ICLQt/ButtonGroupHandle.h>
#include <ICLQt/FSliderHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLQt/DrawHandle.h>
#ifdef ICL_HAVE_OPENGL
#include <ICLQt/DrawHandle3D.h>
#endif
#include <ICLQt/StringHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/ColorHandle.h>
#include <ICLQt/BoxHandle.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>
#include <QSlider>
#include <ICLQt/CompabilityLabel.h>
#include <QPushButton>
#include <ICLQt/Widget.h>
#include <ICLQt/DrawWidget.h>
#ifdef ICL_HAVE_OPENGL
#include <ICLQt/DrawWidget3D.h>
#endif
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Any.h>

#include <ICLQt/ThreadedUpdatableSlider.h>
#include <ICLQt/ThreadedUpdatableTextView.h>
#include <ICLQt/MouseHandler.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::qt;
namespace{


  template<class S, class D>
  struct AssignSpecial : public DataStore::Assign{
    AssignSpecial(const std::string &srcType, const std::string &dstType):
      Assign(srcType,dstType,DataStore::get_type_name<S>(),DataStore::get_type_name<D>()){}
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
  INST_TYPE(std::string)                         \
  INST_TYPE(Any)


  // X = X for other types
#define INST_TYPE(T) template struct AssignSpecial<T,T>;
INST_OTHER_TYPES
#undef INST_TYPE

#define ADD(X,Y) template struct AssignSpecial<X,Y>;


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
    FROM_TO(std::string,S,HOW_TO)         \
    FROM_TO(S,Any,HOW_FROM)               \
    FROM_TO(Any,S,HOW_TO)

#define FROM_TO(S,D,HOW)                                                         \
    template<> struct AssignSpecial<S,D> : public DataStore::Assign{             \
        AssignSpecial(const std::string &srcType, const std::string &dstType):   \
          Assign(srcType,dstType,DataStore::get_type_name<S>(),                  \
                 DataStore::get_type_name<D>()){}                                \
        bool apply(S &src, D &dst){                                              \
          HOW; return true;                                                      \
        }                                                                        \
        virtual bool operator()(void *src, void *dst){                           \
           return apply(*reinterpret_cast<S*>(src),*reinterpret_cast<D*>(dst));  \
        }                                                                        \
      };                                                                         \
      //template struct AssignSpecial<S,D>; // TODO: maybe we need this one in Unix

    // ComboHandle
    FROM_TO_NUM(ComboHandle,dst.setSelectedIndex((int)src),dst=src.getSelectedIndex());
    FROM_TO_STR(ComboHandle,dst=src.getSelectedItem(),dst.setSelectedItem(src));

    // FloatHandle
    FROM_TO_NUM(FloatHandle,dst=(float)src,dst=src.getValue());
    FROM_TO_STR(FloatHandle,dst=str(src.getValue()),dst=parse<float>(src));

    // ImageHandle
    FROM_IMG(ImageHandle,dst=src);
    FROM_IMG_PTR(ImageHandle,dst=src);

    // Color Handle and Color
    FROM_TO(ColorHandle,Color,dst=src.getRGB());
    FROM_TO(ColorHandle,Color4D,dst=src.getRGBA());

    FROM_TO(Color,ColorHandle,dst=src);
    FROM_TO(Color4D,ColorHandle,dst=src);

    FROM_TO(ColorHandle,std::string,dst=str(src.getRGBA()));
    FROM_TO(std::string,ColorHandle,dst=(parse<Color4D>(src))); // ? what if source is an rgb and not rgba-string??

    FROM_TO(Color,Color4D,dst=Color4D(src[0],src[1],src[2],0));
    FROM_TO(Color4D,Color,dst=Color(src[0],src[1],src[2]));

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
              dst.registerCallback(src.cb);                             \
            }else if(src.message=="register-complex"){                  \
              dst.registerCallback(src.cb2);                            \
            }else if(src.message == "enable"){                          \
              dst.enable();                                             \
            }else if(src.message == "disable"){                         \
              dst.disable();                                            \
            }else{                                                      \
              ERROR_LOG("unable to apply function'" << src.message      \
                        << "' on " #T "Handle instances");              \
            });

    //    INST_REGISTER_EVENT_FOR_HANDLE(Button);
    //INST_REGISTER_EVENT_FOR_HANDLE(ButtonGroup);
    //INST_REGISTER_EVENT_FOR_HANDLE(Slider);
    //INST_REGISTER_EVENT_FOR_HANDLE(FSlider);
    //INST_REGISTER_EVENT_FOR_HANDLE(Combo);
    //INST_REGISTER_EVENT_FOR_HANDLE(Spinner);
    //INST_REGISTER_EVENT_FOR_HANDLE(CheckBox);


    INST_REGISTER_EVENT_FOR_HANDLE(Box);
    INST_REGISTER_EVENT_FOR_HANDLE(ButtonGroup);
    INST_REGISTER_EVENT_FOR_HANDLE(Button);
    INST_REGISTER_EVENT_FOR_HANDLE(CheckBox);
    INST_REGISTER_EVENT_FOR_HANDLE(Color);
    INST_REGISTER_EVENT_FOR_HANDLE(Combo);
    INST_REGISTER_EVENT_FOR_HANDLE(Disp);
    INST_REGISTER_EVENT_FOR_HANDLE(Float);
    INST_REGISTER_EVENT_FOR_HANDLE(FSlider);
    INST_REGISTER_EVENT_FOR_HANDLE(Int);
    INST_REGISTER_EVENT_FOR_HANDLE(Label);
    INST_REGISTER_EVENT_FOR_HANDLE(Slider);
    INST_REGISTER_EVENT_FOR_HANDLE(Spinner);
    INST_REGISTER_EVENT_FOR_HANDLE(Splitter);
    INST_REGISTER_EVENT_FOR_HANDLE(State);
    INST_REGISTER_EVENT_FOR_HANDLE(String);
    INST_REGISTER_EVENT_FOR_HANDLE(Tab);
    /*        */
    // maybe more ...


    FROM_TO(DataStore::Data::Event,ImageHandle,
            if(src.message=="render"){
              dst.render();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });
    FROM_TO(ImageHandle,ICLWidget*,dst = *src);

    /// PlotHandle
    FROM_TO(DataStore::Data::Event,PlotHandle,
            if(src.message=="render"){
              dst.render();
            }else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });
    FROM_TO(PlotHandle,PlotWidget*,dst = *src);


    // DrawHandle
    FROM_IMG(DrawHandle,dst=src);
    FROM_IMG_PTR(DrawHandle,dst=src);
    FROM_TO(DataStore::Data::Event,DrawHandle,
            if(src.message=="render"){
              dst.render();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            });
    FROM_TO(DrawHandle,ICLDrawWidget*,dst = *src);
    FROM_TO(DrawHandle,ICLWidget*,dst = *src);

    //FROM_TO(DataStore::Data::Event,DrawHandle,if(src.message=="update")dst.update());
#ifdef ICL_HAVE_OPENGL
    // DrawHandle3D
    FROM_IMG(DrawHandle3D,(*dst)->setImage(&src));
    FROM_IMG_PTR(DrawHandle3D,dst=src);
    //FROM_TO(DataStore::Data::Event,DrawHandle,if(src.message=="update")dst.update());
    FROM_TO(DataStore::Data::Event,DrawHandle3D,
            if(src.message=="render"){
              dst.render();
            }
            else if(src.message=="install"){
              (*dst)->install((MouseHandler*)src.data);
            }else if(src.message=="link"){
              (*dst)->link((ICLDrawWidget3D::GLCallback*)src.data);
            });

    FROM_TO(DrawHandle3D,ICLDrawWidget3D*,dst = *src);
    FROM_TO(DrawHandle3D,ICLDrawWidget*,dst = *src);
    FROM_TO(DrawHandle3D,ICLWidget*,dst = *src);
#endif

    // FPSHandle
    FROM_TO(DataStore::Data::Event,FPSHandle,if(src.message=="render")dst.render());

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
    // FROM_TO(std::string,LabelHandle,dst=src);
    FROM_TO_STR(LabelHandle,dst=src->text().toLatin1().data(),dst = src);
    FROM_IMG(LabelHandle,dst=str(src));
    FROM_IMG_PTR(LabelHandle,dst=str(*src));

    // ButtonHandle
    TO_NUM(ButtonHandle,dst=(*src)->isChecked());

    // CheckBoxHandle
    FROM_TO_NUM(CheckBoxHandle,dst->setChecked(src),dst=src.isChecked());
    FROM_TO_STR(CheckBoxHandle,dst=str(src.isChecked()),dst.doCheck(parse<bool>(src)));


    // StringHandle
    FROM_TO_NUM(StringHandle,dst=str(src),dst=parse<double>(src.getCurrentText()));
    FROM_TO_STR(StringHandle,dst=src.getCurrentText(),dst=src);
    FROM_TO(Point32f,StringHandle, dst = str(src));

    /*
    template<>
    struct AssignSpecial<Point32f,StringHandle> : public DataStore::Assign{
      AssignSpecial(const std::string &srcType, const std::string &dstType):
        Assign(srcType,dstType,DataStore::get_type_name<Point32f>(),
               DataStore::get_type_name<StringHandle>()){}
      bool apply(Point32f &src, StringHandle &dst){ dst = str(src); return true; }
      virtual bool operator()(void *src, void *dst){
        return apply(*reinterpret_cast<Point32f*>(src),*reinterpret_cast<StringHandle*>(dst));
      }
        };*/

    // TabWidget to num
    TO_NUM(TabHandle,dst=src.current());
}


typedef std::map<const std::string, std::map< const std::string, DataStore::Assign*> > AssignMap;

namespace icl{
  namespace qt{

    AssignMap *create_assign_map(){
      AssignMap &m = *new AssignMap;

  #define TYPE(T) (DataStore::get_type_name<T>())
  #define ADD(X,Y) m[TYPE(X)][TYPE(Y)] = new AssignSpecial<X,Y>(str(#X),str(#Y));

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
  #define ADD_T_TO_T(D) ADD(D,D)

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
      ADD(std::string,X)      \
      ADD(X,Any)              \
      ADD(Any,X)


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
      ADD(DataStore::Data::Event,ComboHandle);
      ADD_T_TO_T(ComboHandle);

      // CheckBox
      FROM_TO_STR_ADD(CheckBoxHandle);
      FROM_TO_NUM_ADD(CheckBoxHandle);
      ADD(DataStore::Data::Event,CheckBoxHandle);
      ADD_T_TO_T(CheckBoxHandle);


      // FloatHandle
      ADD(DataStore::Data::Event,FloatHandle);
      FROM_TO_NUM_ADD(FloatHandle);
      FROM_TO_STR_ADD(FloatHandle);
      ADD_T_TO_T(FloatHandle);

      // ImageHandle
      FROM_IMG_ADD(ImageHandle);
      FROM_IMG_PTR_ADD(ImageHandle);
      ADD(DataStore::Data::Event,ImageHandle);
      ADD_T_TO_T(ImageHandle);
      ADD(ImageHandle,ICLWidget*);

      // PlotHandle
      ADD(DataStore::Data::Event,PlotHandle);
      ADD_T_TO_T(PlotHandle);
      ADD(PlotHandle,PlotWidget*);

      // DrawHandle
      FROM_IMG_ADD(DrawHandle);
      FROM_IMG_PTR_ADD(DrawHandle);
      ADD(DataStore::Data::Event,DrawHandle);
      ADD_T_TO_T(DrawHandle);
      ADD(DrawHandle,ICLDrawWidget*);
      ADD(DrawHandle,ICLWidget*);


  #ifdef ICL_HAVE_OPENGL
      // DrawHandle3D
      FROM_IMG_ADD(DrawHandle3D);
      FROM_IMG_PTR_ADD(DrawHandle3D);
      ADD(DataStore::Data::Event,DrawHandle3D);
      ADD_T_TO_T(DrawHandle3D);
      ADD(DrawHandle3D,ICLDrawWidget3D*);
      ADD(DrawHandle3D,ICLDrawWidget*);
      ADD(DrawHandle3D,ICLWidget*);
  #endif

      // FPSHandle
      ADD(DataStore::Data::Event,FPSHandle);
      ADD_T_TO_T(FPSHandle);


      // SliderHandle
      FROM_TO_NUM_ADD(SliderHandle);
      FROM_TO_STR_ADD(SliderHandle);
      ADD(Range8u,SliderHandle);
      ADD(Range32s,SliderHandle);
      ADD(Range32f,SliderHandle);
      ADD(DataStore::Data::Event,SliderHandle);
      ADD_T_TO_T(SliderHandle);


      // IntHandle
      ADD(DataStore::Data::Event,IntHandle);
      FROM_TO_NUM_ADD(IntHandle);
      FROM_TO_STR_ADD(IntHandle);
      ADD_T_TO_T(IntHandle);


      // SpinnerHandle
      FROM_TO_NUM_ADD(SpinnerHandle);
      FROM_TO_STR_ADD(SpinnerHandle);
      ADD(Range8u,SpinnerHandle);
      ADD(Range32s,SpinnerHandle);
      ADD(Range32f,SpinnerHandle);
      ADD(DataStore::Data::Event,SpinnerHandle);
      ADD_T_TO_T(SpinnerHandle);


      // ButtonGroup
      TO_NUM_ADD(ButtonGroupHandle);
      ADD(ButtonGroupHandle,std::string);
      ADD(DataStore::Data::Event,ButtonGroupHandle);
      ADD_T_TO_T(ButtonGroupHandle);

      // FSliderHandle
      FROM_TO_NUM_ADD(FSliderHandle);
      FROM_TO_STR_ADD(FSliderHandle);
      ADD(Range8u,FSliderHandle);
      ADD(Range32s,FSliderHandle);
      ADD(Range32f,FSliderHandle);
      ADD(DataStore::Data::Event,FSliderHandle);
      ADD_T_TO_T(FSliderHandle);

      // LabelHandle
      FROM_NUM_ADD(LabelHandle);
      //    ADD(std::string,LabelHandle);
      FROM_TO_STR_ADD(LabelHandle);
      FROM_IMG_ADD(LabelHandle);
      FROM_IMG_PTR_ADD(LabelHandle);
      ADD_T_TO_T(LabelHandle);


      // ButtonHandle
      TO_NUM_ADD(ButtonHandle);
      ADD(DataStore::Data::Event,ButtonHandle);
      ADD_T_TO_T(ButtonHandle);


      // StringHandle
      FROM_TO_NUM_ADD(StringHandle);
      FROM_TO_STR_ADD(StringHandle);
      ADD_T_TO_T(StringHandle);
      ADD(DataStore::Data::Event,StringHandle);
      ADD(Point32f,StringHandle);

      // BoxHandle
      ADD_T_TO_T(BoxHandle);

      // ColorHandle
      ADD(ColorHandle,Color);
      ADD(ColorHandle,Color4D);

      ADD(Color,ColorHandle);
      ADD(Color4D,ColorHandle);

      ADD(ColorHandle,std::string);
      ADD(std::string,ColorHandle);

      ADD(Color,Color4D);
      ADD(Color4D,Color);
      ADD_T_TO_T(ColorHandle);
      ADD(DataStore::Data::Event,ColorHandle);

      ADD_T_TO_T(TabHandle);
      TO_NUM_ADD(TabHandle);

      return &m;
    }

    AssignMap *create_singelton_assign_map(){
      static AssignMap *am = create_assign_map();
      return am;
    }

    void DataStore::register_assignment_rule(Assign *assign){
      AssignMap &m = *create_singelton_assign_map();
      m[assign->srcRTTI][assign->dstRTTI] = assign;
    }

    void DataStore::Data::assign(void *src, const std::string &srcType,
                                 void *dst, const std::string &dstType) throw (DataStore::UnassignableTypesException){
      AssignMap *am = create_singelton_assign_map();
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


    void DataStore::Data::install(Function<void,const MouseEvent&> f){
      /// crazy local class here!
      struct FunctionMouseHandler : public MouseHandler{
        Function<void,const MouseEvent&> f;
        FunctionMouseHandler(Function<void,const MouseEvent&> f):f(f){}
        void process(const MouseEvent &e){ f(e); }
      };

      install(new FunctionMouseHandler(f));
    }

    typedef AssignMap::const_iterator It;
    typedef std::map<const std::string, DataStore::Assign*>::const_iterator Jt;

    AssignMap translate_assign_map(const AssignMap *m){
      AssignMap d;
      for(It it = m->begin(); it != m->end(); ++it){
        for(Jt jt = it->second.begin(); jt != it->second.end();++jt){
          DataStore::Assign *a = jt->second;
          if(a) d[a->srcType][a->dstType] = a;
        }
      }
      return d;
    }


    void DataStore::list_possible_assignments(const std::string &srcType, const std::string &dstType){
      typedef AssignMap::const_iterator It;
      typedef std::map<const std::string, Assign*>::const_iterator Jt;
      AssignMap am = translate_assign_map(create_singelton_assign_map());

      if(srcType.length()){
        It it = am.find(srcType);
        if(it == am.end()){
          std::cout << "no assignment rules for source type '" << srcType << "' found" << std::endl;
        }
        if(dstType.length()){
          Jt jt = it->second.find(dstType);
          if(jt == it->second.end()){
            std::cout << "no assignment rule for " << dstType << " <- " << srcType << " found" << std::endl;
          }else{
            const Assign *as = jt->second;
            std::cout << as->dstType << " <- " << as->srcType <<
            "  (rtti: " << as->dstRTTI << " <- " << as->srcRTTI << std::endl;
          }
        }else{
          std::cout << "the following assignemt rules for source type " << srcType << " are available:" << std::endl;
          for(Jt jt=it->second.begin(); jt != it->second.end(); ++jt){
            const Assign *as = jt->second;
            std::cout << as->dstType << " <- " << as->srcType <<
            "  (rtti: " << as->dstRTTI << " <- " << as->srcRTTI << std::endl;
          }
        }
      }else{
        if(dstType.length()){
          std::cout << "the following assignemt rules for destination type " << dstType << " are available:" << std::endl;
          for(It it = am.begin(); it != am.end(); ++it){
            Jt jt = it->second.find(dstType);
            if(jt != it->second.end()){
              Assign *a = jt->second;
              std::cout << "   " << a->dstType << " <- " << a->srcType <<
              "  (rtti: " << a->dstRTTI << " <- " << a->srcRTTI << std::endl;
            }
          }
        }else{
          int num = 0;
          for(It it = am.begin(); it != am.end(); ++it){
            num += it->second.size();
          }
          std::cout << "listing all " << num << " assignment rules " << std::endl;
          for(It it = am.begin(); it != am.end(); ++it){
            std::cout << "source type: " << it->first << std::endl;
            for(Jt jt = it->second.begin(); jt != it->second.end();++jt){
              Assign *a = jt->second;
              std::cout << "   " << a->dstType << " <- " << a->srcType <<
              "  (rtti: " << a->dstRTTI << " <- " << a->srcRTTI << std::endl;
            }
          }
        }
      }
    }

  } // namespace qt
}

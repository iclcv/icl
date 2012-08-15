#ifndef ICL_GUI_COMPONENT_H
#define ICL_GUI_COMPONENT_H

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Range.h>

namespace icl{

  
  class GUIComponent{
    
    friend class ContainerGUIComponent;
    
    public:
    struct Options {
    Options():margin(-1),spacing(-1){}
      std::string handle;
      std::string out;
      std::string in;
      std::string label;
      std::string tooltip;
      int margin;
      int spacing;
      Size minSize;
      Size maxSize;
      Size size;
    };
    protected:
    mutable Options m_options;
    
    template<class A, class B, class C> 
    static std::string form_args_3(const A &a, const B &b, const C &c){
      std::ostringstream str;
      str << a << ',' << b << ',' << c;
      return str.str();
    }
    
    template<class A, class B, class C, class D> 
    static std::string form_args_4(const A &a, const B &b, const C &c, const D &d){
      std::ostringstream str;
      str << a << ',' << b << ',' << c << ',' << d;
      return str.str();
    }
    
    std::string m_type;
    std::string m_params;
    
    GUIComponent(const std::string &type, const std::string &params=""):
    m_type(type),m_params(params){}

    public:
      
    const GUIComponent &handle(const std::string &handle) const{
      m_options.handle = handle; return *this;
    }

    const GUIComponent &label(const std::string &label) const{
      m_options.label = label; return *this;
    }

    const GUIComponent &tooltip(const std::string &tooltip) const{
      m_options.tooltip = tooltip; return *this;
    }

    const GUIComponent &size(const Size &size) const {
      m_options.size = size; return *this;
    }

    const GUIComponent &size(int w, int h) const {
      return size(Size(w,h));
    }

    const GUIComponent &minSize(const Size &minSize) const {
      m_options.minSize = minSize; return *this;
    }
      
    const GUIComponent &minSize(int w, int h) const {
      return minSize(Size(w,h));
    }
    
    const GUIComponent &maxSize(const Size &maxSize) const {
      m_options.maxSize = maxSize; return *this;
    }

    const GUIComponent &maxSize(int w, int h) const {
      return maxSize(Size(w,h));
    }

    GUIComponent &handle( std::string &handle) {
      m_options.handle = handle; return *this;
    }
    
    GUIComponent &label( std::string &label) {
      m_options.label = label; return *this;
    }
    
    GUIComponent &tooltip( std::string &tooltip) {
      m_options.tooltip = tooltip; return *this;
    }
    
    GUIComponent &size( Size &size)  {
      m_options.size = size; return *this;
    }
    
    GUIComponent &size(int w, int h)  {
      m_options.size = Size(w,h); return *this;
    }
    
    GUIComponent &minSize( Size &minSize)  {
      m_options.minSize = minSize; return *this;
    }
    
    GUIComponent &minSize(int w, int h)  {
      m_options.minSize = Size(w,h); return *this;
    }
    
    GUIComponent &maxSize( Size &maxSize)  {
      m_options.maxSize = maxSize; return *this;
    }
    
    GUIComponent &maxSize(int w, int h)  {
      m_options.maxSize = Size(w,h); return *this;
    }
    
    std::string toString() const {
      std::ostringstream str;
      str << m_type;
      if(m_params.length()){
        str << '(' << m_params << ')';
      }
      if(m_options.handle.length() ||
         m_options.out.length() ||
         m_options.in.length() ||
         m_options.tooltip.length() ||
         m_options.margin > 0 ||
         m_options.spacing > 0 ||
         m_options.minSize != Size::null ||
         m_options.maxSize != Size::null ||
         m_options.size != Size::null ){
        str << '[';
        if(m_options.handle.length()) str << "@handle=" << m_options.handle;
        if(m_options.out.length()) str << "@out=" << m_options.out;
        if(m_options.in.length()) str << "@in=" << m_options.in;
        if(m_options.label.length()) str << "@label=" << m_options.label;
        if(m_options.tooltip.length()) str << "@tooltip=" << m_options.tooltip;
        if(m_options.margin > 0) str << "@margin=" << m_options.margin;
        if(m_options.spacing > 0) str << "@spacing=" << m_options.spacing;
        if(m_options.minSize != Size::null ) str << "@minsize=" << m_options.minSize;
        if(m_options.maxSize != Size::null ) str << "@maxsize=" << m_options.maxSize;
        if(m_options.size != Size::null ) str << "@size=" << m_options.size;
        str << "]";
      }
      return str.str();
    }
  };

  struct GUIComponentWithOutput : public GUIComponent{
    GUIComponentWithOutput(const std::string &type, const std::string &params):
    GUIComponent(type,params){}

    const GUIComponentWithOutput &out(const std::string &out) const{
      m_options.out = out; return *this;
    }
       
    const GUIComponentWithOutput &handle(const std::string &handle) const{
      m_options.handle = handle; return *this;
    }

    const GUIComponentWithOutput &label(const std::string &label) const{
      m_options.label = label; return *this;
    }

    const GUIComponentWithOutput &tooltip(const std::string &tooltip) const{
      m_options.tooltip = tooltip; return *this;
    }

    const GUIComponentWithOutput &size(const Size &size) const {
      m_options.size = size; return *this;
    }

    const GUIComponentWithOutput &size(int w, int h) const {
      return size(Size(w,h));
    }

    const GUIComponentWithOutput &minSize(const Size &minSize) const {
      m_options.minSize = minSize; return *this;
    }
      
    const GUIComponentWithOutput &minSize(int w, int h) const {
      return minSize(Size(w,h));
    }
    
    const GUIComponentWithOutput &maxSize(const Size &maxSize) const {
      m_options.maxSize = maxSize; return *this;
    }

    const GUIComponentWithOutput &maxSize(int w, int h) const {
      return maxSize(Size(w,h));
    }

    GUIComponentWithOutput &out( std::string &out) {
      m_options.out = out; return *this;
    }
    
    GUIComponentWithOutput &handle( std::string &handle) {
      m_options.handle = handle; return *this;
    }
    
    GUIComponentWithOutput &label( std::string &label) {
      m_options.label = label; return *this;
    }
    
    GUIComponentWithOutput &tooltip( std::string &tooltip) {
      m_options.tooltip = tooltip; return *this;
    }
    
    GUIComponentWithOutput &size( Size &size)  {
      m_options.size = size; return *this;
    }
    
    GUIComponentWithOutput &size(int w, int h)  {
      m_options.size = Size(w,h); return *this;
    }
    
    GUIComponentWithOutput &minSize( Size &minSize)  {
      m_options.minSize = minSize; return *this;
    }
    
    GUIComponentWithOutput &minSize(int w, int h)  {
      m_options.minSize = Size(w,h); return *this;
    }
    
    GUIComponentWithOutput &maxSize( Size &maxSize)  {
      m_options.maxSize = maxSize; return *this;
    }
    
    GUIComponentWithOutput &maxSize(int w, int h)  {
      m_options.maxSize = Size(w,h); return *this;
    }
    
  };
  
  struct Border : public GUIComponent{
    Border(const std::string &label):GUIComponent("border",label){}
  };

  struct Button : public GUIComponentWithOutput{
    Button(const std::string &text, const std::string &toggledText=""):
    GUIComponentWithOutput(toggledText.length() ? "togglebutton" : "button",toggledText.length() ? (text+','+toggledText) : text){}
  };
    

  struct ButtonGroup : public GUIComponentWithOutput{
    ButtonGroup(const std::string &commaSepTexts):
    GUIComponentWithOutput("buttongroup",commaSepTexts){}
  };

#if 0
  // use Button(with two strings instead)
  struct ToggleButton : public GUIComponentWithOutput{
    ToggleButton(const std::string &untoggledText, const std::string &toggledText):
    GUIComponentWithOutput("togglebutton",untoggledText+','+toggledText){}
  };
#endif

  struct CheckBox : public GUIComponentWithOutput{
    CheckBox(const std::string &label, bool checked=false):
    GUIComponentWithOutput("checkbox",label+','+(checked ? "checked":"unchecked")){}
  };
    
  struct Label : public GUIComponent{
    Label(const std::string &text):GUIComponent("label",text){}
  };

  struct Slider : public GUIComponentWithOutput{
    Slider(int min=0, int max=100, int curr=50, bool vertical=false):
    GUIComponentWithOutput("slider",form_args_4(min,max,curr,vertical?"vertical":"horizontal")){}

    Slider(const Range32f &r, int curr, bool vertical=false):
    GUIComponentWithOutput("slider",form_args_4(r.minVal,r.maxVal,curr,vertical?"vertical":"horizontal")){}
  };

  struct FSlider : public GUIComponentWithOutput{
    FSlider(float min=0, float max=1, float curr=0.5, bool vertical=false):
    GUIComponentWithOutput("fslider",form_args_4(min,max,curr,vertical?"vertical":"horizontal")){}
    
    FSlider(const Range32f &r, float curr=0.5, bool vertical=false):
    GUIComponentWithOutput("fslider",form_args_4(r.minVal,r.maxVal,curr,vertical?"vertical":"horizontal")){}
  };

  struct Int : public GUIComponentWithOutput{
    Int(int min=0, int max=100, int curr=50):
    GUIComponentWithOutput("int",form_args_3(min, max, curr)){}
  };

  struct Float : public GUIComponentWithOutput{
    Float(float min=0, float max=1, float curr=0.5):
    GUIComponentWithOutput("float",form_args_3(min, max, curr)){}
  };

  struct String : public GUIComponent{
    // should be with-output, but this cannot be thread-safe !!
    String(const std::string &initText, int maxLen=100):GUIComponent("string",initText+','+str(maxLen)){}
  };

  struct Disp : public GUIComponent{
    Disp(int nxCells, int nyCells):
    GUIComponent("disp",str(nxCells)+','+str(nyCells)){}
  };

  struct Image : public GUIComponent{
    Image():GUIComponent("image"){}
  };

  struct Draw : public GUIComponent{
    Draw(const Size &defaultViewPortsize=Size::VGA):
    GUIComponent("draw",str(defaultViewPortsize)){}
  };

  struct Draw3D : public GUIComponent{
    Draw3D(const Size &defaultViewPortsize=Size::VGA):
    GUIComponent("draw3D",str(defaultViewPortsize)){}
  };

    
  struct Plot : public GUIComponent{
    private:
    static std::string form_args(const Range32f xRange, const Range32f yRange, bool useOpenGL,
                                 const std::string &xLabel,  const std::string &yLabel){
      std::ostringstream str;
      str << xRange.minVal << ',' << xRange.maxVal << ',' << yRange.minVal << ',' << xRange.maxVal << ','
          << (useOpenGL ? "gl" : "noGL");
      if(xLabel.length()){
        str << ',' << xLabel;
        if(xLabel.length()){
          str << ',' << yLabel;
        }
      }
      return str.str();
    }

    public:
    Plot(const Range32f xRange=Range32f(0,0),
         const Range32f yRange=Range32f(0,0),
         bool useOpenGL=false,
         const std::string &xLabel="",
         const std::string &yLabel=""): 
    GUIComponent("plot",form_args(xRange,xRange, useOpenGL, xLabel, yLabel)){}
  };
    
  struct Combo : public GUIComponent{
    // should be with-output, but this cannot be thread-safe !!
    Combo(const std::string &commaSepEntries):
    GUIComponent("combo",commaSepEntries){}
  };
    
  struct Spinner : public GUIComponentWithOutput{
    Spinner(int min, int max, int curr):
    GUIComponentWithOutput("spinner",form_args_3(min,max,curr)){}
  };
    

  struct Fps : public GUIComponent{
    Fps(int timeWindowSize=10):
    GUIComponent("fps",str(timeWindowSize)){}
  };
    
   
  struct CamCfg : public GUIComponent{
    private:
    static inline std::string form_args(const std::string &a, const std::string &b){
      std::ostringstream str;
      if(a.length()){
        str << a;
        if(b.length()){
          str << ',' << b;
        }
      }
      return str.str();
    }
    public:
    CamCfg(const std::string &deviceTypeHint="", const std::string &deviceIDHint=""):GUIComponent("camcfg",form_args(deviceTypeHint,deviceIDHint)){}
  };

  struct Prop : public GUIComponent{
    Prop(const std::string &configurableID):GUIComponent("prop",configurableID){}
  };

    
  struct ColorSelect : public GUIComponentWithOutput{
    ColorSelect(int r, int g, int b, int a=-1):GUIComponentWithOutput("color",a>=0 ? form_args_4(r,g,b,a) : form_args_3(r,g,b)){}
  };
    
  struct Ps : public GUIComponent{
    Ps(int updatePFS=10):GUIComponent("ps",str(updatePFS)){}
  };

  /// [Deprecated]
  struct ICL_DEPRECATED MultiDrawComponent : public GUIComponent{
    MultiDrawComponent(const std::string commanSepTabLabels, bool bufferAll=false, bool deepCopyBuffering=true):
    GUIComponent("multidraw",str(bufferAll?"!all":"!one")+','+(deepCopyBuffering?"!deepcopy":"!shallowcopy")+','+commanSepTabLabels){}
  };

  /// [Deprecated]
  struct ICL_DEPRECATED ConfigFileComponent : public GUIComponent{
    ConfigFileComponent(bool popup=true):GUIComponent("config",popup?"popup":"embedded"){}
  };
    

  struct Dummy : public GUIComponent{
    Dummy():GUIComponent(""){}
  };
    
  struct Show : public GUIComponent{
    Show():GUIComponent("!show"){};
  };

  struct Create : public GUIComponent{
    Create():GUIComponent("!create"){};
  };
    
}

#endif

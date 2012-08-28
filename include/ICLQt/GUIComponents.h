#pragma once

#include <ICLQt/GUIComponentWithOutput.h>

namespace icl{  
  namespace qt{
    /// Button Component
    /** Buttons can either be push- or toggle buttons.
        Buttons create a ButtonHandle. If a toggle button is created, also an output of type bool is created,
        which is true as long as the button is toggled.
    */
    struct Button : public GUIComponentWithOutput{
      private:
      /// utility method
      static std::string form_args(const std::string &text, const std::string &toggledText, bool initiallyToggled){
        if(!toggledText.length()){
          return text;
        }
        std::ostringstream str;
        str << text << ',';
        if(initiallyToggled) str << '!';
        str << toggledText;
        return str.str();
      }
      public:
      /// creates button component (if toggledText is empty, a push-button is created)
      Button(const std::string &text, const std::string &toggledText="", bool initiallyToggled=false):
      GUIComponentWithOutput(toggledText.length() ? "togglebutton" : "button",form_args(text,toggledText,initiallyToggled)){}
    };
      
    /// ButtonGroup component (aka vertical list of radio buttons)
    /** Creates a ButtonGroupHandle and an int-valued output, that contains the index of the currently checked radio-button */
    struct ButtonGroup : public GUIComponentWithOutput{
      ButtonGroup(const std::string &commaSepTexts):
      GUIComponentWithOutput("buttongroup",commaSepTexts){}
    };
  
    /// CheckBox component
    /** Creates a CheckBoxHandle and a boolean output that is true as long as the check box is checked */
    struct CheckBox : public GUIComponentWithOutput{
      /// create a check box component, optionally checked intially
      CheckBox(const std::string &label, bool checked=false):
      GUIComponentWithOutput("checkbox",label+','+(checked ? "checked":"unchecked")){}
    };
      
    /// Label component for displaying text
    /** Creates a LabelHandle, that can be used to display text. The text can have several lines */
    struct Label : public GUIComponent{
      /// create label with optionally given initial text
      Label(const std::string &text=""):GUIComponent("label",text){}
    };
  
    /// Slider component for int-ranges
    /** Creates a SliderHandle and an int-valued ouptut that contains the current slider value.
        Sliders always come with an QLCDNumber component, that displays the current slider value */
    struct Slider : public GUIComponentWithOutput{
      /// creates a slider with given POD parameters
      Slider(int min=0, int max=100, int curr=50, bool vertical=false):
      GUIComponentWithOutput("slider",form_args_4(min,max,curr,vertical?"vertical":"horizontal")){}
  
      /// creates a slider from given int-range
      Slider(const utils::Range32f &r, int curr, bool vertical=false):
      GUIComponentWithOutput("slider",form_args_4(r.minVal,r.maxVal,curr,vertical?"vertical":"horizontal")){}
    };
  
    /// Float-valued Slider component
    /** Creates a FSliderHandle and an float-valued ouptut that contains the current slider value.
        Sliders always come with an QLCDNumber component, that displays the current slider value */
    struct FSlider : public GUIComponentWithOutput{
      /// creates a FSlider with given POD parameters
      FSlider(float min=0, float max=1, float curr=0.5, bool vertical=false):
      GUIComponentWithOutput("fslider",form_args_4(min,max,curr,vertical?"vertical":"horizontal")){}
      
      /// creates a FSlider from given float-range
      FSlider(const utils::Range32f &r, float curr=0.5, bool vertical=false):
      GUIComponentWithOutput("fslider",form_args_4(r.minVal,r.maxVal,curr,vertical?"vertical":"horizontal")){}
    };
  
    /// Text Input component, that allows integer inputs in a given range
    /** Creates an IntHandle and an int-valued output */
    struct Int : public GUIComponentWithOutput{
      /// create integer input component with given range and initial value
      Int(int min=0, int max=100, int curr=50):
      GUIComponentWithOutput("int",form_args_3(min, max, curr)){}
    };
  
    /// Text Input component, that allows float inputs in a given range
    /** Creates an FloatHandle and an float-valued output */
    struct Float : public GUIComponentWithOutput{
      /// create float input component with given range and initial value
      Float(float min=0, float max=1, float curr=0.5):
      GUIComponentWithOutput("float",form_args_3(min, max, curr)){}
    };
  
    /// Text Input component, that allows float inputs with a given maximun length
    /** Creates an StringHandle (but no output) */
    struct String : public GUIComponent{
      /// create string input compoent with given max length
      String(const std::string &initText, int maxLen=100):GUIComponent("string",initText+','+utils::str(maxLen)){}
    };
  
    /// Display component for a 2D Array of labels
    /** Creates a DispHandle that provides access to each label */
    struct Disp : public GUIComponent{
      Disp(int nxCells, int nyCells):
      GUIComponent("disp",utils::str(nxCells)+','+utils::str(nyCells)){}
    };
  
    /// Image visualization component
    /** Creates an ImageHandle for image visualization */
    struct Image : public GUIComponent{
      /// constructor
      Image():GUIComponent("image"){}
    };
  
    /// Image visualization component that allows for overlayed 2D image annotation
    /** Creates an DrawHandle for image visualization and annotation */
    struct Draw : public GUIComponent{
      /// create draw component with given default view port size
      /** The defaultViewPortsize is uses as annotation coordinate frame as long as not image
          is provided */
      Draw(const utils::Size &defaultViewPortsize=utils::Size::VGA):
      GUIComponent("draw",str(defaultViewPortsize)){}
    };
  
    /// Image visualization compoent that allows for 2D and 3D image annotation
    /** Creates a DrawHandle3D for image visualization and annotation. The Draw3D component
        is closely integrated with the icl::geom::Scene class. Together these classes can
        be used to render a 3D scene on top of a camera image */
    struct Draw3D : public GUIComponent{
      /// create Draw3D component with given defaultViewPortsize
      /** The given defaultViewPortsize is to create an OpenGL viewport as long as no
          backgrond image is given. */
      Draw3D(const utils::Size &defaultViewPortsize=utils::Size::VGA):
      GUIComponent("draw3D",str(defaultViewPortsize)){}
    };
  
    /// a 2D function and data plotting component
    /** Creates a PlotHandle */
    struct Plot : public GUIComponent{
      private:
      /// utility method
      static std::string form_args(const utils::Range32f xRange, 
                                   const utils::Range32f yRange, bool useOpenGL,
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
      /// Create Plot component with optionally given range parameters
      /** @param xRange horizontal range of the data view-port. If [0,0], then it is derived from the data 
          @param yRange vertical range of the data view-port. If [0,0], then it is derived from the data 
          @param useOpenGL if true, the renderering is performed in OpenGL (this can be faster in case of transparency) 
          @param xLabel x-axis label 
          @param yLabel y-axis label 
      */
      Plot(const utils::Range32f xRange=utils::Range32f(0,0),
           const utils::Range32f yRange=utils::Range32f(0,0),
           bool useOpenGL=false,
           const std::string &xLabel="",
           const std::string &yLabel=""): 
      GUIComponent("plot",form_args(xRange,xRange, useOpenGL, xLabel, yLabel)){}
  
      /// Create Plot component with optionally given POD parameters
      /** The parameters are described well in the other constructor */
      Plot(float minX, float maxX=0, float minY=0, float maxY=0,
           bool useOpenGL=false,
           const std::string &xLabel="",
           const std::string &yLabel=""): 
      GUIComponent("plot",form_args(utils::Range32f(minX,maxX),utils::Range32f(minY,maxY), useOpenGL, xLabel, yLabel)){}
    };
      
    /// ComboBox GUI component 
    /** Creates a ComboHandle*/
    struct Combo : public GUIComponent{
      private:
      /// utility method
      static std::string form_args(const std::string &entries, int initialIndex){
        if(!initialIndex) return entries;
        std::vector<std::string> ls = utils::tok(entries,",");
        if(initialIndex < 0 || initialIndex >= (int)ls.size()){
          throw utils::ICLException("Combo::Combo(entries,initialIndex): initialIndex is invalid");
        }
        ls[initialIndex] = '!' + ls[initialIndex];
        return utils::cat(ls,",");
      }
      public:
      /// Constructor with given comma separated list of initial entries
      Combo(const std::string &commaSepEntries, int initialIndex=0):
      GUIComponent("combo",form_args(commaSepEntries,initialIndex)){}
    };
      
    /// SpinBox component
    /** Creates a SpinnerHandle and an int-valued output that reflects the current value */
    struct Spinner : public GUIComponentWithOutput{
      /// creat spinbox component with given range and initial value
      Spinner(int min, int max, int curr):
      GUIComponentWithOutput("spinner",form_args_3(min,max,curr)){}
    };
      
  
    /// Frames per second estimator component
    /** create an FpsHandle that must be explicitly re-rendered in every loop-cycle of the 
        FPS-monitored loop */
    struct Fps : public GUIComponent{
      /// creates fps component with given time window for estimating a running-average of the computed FPS-values
      Fps(int timeWindowSize=10):
      GUIComponent("fps",utils::str(timeWindowSize)){}
    };
      
    /// camera configuration component
    /** Create no handle and no output. Results in a button, that opens a real-time camera 
        property configuration dialog. The component can either be used to adjust all camera,
        a grabber was instantiated for or, it can be tuned for a certain camera with given
        non-default constructor parameters.
        <b>Please note:</b> If no constructor parameters are given, only grabbers, that
        have been created before the GUI-creation are referenced by this component
    */
    struct CamCfg : public GUIComponent{
      private:
      /// utility method
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
      /// create a camera configurabion component with optionally specified device
      CamCfg(const std::string &deviceTypeHint="", const std::string &deviceIDHint=""):GUIComponent("camcfg",form_args(deviceTypeHint,deviceIDHint)){}
    };
  
    /// Propery adjustment component for configuable instances
    /** creates an embedded tabbed vertical scroll widget that allows for 
        adjust properties of a configurable instance in real-time. The referenced
        configurable must be create before the GUI-creation.
        Creates no handle and no output */
    struct Prop : public GUIComponent{
      /// create configurable component reflecting the properties of the given Configurable instance
      Prop(const std::string &configurableID):GUIComponent("prop",configurableID){}
    };
  
    /// Color selection component
    /** Creates a ColorHandle. */
    struct ColorSelect : public GUIComponentWithOutput{
      /// create color selection component
      /** if the alpha parameter is set to -1, no alpha value can be specified */
      ColorSelect(int r, int g, int b, int a=-1):GUIComponentWithOutput("color",a>=0 ? form_args_4(r,g,b,a) : form_args_3(r,g,b)){}
    };
      
    /// Process status component
    /** Creates a complex compoment that lists online-process information, such as processor usage, memory consumption and curren thread count */
    struct Ps : public GUIComponent{
      /// create ps component with given update rate
      Ps(int updatePFS=10):GUIComponent("ps",utils::str(updatePFS)){}
    };
  
    /// Creates not component
    struct Dummy : public GUIComponent{
      Dummy():GUIComponent(""){}
    };
      
    /// Finalizes GUI creation (actually creates the Qt-GUI and makes it visible)
    struct Show : public GUIComponent{
      Show():GUIComponent("!show"){};
    };
  
    /// Finalizes GUI creation (actually creates the Qt-GUI but initially hidden)
    struct Create : public GUIComponent{
      Create():GUIComponent("!create"){};
    };
  
  } // namespace qt
}

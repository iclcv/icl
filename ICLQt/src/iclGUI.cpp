#include <iclStrTok.h>
#include <iclSize.h>
#include <iclCore.h>

#include <iclGUI.h>
#include <iclGUIWidget.h>
#include <iclGUIDefinition.h>
#include <iclGUISyntaxErrorException.h>
#include <iclException.h>
#include <iclSimpleMatrix.h>
#include <iclWidget.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QLCDNumber>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>



#include <map>

using namespace std;
using namespace icl;

namespace icl{
  static const std::string &gen_params(){
    // {{{ open

    static std::string op = 
    string("general params are: \n")+
    string("\t@size=WxH     (W and H are positive integers) set min and max size of that widget\n")+
    string("\t@minsize=WxH  (W and H are positive integers) set min. size of that widget\n")+
    string("\t@maxsize=WxH  (W and H are positive integers) set max. size of that widget\n")+
    string("\t@handle=NAME  if defined, the componets handle is allocated with id NAME\n")+
    string("\t              (all size parameters are defined in cells of 15x15 pixles)\n")+
    string("\t@label=L      L is the label of this component\n")+
    string("\t@out=LIST     LIST is a comma-seperated list of output names\n")+
    string("\t@inp=LIST     LIST is a comma-seperated list of output names\n");
    return op;
  }

  // }}}

  struct HBoxGUIWidget : public GUIWidget{
    // {{{ open
    HBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::hboxLayout,0,0,0){
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(this));//def.parentWidget()));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("hbox()[general params]\n")+gen_params();
    }
  };
  
  // }}}
  struct VBoxGUIWidget : public GUIWidget{
    // {{{ open
    VBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::vboxLayout,0,0,0){
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(this));//def.parentWidget()));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("hbox()[general params]\n")+gen_params();
    }
  };

  // }}}
  struct BorderGUIWidget : public GUIWidget{
    // {{{ open

    BorderGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,1){ 
      m_poGroupBox = new QGroupBox(def.param(0).c_str(),def.parentWidget());
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(def.margin());
      m_poLayout->setSpacing(def.spacing());
      m_poGroupBox->setLayout(m_poLayout);
      addToGrid(m_poGroupBox);
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<BorderHandle>(def.handle(),BorderHandle(m_poGroupBox));
        getGUI()->unlockData();
      }
      
    }
    static string getSyntax(){
      return string("border(LABEL)[general params]\n")+
      string("\tLABEL is the border label that is shown\n")+
      gen_params();
    }
    virtual QLayout *getGUIWidgetLayout() { return m_poLayout; }
    private:
    QGroupBox *m_poGroupBox;
    QVBoxLayout *m_poLayout;
  };

  // }}}
  struct ButtonGUIWidget : public GUIWidget{
    // {{{ open
    ButtonGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,1){
      QPushButton *b = new QPushButton(def.param(0).c_str(),def.parentWidget());
      addToGrid(b);
      connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));
      
      if(def.handle() != ""){
        getGUI()->lockData();
        m_poClickedEvent = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(b));
        getGUI()->unlockData();
      }else{
        m_poClickedEvent = 0;
      }
    }
    static string getSyntax(){
      return string("button(TEXT)[general params] \n")+
      string("\tTEXT is the button text\n")+
      gen_params();
    }
    virtual void processIO(){
      if(m_poClickedEvent){
        m_poClickedEvent->trigger();
      }
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    ButtonHandle *m_poClickedEvent;
  };

  // }}}
  struct ButtonGroupGUIWidget : public GUIWidget{
    // {{{ open
    ButtonGroupGUIWidget(const GUIDefinition &def):
      GUIWidget(def,GUIWidget::gridLayout,0,1,-1), m_uiInitialIndex(0){
      if(def.numParams() < 1) throw GUISyntaxErrorException(def.defString(),"at least one param here!a");


      for(unsigned int i=0;i<def.numParams();i++){
        string text = def.param(i);
        if(text.length() && text[0]=='!'){
          m_uiInitialIndex = i;
          text = text.substr(1);
        }
        QRadioButton * b = new QRadioButton(text.c_str(),def.parentWidget());
        m_vecButtons.push_back(b);
        addToGrid(b,0,i);
        connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));
      }
      
      getGUI()->lockData();
      m_uiIdx = &getGUI()->allocValue<unsigned int>(def.output(0),m_uiInitialIndex);
      getGUI()->unlockData();
      
      m_vecButtons[m_uiInitialIndex]->setChecked(true);

      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ButtonGroupHandle>(def.handle(),ButtonGroupHandle(&m_vecButtons));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return string("buttongroup(LIST)[general params] \n")+
      string("\tLIST is a comma seperated list of radio button texts to create\n")+
      string("\tthe button with a '!'-prefix is selected (of index 0 by default)\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_uiIdx = 0;
      for(unsigned int i=0;i<m_vecButtons.size();i++){
        if(m_vecButtons[i]->isChecked()){
          *m_uiIdx = i;
          break;
        }
      }
    }
    virtual Size getDefaultSize() { 
      return Size(4,m_vecButtons.size()); 
    }
  private:
    unsigned int *m_uiIdx;
    vector<QRadioButton*> m_vecButtons;
    unsigned int m_uiInitialIndex ;
  };

  // }}}
  struct ToggleButtonGUIWidget : public GUIWidget{
    // {{{ open
  public:
    ToggleButtonGUIWidget(const GUIDefinition &def):
      GUIWidget(def,GUIWidget::gridLayout,0,1,2){
      m_asText[0] = def.param(0);
      m_asText[1] = def.param(1);
      m_iCurr = 0;
      if(m_asText[1].length() && m_asText[1][0] == '!'){
        m_asText[1]=m_asText[1].substr(1);
        m_iCurr = 1;
      }
      if(m_asText[0].length() && m_asText[0][0] == '!'){
        m_asText[0]=m_asText[0].substr(1);
        m_iCurr = 0;
      }
      
      m_poButton = new QPushButton(m_asText[0].c_str(),def.parentWidget());
      m_poButton->setCheckable(1);
      if(m_iCurr){
        m_poButton->setChecked(true);
        m_poButton->setText(m_asText[1].c_str());
      }
      addToGrid(m_poButton);
      connect(m_poButton,SIGNAL(pressed()),this,SLOT(ioSlot()));
      
      getGUI()->lockData();
      m_pbToggled = &getGUI()->allocValue<bool>(def.output(0),m_iCurr?true:false);
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        m_poHandle = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(m_poButton));
        getGUI()->unlockData();
      }else{
        m_poHandle = 0;
      }
    }
    static string getSyntax(){
      return string("togglebutton(U,T)[general params] \n")+
      string("\tU is the buttons text in untoggled state\n")+
      string("\tT is the buttons text in toggled state\n")+
      string("\tif one of U or T has a '!'-prefix, the button is created with this state\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_pbToggled = !(*m_pbToggled);
      m_poButton->setText(m_asText[*m_pbToggled].c_str());
      if(m_poHandle){
        m_poHandle->trigger();
      }
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    QPushButton *m_poButton;
    ButtonHandle *m_poHandle;
    bool *m_pbToggled;
    string m_asText[2];
    int m_iCurr;
  };

// }}}
  struct LabelGUIWidget : public GUIWidget{
    // {{{ open
    LabelGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,-1){
      if(def.numParams() > 1) throw GUISyntaxErrorException(def.defString(),"need max. 1 parameter here!");
      if(def.numInputs() > 1) throw GUISyntaxErrorException(def.defString(),"need max. 1 input here!");
    
      m_poLabel = new CompabilityLabel(def.numParams()==1?def.param(0).c_str():"",def.parentWidget());
      
      addToGrid(m_poLabel);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<LabelHandle>(def.handle(),LabelHandle(m_poLabel));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("label(TEXT="")[general params] \n")+
      string("\tTEXT is the initial text showed by the label")+
      gen_params();
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    CompabilityLabel *m_poLabel;
  };
  
  // }}}
  struct SliderGUIWidget : public GUIWidget{
    // {{{ open
    SliderGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,-1){
      
      /// param_order = min,max,curr,step=1,orientation=("horizontal")|"vertical"
      if(def.numParams() < 3) throw GUISyntaxErrorException(def.defString(),"need at least 3 parameters here!");
      if(def.numParams() > 4) throw GUISyntaxErrorException(def.defString(),"need max. 4 parameters here!");
      
      m_piValue = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      
      int iVerticalFlag = (def.numParams() == 4) ? (def.param(3)=="vertical") : false;
      int iMin = def.intParam(0);
      int iMax = def.intParam(1);
      int iCurr = def.intParam(2);
      
      if(iVerticalFlag){
        m_poSlider = new QSlider(Qt::Vertical,def.parentWidget());
      }else{
        m_poSlider = new QSlider(Qt::Horizontal,def.parentWidget());
      }
      addToGrid(m_poSlider);
      
      m_poSlider->setMinimum(iMin);
      m_poSlider->setMaximum(iMax);
      m_poSlider->setValue(iCurr);
     
      int iAbsMax = iMax > -iMin ? iMax : -iMin;
      int iAddOneForSign = iMax < -iMin;
      m_poLCD = new QLCDNumber(QString::number(iAbsMax).length()+iAddOneForSign,def.parentWidget());
      m_poLCD->display(iCurr);
      
      if(iVerticalFlag){
        addToGrid(m_poLCD,0,1,1,4);
      }else{
        addToGrid(m_poLCD,1,0,4,1);
      }

      connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      connect(m_poSlider,SIGNAL(valueChanged(int)),m_poLCD,SLOT(display(int)));
      
      m_bVerticalFlag = iVerticalFlag ? true : false;
     
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<SliderHandle>(def.handle(),SliderHandle(m_poSlider));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("slider(MIN,MAX,CURR,ORIENTATION=horizontal)[general params] \n")+
      string("\tMIN is the minimum value of the slider\n")+
      string("\tMAX is the maximum value of the slider\n")+
      string("\tCURR is the initializing value of the slider\n")+
      string("\tORIENTATION is horizontal or vertical\n");
      gen_params();
    }
    virtual void processIO(){
      //iStep is handled as a value that must '%' the slider to 0
      *m_piValue = m_poSlider->value();
    }
    virtual Size getDefaultSize() { 
      return m_bVerticalFlag ? Size(1,4): Size(4,1);
    }
  private:
    QSlider *m_poSlider;
    QLCDNumber *m_poLCD;
    int *m_piValue;
    bool m_bVerticalFlag;
  };

// }}}
  struct FloatSliderGUIWidget : public GUIWidget{
    // {{{ open
    FloatSliderGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,-1){
      //
      // y = mx+b
      // m =dy/dx = max-min/1000
      // b = min 
      //float fMin, float fMax, float fCurr vertical|horizontal
      //
      
      /// param_order = min,max,curr,orientation=("horizontal")|"vertical"
      if(def.numParams() < 3) throw GUISyntaxErrorException(def.defString(),"need at least 3 parameters here!");
      if(def.numParams() > 4) throw GUISyntaxErrorException(def.defString(),"need max. 4 parameters here!");
  
      getGUI()->lockData();
      m_pfValue = &getGUI()->allocValue<float>(def.output(0),def.floatParam(2));
      getGUI()->unlockData();
      
      int iVerticalFlag = (def.numParams() == 4) ? (def.param(3)=="vertical") : false;
      float fMin = def.floatParam(0);
      float fMax = def.floatParam(1);
      float fCurr = def.floatParam(2);
      int nDigits = 6;
      
      m_fM = (fMax-fMin)/10000.0;
      m_fB = fMin;
      
      if(iVerticalFlag){
        m_poSlider = new QSlider(Qt::Vertical,def.parentWidget());
      }else{
        m_poSlider = new QSlider(Qt::Horizontal,def.parentWidget());
      }
      addToGrid(m_poSlider);

      m_poSlider->setMinimum(0);
      m_poSlider->setMaximum(10000);
      m_poSlider->setValue(f2i(fCurr));
      m_poLCD = new QLCDNumber(nDigits,def.parentWidget());
      m_poLCD->display(fCurr);
      
      if(iVerticalFlag){
        addToGrid(m_poLCD,0,1,1,4);
      }else{
        addToGrid(m_poLCD,1,0,4,1);
      }    
      connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<FSliderHandle>(def.handle(),FSliderHandle(m_poSlider,fMin,fMax,10000));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("fslider(MIN,MAX,CURR,ORIENTATION=horizontal)[general params] \n")+
      string("\tMIN is the minimum value of the slider\n")+
      string("\tMAX is the maximum value of the slider\n")+
      string("\tCURR is the initializing value of the slider\n")+
      string("\tORIENTATION is horizontal or vertical\n");
      gen_params();
    }
    virtual void processIO(){
      float value = i2f(m_poSlider->value());
      m_poLCD->display(value);
      //      pritnf("displaying %f \n",value);
      *m_pfValue = value;
    }
  private:
    QSlider *m_poSlider;
    QLCDNumber *m_poLCD;
    float *m_pfValue;
    float m_fM,m_fB;
    int f2i(float f){
      return (int)((f-m_fB)/m_fM);
    }
    float i2f(int i){
      return m_fM*i+m_fB;
    } 
  };

// }}}
  struct IntGUIWidget : public GUIWidget{
    // {{{ open
public:
    IntGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,3){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new QIntValidator(def.intParam(0),def.intParam(1),0));
      m_poLineEdit->setText(QString::number(def.intParam(2)));
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);

      getGUI()->lockData();
      m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      getGUI()->unlockData();
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<IntHandle>(def.handle(),IntHandle(m_poLineEdit));
        getGUI()->unlockData();
      }
    }
    static string getSyntax(){
      return 
      string("int(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum allowed input value\n")+
      string("\tMAX is the maximum allowed input value\n")+
      string("\tCURR is the initial value of the textfield\n")+
      gen_params();
    }
    virtual void processIO(){
      bool iOk;
      int iVal = m_poLineEdit->text().toInt(&iOk);
      if(iOk){
        *m_piOutput = iVal;
      }
    }
  private:
    QLineEdit *m_poLineEdit;
    int *m_piOutput;
  };

// }}}
  struct FloatGUIWidget : public GUIWidget{
    // {{{ open
public:
    FloatGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,3){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new QDoubleValidator(def.floatParam(0),def.floatParam(1),20,0));
      m_poLineEdit->setText(QString::number(def.floatParam(2)));
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);
      getGUI()->lockData();
      m_pfOutput = &getGUI()->allocValue<float>(def.output(0),def.intParam(2));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<FloatHandle>(def.handle(),FloatHandle(m_poLineEdit));
        getGUI()->unlockData();
      }

    }
    static string getSyntax(){
      return 
      string("float(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum allowed input value\n")+
      string("\tMAX is the maximum allowed input value\n")+
      string("\tCURR is the initial value of the textfield\n")+
      gen_params();
    }
    virtual void processIO(){
      bool iOk;
      float fVal = m_poLineEdit->text().toFloat(&iOk);
      if(iOk){
        *m_pfOutput = fVal;
      }
    }
  private:
    QLineEdit *m_poLineEdit;
    float *m_pfOutput;
  };

// }}} 
  struct StringGUIWidget : public GUIWidget{
    // {{{ open
    class StringLenValidator : public QValidator{
    public:
      StringLenValidator(int iMaxLen):QValidator(0){
        this->iMaxLen = iMaxLen;
      }
      virtual State validate(QString &sInput,int &iPos)const{
        (void)iPos;
        return sInput.length()>iMaxLen ?  Invalid : Acceptable;
      }
    private:
      int iMaxLen;
    };
    
    StringGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,2){
      m_poLineEdit = new QLineEdit(def.parentWidget());
      m_poLineEdit->setValidator(new StringLenValidator(def.intParam(1)));
      m_poLineEdit->setText(def.param(0).c_str());
      
      QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));
      
      addToGrid(m_poLineEdit);
      
      getGUI()->lockData();
      m_psOutput = &getGUI()->allocValue<string>(def.output(0),def.param(0));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<StringHandle>(def.handle(),StringHandle(m_poLineEdit));
        getGUI()->unlockData();
      }

    }
    static string getSyntax(){
      return 
      string("string(TEXT,MAXLEN)[general params] \n")+
      string("\tTEXT is the initial value of the textfield\n")+
      string("\tMAXLEN is max. number of characters that might be written into the textfiled\n")+
      gen_params();
    }
    virtual void processIO(){
      getGUI()->lockData();
      *m_psOutput = m_poLineEdit->text().toLatin1().data();
      getGUI()->unlockData();
    }
  private:
    QLineEdit *m_poLineEdit;
    string *m_psOutput;
  };

// }}} 
  struct DispGUIWidget : public GUIWidget{
    // {{{ open
    DispGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,2){
      
      int nW = def.intParam(0);
      int nH = def.intParam(1);
      printf("nW is %d  nH is %d \n",nW,nH);
      if(nW < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");
      if(nH < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");

      m_poLabelMatrix = new LabelMatrix(nW,nH);

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<DispHandle>(def.handle(),DispHandle(m_poLabelMatrix));
        getGUI()->unlockData();  
      }
        
      for(int x=0;x<nW;x++){
        for(int y=0;y<nH;y++){
          CompabilityLabel *l = new CompabilityLabel("",def.parentWidget());
          (*m_poLabelMatrix)[x][y] = LabelHandle(l);
          addToGrid(l,x,y);
        }
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return 
      string("disp(NW,NH)[general params] \n")+
      string("\tNW is width of the display label matrix (must be > 0)")+    
      string("\tNH is height of the display label matrix (must be > 0)")+
      gen_params();
    }
    virtual Size getDefaultSize() { 
      return Size(2*m_poLabelMatrix->w(),m_poLabelMatrix->h()); 
    }
  private:
    LabelMatrix *m_poLabelMatrix;
  };
  
  // }}}
  struct ImageGUIWidget : public GUIWidget{
    // {{{ open
    ImageGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,0){

      m_poWidget = new ICLWidget(def.parentWidget());
      addToGrid(m_poWidget);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ImageHandle>(def.handle(),ImageHandle(m_poWidget));
        getGUI()->unlockData();  
      }
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("image()[general params] \n")+
      gen_params();
    }
  private:
    ICLWidget *m_poWidget;
  };

  // }}}
  struct DrawGUIWidget : public GUIWidget{
    // {{{ open
    DrawGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,0,0){
      m_poWidget = new ICLDrawWidget(def.parentWidget());
      addToGrid(m_poWidget);
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<DrawHandle>(def.handle(),DrawHandle(m_poWidget));
        getGUI()->unlockData();  
      }
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("draw()[general params] \n")+
      gen_params();
    }
  private:
    ICLDrawWidget *m_poWidget;
  };

  // }}}
  struct ComboGUIWidget : public GUIWidget{
    // {{{ open
    ComboGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,-1){
      if(def.numParams() < 1) throw GUISyntaxErrorException(def.defString(),"at least 1 param needed here!");
    
      m_poCombo = new QComboBox(def.parentWidget());
      addToGrid(m_poCombo);

      unsigned int selectedIndex = 0;
      string sFirst = def.param(0);
      for(unsigned int i=0;i<def.numParams();i++){
        const std::string &s = def.param(i);
        if(s.length() && s[0]=='!'){
          sFirst = s.substr(1);
          selectedIndex = i;
          m_poCombo->addItem(s.substr(1).c_str());
        }else{
          m_poCombo->addItem(s.c_str());
        }
      }

      getGUI()->lockData();
      m_psCurrentText = &getGUI()->allocValue<string>(def.output(0),sFirst);
      getGUI()->unlockData();
      
      m_poCombo->setCurrentIndex(selectedIndex);
      
      connect(m_poCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(ioSlot()));	
      
      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<ComboHandle>(def.handle(),ComboHandle(m_poCombo));
        getGUI()->unlockData();  
      }
    }
    static string getSyntax(){
      return string("combo(entry1,entry2,entry3)[general params] \n")+
      string("\tentryX is the the X-th entry of the combo box\n")+
      string("\tif any entry has a '!'-prefix, this entry will be selected initially\n")+
      gen_params();
    }
    virtual void processIO(){
      getGUI()->lockData();
      *m_psCurrentText = m_poCombo->currentText().toLatin1().data();
      getGUI()->unlockData();
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    string *m_psCurrentText;
    QComboBox *m_poCombo;
  };

  // }}}
  struct SpinnerGUIWidget : public GUIWidget{
    // {{{ open
public:
    SpinnerGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::gridLayout,0,1,3){
      m_poSpinBox = new QSpinBox(def.parentWidget());
      m_poSpinBox->setRange(def.intParam(0),def.intParam(1));
      m_poSpinBox->setValue(def.intParam(2));
      
      QObject::connect(m_poSpinBox,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
      
      addToGrid(m_poSpinBox);

      getGUI()->lockData();
      m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
      getGUI()->unlockData();

      if(def.handle() != ""){
        getGUI()->lockData();
        getGUI()->allocValue<SpinnerHandle>(def.handle(),SpinnerHandle(m_poSpinBox));
        getGUI()->unlockData();  
      }

    }
    static string getSyntax(){
      return 
      string("spinner(MIN,MAX,CURR)[general params] \n")+
      string("\tMIN is the minimum possible value\n")+
      string("\tMAX is the maximum possible value\n")+
      string("\tCURR is the initial value of the spinbox\n")+
      gen_params();
    }
    virtual void processIO(){
      *m_piOutput = m_poSpinBox->value();
    }
  private:
    QSpinBox *m_poSpinBox;
    int *m_piOutput;
  };

// }}}

  /**
      struct HContainerGUIWidget : public GUIWidget{
      // {{{ open
    HContainerGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::hboxLayout,2,0,0){
      
      getGUI()->lockData();
      getGUI()->allocValue<QWidget*>(def.input(0),def.parentWidget());
      getGUI()->allocValue<QLayout*>(def.input(1),def.parentLayout());
      getGUI()->unlockData();
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("hcontainer()[general params]\n")+gen_params();
    }
  };
  // }}}
      struct VContainerGUIWidget : public GUIWidget{
      // {{{ open
    VContainerGUIWidget(const GUIDefinition &def):GUIWidget(def,GUIWidget::vboxLayout,2,0,0){
      
      getGUI()->lockData();
      getGUI()->allocValue<QWidget*>(def.input(0),def.parentWidget());
      getGUI()->allocValue<QLayout*>(def.input(1),def.parentLayout());
      getGUI()->unlockData();
      
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    }
    static string getSyntax(){
      return string("vcontainer()[general params]\n")+gen_params();
    }
  };
  // }}}
  **/
      
      /// template for creating arbitrary GUIWidget's
  template<class T>
  GUIWidget *create_widget_template(const GUIDefinition &def){
    // {{{ open
    T *t = 0;
    try{
      t = new  T(def);
    }catch(GUISyntaxErrorException &ex){
      printf("%s\nsyntax is: %s\n",ex.what(),T::getSyntax().c_str());
      return 0;
    }
    return t;
  }

  // }}}<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  
  /// Definition for an arbitrary GUIWidget creator function
  typedef GUIWidget* (*gui_widget_creator_function)(const GUIDefinition &def);
  
  // Type definition for a function map to accelerate the gui creation process
  typedef std::map<string,gui_widget_creator_function> CreatorFuncMap;
  
  /// NEW CREATOR MAP ENTRIES HERE !!!
  /*  
      This function is called by the GUI::create function,
      to create arbitrary widgets. To accelerate the widget creation process
      it build a CreatorFuncMap which uses the GUIDefinitinos type-string as
      identifier to estimate which creation function must be called.         */
  GUIWidget *create_widget(const GUIDefinition &def){
    // {{{ open

    /// use a static map to accelerate the widget creation process
    static bool first = true;
    static CreatorFuncMap MAP_CREATOR_FUNCS;
    if(first){
      first = false;
      /// Fill the map with creator function ( use the template if possible )
      MAP_CREATOR_FUNCS["hbox"] = create_widget_template<HBoxGUIWidget>;
      MAP_CREATOR_FUNCS["vbox"] = create_widget_template<VBoxGUIWidget>;
      MAP_CREATOR_FUNCS["button"] = create_widget_template<ButtonGUIWidget>;
      MAP_CREATOR_FUNCS["border"] = create_widget_template<BorderGUIWidget>;
      MAP_CREATOR_FUNCS["buttongroup"] = create_widget_template<ButtonGroupGUIWidget>;     
      MAP_CREATOR_FUNCS["togglebutton"] = create_widget_template<ToggleButtonGUIWidget>;
      MAP_CREATOR_FUNCS["label"] = create_widget_template<LabelGUIWidget>;
      MAP_CREATOR_FUNCS["slider"] = create_widget_template<SliderGUIWidget>;
      MAP_CREATOR_FUNCS["fslider"] = create_widget_template<FloatSliderGUIWidget>;
      MAP_CREATOR_FUNCS["int"] = create_widget_template<IntGUIWidget>;
      MAP_CREATOR_FUNCS["float"] = create_widget_template<FloatGUIWidget>;
      MAP_CREATOR_FUNCS["string"] = create_widget_template<StringGUIWidget>;
      MAP_CREATOR_FUNCS["disp"] = create_widget_template<DispGUIWidget>;
      MAP_CREATOR_FUNCS["image"] = create_widget_template<ImageGUIWidget>;
      MAP_CREATOR_FUNCS["draw"] = create_widget_template<DrawGUIWidget>;
      MAP_CREATOR_FUNCS["combo"] = create_widget_template<ComboGUIWidget>;
      MAP_CREATOR_FUNCS["spinner"] = create_widget_template<SpinnerGUIWidget>;
      //      MAP_CREATOR_FUNCS["hcontainer"] = create_widget_template<HContainerGUIWidget>;
      //      MAP_CREATOR_FUNCS["vcontainer"] = create_widget_template<VContainerGUIWidget>;
    }
    
    /// find the creator function
    CreatorFuncMap::iterator it = MAP_CREATOR_FUNCS.find(def.type());
    if(it != MAP_CREATOR_FUNCS.end()){
      /// call the function if it could be found
      return it->second(def);
    }else{
      ERROR_LOG("unknown type \""<< def.type() << "\"");
      return 0;
    }
  }

  // }}}
  
  string extract_label(string s){
    // {{{ open

    unsigned int p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("label",0)){
        if(s2.length() < 7) return "";
        return s2.substr(6);
      } 
    }
    return "";
  }

  // }}}
  string extract_minsize(string s){
    // {{{ open
    
    unsigned int p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("minsize",0)){
        if(s2.length() < 9) return "";
        return s2.substr(8);
      } 
    }
    return "";
  }

  // }}}
  string extract_maxsize(string s){
    // {{{ open
    
    unsigned int p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("maxsize",0)){
        if(s2.length() < 9) return "";
        return s2.substr(8);
      } 
    }
    return "";
  }

  // }}}
  string extract_size(string s){
    // {{{ open
    
    unsigned int p = s.find('[');
    if(p == string::npos) return "";
    s = s.substr(p+1);
    if(!s.length()) return "";
    if(s[s.length()-1] != ']') return "";
    StrTok t(s.substr(0,s.length()-1),"@");
    while(t.hasMoreTokens()){
      const string &s2 = t.nextToken();
      if(!s2.find("size",0)){
        if(s2.length() < 6) return "";
        return s2.substr(5);
      } 
    }
    return "";
  }

  // }}}

  string remove_label(const string &s, const std::string &label){
    // {{{ open
    string toRemove = string("@label=")+label;
    unsigned int p = s.find(toRemove);
    return s.substr(0,p) + s.substr(p+toRemove.length());
  } 

  // }}}

  GUI::GUI(const std::string &definition,QWidget *parent):
    // {{{ open
    m_sDefinition(definition),m_poWidget(0),m_bCreated(false),m_poParent(parent){
  }

  // }}}
  GUI::GUI(const GUI &g,QWidget *parent):
    // {{{ open
    m_sDefinition(g.m_sDefinition),
    m_vecChilds(g.m_vecChilds),
    m_poWidget(NULL),m_bCreated(false),
    m_poParent(parent){
  }
  // }}}

  GUI &GUI::operator<<(const std::string &definition){
    // {{{ open
    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    if(definition.length() > 10000) {
      throw GUISyntaxErrorException("-- long text --","definition string was too large! (>10000 characters)");
    }

    usleep(1000*100);
    string label = extract_label(definition);
    string minsize = extract_minsize(definition);
    string maxsize = extract_maxsize(definition);
    string size = extract_size(definition);
    
    if(minsize.length()) minsize = string("@minsize=")+translateSize(translateSize(minsize)+Size(1,1));
    if(maxsize.length()) minsize = string("@maxsize=")+translateSize(translateSize(minsize)+Size(1,1));
    if(size.length()) minsize = string("@size=")+translateSize(translateSize(minsize)+Size(1,1));

    if(label.length()){
      string rest = remove_label(definition,label);
      return ( (*this) << ( GUI(string("border("+label+")["+minsize+maxsize+size+"]")) << rest ) );
    }else{
      m_vecChilds.push_back(new GUI(definition));
      return *this;
    }
  }

  // }}}
  GUI &GUI::operator<<(const GUI &g){
    // {{{ open

    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    string label = extract_label(g.m_sDefinition);
    string minsize = extract_minsize(g.m_sDefinition);
    string maxsize = extract_maxsize(g.m_sDefinition);
    string size = extract_size(g.m_sDefinition);
    
    if(minsize.length()) minsize = string("@minsize=")+translateSize(translateSize(minsize)+Size(1,1));
    if(maxsize.length()) minsize = string("@maxsize=")+translateSize(translateSize(minsize)+Size(1,1));
    if(size.length()) minsize = string("@size=")+translateSize(translateSize(minsize)+Size(1,1));
    
    if(label.length()){
      GUI gNew(g);
      if(gNew.m_sDefinition.length() > 10000) {
        throw GUISyntaxErrorException("-- long text --","definition string was too large! (>10000 characters)");
      }
      gNew.m_sDefinition = remove_label(g.m_sDefinition,label);
      return ( *this << (  GUI(string("border(")+label+")["+minsize+maxsize+size+"]") << gNew ) );
    }else{
      m_vecChilds.push_back(new GUI(g));
      return *this;
    }

    
    //    m_vecChilds.push_back(new GUI(g));
    return *this;
  }

  // }}}
  
  void GUI::create(QLayout *parentLayout,QWidget *parentWidget, GUIDataStore *ds){
    // {{{ open
    if(ds) m_oDataStore = *ds;
    try{
      GUIDefinition def(m_sDefinition,this,parentLayout,parentWidget);
      
      m_poWidget = create_widget(def);
      if(!m_poWidget){
        ERROR_LOG("Widget could not be created ( aborting to avoid errors ) \n");
        exit(0);
      }
      QLayout *layout = m_poWidget->getGUIWidgetLayout();
      if(!layout && m_vecChilds.size()){
        ERROR_LOG("GUI widget has no layout, "<< m_vecChilds.size() <<" child components can't be added!");
        return;
      }
      for(unsigned int i=0;i<m_vecChilds.size();i++){
        m_vecChilds[i]->create(layout,m_poWidget,&m_oDataStore);
      }
      m_bCreated = true;
    }catch(GUISyntaxErrorException &ex){
      printf(ex.what());
      exit(0);
    }
     
  }

  // }}}
  
  void GUI::show(){
    // {{{ open
    if(m_poParent){
      create(m_poParent->layout(),m_poParent,0);
    }else{
      create(0,0,0);
    }
    m_poWidget->show();
  }

  // }}}

  void GUI::waitForCreation(){
    while(!m_bCreated){
      usleep(100*1000);
    }
  }

}

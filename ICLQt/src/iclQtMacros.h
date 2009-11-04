#ifndef ICL_QT_MACROS_H
#define ICL_QT_MACROS_H

#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclDrawWidget3D.h>
#include <iclGUI.h>
#include <iclMouseHandler.h>

#include <QApplication>
#include <iclThread.h>
#include <iclThreadUtils.h>
#include <iclMutex.h>

#include <iclButtonHandle.h>
#include <iclBoxHandle.h>
#include <iclBorderHandle.h>
#include <iclButtonGroupHandle.h>
#include <iclLabelHandle.h>
#include <iclSliderHandle.h>
#include <iclFSliderHandle.h>
#include <iclIntHandle.h>
#include <iclCheckBoxHandle.h>
#include <iclFloatHandle.h>
#include <iclStateHandle.h>
#include <iclStringHandle.h>
#include <iclComboHandle.h>
#include <iclSpinnerHandle.h>
#include <iclImageHandle.h>
#include <iclDrawHandle.h>
#include <iclDrawHandle3D.h>
#include <iclDispHandle.h>
#include <iclFPSHandle.h>
#include <iclMultiDrawHandle.h>
#include <iclTabHandle.h>
#include <iclSplitterHandle.h>

/// extract a given type as reference from GUI object named gui
#define gui_VAL(T,NAME) static T &NAME = gui.getValue<T>(#NAME)

/// extract an int as reference from GUI object named gui
#define gui_int(NAME) gui_VAL(int,NAME)

/// extract a float  as reference from GUI object named gui
#define gui_float(NAME) gui_VAL(float,NAME)

/// extract a std::string as reference from GUI object named gui
#define gui_string(NAME) gui_VAL(std::string,NAME)

/// extract a bool as reference from GUI object named gui
#define gui_bool(NAME) gui_VAL(bool,NAME)

/// extract a ButtonHandle as reference from GUI object named gui
#define gui_ButtonHandle(NAME) gui_VAL(ButtonHandle,NAME)

/// extract a BoxHandle as reference from GUI object named gui
#define gui_BoxHandle(NAME) gui_VAL(BoxHandle,NAME)

/// extract a BorderHandle as reference from GUI object named gui
#define gui_BorderHandle(NAME) gui_VAL(BorderHandle,NAME)

/// extract a ButtonGroupHandle as reference from GUI object named gui
#define gui_ButtonGroupHandle(NAME) gui_VAL(ButtonGroupHandle,NAME)

/// extract a LabelHandle as reference from GUI object named gui
#define gui_LabelHandle(NAME) gui_VAL(LabelHandle,NAME)

/// extract a SliderHandle as reference from GUI object named gui
#define gui_SliderHandle(NAME) gui_VAL(SliderHandle,NAME)

/// extract a FSliderHandle as reference from GUI object named gui
#define gui_FSliderHandle(NAME) gui_VAL(FSliderHandle,NAME)

/// extract a IntHandle as reference from GUI object named gui
#define gui_IntHandle(NAME) gui_VAL(IntHandle,NAME)

/// extract a FloatHandle as reference from GUI object named gui
#define gui_FloatHandle(NAME) gui_VAL(FloatHandle,NAME)

/// extract a StringHandle as reference from GUI object named gui
#define gui_StringHandle(NAME) gui_VAL(StringHandle,NAME)

/// extract a ComboHandle as reference from GUI object named gui
#define gui_ComboHandle(NAME) gui_VAL(ComboHandle,NAME)

/// extract a SpinnerHandle as reference from GUI object named gui
#define gui_SpinnerHandle(NAME) gui_VAL(SpinnerHandle,NAME)

/// extract a ImageHandle as reference from GUI object named gui
#define gui_ImageHandle(NAME) gui_VAL(ImageHandle,NAME)

/// extract a DrawHandle as reference from GUI object named gui
#define gui_DrawHandle(NAME) gui_VAL(DrawHandle,NAME)

/// extract a BoxHandle3D as reference from GUI object named gui
#define gui_DrawHandle3D(NAME) gui_VAL(DrawHandle3D,NAME)

/// extract a DispHandle as reference from GUI object named gui
#define gui_DispHandle(NAME) gui_VAL(DispHandle,NAME)

/// extract a FPSHandle as reference from GUI object named gui
#define gui_FPSHandle(NAME) gui_VAL(FPSHandle,NAME)

/// extract a CheckBoxHandle as reference from GUI object named gui
#define gui_CheckBox(NAME) gui_VAL(CheckBoxHandle,NAME)

/// extract a MultiDrawHandle as reference from GUI object named gui
#define gui_MultiDrawHandle(NAME) gui_VAL(MultiDrawHandle,NAME)

/// extract a TabHandle as reference from GUI object named gui
#define gui_TabHandle(NAME) gui_VAL(TabHandle,NAME)

/// extract a SplitterHandle as reference from GUI object named gui
#define gui_SplitterHandle(NAME) gui_VAL(SplitterHandle,NAME)

/// extract a SplitterHandle as reference from GUI object named gui
#define gui_StateHandle(NAME) gui_VAL(StateHandle,NAME)

#endif

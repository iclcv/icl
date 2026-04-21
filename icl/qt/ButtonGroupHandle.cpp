// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/ButtonGroupHandle.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/utils/Macros.h>

#include <QRadioButton>
namespace icl::qt {

  ButtonGroupHandle::ButtonGroupHandle(RadioButtonVec *buttons, GUIWidget *w)
    : GUIHandle<RadioButtonVec>(buttons, w),
      m_selectedIndex(std::make_shared<std::atomic<int>>(-1)) {
    if (!buttons) return;
    // Seed: find the initial selected index, if any.
    for (int i = 0; i < static_cast<int>(buttons->size()); ++i) {
      if ((*buttons)[i] && (*buttons)[i]->isChecked()) {
        m_selectedIndex->store(i, std::memory_order_relaxed);
        break;
      }
    }
    // Each radio button writes its own index into the cache when
    // toggled on (we don't care about toggled-off: the sibling that
    // was turned on will publish the new index).  The lambda captures
    // the shared_ptr by value so the cache outlives all handle copies;
    // the connection's context is the button, so it's dropped when
    // the button is destroyed.
    auto cache = m_selectedIndex;
    for (int i = 0; i < static_cast<int>(buttons->size()); ++i) {
      QRadioButton *b = (*buttons)[i];
      if (!b) continue;
      QObject::connect(b, &QRadioButton::toggled, b,
                       [cache, i](bool checked){
                         if (checked) cache->store(i, std::memory_order_relaxed);
                       });
    }
  }

  void ButtonGroupHandle::select(int id){
    ICLASSERT_RETURN(valid(id));
    vec()[id]->setChecked(true);
  }
  int ButtonGroupHandle::getSelected() const{
    return m_selectedIndex ? m_selectedIndex->load(std::memory_order_relaxed) : -1;
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

  void ButtonGroupHandle::disable(){
    for(int i=0;i<n();i++){
      vec()[i]->setEnabled(false);
    }
  }

  void ButtonGroupHandle::enable(){
    for(int i=0;i<n();i++){
      vec()[i]->setEnabled(true);
    }
  }

  void ButtonGroupHandle::enable(int index){
    ICLASSERT_RETURN(index >= 0 && index < static_cast<int>(vec().size()));
    vec()[index]->setEnabled(true);
  }
  void ButtonGroupHandle::disable(int index){
    ICLASSERT_RETURN(index >= 0 && index < static_cast<int>(vec().size()));
    vec()[index]->setEnabled(false);
  }


  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::ButtonGroupHandle;
  __attribute__((constructor))
  static void icl_register_button_group_handle_assignments() {
    AssignRegistry::enroll_provider<ButtonGroupHandle,
                                    bool, int, float, double, std::string>();
    AssignRegistry::enroll_identity<ButtonGroupHandle>();
  }
}
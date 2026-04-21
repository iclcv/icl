// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/ComboHandle.h>

#include <icl/utils/AssignRegistry.h>

#include <QComboBox>

namespace icl::qt {

  ComboHandle::ComboHandle(QComboBox *cb, GUIWidget *w)
    : GUIHandle<QComboBox>(cb, w),
      m_cache(std::make_shared<Cache>()) {
    if (cb) {
      m_cache->index = cb->currentIndex();
      m_cache->text  = cb->currentText().toLatin1().data();
    }
    if (!cb) return;
    auto cache = m_cache;
    QObject::connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     cb, [cache, cb](int idx){
                       // Runs on the GUI thread — safe to read the widget.
                       std::string t = cb->itemText(idx).toLatin1().data();
                       std::scoped_lock lock(cache->mutex);
                       cache->index = idx;
                       cache->text = std::move(t);
                     });
  }

  void ComboHandle::add(const std::string &item){
    cbx()->addItem(item.c_str());
  }
  void ComboHandle::remove(const std::string &item){
    int idx = getIndex(item);
    if(idx >= 0 && idx < getItemCount()){
      cbx()->removeItem(idx);
    }
  }
  void ComboHandle::remove(int idx){
    cbx()->removeItem(idx);
  }
  void ComboHandle::clear(){
    while(getItemCount()){
      remove(0);
    }
  }
  std::string ComboHandle::getItem(int idx) const{
    return cbx()->itemText(idx).toLatin1().data();
  }
  int ComboHandle::getIndex(const std::string &item) const{
    return cbx()->findText(item.c_str());
  }

  int ComboHandle::getSelectedIndex() const{
    if (!m_cache) return -1;
    std::scoped_lock lock(m_cache->mutex);
    return m_cache->index;
  }
  std::string ComboHandle::getSelectedItem() const{
    if (!m_cache) return {};
    std::scoped_lock lock(m_cache->mutex);
    return m_cache->text;
  }

  int ComboHandle::getItemCount() const{
    return cbx()->count();
  }

  void ComboHandle::setSelectedIndex(int idx){
    cbx()->setCurrentIndex(idx);
  }
  void ComboHandle::setSelectedItem(const std::string &item){
    int idx = getIndex(item);
    if(idx >=0 && idx< getItemCount()){
      cbx()->setCurrentIndex(idx);
    }
  }


  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::ComboHandle;
  __attribute__((constructor))
  static void icl_register_combo_handle_assignments() {
    AssignRegistry::enroll_symmetric<ComboHandle, int, float, double, std::string>();
    AssignRegistry::enroll_identity<ComboHandle>();
  }
}
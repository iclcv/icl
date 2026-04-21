// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

namespace icl::qt {
/// Default dark-mode stylesheet applied to all ICL Qt applications.
/// Disable with environment variable ICL_THEME=none.
/// Load a custom .qss file with ICL_THEME=/path/to/style.qss.
inline const char *defaultStyleSheet() {
return R"QSS(

/* Palette: bg #2b2b2b, surface #3c3c3c, border #555, text #e0e0e0, accent #3daee9 */

* {
  color: #e0e0e0;
  font-family: "Segoe UI", "SF Pro Text", "Helvetica Neue", sans-serif;
  font-size: 13px;
}

QMainWindow,
QDialog,
QTabWidget::pane,
QScrollArea {
  background-color: #2b2b2b;
}

QWidget:disabled {
  color: #808080;
}

/* --- QMainWindow --- */

QMainWindow {
  background-color: #2b2b2b;
}

QMainWindow::separator {
  background-color: #555555;
  width: 1px;
  height: 1px;
}

/* --- QMenuBar --- */

QMenuBar {
  background-color: #323232;
  border-bottom: 1px solid #555555;
  padding: 2px 0px;
}

QMenuBar::item {
  background: transparent;
  padding: 4px 10px;
  border-radius: 3px;
}

QMenuBar::item:selected {
  background-color: #3daee9;
  color: #ffffff;
}

QMenuBar::item:pressed {
  background-color: #2a8bc5;
}

/* --- QMenu --- */

QMenu {
  background-color: #323232;
  border: 1px solid #555555;
  border-radius: 4px;
  padding: 4px 0px;
}

QMenu::item {
  padding: 5px 30px 5px 20px;
}

QMenu::item:selected {
  background-color: #3daee9;
  color: #ffffff;
  border-radius: 2px;
}

QMenu::separator {
  height: 1px;
  background-color: #555555;
  margin: 4px 8px;
}

QMenu::indicator {
  width: 14px;
  height: 14px;
  margin-left: 4px;
}

/* --- QToolBar --- */

QToolBar {
  background-color: #323232;
  border-bottom: 1px solid #555555;
  spacing: 4px;
  padding: 2px 4px;
}

QToolBar::separator {
  width: 1px;
  background-color: #555555;
  margin: 2px 6px;
}

QToolButton {
  background-color: transparent;
  border: 1px solid transparent;
  border-radius: 4px;
  padding: 4px 8px;
  color: #e0e0e0;
}

QToolButton:hover {
  background-color: #454545;
  border-color: #555555;
}

QToolButton:pressed {
  background-color: #2a8bc5;
  border-color: #3daee9;
}

/* --- QStatusBar --- */

QStatusBar {
  background-color: #323232;
  border-top: 1px solid #555555;
  color: #b0b0b0;
  padding: 2px;
}

QStatusBar::item {
  border: none;
}

/* --- QPushButton --- */

QPushButton {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  border-radius: 5px;
  padding: 5px 16px;
  min-width: 60px;
  color: #e0e0e0;
}

QPushButton:hover {
  background-color: #454545;
  border-color: #3daee9;
}

QPushButton:pressed {
  background-color: #2a8bc5;
  border-color: #3daee9;
  color: #ffffff;
}

QPushButton:checked {
  background-color: #3daee9;
  border-color: #3daee9;
  color: #ffffff;
}

QPushButton:checked:hover {
  background-color: #5bbef5;
}

QPushButton:disabled {
  background-color: #353535;
  border-color: #484848;
  color: #808080;
}

QPushButton:flat {
  background-color: transparent;
  border: none;
}

QPushButton:flat:hover {
  background-color: #454545;
}

/* --- QRadioButton & QCheckBox --- */

QRadioButton,
QCheckBox {
  spacing: 6px;
  padding: 2px;
}

QRadioButton::indicator,
QCheckBox::indicator {
  width: 16px;
  height: 16px;
  border: 2px solid #555555;
  background-color: #3c3c3c;
}

QRadioButton::indicator {
  border-radius: 10px;
}

QCheckBox::indicator {
  border-radius: 3px;
}

QRadioButton::indicator:hover,
QCheckBox::indicator:hover {
  border-color: #3daee9;
}

QRadioButton::indicator:checked {
  background-color: #3daee9;
  border-color: #3daee9;
}

QCheckBox::indicator:checked {
  background-color: #3daee9;
  border-color: #3daee9;
}

QCheckBox::indicator:indeterminate {
  background-color: #3daee9;
  border-color: #3daee9;
}

/* --- QGroupBox --- */

QGroupBox {
  border: 1px solid #555555;
  border-radius: 4px;
  margin-top: 10px;
  padding: 4px 2px 2px 2px;
  font-weight: bold;
  color: #e0e0e0;
}

QGroupBox::title {
  subcontrol-origin: margin;
  subcontrol-position: top left;
  left: 8px;
  padding: 0 4px;
  color: #e0e0e0;
}

/* --- QLineEdit --- */

QLineEdit {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  border-radius: 4px;
  padding: 4px 8px;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
}

QLineEdit:focus {
  border-color: #3daee9;
}

QLineEdit:hover {
  border-color: #6c6c6c;
}

QLineEdit:disabled {
  background-color: #353535;
  color: #808080;
}

/* --- QTextEdit --- */

QTextEdit {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  border-radius: 4px;
  padding: 4px;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
}

QTextEdit:focus {
  border-color: #3daee9;
}

/* --- QComboBox --- */

QComboBox {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  border-radius: 4px;
  padding: 4px 8px;
  min-width: 80px;
}

QComboBox:hover {
  border-color: #3daee9;
}

QComboBox:focus {
  border-color: #3daee9;
}

QComboBox::drop-down {
  subcontrol-origin: padding;
  subcontrol-position: top right;
  width: 24px;
  border-left: 1px solid #555555;
  border-top-right-radius: 4px;
  border-bottom-right-radius: 4px;
  background-color: #454545;
}

QComboBox::down-arrow {
  width: 0px;
  height: 0px;
  border-left: 5px solid transparent;
  border-right: 5px solid transparent;
  border-top: 5px solid #e0e0e0;
}

QComboBox QAbstractItemView {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
  outline: 0;
}

/* --- QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit --- */

QSpinBox,
QDoubleSpinBox,
QDateEdit,
QTimeEdit {
  background-color: #3c3c3c;
  border: 1px solid #555555;
  border-radius: 4px;
  padding: 4px 8px;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
}

QSpinBox:focus,
QDoubleSpinBox:focus,
QDateEdit:focus,
QTimeEdit:focus {
  border-color: #3daee9;
}

QSpinBox::up-button,
QDoubleSpinBox::up-button,
QDateEdit::up-button,
QTimeEdit::up-button {
  subcontrol-origin: border;
  subcontrol-position: top right;
  width: 20px;
  border-left: 1px solid #555555;
  border-bottom: 1px solid #555555;
  border-top-right-radius: 4px;
  background-color: #454545;
}

QSpinBox::down-button,
QDoubleSpinBox::down-button,
QDateEdit::down-button,
QTimeEdit::down-button {
  subcontrol-origin: border;
  subcontrol-position: bottom right;
  width: 20px;
  border-left: 1px solid #555555;
  border-bottom-right-radius: 4px;
  background-color: #454545;
}

QSpinBox::up-button:hover,
QDoubleSpinBox::up-button:hover,
QDateEdit::up-button:hover,
QTimeEdit::up-button:hover,
QSpinBox::down-button:hover,
QDoubleSpinBox::down-button:hover,
QDateEdit::down-button:hover,
QTimeEdit::down-button:hover {
  background-color: #3daee9;
}

QSpinBox::up-arrow,
QDoubleSpinBox::up-arrow,
QDateEdit::up-arrow,
QTimeEdit::up-arrow {
  width: 0px;
  height: 0px;
  border-left: 4px solid transparent;
  border-right: 4px solid transparent;
  border-bottom: 5px solid #e0e0e0;
}

QSpinBox::down-arrow,
QDoubleSpinBox::down-arrow,
QDateEdit::down-arrow,
QTimeEdit::down-arrow {
  width: 0px;
  height: 0px;
  border-left: 4px solid transparent;
  border-right: 4px solid transparent;
  border-top: 5px solid #e0e0e0;
}

/* --- QSlider horizontal --- */

QSlider::groove:horizontal {
  border: 1px solid #555555;
  height: 6px;
  background-color: #3c3c3c;
  border-radius: 3px;
}

QSlider::handle:horizontal {
  background-color: #3daee9;
  border: 1px solid #2a8bc5;
  width: 16px;
  height: 16px;
  margin: -6px 0;
  border-radius: 8px;
}

QSlider::handle:horizontal:hover {
  background-color: #5bbef5;
}

QSlider::handle:horizontal:pressed {
  background-color: #2a8bc5;
}

QSlider::sub-page:horizontal {
  background-color: #3daee9;
  border-radius: 3px;
}

QSlider::add-page:horizontal {
  background-color: #3c3c3c;
  border-radius: 3px;
}

/* --- QSlider vertical --- */

QSlider::groove:vertical {
  border: 1px solid #555555;
  width: 6px;
  background-color: #3c3c3c;
  border-radius: 3px;
}

QSlider::handle:vertical {
  background-color: #3daee9;
  border: 1px solid #2a8bc5;
  width: 16px;
  height: 16px;
  margin: 0 -6px;
  border-radius: 8px;
}

QSlider::handle:vertical:hover {
  background-color: #5bbef5;
}

QSlider::handle:vertical:pressed {
  background-color: #2a8bc5;
}

QSlider::sub-page:vertical {
  background-color: #3c3c3c;
  border-radius: 3px;
}

QSlider::add-page:vertical {
  background-color: #3daee9;
  border-radius: 3px;
}

/* --- QProgressBar --- */

QProgressBar {
  border: 1px solid #555555;
  border-radius: 5px;
  background-color: #3c3c3c;
  text-align: center;
  color: #e0e0e0;
  height: 18px;
}

QProgressBar::chunk {
  background-color: #3daee9;
  border-radius: 4px;
}

/* --- QDial --- */

QDial {
  background-color: #2b2b2b;
}

/* --- QTabWidget & QTabBar --- */

QTabWidget::pane {
  border: 1px solid #555555;
  border-radius: 4px;
  top: -1px;
  background-color: #2b2b2b;
}

QTabBar::tab {
  background-color: #353535;
  border: 1px solid #555555;
  border-bottom: none;
  border-top-left-radius: 5px;
  border-top-right-radius: 5px;
  padding: 6px 16px;
  margin-right: 2px;
  color: #b0b0b0;
}

QTabBar::tab:selected {
  background-color: #2b2b2b;
  color: #3daee9;
  border-bottom: 2px solid #3daee9;
}

QTabBar::tab:hover:!selected {
  background-color: #3c3c3c;
  color: #e0e0e0;
}

/* --- QTableWidget / QTableView --- */

QTableWidget,
QTableView {
  background-color: #2b2b2b;
  alternate-background-color: #333333;
  border: 1px solid #555555;
  border-radius: 4px;
  gridline-color: #484848;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
}

QTableWidget::item,
QTableView::item {
  padding: 4px;
}

QTableWidget::item:hover,
QTableView::item:hover {
  background-color: #3c3c3c;
}

QHeaderView::section {
  background-color: #353535;
  border: 1px solid #555555;
  padding: 4px 8px;
  color: #e0e0e0;
  font-weight: bold;
}

QHeaderView::section:hover {
  background-color: #3c3c3c;
}

/* --- QTreeWidget / QTreeView --- */

QTreeWidget,
QTreeView {
  background-color: #2b2b2b;
  alternate-background-color: #333333;
  border: 1px solid #555555;
  border-radius: 4px;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
  outline: 0;
}

QTreeWidget::item,
QTreeView::item {
  padding: 3px;
  border: none;
}

QTreeWidget::item:hover,
QTreeView::item:hover {
  background-color: #3c3c3c;
}

QTreeWidget::branch {
  background-color: transparent;
}

/* --- QListWidget / QListView --- */

QListWidget,
QListView {
  background-color: #2b2b2b;
  alternate-background-color: #333333;
  border: 1px solid #555555;
  border-radius: 4px;
  selection-background-color: #3daee9;
  selection-color: #ffffff;
  outline: 0;
}

QListWidget::item,
QListView::item {
  padding: 4px;
}

QListWidget::item:hover,
QListView::item:hover {
  background-color: #3c3c3c;
}

/* --- QScrollBar vertical --- */

QScrollBar:vertical {
  background-color: #2b2b2b;
  width: 12px;
  margin: 0;
  border-radius: 6px;
}

QScrollBar::handle:vertical {
  background-color: #555555;
  min-height: 30px;
  border-radius: 5px;
  margin: 2px;
}

QScrollBar::handle:vertical:hover {
  background-color: #6c6c6c;
}

QScrollBar::handle:vertical:pressed {
  background-color: #3daee9;
}

QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
  height: 0px;
}

QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
  background: none;
}

/* --- QScrollBar horizontal --- */

QScrollBar:horizontal {
  background-color: #2b2b2b;
  height: 12px;
  margin: 0;
  border-radius: 6px;
}

QScrollBar::handle:horizontal {
  background-color: #555555;
  min-width: 30px;
  border-radius: 5px;
  margin: 2px;
}

QScrollBar::handle:horizontal:hover {
  background-color: #6c6c6c;
}

QScrollBar::handle:horizontal:pressed {
  background-color: #3daee9;
}

QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
  width: 0px;
}

QScrollBar::add-page:horizontal,
QScrollBar::sub-page:horizontal {
  background: none;
}

/* --- QScrollArea --- */

QScrollArea {
  border: none;
  background-color: #2b2b2b;
}

/* --- QLabel --- */

QLabel {
  background-color: transparent;
  color: #e0e0e0;
}

/* --- QDialog --- */

QDialog {
  background-color: #2b2b2b;
}

/* --- QToolTip --- */

QToolTip {
  background-color: #3c3c3c;
  color: #e0e0e0;
  border: 1px solid #555555;
  border-radius: 3px;
  padding: 4px;
}

/* --- Focus --- */

QWidget:focus {
  outline: none;
}

)QSS";
}

} // namespace icl::qt
#ifndef ICLBORDERBOX_H
#define ICLBORDERBOX_H

#include <QGroupBox>
#include <QVBoxLayout>
/// The icl namespace
namespace icl{
  /// A simple utility class (QWidget with titled border) \ingroup UNCOMMON
  struct BorderBox : public QGroupBox{

    /// Create a new BorderBox Object
    /** @param label title of the border
        @param content child widget
        @param parent optional parent widget*/
    BorderBox(const QString &label, QWidget *content, QWidget *parent) : 
      QGroupBox(label,parent), m_poContent(content){
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(3);
      m_poLayout->addWidget(content);
      setLayout(m_poLayout);
    }
    
    /// returns the content of this widget (the child)
    QWidget *content() { return m_poContent; }
    
    private:
    QVBoxLayout *m_poLayout; //<! layout
    QWidget *m_poContent;    //<! content widget
  };
}


#endif

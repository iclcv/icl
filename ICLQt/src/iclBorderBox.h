#ifndef ICLBORDERBOX_H
#define ICLBORDERBOX_H

#include <QGroupBox>
#include <QVBoxLayout>

namespace icl{
  struct BorderBox : public QGroupBox{
    BorderBox(const QString &label, QWidget *content, QWidget *parent) : 
      QGroupBox(label,parent), m_poContent(content){
      m_poLayout = new QVBoxLayout;
      m_poLayout->setMargin(3);
      m_poLayout->addWidget(content);
      setLayout(m_poLayout);
    }
    
    QWidget *content() { return m_poContent; }
    private:
    QVBoxLayout *m_poLayout;
    QWidget *m_poContent;
  };
}


#endif

#ifndef ICL_CONTAINTER_HANDLE_H
#define ICL_CONTAINTER_HANDLE_H

#include <ICLUtils/Macros.h>
#include <QWidget>
#include <QString>

namespace icl{
  /// Base class for Container like GUI handles as Box- or Tab widgets
  class ContainerHandle{
    protected:
    /// must be subclassed
    ContainerHandle(){}

    public:
    /// pure virtual interface
    virtual void add(QWidget *component, const QString &name=""){
      ERROR_LOG("unable to add components to this widget (name was: " << name.toLatin1().data()  << ")");
    }

    /// pure virtual interface
    virtual void insert(int idx, QWidget *component, const QString &name=""){
      ERROR_LOG("unable to insert components into this widget (name was: " << name.toLatin1().data() << ", id was: " << idx << ")");
    }

  };
}
#endif

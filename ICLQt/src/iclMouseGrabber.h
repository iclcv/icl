#ifndef MOUSE_INTERACTION_RECEIVER_H
#define MOUSE_INTERACTION_RECEIVER_H

#include <QObject>
#include <iclMouseInteractionInfo.h>


namespace icl{
  /// Abstract class for receiving MouseInteractionEvents \ingroup COMMON
  /** <b>TODO</b>: Place an example here! 
      
  */
  class MouseInteractionReceiver : public QObject{
    Q_OBJECT
    public slots:

    /// when called, processMouseInteraction is called
    void mouseInteraction(MouseInteractionInfo *info);

    public:
    /// this function is called if by the mouseInteraction slot 
    /** It can be reimplemented for an individual mouse event handling 
    */
    virtual void processMouseInteraction(MouseInteractionInfo *info);
  };

}

#endif

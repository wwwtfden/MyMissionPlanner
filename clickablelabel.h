#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H
#include <QMouseEvent>
#include <QtWidgets>

class ClickableLabel : public QLabel
{
    Q_OBJECT
signals:
    void clicked();
public:
    ClickableLabel();
   void mousePressEvent(QMouseEvent* event);
   using QLabel::QLabel;
};

#endif // CLICKABLELABEL_H

#include "mousewheelwidgetadjustmentguard.h"

#include <QEvent>
#include <QWidget>

MouseWheelWidgetAdjustmentGuard::MouseWheelWidgetAdjustmentGuard(QObject *parent) : QObject(parent)
{
}

bool MouseWheelWidgetAdjustmentGuard::eventFilter(QObject *o, QEvent *e)
{
    const QWidget* widget = static_cast<QWidget*>(o);
    if (e->type() == QEvent::Wheel && widget && !widget->hasFocus())
    {
        e->ignore();
        return 1;
    }
    return QObject::eventFilter(o, e);
}

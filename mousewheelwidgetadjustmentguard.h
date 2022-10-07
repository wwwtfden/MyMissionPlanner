#ifndef MOUSEWHEELWIDGETADJUSTMENTGUARD_H
#define MOUSEWHEELWIDGETADJUSTMENTGUARD_H
#include <QObject>


class MouseWheelWidgetAdjustmentGuard : public QObject
{
public:
    explicit MouseWheelWidgetAdjustmentGuard(QObject *parent);
protected:
    bool eventFilter(QObject* o, QEvent* e) override;
};

#endif // MOUSEWHEELWIDGETADJUSTMENTGUARD_H

#include "waypointwidget.h"
#include "clickablelabel.h"
#include "mousewheelwidgetadjustmentguard.h"

#include <QLabel>
#include <QDebug>
#include <QtMath>
#include <QGridLayout>


WaypointWidget::WaypointWidget(QWidget *parent)
    : QFrame(parent)
{

    // layout для названий
    QGridLayout *labelsGridLayout = new QGridLayout();

    labelIndex = new QLabel();
    labelIndex->setText("<b>Точка № ");

    labelLatitude = new QLabel();
    labelLatitude->setText("Широта: ");

    labelLongitude = new QLabel();
    labelLongitude->setText("Долгота: ");

    labelAltitude = new QLabel();
    labelAltitude->setText("Высота: ");

    labelSwitchBoxPanel = new QLabel();
    labelSwitchBoxPanel->setText("Команда: ");

    labelDelete = new ClickableLabel();
    connect(labelDelete, SIGNAL(clicked()), this, SIGNAL(labelDeleteClicked()));
    labelDelete->setText("<b><FONT COLOR='#0000CD'>Удалить");
    labelDelete->setCursor(Qt::PointingHandCursor);
  //  labelDelete->installEventFilter(this);

    labelsGridLayout->addWidget(labelIndex);
    labelsGridLayout->addWidget(labelLatitude);
    labelsGridLayout->addWidget(labelLongitude);
    labelsGridLayout->addWidget(labelAltitude);
    labelsGridLayout->addWidget(labelSwitchBoxPanel);
    //labelsGridLayout->addWidget(labelDelete);

    // layout для значений
    valuesGridLayout = new QGridLayout();

    labelIndexValue = new QLabel();
    lineLatitudeValue = new QLineEdit();
    lineLongitudeValue = new QLineEdit();
    lineAltitudeValue = new QLineEdit();
    switchBoxPanel = new QComboBox();
    switchBoxPanel->setEditable(0);
    switchBoxPanel->addItems(cmdTypePanel);
    switchBoxPanel->setFocusPolicy(Qt::StrongFocus);
    switchBoxPanel->installEventFilter(new MouseWheelWidgetAdjustmentGuard(switchBoxPanel));

    valuesGridLayout->addWidget(labelIndexValue);
    valuesGridLayout->addWidget(lineLatitudeValue);
    valuesGridLayout->addWidget(lineLongitudeValue);
    valuesGridLayout->addWidget(lineAltitudeValue);
    valuesGridLayout->addWidget(switchBoxPanel);

    QGridLayout *gridLayoutForDelete = new QGridLayout();
    gridLayoutForDelete->addWidget(labelDelete);

    // общий layout
    generalGridLayout = new QGridLayout();
    generalGridLayout->addLayout(labelsGridLayout, 0, 0);
    generalGridLayout->addLayout(valuesGridLayout, 0, 1);
    generalGridLayout->addLayout(gridLayoutForDelete, 1, 0);

    this->setLayout(generalGridLayout);

    this->setMaximumHeight(170);
    this->setMaximumWidth(225);

    connect(lineLatitudeValue, SIGNAL(editingFinished()), this, SIGNAL(lineEditChanged()));
    connect(lineLongitudeValue, SIGNAL(editingFinished()), this, SIGNAL(lineEditChanged()));
    connect(lineAltitudeValue, SIGNAL(editingFinished()), this, SIGNAL(lineEditChanged()));
    connect(switchBoxPanel, SIGNAL(currentIndexChanged(int)), this, SIGNAL(switchBoxPanelChanged(int)));
}

WaypointWidget::~WaypointWidget()
{

    delete labelIndex;
    delete labelLatitude;
    delete labelLongitude;
    delete labelAltitude;
    delete labelDelete;
    delete labelSwitchBoxPanel;
    delete valuesGridLayout;
    delete labelIndexValue;
    delete generalGridLayout;
    delete lineLatitudeValue;
    delete lineLongitudeValue;
    delete lineAltitudeValue;
    delete switchBoxPanel;

};

void WaypointWidget::setWaypointWidgetData(int index, double lat, double lon, float alt, int pointtype)
{

    qlat = lat;
    qindex = index;
    qlon = lon;
    qalt = alt;
    qindex = index;
    qpointtype = pointtype;
    labelIndexValue->setText(QString::number(qindex));
    lineLatitudeValue->setText(QString::number(qlat));
    lineLongitudeValue->setText(QString::number(qlon));
    lineAltitudeValue->setText(QString::number(qalt));
    switchBoxPanel->setCurrentText(setPointtypeForWgt(qpointtype));

}

void WaypointWidget::resetFocus()
{
    switchBoxPanel->setFocusPolicy(Qt::StrongFocus);
}



QString WaypointWidget::getLatFromWgt()
{
    return lineLatitudeValue->text();
}

QString WaypointWidget::getLonFromWgt()
{
    return lineLongitudeValue->text();
}

QString WaypointWidget::getAltFromWgt()
{
    return lineAltitudeValue->text();
}

QString WaypointWidget::getPointtypeFromWgt()
{
    return switchBoxPanel->currentText();

//    switch (pointtype) {
//    case 16: return "WAYPOINT";
//    case 21: return "LAND";
//    case 208: return "DO PARACHUTE";
//    case 178: return "DO CHANGE SPEED";
//    default: return "OTHER";
        //    }
}

QString WaypointWidget::setPointtypeForWgt(int pointtype)
{
        switch (pointtype) {
        case 16: return "WAYPOINT";
        case 21: return "LAND";
        case 208: return "DO PARACHUTE";
        case 178: return "DO CHANGE SPEED";
        default: return "OTHER";
                }
}

//void WaypointWidget::getDataFromWpWidget(){

//}


#ifndef WAYPOINTWIDGET_H
#define WAYPOINTWIDGET_H

#include <QWidget>
#include <QtWidgets>

class WaypointWidget : public QFrame { // был QWidget

    Q_OBJECT

public:
    WaypointWidget(QWidget *parent);
    ~WaypointWidget();

    void setWaypointWidgetData(int index, double lat, double lon, float alt, int pointtype);
    void resetFocus();


    QLabel *labelIndex;
    QLabel *labelLatitude;
    QLabel *labelLongitude;
    QLabel *labelAltitude;
    QLabel *labelSwitchBoxPanel;
    QLabel *labelDelete;

    QString getLatFromWgt();
    QString getLonFromWgt();
    QString getAltFromWgt();
    QString getPointtypeFromWgt();
    QString setPointtypeForWgt(int pointtype);

private:

    int qindex; // порядковый номер waypoint'а
    QStringList cmdTypePanel = {"WAYPOINT", "DO CHANGE SPEED", "DO PARACHUTE", "LAND", "OTHER"};
    double qlat; // широта
    double qlon; // долгота
    float qalt; // высота
    int qpointtype; // тип команды

    QGridLayout *valuesGridLayout;
    QLabel *labelIndexValue;
    QLineEdit *lineLatitudeValue;
    QLineEdit *lineLongitudeValue;
    QLineEdit *lineAltitudeValue;
    QComboBox *switchBoxPanel;
    QGridLayout *generalGridLayout;

public slots:
   // void getDataFromWpWidget();

signals:
    void labelDeleteClicked();
    void lineEditChanged();
    void switchBoxPanelChanged(int);
//    void textChanged();

};

#endif // WAYPOINTWIDGET_H

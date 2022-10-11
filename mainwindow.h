#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "waypoint.h"
#include "waypointwidget.h"
#include <QList>

#include <QButtonGroup>
#include <QComboBox>
//#include "routemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void quit();
    void getwpdata(QFile &file); // открытие файла и перенос данных в оперативную память
    void displaywpdata(bool panel); // обновление информации в интерфейсе
    void displaywpdataWithoutUpdPanel();

    void savewpdata(QFile &file); // запись данных в файл
    QString cmdName(int pointtype);
    int cmdNum (QString cmdName);
    void setToolTipsForTable();
    void updateTable();
    void clearPanel();
    void updPanel();
    void calcDists();
    void calcHeights();
    QVBoxLayout *vlayout;


    QTableWidgetItem* wti = 0;

    int minRange, maxRange;
    bool op = false; // флаг открытия таблицы
    double def_lat = 56.590724; //Координаты церкви в Хребтово
    double def_lon = 38.274173;
    int curr_row;

    // combo box
    //QComboBox *switchBox;

    QStringList headerLabels;


private:
    Ui::MainWindow *ui;
    QString m_strFileName;

    QList<Waypoint*> waypointsList; // делаем QList для массива объектов класса Waypoint
    QList<QPushButton*> buttonList; // делаем QList для массива кнопок, которые будут вставляться в таблицу
    QList<QPushButton*> buttonUPList;
    QList<QPushButton*> buttonDOWNList;
    QList<QComboBox*> cmdList; // делаем QList для выпадающего списка
    QStringList cmdType = {"WAYPOINT", "DO CHANGE SPEED", "DO PARACHUTE", "LAND", "OTHER"};

    QList<WaypointWidget*> panelWidgetList;

    QFile *file;
    QLabel *statusLayerLbl;

 //   QWidget* createButtonWidget() const;


private slots:
    //void on_actionOpen_triggered();
    void deleteSelectedRow();
    void on_tableWidget_cellChanged(int row, int column);

    void slotOpenFileName();
    void slotSaveFile();
    void slotSaveAs();
    void slotCloseAll();
    void slotSelectSpif();
    void centerCoord(); // центрирование карты

    //void on_tableWidget_cellDoubleClicked(int row, int column);



public slots:
  //  void clickDelButton();
    void slotZoom();
    void valueChanged(int &layer);
    void avalueChanged(int layer);
    void userClicked(int row, int column);
    void setSlider(int layer);
    void updatePointTypeForWaypointListRow(int index);
    void addWp();
    void addWpWithData(double lat, double lon);
    void addWpWithIndex(double lat, double lon, int index);
    void tabClicked();
    void clearLayout();
    void deleteRowFromTable();
    void resetIndex();
    void deleteWp(int index);
    void updateDataFromPanel();
    void moveWp(double lat, double lon, int index);
    void wpMoved();

    void upButton();
    void downButton();
};
#endif // MAINWINDOW_H

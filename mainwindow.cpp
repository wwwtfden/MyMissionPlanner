#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QObject>
#include <QtWidgets>
#include "waypoint.h"
#include "cachepainter.h"
#include "flightmapwidget.h"
//#include "routemanager.h"
#include "waypointwidget.h"
#include "clickablelabel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->file = nullptr;
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(8,70); // подстраиваем параметры таблицы
    ui->tableWidget->setColumnWidth(10,50);
    ui->tableWidget->setColumnWidth(11,50);
    ui->tableWidget->setColumnWidth(12,60);
    ui->tableWidget->setColumnWidth(13,60);
    ui->ShowMapWidget->getLayersRange(minRange, maxRange);
    ui->zoomSlider->setRange(minRange,maxRange);
    ui->zoomSlider->setValue(ui->ShowMapWidget->getCurrentLayer());
    headerLabels << "Команда" << "Param1" << "Param 2" << "Param 3" << "Param 4" << "Широта" << "Долгота" << "Высота" << "Удалить" << "Дистанция" << "Вверх" << "Вниз" << "Углы" << "Град, %";
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);


    MainWindow::setWindowTitle("Mission Planner 02");

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::slotOpenFileName); //подключаем возможность открытие wp-файла в меню
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit); // подключение прочих функций
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::slotSaveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::slotSaveAs);
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::slotCloseAll);
    connect(ui->action_spif, &QAction::triggered, this, &MainWindow::slotSelectSpif);
    connect(ui->centerForWp, &QPushButton::clicked, this, &MainWindow::centerCoord);
    connect(ui->zoomOutButton, &QPushButton::clicked, this, &MainWindow::slotZoom);
    connect(ui->zoomInButton, &QPushButton::clicked, this, &MainWindow::slotZoom);
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(avalueChanged(int)));
    connect(ui->updButton, &QPushButton::clicked, this, &MainWindow::updateTable);
    connect(ui->ShowMapWidget, &FlightMapWidget::layerWasChanged, this, &MainWindow::setSlider);
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addWp);
    connect(ui->ShowMapWidget, &FlightMapWidget::setLabelStatusBarText, ui->label, &QLabel::setText);
    connect(ui->ShowMapWidget, &FlightMapWidget::wpDeleted, this, &MainWindow::deleteWp);
    connect(ui->ShowMapWidget, &FlightMapWidget::wpAdded, this, &MainWindow::addWpWithData);
    connect(ui->ShowMapWidget, &FlightMapWidget::wpWithIndexAdded, this, &MainWindow::addWpWithIndex);
 //   this->ui->actionSave->setDisabled(true); // деактивировать кнопку "Сохранить"
 //   this->ui->actionSave_As->setDisabled(true);
    connect(ui->ShowMapWidget, &FlightMapWidget::pointWasMoved, this, &MainWindow::moveWp);
    connect(ui->ShowMapWidget, &FlightMapWidget::pointMoveFinished, this, &MainWindow::wpMoved);

    // создание элементов боковой панели
    vlayout = new QVBoxLayout();
    ui->scrollAreaWidgetContents->setLayout(vlayout);
    ui->scrollAreaWidgetContents->setEnabled(1);

    qDebug() << file;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::slotOpenFileName() // слот по открытию файла
{
   // ui->scrollAreaWidgetContents->setEnabled(1);
    QString str = QFileDialog::getOpenFileName(0, "Открыть файл", "", "*.waypoints");
    this->file = new QFile(str);
    if (file->open(QIODevice::ReadOnly)) {
       m_strFileName = str;
       ui->label->setText("File opened: " + m_strFileName);
       MainWindow::setWindowTitle("Mission Planner 02 " + m_strFileName);
   //  waypointsList.clear(); // предварительная очистка
       getwpdata(*file);
       file->close();
       displaywpdata(1);
       ui->ShowMapWidget->clearWaypointsRoute();
       ui->ShowMapWidget->setWaypointsRoute(this->waypointsList); // прорисовка маршрута на карту
       ui->ShowMapWidget->update();
       ui->ShowMapWidget->setCenterOfCachePainter(waypointsList.at(0)->show_lat(),waypointsList.at(0)->show_lon()); // установка указателя на нулевой wp
   }

    this->ui->actionSave->setEnabled(true); // включаем возможность использовать файловые команды
    this->ui->actionSave_As->setEnabled(true);
    this->ui->actionClose->setEnabled(true);

}

void MainWindow::valueChanged (int &layer){ // сигнал от зум слайдера
    layer = ui->ShowMapWidget->getCurrentLayer();
}

void MainWindow::avalueChanged(int layer) // слот изменяющий значения при сигнале с зум слайдера
{
    ui->ShowMapWidget->setLayer(layer);
    emit ui->ShowMapWidget->setLabelStatusBarText(QString("Широта:  %1°   Долгота:  %2°   Текущий слой:  %3").arg(ui->ShowMapWidget->getBCursorPosition(), 0, 'f', 6).arg(ui->ShowMapWidget->getLCursorPosition(), 0, 'f', 6).arg(layer));
}

void MainWindow::userClicked(int row, int column) // Определяет нажатие на ячейку таблицы
{
    qDebug() << row;
    qDebug() << "Тип точки";
    if (waypointsList.at(row)->show_pointtype() == 16){ // здесь в зависимости от выбранной команды в боксе и соответствия ей ряда устанавливаются хедеры таблицы
        qDebug() << "Waypoint";
        headerLabels.replace(1, "Delay");
        headerLabels.replace(2, "");
        headerLabels.replace(3, "");
        headerLabels.replace(4, "");
        ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    }
    if (waypointsList.at(row)->show_pointtype() == 208){
        qDebug() << "Do Parachute";
        headerLabels.replace(1, "Enable");
        headerLabels.replace(2, "");
        headerLabels.replace(3, "");
        headerLabels.replace(4, "");
        ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    }
    if (waypointsList.at(row)->show_pointtype() == 178){
        qDebug() << "Do Change Speed";
        headerLabels.replace(1, "");
        headerLabels.replace(2, "Airspeed");
        headerLabels.replace(3, "");
        headerLabels.replace(4, "");
        ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
}
    if (waypointsList.at(row)->show_pointtype() == 21){
        qDebug() << "Land";
        headerLabels.replace(1, "");
        headerLabels.replace(2, "");
        headerLabels.replace(3, "");
        headerLabels.replace(4, "");
        ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    }

}

void MainWindow::setSlider(int layer)
{
    ui->zoomSlider->setValue(layer);
}

void MainWindow::updatePointTypeForWaypointListRow(int index)
{
    int rowId = this->cmdList.indexOf((QComboBox*)QObject::sender());
    qDebug() << "MainWindow::updatePointTypeForWaypointListRow(int index)" << index;
    this->waypointsList.at(rowId)->write_pointtype( cmdNum( this->cmdList[rowId]->itemText(index) ) );
    qDebug()<< "waypointId: " << this->waypointsList.at(rowId)->show_num();
    qDebug() << rowId << ": " << waypointsList.at(rowId)->show_pointtype() << this->cmdList[rowId]->itemText(index);
    displaywpdata(1);
}


void MainWindow::addWp()
{
    Waypoint* waypoint = new Waypoint();
    waypoint->write_num(waypointsList.count());
    waypoint->write_lat(ui->ShowMapWidget->stock_lat);
    waypoint->write_lon(ui->ShowMapWidget->stock_lon);
    if (!waypointsList.isEmpty()) {
        float tmpalt = waypointsList.at(waypointsList.count()-1)->show_alt();
        waypoint->write_alt(tmpalt);
    }
    this->waypointsList.insert(waypointsList.count(), waypoint);
    displaywpdata(1);
    // нужно высвободить память
    delete waypoint;
}

void MainWindow::addWpWithData(double lat, double lon)
{
    qDebug() << "Received" << lat << lon;
    Waypoint* waypoint = new Waypoint();
    waypoint->write_num(waypointsList.count());
    qDebug() << "Written index to WP: " << waypointsList.count();
    waypoint->write_lat(lat);
    waypoint->write_lon(lon);
    if (!waypointsList.isEmpty()) {
        float tmpalt = waypointsList.at(waypointsList.count()-1)->show_alt();
        waypoint->write_alt(tmpalt);
    }
    this->waypointsList.append(waypoint);
   // this->waypointsList.insert(waypointsList.count(), waypoint);
  //  resetIndex();
    displaywpdata(1);
    // нужно высвободить память
    delete waypoint;
}

void MainWindow::addWpWithIndex(double lat, double lon, int index)
{
    qDebug() << "Received" << lat << lon << index;
    Waypoint* waypoint = new Waypoint();
    waypoint->write_num(index);
    waypoint->write_lat(lat);
    waypoint->write_lon(lon);
    if (index > 0) {
        float tmpalt = waypointsList.at(index-1)->show_alt();
        waypoint->write_alt(tmpalt);
    }
 //   this->waypointsList.append(waypoint);
    this->waypointsList.insert(index, waypoint);
    displaywpdata(1);
    // нужно высвободить память
    delete waypoint;
}

void MainWindow::tabClicked()
{
    curr_row = waypointsList.count();
    qDebug() << "Activ";
}

void MainWindow::clearLayout()
{
    double plat1 = ui->ShowMapWidget->stock_lat;
    double plon1 = ui->ShowMapWidget->stock_lon;
    double plat2 = waypointsList.at(0)->show_lat();
    double plon2 = waypointsList.at(0)->show_lon();
    qDebug() << ui->ShowMapWidget->dist_btw_points(plat1, plon1, plat2, plon2);
}

void MainWindow::deleteRowFromTable()
{
    qDebug() << "deleteRowFromTable";
    int rowId = panelWidgetList.indexOf((WaypointWidget*)QObject::sender()); // в rowId кидаем номер строки сендера, пославшего сигнал
    qDebug() << rowId;

    ui->tableWidget->removeRow(rowId);

    // удалить кнопку из списка для удаленной строки
    buttonList.removeAt(rowId);
    buttonUPList.removeAt(rowId);
    buttonDOWNList.removeAt(rowId);
    vlayout->removeWidget(panelWidgetList[rowId]);

    // удалить информацию о waypoint из QList
    delete panelWidgetList[rowId];
    waypointsList.removeAt(rowId);

    panelWidgetList.removeAt(rowId);
    displaywpdata(1);
    ui->ShowMapWidget->update();
}

void MainWindow::resetIndex()
{
    for (int i = 0; i<waypointsList.count(); i++){
        waypointsList.at(i)->write_num(i);
    }
}

void MainWindow::deleteWp(int index)
{
    ui->tableWidget->removeRow(index);

    // удалить кнопку из списка для удаленной строки
    buttonList.removeAt(index);
    buttonUPList.removeAt(index);
    buttonDOWNList.removeAt(index);
    vlayout->removeWidget(panelWidgetList[index]);
    //cmdList.removeAt(index);

    // удалить информацию о waypoint из QList
 //   delete waypointsList[rowId];
    delete panelWidgetList[index];
    waypointsList.removeAt(index);

    panelWidgetList.removeAt(index);
    displaywpdata(1);
    ui->ShowMapWidget->update();
}

void MainWindow::updateDataFromPanel()
{
    qDebug() << panelWidgetList.indexOf((WaypointWidget*)QObject::sender());
    int rowId = panelWidgetList.indexOf((WaypointWidget*)QObject::sender()); // в rowId кидаем номер строки сендера, пославшего сигнал
    panelWidgetList.at(rowId)->resetFocus();
    double nlat = panelWidgetList.at(rowId)->getLatFromWgt().toDouble();
    double nlon = panelWidgetList.at(rowId)->getLonFromWgt().toDouble();
    float nalt = panelWidgetList.at(rowId)->getAltFromWgt().toFloat();
    int npointtype = cmdNum(panelWidgetList.at(rowId)->getPointtypeFromWgt());
    waypointsList.at(rowId)->write_lat(nlat);
    waypointsList.at(rowId)->write_lon(nlon);
    waypointsList.at(rowId)->write_alt(nalt);
    waypointsList.at(rowId)->write_pointtype(npointtype);
    displaywpdata(0);
    //displaywpdataWithoutUpdPanel();

}

void MainWindow::moveWp(double lat, double lon, int index)
{
    this->waypointsList.at(index)->write_lat(lat);
    this->waypointsList.at(index)->write_lon(lon);
    displaywpdata(0);
}

void MainWindow::wpMoved()
{
    displaywpdata(1);
}

void MainWindow::upButton()
{
    int rowId = buttonUPList.indexOf((QPushButton*)QObject::sender());
    if (rowId > 0){
    qDebug() << "upButton" << rowId <<  "clicked";
    waypointsList.swap(rowId-1, rowId);
    resetIndex();
    displaywpdata(1);
    }
}

void MainWindow::downButton()
{
    int rowId = buttonDOWNList.indexOf((QPushButton*)QObject::sender());
    if (rowId < waypointsList.count()-1){
    qDebug() << "downButton" << rowId <<  "clicked";
        waypointsList.swap(rowId, rowId+1);
        resetIndex();
        displaywpdata(1);
        }
}



void MainWindow::slotSaveFile()
{
    if (this->file == nullptr){
        slotSaveAs();
    }
    else {
          if (file->open(QIODevice::WriteOnly | QIODevice::Truncate)){
              savewpdata(*file);
              ui->label->setText("File saved.");
                file->close();
    }
          else slotSaveAs();

}

}

void MainWindow::slotSaveAs()
{
    QString savefileas = QFileDialog::getSaveFileName(0,"Сохранить как...", "", "*.waypoints" );
    qDebug() << "savefileas" << savefileas;
    if (savefileas == "") savefileas = m_strFileName;
    this->file = new QFile(savefileas);
    if (file->open(QIODevice::WriteOnly | QIODevice::Truncate)){
        savewpdata(*file);
    ui->label->setText("File saved.");
}
    file->close();
}

void MainWindow::slotCloseAll()
{
    qDebug() << "File closed" << file;
    MainWindow::setWindowTitle("Mission Planner 02 ");
  //  delete file; // удаляет указатель на файл
    this->file = nullptr;
    waypointsList.clear();
    ui->label->setText("File closed.");
    op = false;

    this->ui->actionSave->setDisabled(true);
    this->ui->actionSave_As->setDisabled(true);
    this->ui->actionClose->setDisabled(true);

  //  displaywpdata(0);

    buttonList.clear();
    buttonUPList.clear();
    buttonDOWNList.clear();
    cmdList.clear();
    ui->tableWidget->setRowCount(waypointsList.count());
    clearPanel();
    ui->ShowMapWidget->clearWaypointsRoute();
  //  ui->ShowMapWidget->resetCachePainter();
}

void MainWindow::slotSelectSpif()
{
    QString spifFileName;
    spifFileName = ui->ShowMapWidget->getSpifFile();
    QString newSpifFileName = QFileDialog::getOpenFileName(0, "Путь к spif cache:", spifFileName, "*.spif");
    if (!newSpifFileName.isEmpty())
    {
        spifFileName = newSpifFileName;
        ui->ShowMapWidget->setSpifFile(spifFileName);
        ui->ShowMapWidget->resetCachePainter();
    }
}

void MainWindow::centerCoord()
{
    if(op){
        ui->ShowMapWidget->setCenterOfCachePainter(waypointsList.at(0)->show_lat(),waypointsList.at(0)->show_lon());
    }
    else
    {
        ui->ShowMapWidget->setCenterOfCachePainter(def_lat, def_lon);
    }
}

void MainWindow::deleteSelectedRow()
{
    qDebug() << "deleteSelectedRow";

    int rowId = buttonList.indexOf((QPushButton*)QObject::sender()); // в rowId кидаем номер строки сендера, пославшего сигнал
    qDebug() << rowId;

    ui->tableWidget->removeRow(rowId);

    // удалить кнопку из списка для удаленной строки
    buttonList.removeAt(rowId);
    buttonUPList.removeAt(rowId);
    buttonDOWNList.removeAt(rowId);
    vlayout->removeWidget(panelWidgetList[rowId]);

    // удалить информацию о waypoint из QList
 //   delete waypointsList[rowId];
    delete panelWidgetList[rowId];
    waypointsList.removeAt(rowId);

    panelWidgetList.removeAt(rowId);
    displaywpdata(1);
    ui->ShowMapWidget->update();
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::getwpdata(QFile &file){
    waypointsList.clear();
    op = true;
    QTextStream stream(&file);
    if(stream.readLine() == "QGC WPL 110")
    {
        while(!stream.atEnd())
        {
            Waypoint* waypoint = new Waypoint();
            QStringList lst = stream.readLine().split("\t");
            int num = lst.at(0).toInt();
            int typep = lst.at(1).toInt();
            int alttype = lst.at(2).toInt();
            int pointtype = lst.at(3).toInt();
            float param1 = lst.at(4).toFloat();
            float param2 = lst.at(5).toFloat();
            float param3 = lst.at(6).toFloat();
            float param4 = lst.at(7).toFloat();
            double lat = lst.at(8).toDouble();
            double lon = lst.at(9).toDouble();
            float alt = lst.at(10).toFloat();
            int automode = lst.at(11).toInt();

            waypoint->write_num(num);
            waypoint->write_typep(typep);
            waypoint->write_alttype(alttype);
            waypoint->write_pointtype(pointtype);
            waypoint->write_param1(param1);
            waypoint->write_param2(param2);
            waypoint->write_param3(param3);
            waypoint->write_param4(param4);
            waypoint->write_lat(lat);
            waypoint->write_lon(lon);
            waypoint->write_alt(alt);
            waypoint->write_automode(automode);

            this->waypointsList.append(waypoint);
            delete waypoint;

        }
    }
}


void MainWindow::displaywpdata(bool panel){
    if (!(waypointsList.isEmpty())) resetIndex();
    if (panel) clearPanel();
    ui->tableWidget->blockSignals(true);
    buttonList.clear();
    cmdList.clear();
    buttonUPList.clear();
    buttonDOWNList.clear();

     ui->tableWidget->setRowCount(waypointsList.count());
     QStringList vertical;
     for (int i = 0; i<waypointsList.count(); i++)
     {
         vertical << QString::number(i); // делаем список для отображения номеров строк в интерфейсе

         QComboBox* switchBox = new QComboBox();  // комбобокс для выбора типа команды
         switchBox->setEditable(0);
         switchBox->addItems(cmdType);
         cmdList.append(switchBox); // для них предусмотрен отдельный массив
         switchBox->setCurrentText(cmdName(waypointsList.at(i)->show_pointtype())); // устанавливаем текущую позицию комбобокса
         ui->tableWidget->setCellWidget(i, 0, switchBox);

         ui->tableWidget->setItem(i,1, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_param1(), 0, 'f', 8)));
         ui->tableWidget->setItem(i,2, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_param2(), 0, 'f', 8)));
         ui->tableWidget->setItem(i,3, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_param3(), 0, 'f', 8)));
         ui->tableWidget->setItem(i,4, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_param4(), 0, 'f', 8)));
         ui->tableWidget->setItem(i,5, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_lat(), 0, 'd', 8)));
         ui->tableWidget->setItem(i,6, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_lon(), 0, 'd', 8)));
         ui->tableWidget->setItem(i,7, new QTableWidgetItem(QString("%1").arg(waypointsList.at(i)->show_alt(), 0, 'f', 6)));


         QPushButton *delPB = new QPushButton(); // создание кнопок для удаления элементов
         delPB->setText("X");
         buttonList.append(delPB);
         ui->tableWidget->setCellWidget(i, 8, delPB);

         QPushButton *upPB = new QPushButton();
         QPushButton *downPB = new QPushButton();
         QCommonStyle upPBStyle, downPBStyle;
         upPB->setIcon(upPBStyle.standardIcon(QStyle::SP_ArrowUp));
         downPB->setIcon(upPBStyle.standardIcon(QStyle::SP_ArrowDown));
         buttonUPList.append(upPB);
         buttonDOWNList.append(downPB);
         ui->tableWidget->setCellWidget(i, 10, upPB);
         ui->tableWidget->setCellWidget(i, 11, downPB);


         connect(upPB, &QPushButton::clicked, this, &MainWindow::upButton);
         connect(downPB, &QPushButton::clicked, this, &MainWindow::downButton);
         connect(delPB, &QPushButton::clicked, this, &MainWindow::deleteSelectedRow);
         connect(switchBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePointTypeForWaypointListRow(int)));

     }
     if (!waypointsList.isEmpty()) { calcDists(); calcHeights(); }
     setToolTipsForTable();

     ui->ShowMapWidget->clearWaypointsRoute();
     ui->ShowMapWidget->setWaypointsRoute(this->waypointsList); // прорисовка маршрута на карту
     ui->ShowMapWidget->update();
     connect(ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(userClicked(int, int)));
     qDebug()<< vertical;
     ui->tableWidget->setVerticalHeaderLabels(vertical);
     ui->tableWidget->blockSignals(false);

    if (panel) updPanel();
}



void MainWindow::savewpdata(QFile &file){
    QTextStream stream(&file);
    stream << "QGC WPL 110\n";
    for (int i = 0; i<waypointsList.count(); i++){
        stream << waypointsList.at(i)->show_num() << "\t"
               << waypointsList.at(i)->show_typep() << "\t"
               << waypointsList.at(i)->show_alttype() << "\t"
               << waypointsList.at(i)->show_pointtype() << "\t"
               << QString::number(waypointsList.at(i)->show_param1(),'f',8) << "\t"
               << QString::number(waypointsList.at(i)->show_param2(),'f',8) << "\t"
               << QString::number(waypointsList.at(i)->show_param3(),'f',8) << "\t"
               << QString::number(waypointsList.at(i)->show_param4(),'f',8) << "\t"
               << QString::number(waypointsList.at(i)->show_lat(),'d',8) << "\t"
               << QString::number(waypointsList.at(i)->show_lon(),'d',8) << "\t"
               << QString::number(waypointsList.at(i)->show_alt(),'f',6) << "\t"
               << waypointsList.at(i)->show_automode();

        if(i < waypointsList.count()-1)
                  stream << "\n";
    }
}

QString MainWindow::cmdName(int pointtype)
{
    switch (pointtype) {
    case 16: return "WAYPOINT";
    case 21: return "LAND";
    case 208: return "DO PARACHUTE";
    case 178: return "DO CHANGE SPEED";
    default: return "OTHER";
    }

}

int MainWindow::cmdNum(QString cmdName)
{
    if (cmdName == "WAYPOINT") return 16;
    if (cmdName == "LAND") return 21;
    if (cmdName == "DO PARACHUTE") return 208;
    if (cmdName == "DO CHANGE SPEED") return 178;
    if (cmdName == "WAYPOINT") return 16;
    else return 0;
}

void MainWindow::setToolTipsForTable()
{
    for (int i = 0; i<waypointsList.count(); i++){
        if (waypointsList.at(i)->show_pointtype() == 178) ui->tableWidget->item(i,2)->setToolTip("Airspeed");
        if (waypointsList.at(i)->show_pointtype() == 16) {
            if (waypointsList.at(i)->show_param1() == 1) ui->tableWidget->item(i,1)->setToolTip("Takeoff");
        }
         if (waypointsList.at(i)->show_pointtype() == 208){
             if (waypointsList.at(i)->show_param1() == 1) ui->tableWidget->item(i,1)->setToolTip("Parachute Ready");
              if (waypointsList.at(i)->show_param1() == 2) ui->tableWidget->item(i,1)->setToolTip("Parachute Released");
         }
    }
}



void MainWindow::slotZoom()
{
    QString sign = qobject_cast<QPushButton *>(sender())->objectName();
    ui->ShowMapWidget->zoomFunc(&sign);
    setSlider(ui->ShowMapWidget->getCurrentLayer());
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{

    QString tmp = ui->tableWidget->item(row, column)->text();
    bool ok;
    qDebug() << "tmp" << tmp;
    tmp.toDouble(&ok);
    qDebug() << "OK" << ok;
    if(ok)
    {
        switch(column){
            case 0:
            {

                waypointsList[row]->write_pointtype(tmp.toInt());
                break;
            }
            case 1:
            {
                waypointsList[row]->write_param1(tmp.toFloat());
                break;
            }
            case 2:
            {
                waypointsList[row]->write_param2(tmp.toFloat());
                break;
            }
            case 3:
            {
                waypointsList[row]->write_param3(tmp.toFloat());
                break;
            }
            case 4:
            {
                waypointsList[row]->write_param4(tmp.toFloat());
                break;
            }
            case 5:
            {
                if ((tmp.toDouble()<360)&&(tmp.toDouble()>0))
                    waypointsList[row]->write_lat(tmp.toDouble());
                break;
            }
            case 6:
            {
                if ((tmp.toDouble()<360)&&(tmp.toDouble()>0))
                    waypointsList[row]->write_lon(tmp.toDouble());
                break;
            }
            case 7:
            {
                waypointsList[row]->write_alt(tmp.toInt());
                break;
            }
        }
    }

    displaywpdata(1);
}


void MainWindow::updateTable(){
//    ui->tableWidget->setRowCount(0); // обнуление таблицы
    displaywpdata(1);
}


void MainWindow::updPanel()
{
    //------добавление в боковую панель
            for (int i = 0; i < waypointsList.count(); i++) {
                WaypointWidget *widget = new WaypointWidget(ui->frameParent);
                widget->setWaypointWidgetData(waypointsList.at(i)->show_num(), waypointsList.at(i)->show_lat(), waypointsList.at(i)->show_lon(), waypointsList.at(i)->show_alt(), waypointsList.at(i)->show_pointtype());
                widget->setFrameStyle(QFrame::Box);
                connect(widget, SIGNAL(labelDeleteClicked()), this, SLOT(deleteRowFromTable()));
                connect(widget, SIGNAL(lineEditChanged()), this, SLOT(updateDataFromPanel()));
                connect(widget, SIGNAL(switchBoxPanelChanged(int)), this, SLOT(updateDataFromPanel()));
                panelWidgetList.append(widget);
                vlayout->addWidget(widget);
            }
            vlayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
   ui->scrollAreaWidgetContents->update(); // под вопросом, нужны ли они два раза
   ui->scrollAreaWidgetContents->repaint();
}

void MainWindow::calcDists()
{
    double dist = ui->ShowMapWidget->dist_btw_points(ui->ShowMapWidget->stock_lat, ui->ShowMapWidget->stock_lon, waypointsList.at(0)->show_lat(), waypointsList.at(0)->show_lon());
    ui->tableWidget->setItem(0,9, new QTableWidgetItem(QString("%1").arg(dist, 0, 'f', 1)));
    for (int i = 1; i<waypointsList.count(); i++)
    {
        dist = ui->ShowMapWidget->dist_btw_points(waypointsList.at(i-1)->show_lat(), waypointsList.at(i-1)->show_lon(), waypointsList.at(i)->show_lat(), waypointsList.at(i)->show_lon());
        ui->tableWidget->setItem(i,9, new QTableWidgetItem(QString("%1").arg(dist, 0, 'f', 1)));
        ui->tableWidget->setColumnWidth(9, 100);
    }

}

void MainWindow::calcHeights()
{
    double dist = 0;
    float angle = 0;
    float percent = 0;
    // ui->tableWidget->setItem(0,12, new QTableWidgetItem(QString("%1").arg(angle, 0, 'f', 1)));
    for (int i = 1; i<waypointsList.count(); i++)
    {
        dist =  ui->ShowMapWidget->dist_btw_points(waypointsList.at(i-1)->show_lat(), waypointsList.at(i-1)->show_lon(), waypointsList.at(i)->show_lat(), waypointsList.at(i)->show_lon());
        angle = ui->ShowMapWidget->angle_btw_alts(waypointsList.at(i-1)->show_alt(), waypointsList.at(i)->show_alt(),dist);
        percent = ui->ShowMapWidget->percent_pp(waypointsList.at(i-1)->show_alt(), waypointsList.at(i)->show_alt(),dist);
        ui->tableWidget->setItem(i,12, new QTableWidgetItem(QString("%1").arg(angle, 0, 'f', 1)));
        ui->tableWidget->setItem(i,13, new QTableWidgetItem(QString("%1").arg(percent, 0, 'f', 1)));
        ui->tableWidget->setColumnWidth(12, 60);
         ui->tableWidget->setColumnWidth(13, 60);
    }
//    double height = ui->ShowMapWidget->dist_btw_points(ui->ShowMapWidget->stock_lat, ui->ShowMapWidget->stock_lon, waypointsList.at(0)->show_lat(), waypointsList.at(0)->show_lon());
//    ui->tableWidget->setItem(0,12, new QTableWidgetItem(QString("%1").arg(dist, 0, 'f', 1)));
//    for (int i = 1; i<waypointsList.count(); i++)
//    {
//        dist = ui->ShowMapWidget->dist_btw_points(waypointsList.at(i-1)->show_lat(), waypointsList.at(i-1)->show_lon(), waypointsList.at(i)->show_lat(), waypointsList.at(i)->show_lon());
//        ui->tableWidget->setItem(i,12, new QTableWidgetItem(QString("%1").arg(dist, 0, 'f', 1)));
//        ui->tableWidget->setColumnWidth(12, 100);
//    }

}

void MainWindow::clearPanel()
{
    while (!panelWidgetList.isEmpty()) {
            vlayout->removeWidget(panelWidgetList.last());
            delete panelWidgetList.last();
            panelWidgetList.removeLast();
        }
        if(panelWidgetList.isEmpty()) qDebug() << "Empty list";

        delete vlayout;
        vlayout = new QVBoxLayout();
        ui->scrollAreaWidgetContents->setLayout(vlayout);
        ui->scrollAreaWidgetContents->update();
        ui->scrollAreaWidgetContents->repaint();
}

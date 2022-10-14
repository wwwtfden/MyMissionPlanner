#include "flightmapwidget.h"
#include "fmwdialog.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QtMath>


FlightMapWidget::FlightMapWidget(QWidget *parent)
    : QWidget(parent)
{
    addPoint = new QPointF;
    this->wppix = QPixmap(":/icons/resources/icons/wp.png").scaled(50,50);
    this->setSpifFile("E:/AP2-autopilot/Cache/4_Hrebtovo_red_GK07_tif.spif"); //E:/AP2-autopilot/Cache/4_Hrebtovo_red_GK07_tif.spif
        if(!QFile(spifFile).exists()) {
            qDebug() << "Stock cache not exists.";
            spifFile = QFileDialog::getOpenFileName(0, "Открыть map cache", "", "*.spif");
        }
    // подгрузка карты/растра
    this->cachePainter = new CachePainter(spifFile);
    //this->cachePainter = new CachePainter("H:/[26-12-2019] Workflow/openGLviewer/4_Kolomna_red_GK07_tif.spif");

    // первоначальная настройка параметров камеры
 //   this->cachePainter->setCameraParameters(2.97, 0.0056, 960, 640);

    // событие нажатия мышки
    this->isPressEvent = false;
    // выделение области на QWidget
//    this->isSelectAreaEvent = false;

    this->xStartCursorPosition = 0;
    this->yStartCursorPosition = 0;

    this->xEndCursorPosition = 0;
    this->yEndCursorPosition = 0;

    this->xPixelShift = 0;
    this->yPixelShift = 0;

    // updated 20.04.2020
 //   this->currentAircraftLatitudeB = 0; // grad
  //  this->currentAircraftLongitudeL = 0; // grad

 //   this->currentAircraftAltitudeH = 0; // meters

 //   this->currentAircraftRollGamma = 0;
 //   this->currentAircraftPitchTheta = 0;
 //   this->currentAircraftYawPsi = 0;

    // подгружаем первоначальное изображение карты при первом открытии окна
    this->setFlightMapWidgetStartState();

    // разрешаем отслеживание курсора
    this->setMouseTracking(true);

    this->planePixmap = QPixmap(":/icons/resources/icons/fixed-wings-icon.png").scaled(64, 64);

    //this->observedImageInSeparateWindow = QImage("E:/20-03-2020-Math-model/defaultObservedImage.bmp");

}

void FlightMapWidget::setCenterOfCachePainter(double lat, double lon){
    //int x,y;
    // cachePainter->convertCoordinatesToPixelsRoi(lat, lon, x, y);
    // cachePainter->setCenterRoi(x,y);
    cachePainter->setCenterRoi(lat, lon);
}

void FlightMapWidget::setCurrentRoiImage(QImage roiImage) {
    this->currentRoiImage = roiImage;
}

//void FlightMapWidget::savePrevoiusRoiImage(QImage roiImage) {
//    this->previousRoiImage = roiImage;
//}

void FlightMapWidget::setFlightMapWidgetStartState() {

    cachePainter->setLayer(17); // текущий слой

    // координаты церкви в Хребтово
//    this->currentBCenterRoi = 56.590740;
//    this->currentLCenterRoi = 38.274085;
    cachePainter->getCenterCache(this->currentBCenterRoi, this->currentLCenterRoi);

    // настраиваем текущий центр QImage
    cachePainter->setCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);
    cachePainter->setRoiSize(this->width(), this->height()); // задаем для искомого изображения карты ширину и высоту QWidget'a

    //cachePainter->setRoiSize(1920, 1200); // разрешение QImage

    qDebug() << "setFlightMapWidgetStartState()" << this->xPixelPosition << "; " << this->yPixelPosition;
}

void FlightMapWidget::clearWaypointsRoute()
{
    this->waypointsRoute.clear();
    this->altsData.clear();
    this->airSpeedData.clear();
    this->update();
}


// настройка заданного маршрута (выбор и конвертация маршрута из формата QList<PPM*> в QPolygonF)
void FlightMapWidget::setWaypointsRoute(QList<Waypoint*> waypointsList) {
    float tmp = 0;
    for (int i = 0; i < waypointsList.size(); i++) {
                                                                   // в градусы                             // в градусы
        this->waypointsRoute.append(QPointF(waypointsList[i]->show_lat(), waypointsList[i]->show_lon()) );
        this->altsData.append(waypointsList[i]->show_alt());

        if (waypointsList[i]->show_pointtype() == 178){
        this->airSpeedData.append(waypointsList[i]->show_param2());
        tmp = waypointsList[i]->show_param2();
        }
        else {
             this->airSpeedData.append(tmp);
        }
    }
    qDebug() << airSpeedData;
}

//void FlightMapWidget::updateAircraftRoute(QPointF newCoordinates) { // newCoordinates: B, L

//     this->aircraftRoute.append(newCoordinates);
//     //this->update();
//}

// обновить траекторию полета моксана
//void FlightMapWidget::updateMocsanRoute(QPointF newCoordinates) {

//    this->mocsanRoute.append(newCoordinates);
//}
//// обновить траекторию полета СПВС
//void FlightMapWidget::updateAirCalculatedRoute(QPointF newCoordinates) {

//    this->airCalculatedRoute.append(newCoordinates);
//}

int FlightMapWidget::checkIndex()
{
    double xsp, ysp;
    this->cachePainter->convertPixelsRoiToCoordinates(xStartCursorPosition, yStartCursorPosition, xsp ,ysp);
    int bestindex = waypointsRoute.count();
   // cachePainter->getRoiSize(bx, by);
   // double dist = sqrt((bx)^2+(by)^2);
    double dist = 9999999;
    qDebug() << "checkIndex()" << xsp << ysp;
    for (int i = 0; i < waypointsRoute.count(); i++){
//            int tmpx = abs(xStartCursorPosition - waypointsRoute.at(i).x());
//            int tmpy = abs(yStartCursorPosition - waypointsRoute.at(i).y());
            double tmpx, tmpy;
            this->cachePainter->convertPixelsRoiToCoordinates(this->waypointsRoute.at(i).x(), this->waypointsRoute.at(i).y(), tmpx, tmpy);
            tmpx = this->waypointsRoute.at(i).x();
            tmpy = this->waypointsRoute.at(i).y();
         //   qDebug() << "WP" << i << "coords " << this->waypointsRoute.at(i).x() << this->waypointsRoute.at(i).y();
             qDebug() << "WP pix" << i << "coords " << tmpx << tmpy;
            double tmp1x = abs(xsp - tmpx);
            double tmp1y = abs(ysp - tmpy);
//            double tmpdist = (tmp1x*tmp1x)+(tmp1y*tmp1y);
//            tmpdist = sqrt(tmpdist);
            double tmpdist = sqrt((tmp1x*tmp1x)+(tmp1y*tmp1y));
            qDebug() << "tmpdist" << i << tmpdist;
            if (tmpdist < dist){
                dist = tmpdist;
                bestindex = i;
            }
    }

    return bestindex;
}

double FlightMapWidget::checkDist()
{
    double xsp, ysp;
    this->cachePainter->convertPixelsRoiToCoordinates(xStartCursorPosition, yStartCursorPosition, xsp ,ysp);
    int bestindex = waypointsRoute.count();
    double dist = 9999999;
   // qDebug() << "checkIndex()" << xsp << ysp;
    for (int i = 0; i < waypointsRoute.count(); i++){
            double tmpx, tmpy;
            this->cachePainter->convertPixelsRoiToCoordinates(this->waypointsRoute.at(i).x(), this->waypointsRoute.at(i).y(), tmpx, tmpy);
            tmpx = this->waypointsRoute.at(i).x();
            tmpy = this->waypointsRoute.at(i).y();
           //  qDebug() << "WP pix" << i << "coords " << tmpx << tmpy;
            double tmp1x = abs(xsp - tmpx);
            double tmp1y = abs(ysp - tmpy);
            double tmpdist = sqrt((tmp1x*tmp1x)+(tmp1y*tmp1y));
            qDebug() << "tmpdist" << i << tmpdist;
            if (tmpdist < dist){
                dist = tmpdist;
                bestindex = i;
            }
    }

    return dist;
}

void FlightMapWidget::setSpifFile(QString str)
{
    this->spifFile = str;
}

QString FlightMapWidget::getSpifFile()
{
    return spifFile;
}

void FlightMapWidget::resetCachePainter()
{
    this->cachePainter = new CachePainter(spifFile);
    //this->cachePainter->setCameraParameters(2.97, 0.0056, 960, 640);
//    this->isPressEvent = false;
//    this->isSelectAreaEvent = false;
//    this->xStartCursorPosition = 0;
//    this->yStartCursorPosition = 0;
//    this->xEndCursorPosition = 0;
//    this->yEndCursorPosition = 0;
//    this->xPixelShift = 0;
//    this->yPixelShift = 0;
//    this->currentAircraftLatitudeB = 0; // grad
//    this->currentAircraftLongitudeL = 0; // grad
//    this->currentAircraftAltitudeH = 0; // meters
//    this->currentAircraftRollGamma = 0;
//    this->currentAircraftPitchTheta = 0;
//    this->currentAircraftYawPsi = 0;
    this->setFlightMapWidgetStartState();
    this->setMouseTracking(true);
    this->planePixmap = QPixmap(":/icons/resources/icons/fixed-wings-icon.png").scaled(64, 64);
}


// обновить текущее пространственное положение самолета
//void FlightMapWidget::updateAircraftSpatialPosition(double currentLatitudeB, double currentLongitudeL, double currentAltitudeH, double currentRollGamma, double currentPitchTheta, double currentYawPsi) {

//    this->currentAircraftLatitudeB = currentLatitudeB;
//    this->currentAircraftLongitudeL = currentLongitudeL;

//    this->currentAircraftAltitudeH = currentAltitudeH;

//    this->currentAircraftRollGamma = currentRollGamma;
//    this->currentAircraftPitchTheta = currentPitchTheta;
//    this->currentAircraftYawPsi = currentYawPsi;
//}

// отрисовка заданного маршрута
void FlightMapWidget::showWaypointsRoute(QPainter &widgetPainter) {

    if (! this->waypointsRoute.isEmpty()) { // если заданный маршрут не пустой и был настроен

        //qDebug() << "Waypoints route is drawing on the map.";

        widgetPainter.setPen(QPen(Qt::yellow, 2));
        widgetPainter.setBrush(Qt::yellow);

        // настройки шрифта для отрисовки нумерации ППМов
        QFont waypointsNumberFont = widgetPainter.font();
        QFont paramsFont = widgetPainter.font();
        paramsFont.setPixelSize(12);
        waypointsNumberFont.setPixelSize(24);
        widgetPainter.setFont(waypointsNumberFont);


        this->cachePainter->convertCoordinatesToPixelsRoi(this->waypointsRoute.at(0).x(), this->waypointsRoute.at(0).y(),
                                                    this->xPixelPosition, this->yPixelPosition);

        QPointF firstPoint(this->xPixelPosition, this->yPixelPosition);
        widgetPainter.drawText(firstPoint.x()+25, firstPoint.y()-25, QString::number(0));
        widgetPainter.drawPixmap(firstPoint.x()-25, firstPoint.y()-50, wppix);
        widgetPainter.drawEllipse(firstPoint.x()-4, firstPoint.y()-4, 8, 8);
        widgetPainter.setFont(paramsFont);
        widgetPainter.drawText(firstPoint.x()+30, firstPoint.y()-10, "Alt: " + QString::number(altsData.at(0)));     // отрисовка высоты
        widgetPainter.drawText(firstPoint.x()+30, firstPoint.y(), "Spd: -");
        widgetPainter.setFont(waypointsNumberFont);
        bool flagdef = 1; // дальше конкретный быдлокод
        for(int i = 0; i < this->waypointsRoute.count() - 1; i++) {
            //qDebug() << "Waypoints 0" << this->waypointsRoute.at(i).x() << "; " << this->waypointsRoute.at(i).y();
            //qDebug() << "Waypoints 1" << this->waypointsRoute.at(i+1).x() << "; " << this->waypointsRoute.at(i+1).y();

            // Waypoints route [B, L] --> QWidget [x, y]
            this->cachePainter->convertCoordinatesToPixelsRoi(this->waypointsRoute.at(i).x(), this->waypointsRoute.at(i).y(),
                                                        this->xPixelPosition, this->yPixelPosition);
            this->cachePainter->convertCoordinatesToPixelsRoi(this->waypointsRoute.at(i+1).x(), this->waypointsRoute.at(i+1).y(),
                                                        this->x1PixelPosition, this->y1PixelPosition);

            QPointF point0(this->xPixelPosition, this->yPixelPosition), point1(this->x1PixelPosition, this->y1PixelPosition);

            //qDebug() << "Points 0" << point0.x() << "; " << point0.y();
            //qDebug() << "Points 1" << point1.x() << "; " << point1.y();

            widgetPainter.setPen(QPen(Qt::cyan, 2));
            widgetPainter.setBrush(Qt::cyan);
            widgetPainter.drawLine(point0, point1);
            widgetPainter.drawPixmap(point1.x()-25, point1.y()-50, wppix);
            widgetPainter.drawEllipse(point1.x()-4, point1.y()-4, 8, 8);
            widgetPainter.setPen(QPen(Qt::yellow, 2));
            widgetPainter.setBrush(Qt::yellow);

        //    widgetPainter.drawText(point1, QString::number(i+1));
            widgetPainter.setFont(waypointsNumberFont);
            widgetPainter.drawText(point1.x()+25, point1.y()-25, QString::number(i+1));
            widgetPainter.setFont(paramsFont);
            widgetPainter.drawText(point1.x()+30, point1.y()-10, "Alt: " + QString::number(altsData.at(i+1)));

            if (airSpeedData.at(i+1) == 0.0 && flagdef) { widgetPainter.drawText(point1.x()+30, point1.y(), "Spd: -"); }
            else { widgetPainter.drawText(point1.x()+30, point1.y(), "Spd: " + QString::number(airSpeedData.at(i+1))); flagdef = 0; }

           // QPixmap scaled = wppix.scaled(QSize(64,64));
//            widgetPainter.setBrush(QBrush(QPixmap (scaled)));
      //      widgetPainter.drawPixmap(point1, wppix);


//            widgetPainter.setPen(QPen(Qt::green, 3));
//            widgetPainter.setBrush(Qt::green);

        } // for

    } // if (! this->waypointsRoute.isEmpty())
}



void FlightMapWidget::paintEvent(QPaintEvent *event) {

    // настраиваем текущий центр QImage
    // cachePainter->setCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);

    //cachePainter->setRoiSize(this->width(), this->height()); // задаем для искомого изображения карты ширину и высоту QWidget'a
    //qDebug() << "QWidget -> width ; height: " << this->width() << " ; " << this->height();

    QImage roiImage = cachePainter->getRoiImage(); // получаем искомое изображение карты
    this->setCurrentRoiImage(roiImage);

    // получаем координаты текущего центра для актуального QImage
    cachePainter->getCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);

    // считаем текущий центр QWidget'a в пикселях
    this->cachePainter->convertCoordinatesToPixelsRoi(this->currentBCenterRoi, this->currentLCenterRoi, this->currentXCenterRoi, this->currentYCenterRoi);

    // отрисовываем картинку
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, this->currentRoiImage);

    widgetPainter.setPen(Qt::red);

    // отрисовка в виде красного кружка
    // координаты Хребтовского полигона Алмаз-Антея
    cachePainter->convertCoordinatesToPixelsRoi(56.587371, 38.274083, this->xPixelPosition, this->yPixelPosition); // пересчитываем координаты в пиксели QWidget'a
    widgetPainter.drawEllipse(QPointF(this->xPixelPosition, this->yPixelPosition), 10, 10);
    //qDebug() << "QWidget -> xPixelPosition ; yPixelPosition : " << this->xPixelPosition << "; " << this->yPixelPosition;




    // отрисовка заданной траектории
    this->showWaypointsRoute(widgetPainter);


    // если пользователь выделяет необходимую прямоугольную область для ее увеличения
//    if (this->isSelectAreaEvent) {

//        if (this->xEndCursorPosition != 0 && this->yEndCursorPosition != 0) {

//            // отрисовываем полупрозрачный синий прямоугольник с белыми штрих-границами
//            QPen penOfArea;
//            penOfArea.setColor(Qt::white);
//            penOfArea.setStyle(Qt::DashLine);
//            widgetPainter.setPen(penOfArea);
//            widgetPainter.setBrush(QColor(0, 0, 70, 70)); // полупрозрачный синий
//            // прямоугольник от начала зажатия правой кнопки мыши до ее отпускания
//            widgetPainter.drawRect(this->xStartCursorPosition, this->yStartCursorPosition,
//                                   this->xEndCursorPosition - this->xStartCursorPosition,
//                                   this->yEndCursorPosition - this->yStartCursorPosition);
//        }

//    }

    widgetPainter.end();

    //this->updateGeometry();
    this->update();
}

void FlightMapWidget::resizeEvent(QResizeEvent *event) {

    cachePainter->setRoiSize(this->width(), this->height()); // задаем для искомого изображения карты ширину и высоту QWidget'a
    this->update();
}

void FlightMapWidget::mouseMoveEvent(QMouseEvent *event) {

    //this->currentMouseMoveCounter ++;

    // перетаскивание карты "лапкой" - левой кнопкой мыши
    if (event->buttons() & Qt::LeftButton) {
        this->setCursor(Qt::ClosedHandCursor);
        // фиксируем конечное положение курсора
        this->xEndCursorPosition = event->pos().x();
        this->yEndCursorPosition = event->pos().y();

        // вычисляем смещение
        this->xPixelShift = this->xEndCursorPosition - this->xStartCursorPosition;
        this->yPixelShift = this->yEndCursorPosition - this->yStartCursorPosition;

        qDebug() << "PixelShift -> x ; y: " << this->xPixelShift << " ; " << this->yPixelShift;

        // обновляем начальное положение курсора
        this->xStartCursorPosition = event->pos().x();
        this->yStartCursorPosition = event->pos().y();

        // центр QWidget'a с учетом сдвига
        this->currentXCenterRoi = this->currentXCenterRoi - this->xPixelShift;
        this->currentYCenterRoi = this->currentYCenterRoi - this->yPixelShift;

        // новый центр Roi
        this->cachePainter->convertPixelsRoiToCoordinates(this->currentXCenterRoi, this->currentYCenterRoi,
                                                          this->currentBCenterRoi, this->currentLCenterRoi);

        qDebug() << "currentBCenterRoi ; currentLCenterRoi: " << this->currentBCenterRoi << " ; " << this->currentLCenterRoi;

        // настраиваем текущий центр QImage
        this->cachePainter->setCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);

        // обновляем QWidget
        this->update();

    }

    if (event->buttons() & Qt::RightButton) {

        this->xEndCursorPosition = event->pos().x();
        this->yEndCursorPosition = event->pos().y();
       // this->waypointsRoute.at(last_wp).x()
        if(!waypointsRoute.isEmpty()){

        if (distanceForClosestWp < 0.0003) emit this->pointWasMoved(BCursorPosition, LCursorPosition, last_wp);


    }
    }

    // отображение текущих координат в QLabel
    this->xCursorPosition = event->pos().x();
    this->yCursorPosition = event->pos().y();

    this->cachePainter->convertPixelsRoiToCoordinates(this->xCursorPosition, this->yCursorPosition,
                                                      this->BCursorPosition, this->LCursorPosition);

//    emit this->setLabelStatusBarText(QString("B:  %1°   L:  %2°     layerZ:  %3")
//                                     .arg(this->BCursorPosition, 0, 'f', 6)
//                                     .arg(this->LCursorPosition, 0, 'f', 6)
//                                     .arg(this->cachePainter->getLayer()));

 //   emit this->setLabelStatusBarText(QString("Текущий слой: %1").arg(this->cachePainter->getLayer()));
    emit this->setLabelStatusBarText(QString("Широта:  %1°   Долгота:  %2°   Текущий слой:  %3")
                                     .arg(this->BCursorPosition, 0, 'f', 6)
                                     .arg(this->LCursorPosition, 0, 'f', 6)
                                     .arg(this->cachePainter->getLayer()));


    // Пробуем отображать Tooltip

//    for (int i = 0; i<waypointsRoute.count(); i++){
//        double currentX = waypointsRoute.at(i).x();
//        double currentY = waypointsRoute.at(i).y();
//        qDebug() << currentX << currentY;
//    }

}


void FlightMapWidget::mousePressEvent(QMouseEvent *event) {

    // если нажата левая кнопка мыши
    if (event->button() == Qt::LeftButton) {

        this->isPressEvent = true;
        //this->setCursor(Qt::ClosedHandCursor);

        qDebug() << "Left-button / QCursor -> x ; y: " << event->pos().x() << " ; " << event->pos().y();

        // фиксируем начальное положение курсора
        this->xStartCursorPosition = event->pos().x();
        this->yStartCursorPosition = event->pos().y();
    }

    // если нажата правая кнопка мыши (выделение необходимой прямоугольной области)
    if (event->button() == Qt::RightButton) {

        qDebug() << "Right-button / QCursor -> x ; y: " << event->pos().x() << " ; " << event->pos().y();

        // фиксируем начальное положение курсора
        this->xStartCursorPosition = event->pos().x();
        this->yStartCursorPosition = event->pos().y();
        if (!waypointsRoute.isEmpty()){
            last_wp = checkIndex();
            qDebug() << "Closest WP: " << last_wp;
            distanceForClosestWp = checkDist();

            if (distanceForClosestWp < 0.0001){
        qDebug() << "Catched WP <" << last_wp << ">, distance for WP: " << distanceForClosestWp;
            }
        }
        // меняем вид курсора на "крестик"
//        setCursor(QCursor(Qt::CrossCursor));
//        this->isSelectAreaEvent = true;
    }

}

void FlightMapWidget::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "event->button(): " << event->button();
   // qDebug () << "Start Cursor Position" << this->xStartCursorPosition << this->yStartCursorPosition;
  // qDebug() << "End Cursor Position" << this->xCursorPosition << this->yCursorPosition;
    bool flag_moved = 0;
    if ((this->xStartCursorPosition != this->xCursorPosition) || (this->yStartCursorPosition != this->yCursorPosition)) {
        emit this->pointMoveFinished(last_wp);
        flag_moved = 1;
    }

    if (event->button() == Qt::LeftButton) {

        this->isPressEvent = false;
        this->setCursor(Qt::OpenHandCursor);

        this->xStartCursorPosition = 0;
        this->yStartCursorPosition = 0;

        this->xEndCursorPosition = 0;
        this->yEndCursorPosition = 0;

    }

    if (event->button() == Qt::RightButton) {


        qDebug() << "Right-button / QCursor -> x ; y: " << event->pos().x() << " ; " << event->pos().y();
          //  qDebug() << "Closest WP: " << checkIndex();

        // фиксируем начальное положение курсора
        this->xStartCursorPosition = event->pos().x();
        this->yStartCursorPosition = event->pos().y();
        bool flagadd = 0;
        for(int i = 0; i < this->waypointsRoute.count(); i++) {

           // конвертация в пиксели
           this->cachePainter->convertCoordinatesToPixelsRoi(this->waypointsRoute.at(i).x(), this->waypointsRoute.at(i).y(),
                                                              this->xPixelPosition, this->yPixelPosition);

       //     qDebug() << this->xPixelPosition << this->yPixelPosition;

            if ( ((this->xStartCursorPosition < this->xPixelPosition + 20) && (this->xStartCursorPosition > this->xPixelPosition - 20))
                 && ((this->yStartCursorPosition < this->yPixelPosition + 20) && (this->yStartCursorPosition > this->yPixelPosition - 20)) && (!flag_moved)) {
                qDebug() << "Waypoint #" << i << "was clicked;";
                pmnu = new QMenu(this);
                QString nameOfDeleteAction = "&Delete Waypoint " + QString::number(i);
                pmnu->addAction(nameOfDeleteAction);
               // QPointF point(this->xStartCursorPosition, this->yStartCursorPosition);
                connect(pmnu, SIGNAL(triggered(QAction*)), SLOT(slotMenuActivated(QAction*)));
                pmnu->exec(event->globalPos());
                //connect(pmnu, SIGNAL(triggered(QAction*)), SIGNAL(wpDeleted));

                //connect(pmnu, SIGNAL(triggered(QAction*)), , SLOT(deleteWp));

                flagadd = 1;
                break;
            }

        }
        if ((!flagadd) && (!flag_moved)){

      //  addPoint(this->xStartCursorPosition, this->yStartCursorPosition) = new QPointF;
            addPoint->setX(this->xStartCursorPosition);
            addPoint->setY(this->yStartCursorPosition);
        pmnu2 = new QMenu(this);
         pmnu2->addAction("&Add");
         connect(pmnu2, SIGNAL(triggered(QAction*)), SLOT(slotMenuWithPointActivated(QAction*)));
         pmnu2->exec(event->globalPos());


        }
        // break;

        // меняем вид курсора на "крестик"
//        setCursor(QCursor(Qt::CrossCursor));
//        this->isSelectAreaEvent = true;

        //----------------------------зуммирование по прямоугольнику, сейчас пока не актуально

//        if (this->xEndCursorPosition != 0 && this->yEndCursorPosition != 0) {

//            this->isSelectAreaEvent = false;

//            QRect selectedArea(this->xStartCursorPosition, this->yStartCursorPosition,
//                               this->xEndCursorPosition - this->xStartCursorPosition,
//                               this->yEndCursorPosition - this->yStartCursorPosition);


//            if (selectedArea.width() != 0 && selectedArea.height() != 0) {

//                // новый центр Roi
//                this->cachePainter->convertPixelsRoiToCoordinates(selectedArea.center().x(), selectedArea.center().y(),
//                                                                  this->currentBCenterRoi, this->currentLCenterRoi);

//                qDebug() << "SelectedArea | currentBCenterRoi ; currentLCenterRoi: " << this->currentBCenterRoi << " ; " << this->currentLCenterRoi;

//                // меняем координаты центра QImage на координаты центра выделенной области: selectedArea.center().x(), selectedArea.center().y()
//                // настраиваем текущий центр QImage
//                this->cachePainter->setCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);

//                // коэффициенты масштабирования выделенной области относительно основного QWidget
//                int xZoomCoef, yZoomCoef, zoomCoef;
//                qDebug() << "Roi | this->width() ; this->width(): " << this->width() << " ; " << this->height();
//                qDebug() << "Roi | selectedArea.width() ; selectedArea.height(): " << selectedArea.width() << " ; " << selectedArea.height();

//                xZoomCoef = qFloor(this->width() / selectedArea.width());
//                yZoomCoef = qFloor(this->height() / selectedArea.height());

//                zoomCoef = qFloor( ( qMax(xZoomCoef, yZoomCoef) ) / 2 );

//                qDebug() << "zoomCoef: " << zoomCoef; // насколько увеличивать картинку

//                int minLayer, maxLayer;
//                cachePainter->getLayersRange(minLayer, maxLayer);

//                // меняем слой QImage относительно коэффициентов масштабирования
//                if (cachePainter->getLayer() + zoomCoef < maxLayer)
//                    cachePainter->setLayer(cachePainter->getLayer() + zoomCoef);
//                else
//                    cachePainter->setLayer(maxLayer);

//                // меняем тип курсора на стрелку
//                this->setCursor(Qt::ArrowCursor);

//                // отображение текущих координат в QLabel
//                this->xCursorPosition = event->pos().x();
//                this->yCursorPosition = event->pos().y();

//                this->cachePainter->convertPixelsRoiToCoordinates(this->xCursorPosition, this->yCursorPosition,
//                                                                  this->BCursorPosition, this->LCursorPosition);

//                emit this->setLabelStatusBarText(QString("B:  %1°   L:  %2°     layerZ:  %3")
//                                                 .arg(this->BCursorPosition, 0, 'f', 6)
//                                                 .arg(this->LCursorPosition, 0, 'f', 6)
//                                                 .arg(this->cachePainter->getLayer()));

//            }

//            this->xStartCursorPosition = 0;
//            this->yStartCursorPosition = 0;

//            this->xEndCursorPosition = 0;
//            this->yEndCursorPosition = 0;

//            this->update();

//        }
//        setCursor(QCursor(Qt::ArrowCursor));

    }
    setCursor(Qt::ArrowCursor);
    this->update();
    this->xPixelShift = 0;
    this->yPixelShift = 0;

}

void FlightMapWidget::wheelEvent(QWheelEvent *event) {

    int minLayer, maxLayer;
    cachePainter->getLayersRange(minLayer, maxLayer);


    // текущие координаты курсора
    this->xCursorPosition = event->pos().x();
    this->yCursorPosition = event->pos().y();

    int currentLayer;
    currentLayer = cachePainter->getLayer();

    if (event->delta() > 0 && currentLayer < maxLayer) {

        currentLayer ++;
    }
    else if (event->delta() < 0 && currentLayer > minLayer) {

        currentLayer --;
    }

    cachePainter->setLayer( currentLayer, this->xCursorPosition, this->yCursorPosition);

    emit layerWasChanged(currentLayer); // вызывает сигнал смены слоя нак карте для слайдера
    //ui->zoomSlider->setValue(ui->ShowMapWidget->getCurrentLayer());



    this->update();

    emit this->setLabelStatusBarText(QString("Широта:  %1°   Долгота:  %2°   Текущий слой:  %3")
                                     .arg(this->BCursorPosition, 0, 'f', 6)
                                     .arg(this->LCursorPosition, 0, 'f', 6)
                                     .arg(this->cachePainter->getLayer()));
    //    emit this->setLabelStatusBarText(QString("Текущий слой: %1").arg(this->cachePainter->getLayer()));
}



double FlightMapWidget::getBCursorPosition()
{
    return BCursorPosition;
}

double FlightMapWidget::getLCursorPosition()
{
    return LCursorPosition;
}

void FlightMapWidget::clickCenterMapButton() {

    cachePainter->setLayer(17); // текущий слой

    // координаты церкви в Хребтово
    this->currentBCenterRoi = stock_lat;
    this->currentLCenterRoi = stock_lon;

    // настраиваем текущий центр QImage
    cachePainter->setCenterRoi(this->currentBCenterRoi, this->currentLCenterRoi);

    // очистка сдвигов и позиций курсора
    this->xStartCursorPosition = 0;
    this->yStartCursorPosition = 0;

    this->xEndCursorPosition = 0;
    this->yEndCursorPosition = 0;

    this->xPixelShift = 0;
    this->yPixelShift = 0;

}

void FlightMapWidget::slotMenuActivated(QAction* pAction)
{
       // if (pAction->text() == "&Add"){
       //     qDebug() << "Add";
       // }
      //  else {
        QString str = pAction->text().remove("&Delete Waypoint");
        int index = str.toInt();
        qDebug() << "Delete Waypoint" << index;
        emit this->wpDeleted(index);
   //     }

}

void FlightMapWidget::slotMenuWithPointActivated(QAction* pAction)
{

    qDebug() << "Add " << addPoint->x() << " " << addPoint->y();
    this->cachePainter->convertPixelsRoiToCoordinates(addPoint->x(), addPoint->y(), export_lat, export_lon);
    qDebug() << "Converted" << export_lat << export_lon;

    //int stockIndex = checkIndex();
    fmwDialog* takeIndex = new fmwDialog;
   // takeIndex->setIndexValidator(waypointsRoute.count());
    if(takeIndex->exec() == QDialog::Accepted){
        QString inf = takeIndex->txt();
        if (inf.isEmpty()){ emit this->wpAdded(export_lat, export_lon); }
        else { qDebug() << "Index entered in dialog: " << inf;
            int newindex = inf.toInt();
            if (newindex<= waypointsRoute.count()){
            emit this->wpWithIndexAdded(export_lat, export_lon, newindex); }
            else emit this->wpAdded(export_lat, export_lon);
            }
    }
    else {
        emit this->wpAdded(export_lat, export_lon);
      //  emit this->wpWithIndexAdded(export_lat, export_lon, checkIndex());
    }

}


// комментарии на всякий, если понадобится

// 02/03/2020
//widgetPainter.begin(this);
//widgetPainter.drawImage(this->rect(), this->currentRoiImage, this->currentRoiImage.rect());
//drawImage(this->currentRoiImage.rect().topLeft().x(), this->currentRoiImage.rect().topLeft().y(), this->currentRoiImage);

//this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//this->updateGeometry();

// центрируем виджет с картой относительно родительского
//this->move(this->parentWidget()->rect().center() - this->rect().center());
// настраиваем размер, сходный с размером отображаемого roiImage
//this->resize(this->currentRoiImage.width(), this->currentRoiImage.height());
//this->setGeometry(this->xPixelPosition, this->yPixelPosition, this->currentRoiImage.width(), this->currentRoiImage.height());

//this->size().scaled(this->parentWidget()->width(), this->parentWidget()->height(), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
//qDebug() << "layout()->geometry(): " << this->layout()->geometry().size().width() << " ; " << this->layout()->geometry().size().height();

//this->resize(this->parentWidget()->width(), this->parentWidget()->height());
//cachePainter->setRoiSize(this->parentWidget()->width(), this->parentWidget()->height());

//this->resize(this->width(), this->height());

//void FlightMapWidget::setLabelStatusBar(QLabel *labelStatusBar) {

//    this->labelStatusBar = labelStatusBar;
//}

//void FlightMapWidget::setLabelStatusBarText(QString textForDisplay) {

//    this->labelStatusBar->setText(textForDisplay);
//}


    //_child->move(this->rect().center() - _child->rect().center()); // центровка
    //widgetPainter.drawImage(this->xPixelPosition, this->yPixelPosition, this->currentRoiImage);
    //this->rect().bottomLeft().x(), this->rect().bottomLeft().y()
    //this->rect().center() - _child->rect().center()
    //widgetPainter.drawImage(this->currentRoiImage.rect().topRight().x(), this->currentRoiImage.rect().topRight().y(), this->currentRoiImage);

//    QPainter painter(this);

//    for (int i = 0; i < this->currentImage.width(); ++i) {
//        for (int j = 0; j < this->currentImage.height(); ++j)
//          drawImagePixel(&painter, i, j);
//    }

//    QPainter painter(this);

//    QRectF target(0.0, 0.0, 1024, 1024);
//    QRectF source(0.0, 0.0, 512.0, 512.0);

//    painter.drawImage(target, currentImage, source);

//    QPainter imagePainter(&currentImage);
//    imagePainter.initFrom(this);
//    imagePainter.setRenderHint(QPainter::Antialiasing, true);
//    imagePainter.eraseRect(rect());
//    imagePainter.end();

//    QPainter widgetPainter(this);
//    widgetPainter.drawImage(0, 0, this->currentImage);

 //   QPainter painter(this);
    //painter.drawImage(this->, 920, currentImage.scaled(this->width(), this->height()));

//void FlightMapWidget::drawImagePixel(QPainter *painter, int i, int j) {

//    QColor color;
//    QRgb rgb = currentImage.pixel(i, j);

//    painter->fillRect(i, j, 1, 1, color);

//}

//void FlightMapWidget::setImage(QImage newImage) {
//    this->currentImage = newImage;
//}



void FlightMapWidget::zoomFunc(QString* sign){
  //  int rowId = buttonList.indexOf((QPushButton*)QObject::sender());
//   QString sign = qobject_cast<QPushButton *>(sender())->objectName();

//            qDebug() << sign;

    int minLayer, maxLayer;
    cachePainter->getLayersRange(minLayer, maxLayer);

    int currentLayer;
    currentLayer = cachePainter->getLayer();
    if (*sign == "zoomOutButton"){
        if (currentLayer>=minLayer)
                currentLayer--;
    }
    if (*sign == "zoomInButton"){
        if (currentLayer<=maxLayer)
                currentLayer++;
    }
    cachePainter->setLayer(currentLayer, this->xCursorPosition, this->yCursorPosition);
 //   emit this->setLabelStatusBarText(QString("Текущий слой: %1").arg(this->cachePainter->getLayer()));
    emit this->setLabelStatusBarText(QString("Широта:  %1°   Долгота:  %2°   Текущий слой:  %3")
                                     .arg(this->BCursorPosition, 0, 'f', 6)
                                     .arg(this->LCursorPosition, 0, 'f', 6)
                                     .arg(this->cachePainter->getLayer()));
}


void FlightMapWidget::getLayersRange(int &minLayer, int &maxLayer){
    cachePainter->getLayersRange(minLayer, maxLayer);
}

int FlightMapWidget::getCurrentLayer()
{
    return cachePainter->getLayer();
}

void FlightMapWidget::setLayer(int layer)
{
    cachePainter->setLayer(layer);
}

double FlightMapWidget::dist_btw_points(double lat1, double lon1, double lat2, double lon2)
{
    double pi = 3.14159265358979;
    double rad = 6371;
    double num = 7;
    double lat1r, lon1r, lat2r, lon2r, u, v;

    lat1r = lat1*pi/180;
    lat2r = lat2*pi/180;
    lon1r = lon1*pi/180;
    lon2r = lon2*pi/180;

    u = sin((lat2r - lat1r)/2);
    v = sin((lon2r - lon1r)/2);
    return 2.0 * rad * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v)) * 1000;

}

float FlightMapWidget::angle_btw_alts(float alt1, float alt2, float dist)
{
  //  if (alt1 == alt2) { return 0; }
    double pi = 3.14159265358979;
    float deltaalt = alt2-alt1;

    float tanangle = deltaalt / dist;
    float angle = atan(tanangle);
    angle = angle*180 / pi;
    qDebug() << deltaalt << dist;
    return angle;
}

float FlightMapWidget::percent_pp(float alt1, float alt2, float dist)
{
    double pi = 3.14159265358979;
    float deltaalt = alt2-alt1;
    float tanangle = deltaalt / dist;
    return tanangle*100;
}

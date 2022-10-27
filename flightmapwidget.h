#ifndef FLIGHTMAPWIDGET_H
#define FLIGHTMAPWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include "cachepainter.h"
#include "waypoint.h"
#include <QVector>
#include <QPointF>

class FlightMapWidget : public QWidget {

    Q_OBJECT

public:
    FlightMapWidget(QWidget *parent);
    ~FlightMapWidget();

private:
//    virtual void contextMenuEvent(QContextMenuEvent* pe)
//    {
//        pmnu->exec(pe->globalPos());
//    }
    QString spifFile;
    QMenu* pmnu;
    QMenu* pmnu2;
    int last_wp;
    double distanceForClosestWp;
    // Waypoints route // [B, L]
    QPolygonF waypointsRoute;

    QList<float> altsData;
    QList<float> airSpeedData;


    // отрисовщик фрагмента карты
    CachePainter *cachePainter;


 //   QImage previousRoiImage;
    // текущий Roi Image - фрагмент карты
    QImage currentRoiImage;
    QTransform transform; // для поворота картинки на курсовой угол

    double previousBCenterRoi;
    double previousLCenterRoi;

    // текущий центр Roi Image - фрагмента карты
    double currentBCenterRoi; // широта
    double currentLCenterRoi; // долгота

    // текущий центр Roi Image [или QWidget'a]
    int currentXCenterRoi; // х - в пикселях
    int currentYCenterRoi; // y - в пикселях

    // press event
    bool isPressEvent;
    // режим/событие выбора необходимой прямоугольной области
 //   bool isSelectAreaEvent;

    // for shift position tracking
    // начальное положение курсора
    int xStartCursorPosition;
    int yStartCursorPosition;

    // конечное положение курсора
    int xEndCursorPosition;
    int yEndCursorPosition;

    // сдвиг изображения карты
    int xPixelShift;
    int yPixelShift;

    // для конвертации широты+долготы в пиксели QWidget'a
    // в данный момент для отрисовки тестового полигона в виде красного кружка
    // и при отрисовке маршрута

    int xPixelPosition;
    int yPixelPosition;

    // вторая точка - для отрисовки маршрута
    int x1PixelPosition;
    int y1PixelPosition;


    // for mouse move tracking [вывод B-L положения курсора в Label под картой]
    int xCursorPosition; // pixels - x
    int yCursorPosition; // pixels - y

    double BCursorPosition; // latitude
    double LCursorPosition; // longitude

    QPixmap wppix;

//protected:

  // переопределение отрисовки
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  // переопределение масштабирования QWidget'a
  void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  // переопределение движения курсора
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  // переопределение нажатия мышки на QWidget
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  // переопределение освобождения курсора на QWidget'е
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  // переопределение масштабирования с помощью колесика мыши
  void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

public:
  const double stock_lat = 56.590740;
  const double stock_lon = 38.274085;
  double export_lat, export_lon;
//  CachePainter *cachePainter;
  double getBCursorPosition();
  double getLCursorPosition();
  void setCenterOfCachePainter(double lat, double lon); //центровка изображения
  void zoomFunc(QString* sign);
  void getLayersRange(int &minLayer, int &maxLayer);
  int getCurrentLayer();
  void setLayer(int layer);
  double dist_btw_points(double lat1, double lon1, double lat2, double lon2);
  float angle_btw_alts(float alt1, float alt2, float dist);
  float percent_pp(float alt1, float alt2, float dist);

  // настройка заданного маршрута (выбор и конвертация маршрута из формата QList<PPM*> в QPolygonF)
  void clearWaypointsRoute();

  // настройка заданного маршрута (выбор и конвертация маршрута из формата QList<PPM*> в QPolygonF)
  void setWaypointsRoute(QList<Waypoint*> waypointsList);

  int checkIndex();
  double checkDist();
  void setSpifFile(QString str);
  QString getSpifFile();
  void resetCachePainter();



private:
  // задать текущее отображаемое изображение
  void setCurrentRoiImage(QImage roiImage);
  // сохранить предыдущее изображение
//  void savePrevoiusRoiImage(QImage roiImage);

  // настройка первоначальной позиции при первом открытии окна
  void setFlightMapWidgetStartState();

  // отрисовка заданного маршрута
  void showWaypointsRoute(QPainter &widgetPainter);

public:
  QPointF* addPoint;
//  QPolygonF waypointsRoute;

signals:
  // отобразить необходимый текст в QLabel (координаты курсора)
  void setLabelStatusBarText(QString textForDisplay);
  void layerWasChanged(int currentLayer);
  void wpDeleted(int index);
  void wpAdded(double lat, double lon);
  void wpWithIndexAdded(double lat, double lon, int index);
  void pointWasMoved(double BCursorPosition, double LCursorPosition, int index);
  void pointMoveFinished(int index);


public slots:
    // центрировать карту при нажатии кнопки
    void clickCenterMapButton();
    void slotMenuActivated(QAction* pAction);
    void slotMenuWithPointActivated(QAction* pAction);
};



#endif // FLIGHTMAPWIDGET_H

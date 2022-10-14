#ifndef CACHEPAINTER_H
#define CACHEPAINTER_H

#define maxCacheLength 1000 //максимальный размер внутреннего кеша
#define maxTileCount 800 //ограничение на количество тайлов observed image

#include <QImage>
#include <QString>
#include <QSettings>

struct quadrilateral
{
    double x0, y0; //центр (точка пересечения диагоналей)
    double x1, y1; //1-ая вершина (левая нижняя)
    double x2, y2; //2-ая вершина (левая верхняя)
    double x3, y3; //3-ая вершина (правая верхняя)
    double x4, y4; //4-ая вершина (правая нижняя)
};

class CachePainter {
public:
    CachePainter(QString spifPath);

private:
    double BCenter, LCenter;
    int roiWidth, roiHeight;
    int layerZ;
    QSettings *spifFile;
    QImage roiImage;

    QStringList tileCacheFileNames;
    QList<QImage> tileCacheImages;

    double focalLength, pixelSize;
    int cameraMatrixWidth, cameraMatrixHeight;

    double Baircraft, Laircraft, Haircraft;
    double roll, pitch, yaw;

private slots:
    QString xyzToTqrst(int x, int y, int z);
    void GEOLatLonToXY(double B, double L, int z, int &tilesX, int &tilesY, int &picX, int &picY, int &flatX, int &flatY);
    void XYToGEOLatLon(int tilesX, int tilesY, int picX, int picY, int z, double &B, double &L);
    void updateRoiImage();

public slots:
    // получить диапазон доступных слоев
    void getLayersRange(int &minLayer, int &maxLayer);
    // получить координаты центра растра
    void getCenterCache(double &BCenter, double &LCenter);

    // получить координаты углов
    void getCacheTopLeftCorner(double &B, double &L);
    void getCacheTopRightCorner(double &B, double &L);
    void getCacheBottomLeftCorner(double &B, double &L);
    void getCacheBottomRightCorner(double &B, double &L);

    // задать и получить координаты центра Roi
    void setCenterRoi(double BCenter, double LCenter);
    void getCenterRoi(double &BCenter, double &LCenter);

    // задать и получить размер Roi в пикселях
    void setRoiSize(int width, int height);
    void getRoiSize(int &width, int &height);

    // задать номер текущего слоя
    void setLayer(int layer);

    // задать номер текущего слоя с фиксацией положения точки с координатами (x, y) в пикселях Roi
    void setLayer(int layer, int x, int y);

    // получить номер текущего слоя
    int getLayer();

    // получить текущее изображение, меняющееся в завистимости от действий пользователя
    QImage getRoiImage();

    // конвертация пикселей Roi в геодезические координаты и обратно
    void convertPixelsRoiToCoordinates(int x, int y, double &B, double &L);
    void convertCoordinatesToPixelsRoi(double B, double L, int &x, int &y);

    // конвертация метров сферического Меркатора в геодезические координаты и обратно
    // для метров сферического Меркатора необходим масштабный коэффициент, равный секансу широты
    void PseudoMercatorToGEOLatLon(double X, double Y, double &B, double &L);
    void GEOLatLonToPseudoMercator(double B, double L, double &X, double &Y);

    // прибавить к точке с геодезическими координатами B, L смещение в метрах на север dN и на восток dE
    // новая точка будет иметь геодезические координты Bshift, Lshift
    // пересчет корректен только для малых dN и dE (не более нескольких десятков километров)
    void calcShiftToGEOLatLon(double B, double L, double dN, double dE, double &Bshift, double &Lshift);

};

#endif // CACHEPAINTER_H

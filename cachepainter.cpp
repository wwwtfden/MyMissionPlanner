#include "cachepainter.h"

#include <QPainter>
#include <QtMath>
#include <QFileInfo>
#include <QDebug>

#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>

#define radiusEarth (6378137.0) //радиус Земли для сферического Меркатора. Равен большему радиусу эллипсоида WGS-84

CachePainter::CachePainter(QString spifPath)
: roiWidth(0),
  roiHeight(0),
  layerZ(0),
  BCenter(0),
  LCenter(0),
  focalLength(0),
  pixelSize(0),
  cameraMatrixWidth(0),
  cameraMatrixHeight(0),
  Baircraft(0),
  Laircraft(0),
  Haircraft(0),
  roll(0),
  pitch(0),
  yaw(0){

    spifFile = new QSettings(spifPath, QSettings::IniFormat);
    roiImage = QImage();
}

// получить диапазон существующих слоев
void CachePainter::getLayersRange(int &minLayer, int &maxLayer) {
    minLayer = spifFile->value("Layers/First").toInt()+1; // минимальный слой частично ограничен
    maxLayer = spifFile->value("Layers/Last").toInt();
}

// получить координаты центра растра
void CachePainter::getCenterCache(double &BCenter, double &LCenter) {
    double Btl, Ltl, Bbr, Lbr;
    getCacheTopLeftCorner(Btl, Ltl);
    getCacheBottomRightCorner(Bbr, Lbr);
    BCenter = Bbr + (Btl - Bbr)/2;
    LCenter = Lbr + (Ltl - Lbr)/2;
}

// получить координаты углов
void CachePainter::getCacheTopLeftCorner(double &B, double &L) {
    B = spifFile->value("WGS84/latNW").toDouble();
    L = spifFile->value("WGS84/lonNW").toDouble();
}

void CachePainter::getCacheTopRightCorner(double &B, double &L) {
    B = spifFile->value("WGS84/latNE").toDouble();
    L = spifFile->value("WGS84/lonNE").toDouble();
}

void CachePainter::getCacheBottomLeftCorner(double &B, double &L) {
    B = spifFile->value("WGS84/latSW").toDouble();
    L = spifFile->value("WGS84/lonSW").toDouble();
}

void CachePainter::getCacheBottomRightCorner(double &B, double &L) {
    B = spifFile->value("WGS84/latSE").toDouble();
    L = spifFile->value("WGS84/lonSE").toDouble();
}

// задать и получить координаты центра Roi
void CachePainter::setCenterRoi(double BCenter, double LCenter) {
    this->BCenter = BCenter;
    this->LCenter = LCenter;
    updateRoiImage();
}

void CachePainter::getCenterRoi(double &BCenter, double &LCenter) {
    BCenter = this->BCenter;
    LCenter = this->LCenter;
}

void CachePainter::setRoiSize(int width, int height) {
    roiWidth = width;
    roiHeight = height;
    updateRoiImage();
}

void CachePainter::getRoiSize(int &width, int &height) {
    width = roiWidth;
    height = roiHeight;
}

// задать и получить номер текущего слоя
void CachePainter::setLayer(int layer) {
    int minLayer, maxLayer;
    getLayersRange(minLayer, maxLayer);
    if(layer >= minLayer && layer <= maxLayer)
        layerZ = layer;
    updateRoiImage();
}

void CachePainter::setLayer(int layer, int x, int y){
    int minLayer, maxLayer;
    getLayersRange(minLayer, maxLayer);
    if(layer >= minLayer && layer <= maxLayer)
    {
        double B, L;
        convertPixelsRoiToCoordinates(x, y, B, L);

        layerZ = layer;

        int tilesX, tilesY, picX, picY, flatX, flatY;
        GEOLatLonToXY(B, L, layerZ, tilesX, tilesY, picX, picY, flatX, flatY);

        flatX -= x - roiWidth/2;
        flatY -= y - roiHeight/2;

        tilesX = flatX / 256;
        tilesY = flatY / 256;

        picX = flatX - tilesX * 256;
        picY = flatY - tilesY * 256;

        double BCenter, LCenter;
        XYToGEOLatLon(tilesX, tilesY, picX, picY, layerZ, BCenter, LCenter);
        setCenterRoi(BCenter, LCenter);
    }
}

int CachePainter::getLayer() {
    return layerZ;
}

// получить текущее изображение, меняющееся в завистимости от действий пользователя
QImage CachePainter::getRoiImage() {
    return roiImage;
}

// конвертация пикселей Roi в геодезические координаты и обратно
void CachePainter::convertPixelsRoiToCoordinates(int x, int y, double &B, double &L) {
    int tilesX, tilesY, picX, picY, flatX, flatY;
    GEOLatLonToXY(BCenter, LCenter, layerZ, tilesX, tilesY, picX, picY, flatX, flatY);

    int flatTL_X = flatX - roiWidth/2;
    int flatTL_Y = flatY - roiHeight/2;

    int bm0 = 256 * qPow(2, layerZ-1) / 2;

    flatX = x + flatTL_X;
    flatY = y + flatTL_Y;

    double lonrad = M_PI * (flatX - bm0) / bm0;
    double c1 = 2 * M_PI * (bm0 - flatY) / bm0;
    double latrad = qAsin((qExp(c1) - 1) / (qExp(c1) + 1));

    B = qRadiansToDegrees(latrad);
    L = qRadiansToDegrees(lonrad);
}

void CachePainter::convertCoordinatesToPixelsRoi(double B, double L, int &x, int &y) {
    int tilesX, tilesY, picX, picY, flatX, flatY;
    GEOLatLonToXY(BCenter, LCenter, layerZ, tilesX, tilesY, picX, picY, flatX, flatY);

    int flatTL_X = flatX - roiWidth/2;
    int flatTL_Y = flatY - roiHeight/2;

    int bm0 = 256 * qPow(2, layerZ-1) / 2;

    double latrad = qDegreesToRadians(B);
    double lonrad = qDegreesToRadians(L);

    flatX = qRound(bm0 * (1 + lonrad / M_PI));
    flatY = qRound(bm0 * (1 - 0.5*qLn((1 + qSin(latrad)) / (1 - qSin(latrad))) / M_PI));

    x = flatX - flatTL_X;
    y = flatY - flatTL_Y;
}


// private slots:
QString CachePainter::xyzToTqrst(int x, int y, int z) {
    if(z<1 || z>32)
        return QString();

    QString str = "t";
    int mask = 1 << (z - 1);
    x %= mask;
    if(x < 0)
        x += mask;
    for(int i=2; i<=z; i++)
    {
        mask >>= 1;
        if(x & mask)
        {
            if(y & mask)
                str += "s";
            else
                str += "r";
        }
        else
        {
            if(y & mask)
                str += "t";
            else
                str += "q";
        }
    }
    return str;
}

void CachePainter::GEOLatLonToXY(double B, double L, int z, int &tilesX, int &tilesY, int &picX, int &picY, int &flatX, int &flatY) {
    int bm0 = 256 * qPow(2, z-1) / 2;

    double latrad = qDegreesToRadians(B);
    double lonrad = qDegreesToRadians(L);

    flatX = qRound(bm0 * (1 + lonrad / M_PI));
    flatY = qRound(bm0 * (1 - 0.5 * qLn((1 + qSin(latrad)) / (1 - qSin(latrad))) / M_PI));

    tilesX = flatX / 256;
    tilesY = flatY / 256;

    picX = flatX - tilesX * 256;
    picY = flatY - tilesY * 256;
}

void CachePainter::XYToGEOLatLon(int tilesX, int tilesY, int picX, int picY, int z, double &B, double &L) {
    int bm0 = 256 * qPow(2, z-1) / 2;

    int flatX = tilesX * 256 + picX;
    int flatY = tilesY * 256 + picY;

    double lonrad = M_PI * (flatX - bm0) / bm0;
    double c1 = 2 * M_PI * (bm0 - flatY) / bm0;
    double latrad = qAsin((qExp(c1) - 1) / (qExp(c1) + 1));

    B = qRadiansToDegrees(latrad);
    L = qRadiansToDegrees(lonrad);
}

void CachePainter::PseudoMercatorToGEOLatLon(double X, double Y, double &B, double &L){
    L = qRadiansToDegrees(X / radiusEarth);
    B = qRadiansToDegrees(2 * qAtan(qExp(Y / radiusEarth)) - M_PI_2);
}

void CachePainter::GEOLatLonToPseudoMercator(double B, double L, double &X, double &Y){
    X = radiusEarth * qDegreesToRadians(L);
    Y = radiusEarth * qLn(qTan(M_PI_4 + qDegreesToRadians(B)/2));
}

void CachePainter::calcShiftToGEOLatLon(double B, double L, double dN, double dE, double &Bshift, double &Lshift){
    double Brad = qDegreesToRadians(B);
    double Lrad = qDegreesToRadians(L);

    double X = radiusEarth * Lrad;
    double Y = radiusEarth * qLn(qTan(M_PI_4 + Brad/2));

    X *= qCos(Brad);
    Y *= qCos(Brad);

    X += dE;
    Y += dN;

    X /= qCos(Brad);
    Y /= qCos(Brad);

    Bshift = qRadiansToDegrees(2 * qAtan(qExp(Y / radiusEarth)) - M_PI_2);
    Lshift = qRadiansToDegrees(X / radiusEarth);
}

void CachePainter::updateRoiImage()
{
    if(roiWidth==0 || roiHeight==0 || layerZ==0 || BCenter==0 || LCenter==0)
        return;

    roiImage = QImage(roiWidth, roiHeight, QImage::Format_ARGB32_Premultiplied);
    roiImage.fill(Qt::transparent);

    int tilesX, tilesY, picX, picY, flatX, flatY;
    GEOLatLonToXY(BCenter, LCenter, layerZ, tilesX, tilesY, picX, picY, flatX, flatY);

    int flatTL_X = flatX - roiWidth/2;
    int flatTL_Y = flatY - roiHeight/2;

    int tilesTL_X = flatTL_X / 256;
    int tilesTL_Y = flatTL_Y / 256;

    int picTL_X = flatTL_X - tilesTL_X * 256;
    int picTL_Y = flatTL_Y - tilesTL_Y * 256;

    QPainter painter(&roiImage);
    QString cacheDirName = QFileInfo(spifFile->fileName()).absolutePath() + "/" + QFileInfo(spifFile->fileName()).baseName();
    for(int i = 0 ; i < roiWidth + picTL_X; i += 256)
    {
        for(int j = 0 ; j < roiHeight + picTL_Y; j += 256)
        {
            int xCur = tilesTL_X + i/256;
            int yCur = tilesTL_Y + j/256;
            QString tileFileName = cacheDirName + "/" + QString::number(layerZ).rightJustified(2,'0') + "/" + xyzToTqrst(xCur, yCur, layerZ); // + ".png"?
            QImage tileImage;
            if(tileCacheFileNames.contains(tileFileName))
            {
                int tileIndex =  tileCacheFileNames.indexOf(tileFileName);
                tileCacheFileNames.move(tileIndex, tileCacheFileNames.count()-1);
                tileCacheImages.move(tileIndex, tileCacheFileNames.count()-1);
                tileImage = tileCacheImages.last();
            }
            else
            {
                if(tileImage.load(tileFileName))
                {
                    tileCacheFileNames.append(tileFileName);
                    tileCacheImages.append(tileImage);
                    if(tileCacheFileNames.count() > maxCacheLength)
                    {
                        tileCacheFileNames.removeFirst();
                        tileCacheImages.removeFirst();
                    }
                }
            }
            if(!tileImage.isNull())
                painter.drawImage(i-picTL_X, j-picTL_Y, tileImage);
        }
    }
}

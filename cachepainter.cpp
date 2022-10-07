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

void CachePainter::setCameraParameters(double focalLength, double pixelSize, int cameraMatrixWidth, int cameraMatrixHeight)
{
    this->focalLength = focalLength;
    this->pixelSize = pixelSize;
    this->cameraMatrixWidth = cameraMatrixWidth;
    this->cameraMatrixHeight = cameraMatrixHeight;
}

void CachePainter::getCameraParameters(double &focalLength, double &pixelSize, int &cameraMatrixWidth, int &cameraMatrixHeight)
{
    focalLength = this->focalLength;
    pixelSize = this->pixelSize;
    cameraMatrixWidth = this->cameraMatrixWidth;
    cameraMatrixHeight = this->cameraMatrixHeight;
}

void CachePainter::setAircraftSpatialPosition(double B, double L, double H, double roll, double pitch, double yaw){
    this->Baircraft = B;
    this->Laircraft = L;
    this->Haircraft = H;
    this->roll = roll;
    this->pitch = pitch;
    this->yaw = yaw;
}

void CachePainter::getAircraftSpatialPosition(double &B, double &L, double &H, double &roll, double &pitch, double &yaw){
    B = this->Baircraft;
    L = this->Laircraft;
    H = this->Haircraft;
    roll = this->roll;
    pitch = this->pitch;
    yaw = this->yaw;
}

void CachePainter::getObservedQuadrilateral(quadrilateral &observedQuadrilateral){

    if(focalLength==0 || pixelSize==0 || cameraMatrixWidth==0 || cameraMatrixHeight==0)
        return;

    using namespace cv;

    double gamma = qDegreesToRadians(roll);  //крен
    double tetta = qDegreesToRadians(pitch); //танагаж
    double psi = qDegreesToRadians(yaw);     //курс

    double Rx[3][3], Rz[3][3], Ry[3][3];

    Rx[0][0] = 1; Rx[0][1] =            0; Rx[0][2] =           0;
    Rx[1][0] = 0; Rx[1][1] =  qCos(tetta); Rx[1][2] = qSin(tetta);
    Rx[2][0] = 0; Rx[2][1] = -qSin(tetta); Rx[2][2] = qCos(tetta);

    Rz[0][0] = qCos(psi);  Rz[0][1] = -qSin(psi);  Rz[0][2] = 0;
    Rz[1][0] = qSin(psi);  Rz[1][1] =  qCos(psi);  Rz[1][2] = 0;
    Rz[2][0] =         0;  Rz[2][1] =          0;  Rz[2][2] = 1;

    Ry[0][0] = qCos(gamma); Ry[0][1] = 0; Ry[0][2] = -qSin(gamma);
    Ry[1][0] =           0; Ry[1][1] = 1; Ry[1][2] =            0;
    Ry[2][0] = qSin(gamma); Ry[2][1] = 0; Ry[2][2] =  qCos(gamma);

    Mat M_Rx = Mat(3, 3, CV_64FC1, Rx);
    Mat M_Rz = Mat(3, 3, CV_64FC1, Rz);
    Mat M_Rxy = Mat(3, 3, CV_64FC1);
    Mat M_Ry = Mat(3, 3, CV_64FC1, Ry);
    Mat M_R = Mat(3, 3, CV_64FC1);

    M_Rxy = M_Rx * M_Ry;
    M_R = M_Rxy * M_Rz;

    Mat vx = M_R.col(0);
    Mat vy = M_R.col(1);
    Mat vz = M_R.col(2);

    double p0[3] = {                   0.0,                   0.0, -focalLength/pixelSize};
    double p1[3] = {-cameraMatrixWidth/2.0, -cameraMatrixHeight/2, -focalLength/pixelSize};
    double p2[3] = {-cameraMatrixWidth/2.0,  cameraMatrixHeight/2, -focalLength/pixelSize};
    double p3[3] = { cameraMatrixWidth/2.0,  cameraMatrixHeight/2, -focalLength/pixelSize};
    double p4[3] = { cameraMatrixWidth/2.0, -cameraMatrixHeight/2, -focalLength/pixelSize};

    // главная точка (пока полагаем, что она в центре)
    double C[3] = {0, 0, 0};
    Mat V_C = Mat(1, 3, CV_64FC1, C);

    // Координаты вершин при смещенной главной точкой на С от геометрического центра
    Mat V_p0 = Mat(1, 3, CV_64FC1, p0);
    Mat V_p1 = Mat(1, 3, CV_64FC1, p1);
    Mat V_p2 = Mat(1, 3, CV_64FC1, p2);
    Mat V_p3 = Mat(1, 3, CV_64FC1, p3);
    Mat V_p4 = Mat(1, 3, CV_64FC1, p4);

    V_p0 = V_p0;
    V_p1 = V_p1 - V_C;
    V_p2 = V_p2 - V_C;
    V_p3 = V_p3 - V_C;
    V_p4 = V_p4 - V_C;

    Mat vx0 = vx.t() * V_p0.t();
    Mat vx1 = vx.t() * V_p1.t();
    Mat vx2 = vx.t() * V_p2.t();
    Mat vx3 = vx.t() * V_p3.t();
    Mat vx4 = vx.t() * V_p4.t();

    Mat vy0 = vy.t() * V_p0.t();
    Mat vy1 = vy.t() * V_p1.t();
    Mat vy2 = vy.t() * V_p2.t();
    Mat vy3 = vy.t() * V_p3.t();
    Mat vy4 = vy.t() * V_p4.t();

    Mat vz0 = vz.t() * V_p0.t();
    Mat vz1 = vz.t() * V_p1.t();
    Mat vz2 = vz.t() * V_p2.t();
    Mat vz3 = vz.t() * V_p3.t();
    Mat vz4 = vz.t() * V_p4.t();

    // Координаты центральной проекции в плане углов снимка в м
    double P0[3] = {-focalLength/pixelSize * vx0.at<double>(0, 0) / vz0.at<double>(0, 0), -focalLength/pixelSize * vy0.at<double>(0, 0) / vz0.at<double>(0, 0), -focalLength/pixelSize};
    double P1[3] = {-focalLength/pixelSize * vx1.at<double>(0, 0) / vz1.at<double>(0, 0), -focalLength/pixelSize * vy1.at<double>(0, 0) / vz1.at<double>(0, 0), -focalLength/pixelSize};
    double P2[3] = {-focalLength/pixelSize * vx2.at<double>(0, 0) / vz2.at<double>(0, 0), -focalLength/pixelSize * vy2.at<double>(0, 0) / vz2.at<double>(0, 0), -focalLength/pixelSize};
    double P3[3] = {-focalLength/pixelSize * vx3.at<double>(0, 0) / vz3.at<double>(0, 0), -focalLength/pixelSize * vy3.at<double>(0, 0) / vz3.at<double>(0, 0), -focalLength/pixelSize};
    double P4[3] = {-focalLength/pixelSize * vx4.at<double>(0, 0) / vz4.at<double>(0, 0), -focalLength/pixelSize * vy4.at<double>(0, 0) / vz4.at<double>(0, 0), -focalLength/pixelSize};

    Mat V_P0 = Mat(1, 3, CV_64FC1, P0);
    Mat V_P1 = Mat(1, 3, CV_64FC1, P1);
    Mat V_P2 = Mat(1, 3, CV_64FC1, P2);
    Mat V_P3 = Mat(1, 3, CV_64FC1, P3);
    Mat V_P4 = Mat(1, 3, CV_64FC1, P4);

    V_P0 = V_P0.t();
    V_P1 = V_P1.t();
    V_P2 = V_P2.t();
    V_P3 = V_P3.t();
    V_P4 = V_P4.t();

    double resolution = Haircraft * pixelSize / focalLength;

    observedQuadrilateral.x0 = V_P0.at<double>(0,0) * resolution;
    observedQuadrilateral.y0 = V_P0.at<double>(1,0) * resolution;

    observedQuadrilateral.x1 = V_P1.at<double>(0,0) * resolution;
    observedQuadrilateral.y1 = V_P1.at<double>(1,0) * resolution;

    observedQuadrilateral.x2 = V_P2.at<double>(0,0) * resolution;
    observedQuadrilateral.y2 = V_P2.at<double>(1,0) * resolution;

    observedQuadrilateral.x3 = V_P3.at<double>(0,0) * resolution;
    observedQuadrilateral.y3 = V_P3.at<double>(1,0) * resolution;

    observedQuadrilateral.x4 = V_P4.at<double>(0,0) * resolution;
    observedQuadrilateral.y4 = V_P4.at<double>(1,0) * resolution;
}

QImage CachePainter::getObservedImage(){

    if(focalLength==0 || pixelSize==0 || cameraMatrixWidth==0 || cameraMatrixHeight==0)
        return QImage();

    quadrilateral observedQuadrilateral;
    getObservedQuadrilateral(observedQuadrilateral);

    double B[5], L[5];
    calcShiftToGEOLatLon(Baircraft, Laircraft, observedQuadrilateral.y0, observedQuadrilateral.x0, B[0], L[0]);
    calcShiftToGEOLatLon(Baircraft, Laircraft, observedQuadrilateral.y1, observedQuadrilateral.x1, B[1], L[1]);
    calcShiftToGEOLatLon(Baircraft, Laircraft, observedQuadrilateral.y2, observedQuadrilateral.x2, B[2], L[2]);
    calcShiftToGEOLatLon(Baircraft, Laircraft, observedQuadrilateral.y3, observedQuadrilateral.x3, B[3], L[3]);
    calcShiftToGEOLatLon(Baircraft, Laircraft, observedQuadrilateral.y4, observedQuadrilateral.x4, B[4], L[4]);

    int tilesX[5], tilesY[5];
    int picX[5], picY[5];
    int flatX[5], flatY[5];

    int minLayer, maxLayer;
    getLayersRange(minLayer, maxLayer);
    int z;
    int maxCameraSize = qMax(cameraMatrixWidth, cameraMatrixHeight);
    for(z = minLayer; z <= maxLayer; z++)
    {
        for(int i=0; i<5; i++)
            GEOLatLonToXY(B[i], L[i], z, tilesX[i], tilesY[i], picX[i], picY[i], flatX[i], flatY[i]);
        double l12 = QLineF(flatX[1], flatY[1], flatX[2], flatY[2]).length();
        double l23 = QLineF(flatX[2], flatY[2], flatX[3], flatY[3]).length();
        double l34 = QLineF(flatX[3], flatY[3], flatX[4], flatY[4]).length();
        double l14 = QLineF(flatX[1], flatY[1], flatX[4], flatY[4]).length();

        if(l12>maxCameraSize && l23>maxCameraSize && l34>maxCameraSize && l14>maxCameraSize) //как оптимизировать?
            break;
    }

    int minTilesX = tilesX[0];
    int maxTilesX = tilesX[0];
    int minTilesY = tilesY[0];
    int maxTilesY = tilesY[0];
    for(int i=1; i<5; i++)
    {
        minTilesX = qMin(minTilesX, tilesX[i]);
        maxTilesX = qMax(maxTilesX, tilesX[i]);
        minTilesY = qMin(minTilesY, tilesY[i]);
        maxTilesY = qMax(maxTilesY, tilesY[i]);
    }

    if((maxTilesX - minTilesX) * (maxTilesY - minTilesY) < maxTileCount) //ограничим max. размер
    {
        QImage img((maxTilesX - minTilesX + 1)*256, (maxTilesY - minTilesY + 1)*256, QImage::Format_RGB888);
        img.fill(Qt::white);
        QPainter p(&img);
        QString cacheDirName = QFileInfo(spifFile->fileName()).absolutePath() + "/" + QFileInfo(spifFile->fileName()).baseName();

        for(int i=minTilesX; i<=maxTilesX; i++)
        {
            for(int j=minTilesY; j<=maxTilesY; j++)
            {
                QString tileFileName = cacheDirName + "/" + QString::number(z).rightJustified(2,'0') + "/" + xyzToTqrst(i, j, z); // + ".png"?
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
                    p.drawImage((i-minTilesX)*256, (j-minTilesY)*256, tileImage);
            }
        }

        using namespace cv;
        Point2f srcQuad[4], dstQuad[4];

        int minFlatX = minTilesX * 256;
        int minFlatY = minTilesY * 256;

        srcQuad[0].x = flatX[1] - minFlatX;
        srcQuad[0].y = flatY[1] - minFlatY;

        srcQuad[1].x = flatX[2] - minFlatX;
        srcQuad[1].y = flatY[2] - minFlatY;

        srcQuad[2].x = flatX[3] - minFlatX;
        srcQuad[2].y = flatY[3] - minFlatY;

        srcQuad[3].x = flatX[4] - minFlatX;
        srcQuad[3].y = flatY[4] - minFlatY;

        dstQuad[0].x = 0;
        dstQuad[0].y = cameraMatrixHeight-1;

        dstQuad[1].x = 0;
        dstQuad[1].y = 0;

        dstQuad[2].x = cameraMatrixWidth - 1;
        dstQuad[2].y = 0;

        dstQuad[3].x = cameraMatrixWidth - 1;
        dstQuad[3].y = cameraMatrixHeight - 1;

        Mat lambda(3, 3, CV_32FC1);
        lambda = getPerspectiveTransform(srcQuad,  dstQuad);

        Mat src(img.height(), img.width(), CV_8UC3, (uchar*)img.bits(), img.bytesPerLine());
        cvtColor(src, src, CV_RGB2BGR);
        Mat dst(cameraMatrixHeight, cameraMatrixWidth, CV_8UC3);

        warpPerspective(src, dst, lambda, dst.size());

        QImage imgPerspective((uchar*)dst.data, dst.cols, dst.rows, static_cast<int>(dst.step), QImage::Format_RGB888);

        return imgPerspective;
    }

    QImage imgSky(cameraMatrixWidth, cameraMatrixWidth, QImage::Format_RGB888);
    imgSky.fill(QColor(117, 187, 253));
    return imgSky;
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

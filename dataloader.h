#ifndef DATALOADER_H
#define DATALOADER_H
#include <QtConcurrent/QtConcurrent>
#include <QCoreApplication>
#include <QApplication>
#include "ZS2_Decoder.h"


class dataLoader {

public:
    dataLoader(QString path);
    dataLoader(QFile file);

    QMap<QString, QVector<QPointF>*> y_dataMap;
    QMap<QString, QVector<qreal>*> x_dataMap;
    QString haptic_serie = "";
    QMap<QString, QPointF> hapticPoints;
    QMap<QString, int> hapticPointsRow;

    QList<QString> series_names;
    QList<QVector<QPointF>> series_data;
    QString X_name;
    QMap <QString, int> Y_axes_series_correlation;


    void loadLabViewCsv();

    enum dataType{MFU, CC, Durability_robot, Durability_zwick, LabView, Haptic, BLF_Export, Zwick, KWM, SYSTEC, Undefined};

    QString path;
    QString serieName;
    QString folder;
    bool good;

    dataType type;
    dataType checkDataType(QString file);

    int parseLabViewLog(QString path);

    int parseRobotLog(QString path);
    int parseBLFExport(QString path);
    int parseZwickFile(QString path);
    int parseKWM(QString path);
    int parseSYSTEC(QString path);

    void findHapticPoints();

    qreal offset {0.0};
    int approachrow = 0;
    bool append_Mode = false;
    double initial_time = 0;

private:

    qreal  fast_atof(const char* num, bool* ok) ;
    qreal  pow10(int n);


};











#endif // DATALOADER_H

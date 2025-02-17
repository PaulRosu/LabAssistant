#ifndef ZS2_H
#define ZS2_H

#include "qcompressor.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QRegularExpression>

#include <algorithm>
#include <numeric>
#include <iterator>




class MeasSpecimen{
public:
    QString name = "";
    QString username = "";
    uint samples = 0;

    float differential_factor = 0.25;

    float approachDistance = 0;
    int approachRow = 0;

    QList<uint> InterestPointsList;
    QList<qreal> IP_TravelList;
    QList<qreal> IP_ForceList;

    QList<uint> SW1_state;
    QList<uint> SW1_row;
    QList<uint> SW2_state;
    QList<uint> SW2_row;
    int maxForce_row = 0;
    int BMW_F3_row = 0;

    QList<QString> sequence;

    QVector<double> time;
    QVector<float> force;
    QVector<float> travel;
    QVector<uint> contact1;
    QVector<uint> cycle;

};

class MeasSerie{
public:

QString folder;
//QMap<QString, MeasSpecimen> specimenList;
QList<MeasSpecimen> specimenList;
QList<QString> cycles;
bool status;
QString name;
QString fileName;


};

MeasSerie loadData(QString fileName);
//void findInterestPoints (MeasSpecimen *specimen, MeasSerie *serie);
void findInterestPoints ( std::shared_ptr<MeasSpecimen> specimen, MeasSerie *serie);
void saveCSV(MeasSpecimen *specimen, MeasSerie *serie);

#endif

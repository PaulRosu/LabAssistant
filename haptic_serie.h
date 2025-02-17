#ifndef HAPTIC_SERIE_H
#define HAPTIC_SERIE_H

#include <QtCharts>
#include <QtWidgets/QGraphicsView>
#include "callout.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QTChart;
QT_END_NAMESPACE

class Callout;

QT_USE_NAMESPACE

class hapticSerie : public QScatterSeries
{
    Q_OBJECT

  public:
    //    hapticSerie(QtCharts::QChart *parent = 0);
    explicit hapticSerie(QObject *parent = nullptr);

    QMap<QString, QPointF> hapticPoints;
    QMap<QString, int> hapticPointsRow;

    QMap<QString, QVector<QPointF> *> robotData;

    QList<Callout *> callouts;

    int load_robottxt_opt(QString path);

    void insertTooltip(QPointF point, QString caption, QColor color);
    void findHapticPoints();
    qreal fast_atof(const char *num, bool *ok);
    qreal pow10(int n);
    qreal offset{0.0};
    int approachrow = 0;
};

#endif // HAPTIC_SERIE_H

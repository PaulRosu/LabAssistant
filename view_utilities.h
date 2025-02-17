#pragma once
#include <QtCharts>

class View;  // Forward declaration

void hideUnusedAxes(QChart *chart);
void adjustAxesRange(QChart *chart);
void calculateMovingAverage(QScatterSeries *current_activ,
                            QLineSeries *current_sleep_avg);
QString stringBetween(const QString &source, const QString &start, const QString &end); 
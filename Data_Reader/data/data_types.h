#ifndef DATA_TYPES_H
#define DATA_TYPES_H
#include "dbc_parser/dbciterator.hpp"
#include <QtCore>
class DataSerie
{
public:
  QList<QPointF> points;
  bool checked;
  QString name;
  QString source;
  double minX, minY, maxX, maxY;
  static const double secondsInADay;
  double offset = -1.0;
  QHash<QString, double> operatingModes;
  bool isOperatingModeSeries = false; 

  DataSerie() : checked(false)
  {
    points = *new QList<QPointF>;
  }

  void addPoint(QPointF point)
  {
    points.append(point);
  }

    void addOperatingModePoint(const QString& modeName, double timestamp)
  {
      if (!operatingModes.contains(modeName)) {
          operatingModes[modeName] = operatingModes.size();
      }
      points.append(QPointF(timestamp, operatingModes[modeName]));
  }

  void clear()
  {
    points.clear();
    points.squeeze();
    operatingModes.clear(); 
  }
};

class DataCollection
{
private:
  QList<DataSerie> series;
  QHash<unsigned int, BusMessage> *DBC;

public:
};

#endif // DATA_TYPES_H

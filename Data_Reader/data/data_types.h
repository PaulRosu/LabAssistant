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
  double offset              = -1.0;

  // Constructor
  DataSerie() : checked(false)
  {
      points = *new QList<QPointF>;
      //        points->reserve(100000);
    }

    void addPoint(QPointF point)
    {

      points.append(point);
    }

    void clear()
    {
        points.clear();
        points.squeeze();
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

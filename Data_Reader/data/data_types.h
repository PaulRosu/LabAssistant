#ifndef DATA_TYPES_H
#define DATA_TYPES_H
#include "dbc_parser/dbciterator.hpp"
#include <QtCore>

// Define data types enum
enum class DataType {
  Numeric,        // Regular numeric data
  OperatingMode,  // String data representing operating modes
  Error,          // String data representing error events
  DateTime,       // Date/time data
  NotUsed,        // Data that should be skipped/not used
  Unknown         // Default/unknown type
};

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
  DataType dataType = DataType::Unknown;  // Default to unknown type

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

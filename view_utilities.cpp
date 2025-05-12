#include "view_utilities.h"
#include "view.h"

extern QString stringBetween(const QString &source, const QString &start,
                             const QString &end);

void hideUnusedAxes(QChart *chart) {
  // Check for nullptr just in case
  if (!chart)
    return;

  // Lists to keep track of axes being used
  QList<QAbstractAxis *> usedXAxes;
  QList<QAbstractAxis *> usedYAxes;

  // Iterate over all series to find used axes
  for (auto series : chart->series()) {
    // Check for associated axes and add them to the list if not already present
    for (auto axis : chart->axes(Qt::Horizontal, series)) {
      if (!usedXAxes.contains(axis)) {
        usedXAxes.append(axis);
      }
    }
    for (auto axis : chart->axes(Qt::Vertical, series)) {
      if (!usedYAxes.contains(axis)) {
        usedYAxes.append(axis);
      }
    }
  }

  // Now, iterate over all axes in the chart and hide unused ones
  // but also show the used ones
  for (auto axis : chart->axes()) {
    if (qobject_cast<QValueAxis *>(axis)) {
      if (usedXAxes.contains(axis) || usedYAxes.contains(axis)) {
        axis->show();
      } else {
        axis->hide();
      }
    }
  }
  // for (auto axis : chart->axes()) {
  //   if (axis->orientation() == Qt::Horizontal && !usedXAxes.contains(axis)) {
  //     axis->hide();
  //   } else if (axis->orientation() == Qt::Vertical &&
  //              !usedYAxes.contains(axis)) {
  //     axis->hide();
  //   }
  // }

}

void adjustAxesRange(QChart *chart) {
  // Temporary variables to hold the min and max values for each axis
  QHash<QValueAxis *, QPair<qreal, qreal>> axisRange;

  // Initialize the map with maximum and minimum possible values
  for (QAbstractAxis *axis : chart->axes()) {
    QValueAxis *valueAxis = qobject_cast<QValueAxis *>(axis);
    if (valueAxis) {
      axisRange[valueAxis] = qMakePair(std::numeric_limits<qreal>::max(),
                                       std::numeric_limits<qreal>::min());
    }
  }

  // Iterate over all series to find min/max for each axis
  for (QAbstractSeries *series : chart->series()) {
    // Get the attached axes
    QList<QAbstractAxis *> attachedAxes = series->attachedAxes();
    for (QAbstractAxis *axis : attachedAxes) {
      QValueAxis *valueAxis = qobject_cast<QValueAxis *>(axis);
      if (valueAxis) {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);
        // Iterate over all points in the series
        for (const QPointF &point : xySeries->points()) {
          // Update min and max for the axis
          auto &[min, max] = axisRange[valueAxis];
          min = qMin(min, valueAxis->orientation() == Qt::Vertical ? point.y()
                                                                   : point.x());
          max = qMax(max, valueAxis->orientation() == Qt::Vertical ? point.y()
                                                                   : point.x());
        }
      }
    }
  }

  // Now, set the min and max for each axis
  for (auto it = axisRange.begin(); it != axisRange.end(); ++it) {
    QValueAxis *axis = it.key();
    const auto &[min, max] = it.value();
    axis->setRange(min, max);
    qDebug() << axis->objectName() << min << max;
  }
}

QValueAxis* View::constructValueAxis(QString name, QString title,
                                   QString format, QColor color,
                                   Qt::AlignmentFlag alignment) {
  Q_UNUSED(title);

  QValueAxis *axis = nullptr;
  // for (auto existing_axis : m_chart->axes()) {
  //   if (existing_axis->objectName() == name)
  //     axis = qobject_cast<QValueAxis *>(existing_axis);
  // }

  // if (axis == nullptr)
  axis = new QValueAxis(m_chart);
  axis->setObjectName(name);
  axis->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis->setLinePen(QPen(color, 3, Qt::SolidLine));
  axis->setMinorTickCount(1);
  axis->setTickCount(11);
  // if (name != "axisX")
  axis->setLabelsColor(color);
  // axis->setGridLineColor(color);
  // axis->setMinorGridLineColor(color);
  // axis->setTitleText(title);
  // axis->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis->setLabelFormat(format); // QString::fromLatin1("°C")).toUtf8()
  axis->setLabelsEditable(true);
  m_chart->addAxis(axis, alignment);
  return axis;
}

void calculateMovingAverage(QScatterSeries *current_activ,
                            QLineSeries *current_sleep_avg) {
  const double windowSizeDays = 0.041 / 86400; // 1 second expressed in days

  if (!current_activ || current_activ->count() == 0)
    return;

  QList<QPointF> points = current_activ->points();
  QList<QPointF> newPoints; // List to hold the averaged points
  double windowSum = 0;
  int windowCount = 0;
  int startIdx = 0;

  for (int i = 0; i < points.size(); ++i) {
    // Add the current point to the window
    windowSum += points[i].y();
    ++windowCount;

    // Check if the window extends beyond 1 second
    while (points[i].x() - points[startIdx].x() > windowSizeDays) {
      windowSum -= points[startIdx].y();
      --windowCount;
      ++startIdx;
    }

    // Calculate the average and add to the newPoints list if the window has
    // data
    if (windowCount > 0) {
      double avg = windowSum / windowCount;
      double midPointTime = (points[startIdx].x() + points[i].x()) /
                            2; // midpoint of the time window for plotting
      newPoints.append(QPointF(midPointTime, avg));
    }
  }

  // Replace all points in current_sleep_avg at once
  current_sleep_avg->replace(newPoints);
}



// view class methods
void View::handleCosmeticReset() { qDebug() << "handling cosmetic reset!"; }

void View::resetView() {
  for (auto axis : m_chart->axes()) {
    m_chart->removeAxis(axis);
    axis->disconnect();
    axis->deleteLater();
  }
  for (auto serie : m_chart->series()) {
    m_chart->removeSeries(serie);
    serie->deleteLater();
  }
  QApplication::processEvents();
}

// Implementation of pow10 function
// This function calculates 10 raised to the power of x (10^x)
double pow10(double x) {
    return std::pow(10.0, x);
}


double View::niceTickRange(double range, int tickCount) {
  double unroundedTickSize = range / (tickCount - 1);
  double x = ceil(log10(unroundedTickSize) - 1);
  double pow10x = pow10(x);
  double roundedTickRange = ceil(unroundedTickSize / pow10x) * pow10x;
  return roundedTickRange;
}


QString View::engineering_Format(double value) {
  QString unit;

  value = qAbs(value);

  if (value >= 3000000) {
    value /= 1000000;
    unit = QChar(0x004D); // "M"
  } else if (value >= 3000) {
    value /= 1000;
    unit = QChar(0x004B); // "K"
  } else if (value >= 3) {
    // No unit needed
  } else if (value >= 0.003) {
    value *= 1000;
    unit = QChar(0x006D); // "m"
  } else if (value >= 0.000003) {
    value *= 1000000;
    unit = QChar(0x00B5); // "µ"
  } else {
    value *= 1000000000;
    unit = QChar(0x006E); // "n"
  }

  return QString::asprintf("%4.1f%s", value, qPrintable(unit));
}



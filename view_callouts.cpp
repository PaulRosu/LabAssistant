#include "view.h"
class View;



void View::keepCallout() {
  if (sender() != nullptr) {
    QScatterSeries *serie = qobject_cast<QScatterSeries *>(sender());
    intPoints[serie->name()].append(m_tooltip);
  } else {
    m_callouts.append(m_tooltip);
  }
  m_tooltip = new Callout(m_chart);

  View::update();
}

void View::tooltip(QPointF point, bool state) {
  // *** QAbstractSeries *series = qobject_cast<QAbstractSeries *>(sender());
  if (m_tooltip == 0)
    m_tooltip = new Callout(m_chart);

  if (state) {

    QFont bold("Verdana", 12);
    bold.setBold(true);
    m_tooltip->m_font = bold;
    m_tooltip->color = Qt::red;

    m_tooltip->setOffset(QPoint(10, -50));
    //        m_tooltip->setText(QString("F: %1 \nS: %2")
    //                           .arg(QString::number(point.y(), 'g', 3 ))
    //                           .arg(QString::number(point.x(), 'g', 3 )));
    char force[40];
    char travel[40];
    char position[40];
    bool has_position = false;
    sprintf(force, "%.3f", point.y());
    sprintf(travel, "%.3f", point.x());

    for (auto serie : m_chart->series()) {
      if (serie->name().contains("position")) {

        QScatterSeries *s_serie = qobject_cast<QScatterSeries *>(serie);
        for (auto p_point : s_serie->points()) {
          if (p_point.x() >= point.x()) {
            sprintf(position, "%.1f", p_point.y());
            has_position = true;
            break;
          }
        }
      }
      if (has_position)
        break;
    }

    if (has_position) {
      m_tooltip->setText(QString("F: %1 \nS: %2 \npos: %3\%")
                             .arg(force)
                             .arg(travel)
                             .arg(position));
    } else {
      m_tooltip->setText(QString("F: %1 \nS: %2").arg(force).arg(travel));
    }

    // qDebug() << QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y());
    m_tooltip->setAnchor(point);
    m_tooltip->setZValue(1);
    m_tooltip->updateGeometry();

    m_tooltip->show();
    m_tooltip->selected = false;
  } else {
    m_tooltip->hide();
    View::m_chart->update();
  }

  View::update();
}

void View::tooltip_el(QPointF point, bool state, bool swstate,
                      QString caption) {
  if (m_tooltip == 0)
    m_tooltip = new Callout(m_chart);

  if (state) {

    QFont bold("Verdana", 10);
    bold.setBold(true);
    m_tooltip->m_font = bold;

    if (swstate) {
      m_tooltip->color = Qt::green;

      m_tooltip->setOffset(QPoint(10, -50));
      m_tooltip->setText(
          QString(caption + "\nS: %1 \nF: %2 ").arg(point.x()).arg(point.y()));
    } else {
      m_tooltip->color = Qt::black;

      m_tooltip->setOffset(QPoint(10, -50));
      m_tooltip->setText(
          QString(caption + "\nS: %1 \nF: %2 ").arg(point.x()).arg(point.y()));
    }

    // qDebug() << QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y());
    m_tooltip->setAnchor(point);
    m_tooltip->setZValue(1);
    m_tooltip->updateGeometry();
    m_tooltip->show();
    m_tooltip->selected = false;
  } else {
    m_tooltip->hide();
    View::m_chart->update();
  }

  View::update();
}

void View::tooltip_ex(QPointF point, bool state, QColor color, QString caption) {

  if (m_tooltip == nullptr) {
    m_tooltip = new Callout(m_chart);
  }

  if (state) {

    QFont bold("Verdana", 10);
    bold.setBold(true);
    m_tooltip->m_font = bold;

    m_tooltip->color = color;
    m_tooltip->setOffset(QPoint(10, -50));
    //            m_tooltip->setText(QString(caption + "\nS: %1 \nF: %2
    //            ").arg(point.x()).arg(point.y()));

    m_tooltip->setText(QString(caption + "\nF: %1 \nS: %2")
                           .arg(QString::number(point.y(), 'f', 3))
                           .arg(QString::number(point.x(), 'f', 3)));

    // qDebug() << QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y());
    m_tooltip->setAnchor(point);
    m_tooltip->setZValue(1);
    m_tooltip->updateGeometry();
    m_tooltip->show();
    m_tooltip->selected = false;
  } else {
    m_tooltip->hide();
    View::m_chart->update();
  }

  View::update();
}

void View::actionToggleCallouts() {
  qDebug() << "actionToggleCallouts";
  for (auto serie : m_chart->series()) {
    //        if(!serie->isVisible ()) continue;
    for (Callout *callout : intPoints[serie->name()]) {
      if (callout->in_view) {
        if (serie->isVisible())
          callout->setVisible(!callout->isVisible());
      }
      callout->supressed = !callout->supressed;
    }
  }
  updateCallouts();
}


void View::updateCallouts() {

  for (auto serie : m_chart->series()) {
    //        qDebug() << intPoints << serie->name();
    if (serie->isVisible())
      for (Callout *callout : intPoints[serie->name()])
        callout->updateGeometry();
  }
}

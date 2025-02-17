/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef VIEW_H
#define VIEW_H
#include <QtCharts>
#include <QtWidgets/QGraphicsView>
// #include <argvars.h>
#include "dataloader.h"
#include "haptic_serie.h"

#include "dataloader.h"
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>
// #include <QtCharts/QChart>
// #include <QtCharts/QLineSeries>
// #include <QtCharts/QSplineSeries>
// #include <QtCharts/QScatterSeries>
#include "ZS2_Decoder.h"
#include "callout.h"
#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QGraphicsTextItem>

#include <QtCharts>
// #include <QtCharts/QChartView>
#include <QApplication>
#include <QChartView>
#include <QFutureWatcher>
#include <QtConcurrent>

#include "data/data.h"
#include "haptic_serie.h"
// #include "view_utilities.cpp"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QChart;
QT_END_NAMESPACE

extern QString fileNameArg;
extern bool argFile;
extern bool labViewEval;
extern dataLoader *tempDataLoader;

class Callout;

QT_USE_NAMESPACE

// QMap<QString, QPointF> findHapticPoints ( hapticSerie * serie);

class View : public QGraphicsView {
  Q_OBJECT

public:
  View(QWidget *parent = 0, QString folder = "");

  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void resetView();
  QString serieName;
  QString folder;
  QString path;
  int exportNr = 0;
  bool cutMode = false;
  bool is_durability = false;
  bool is_robot = false;

  QChart *m_chart = new QChart();

  Data data = Data();

  QMap<QString, QList<Callout *>> intPoints;

  QValueAxis *axis_errors = nullptr;
  QValueAxis *axis_temperature = nullptr;
  QValueAxis *axis_humidity = nullptr;
  QValueAxis *axisX = nullptr;
  QValueAxis *axisX2 = nullptr;
  QValueAxis *axis_current = nullptr;
  QValueAxis *axis_sleep_current = nullptr;
  QValueAxis *axis_voltage = nullptr;
  QValueAxis *axis_opmode = nullptr;
  QValueAxis *axis_durability_signals = nullptr;

  QValueAxis *axis_force = nullptr;
  QValueAxis *axis_travel = nullptr;
  dataLoader *data_loader = nullptr;

  bool tooltip_moving = false;
  bool rmb_clicked = false;
  bool mid_clicked = false;

  QString engineering_Format(double value);
  void exportDurability();
  void exportRobot();

  int loadCSV_opt(QString path);

  QLineSeries *temperature = new QLineSeries(this);
  QLineSeries *temperature_sv = new QLineSeries(this);
  QLineSeries *humidity = new QLineSeries(this);
  QLineSeries *humidity_sv = new QLineSeries(this);
  QLineSeries *voltage = new QLineSeries(this);
  QScatterSeries *current_activ = new QScatterSeries(this);
  QScatterSeries *current_sleep = new QScatterSeries(this);
  QScatterSeries *current_KL30C = new QScatterSeries(this);
  QLineSeries *OM = new QLineSeries(this);

  QString OM1_name;
  QString OM2_name;
  QString OM3_name;

  // QMap<QString, QScatterSeries*> AS_volts;
  QMap<QString, QVector<QPointF> *> AS_volts;
  QMap<QString, QVector<QPointF> *> AS_currents;

  QMap<QString, QVector<QPointF> *> errors;
  QMap<QString, QVector<QPointF> *> durability_errors;
  QMap<QString, QVector<QPointF> *> durability_signals;
  QMap<QValueAxis *, QPointF> initialScale;

  static QMap<QString, QColor> m_colors;

  double t_max = -33;
  double t_min = -33;
  double u_max = -33;
  double u_min = -33;
  bool ranged = false;
  bool good = false;

  void checkAndCreateIniFileWithUUID(const QString &UUID);

  ~View() = default;

protected:
  void resizeEvent(QResizeEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

  double pow10(int n);
  double niceTickRange(double range, int tickCount);
  void adjustAxisRange(QValueAxis *axis);

  int translateOM(QString om);

  double fast_atof(const char *num);

  bool isGraphtec(QString path);
  bool isIllum(QString path);

  QRubberBand *rubberBand = nullptr;
  QPoint rband_origin;

  void construct_MFU_chart(QString path);
  void construct_haptic_chart(QString path);
  void construct_haptic_chart_l(QString path);
  void construct_LabView_Chart(QString path);
  void construct_LabView_Chart_eval(QString path);
  void construct_ParamChart(QString path);
  void construct_BLF_export();
  void constructDataChart();

  void construct_robot_haptic_chart(QString path);
  int load_robottxt_opt(QString path);

  void init_colors();

  bool released_from_item = false;

public slots:
  void keepCallout();
  void updateCallouts();
  void tooltip(QPointF point, bool state);
  void tooltip_el(QPointF point, bool state, bool swstate, QString caption);
  void tooltip_ex(QPointF point, bool state, QColor color, QString caption);
  void handleMarkerClicked();
  void handleMarkerToggle(QLegendMarker *marker);
  void handleTimeRangeChanged(qreal min, qreal max);
  void handleTime2RangeChanged(qreal min, qreal max);
  void handleOpMRangeChanged(qreal min, qreal max);
  void handleSerieClick(QPointF point);
  void handleSerieRelease(QPointF point);
  void setScaleAxis(bool state);
  void actionTitleEdit();
  void actionOpmEdit();
  void adjustAxisFormat(QValueAxis *axis, double min, double max);

  void savePNG(QString name = "Export"); // serieName
  void exportAction();
  void applyNiceNumbers();
  void tenTicks();
  void proc_rangeChanged(qreal min, qreal max);
  void cutGraph(double start, double stop);
  void toggleLegendMarker(QString name, bool state);
  void actionToggleCallouts();

  void actionShowDataManager();

  void handleParsingProgress(int percent, QString name);
  void handleCosmeticReset();

public:
signals:
  void fileDropped(QString filePath);
  void rangeChanged(qreal min, qreal max, QString name);
  void saved();
  void propertyChanged();
  void I_cut_my_graph(double start, double end);
  void markerToggled(QString name);
  void mouseEvent(QString button, bool state);
  void dataChanged();

public:
  bool userInteracted = false;
  bool constructionInProgress = false;
  bool templateSettingsFound = false;

private:
  QGraphicsSimpleTextItem *m_coordX = new QGraphicsSimpleTextItem(m_chart);
  QGraphicsSimpleTextItem *m_coordY = new QGraphicsSimpleTextItem(m_chart);
  QValueAxis *constructValueAxis(QString name, QString title, QString format,
                                 QColor color, Qt::AlignmentFlag alignment);
  void constructSerie(QList<QPointF> &points, QString name, QString symbol,
                      QColor color, QValueAxis *axis);
  Callout *m_tooltip;
  QList<Callout *> m_callouts;

  QPointF m_lastMousePos;
  float origXMax;
  float origYMax;

  friend void hideUnusedAxes(QChart *chart);
  friend void adjustAxesRange(QChart *chart);
};

#endif

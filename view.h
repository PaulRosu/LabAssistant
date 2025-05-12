
#ifndef VIEW_H
#define VIEW_H
#include <QtCharts>
#include <QtWidgets/QGraphicsView>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>

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

  QValueAxis *axisX = nullptr;
  QValueAxis *axisX2 = nullptr;


  QChart *m_chart = new QChart();

  Data data = Data();

  QMap<QString, QList<Callout *>> intPoints;

  bool tooltip_moving = false;
  bool rmb_clicked = false;
  bool mid_clicked = false;

  QString engineering_Format(double value);

  QMap<QValueAxis *, QPointF> initialScale;

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

  double niceTickRange(double range, int tickCount);
  void adjustAxisRange(QValueAxis *axis);


  QRubberBand *rubberBand = nullptr;
  QPoint rband_origin;

  void constructDataChart();

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSvgWidget>

// #include "argvars.h"
// #include "ZS2_Decoder.h"
#include "callout.h"
#include "view.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);

  QMap<QString, QMap<QString, double>> statistics;

  QSvgWidget *svg_widget = nullptr;
  QString mouse_svg;

  struct Tolerances {
    double min_factor;
    double max_factor;
    double min_offset;
    double max_offset;
  } Tolerance;
  QList<QWidget *> tolerance_widgets;

public slots:

  void calloutsActionVisibility(int index);
  void messageHandler(quint32 instanceId, QByteArray message);

  void closeTabHandler(int index);
  void current_NiceNumbers();
  void current_ScaleAxis(bool state);
  void current_NamedSave();
  void current_ChangeOpMNames();

  bool addTab(QString file_path);
  void graphRangeChanged(qreal min, qreal max, QString name);
  void axes_link(bool state);
  void graph_saved();
  void graphPropertyChanged();
  void refreshStatistics();
  // void newRefreshStatistics();
  void tenTicks();
  void cutGraphs(double start, double end);
  void setCutMode(bool state);
  void copyStatistics(const char *key);
  void markerToggled(QString name);

  void rescale_chart();

  void importFiles();

  void mouse_svg_transform(QString button, bool state);
  void help();
  void toggleCallouts();
  void toleranceChanged();

private:
  Ui::MainWindow *ui;
  bool linkedGraphs = false;
  bool loadingFiles = false;
  bool has_toleranceinput = false;

  void addStatisticsTable();
  void addStatisticsTableHTML();
  QString statistics_HTML = "";

  void setMouseSVG_data();
  void resizeEvent(QResizeEvent *event);

signals:
  void actionDataManagerDialog();

protected:
  void keyPressEvent(QKeyEvent *event);
private slots:
  void on_actionDataManagerDialog_triggered();
  void on_actionsetZeroX_triggered();
};
#endif // MAINWINDOW_H

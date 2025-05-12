#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include <QAxObject>
// #include <QAxWidget>
#include <QCollator>
#include <QMessageBox>
#include <QShortcut>
#include <QSvgWidget>
#include <QTextBrowser>

#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QObject::connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this,
                   &MainWindow::closeTabHandler, Qt::QueuedConnection);
  QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, this,
                   &MainWindow::calloutsActionVisibility, Qt::QueuedConnection);

  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  // auto layout =
  View *view = qobject_cast<View *>(graph);

  if (view->good) {
    QFileInfo fi(fileNameArg);
    //        qDebug() << "tab 1 set name:" << fi.baseName ();
    ui->tabWidget->setTabText(
        0, fi.fileName().remove(fi.fileName().size() - 4, 4));

    QObject::connect(view, &View::fileDropped, this, &MainWindow::addTab,
                     Qt::QueuedConnection);
    QObject::connect(view, &View::rangeChanged, this,
                     &MainWindow::graphRangeChanged, Qt::QueuedConnection);
    QObject::connect(view, &View::saved, this, &MainWindow::graph_saved,
                     Qt::QueuedConnection);
    QObject::connect(view, &View::propertyChanged, this,
                     &MainWindow::graphPropertyChanged, Qt::QueuedConnection);
    QObject::connect(view, &View::I_cut_my_graph, this, &MainWindow::cutGraphs,
                     Qt::QueuedConnection);
    QObject::connect(view, &View::markerToggled, this,
                     &MainWindow::markerToggled, Qt::QueuedConnection);

    QObject::connect(view, &View::mouseEvent, this,
                     &MainWindow::mouse_svg_transform, Qt::QueuedConnection);

  } else {

    loadingFiles = true;
    // check if the a file was loaded in view and failed, and offer try again
    // option
    QFileInfo fi(view->folder);
    if (fi.isFile()) {
      QMessageBox msgBox;
      //                msgBox.setParent (this);
      msgBox.setText("The file " + fi.fileName() + " is not supported!");
      msgBox.setInformativeText("Choose another file?");
      QPushButton *anotherFile =
          msgBox.addButton(tr("Another file"), QMessageBox::ActionRole);
      QPushButton *abort = msgBox.addButton(QMessageBox::Abort);
      msgBox.setDefaultButton(QMessageBox::Save);
      msgBox.exec();

      if (msgBox.clickedButton() == anotherFile) {
        qDebug() << "another file";
        QString folder = "";

        auto paths = QFileDialog::getOpenFileNames(this, "Open files:", folder,
                                                   "(*.CSV *.ZS2 *.TXT)");
        ui->tabWidget->clear();

        for (auto path : paths) {
          qDebug() << "path:" << path;
          this->addTab(path);
        }
        loadingFiles = false;

      } else if (msgBox.clickedButton() == abort) {
        qDebug() << "abort";
        ui->tabWidget->clear();
        loadingFiles = false;
      }
    } else {
      QString folder = "";
      auto paths = QFileDialog::getOpenFileNames(this, "Open files:", folder,
                                                 "(*.CSV *.ZS2 *.TXT)");
      ui->tabWidget->clear();

      bool valid_file_loaded = false;
      if (paths.isEmpty())
        valid_file_loaded = true;
      for (auto path : paths) {
        if (this->addTab(path))
          valid_file_loaded = true;
      }

      if (!valid_file_loaded) {
        // if the file was not and offer try again option
        QMessageBox msgBox;
        //                       msgBox.setParent (this);
        msgBox.setText("No valid file was chosen!");
        msgBox.setInformativeText("Choose another file?");
        QPushButton *anotherFile =
            msgBox.addButton(tr("Another file"), QMessageBox::ActionRole);
        QPushButton *abort = msgBox.addButton(QMessageBox::Abort);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.exec();

        if (msgBox.clickedButton() == anotherFile) {
          qDebug() << "another file";
          QString folder = "";

          auto paths = QFileDialog::getOpenFileNames(
              this, "Open files:", folder, "(*.CSV *.ZS2 *.TXT)");
          ui->tabWidget->clear();

          for (auto path : paths) {
            qDebug() << "path:" << path;
            this->addTab(path);
          }
          loadingFiles = false;

        } else if (msgBox.clickedButton() == abort) {
          qDebug() << "abort";
          ui->tabWidget->clear();
          loadingFiles = false;
        }
      }

      loadingFiles = false;
    }
  }

  loadingFiles = false;
  setAcceptDrops(true);

  addStatisticsTableHTML();
  refreshStatistics();

  svg_widget = new QSvgWidget(this);
  svg_widget->setGeometry(QRect(this->width() - 100, 0, 75, 60));
  this->setMouseSVG_data();
  svg_widget->load(mouse_svg.toLatin1());
  svg_widget->show();
  ui->actiontogglecallouts->setVisible(false);
  calloutsActionVisibility(0);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::mouse_svg_transform(QString button, bool state) {
  //    qDebug() << "main mouse event";
  //    qDebug() << button << state;
  if (svg_widget == nullptr)
    return;

  if (button == "up" and state) {
    mouse_svg.replace(
        "  <path"
        "     id=\"wheel-up\""
        "     d=\"M 780.9929,800.71842 935.98137,590.16542 876.16941,529.96747 "
        "1116.5946,391.95268 1035.6675,690.49495 975.85553,630.297 "
        "820.86708,840.85007 Z\""
        "     "
        "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;stroke-width:11."
        "0943px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.02 4044.686 330.142 143.581 35.895 168.045 "
        "0 1@fdee07bb\""
        "     class=\"visited\" />",
        "  <path"
        "     id=\"wheel-up\""
        "     d=\"M 780.9929,800.71842 935.98137,590.16542 876.16941,529.96747 "
        "1116.5946,391.95268 1035.6675,690.49495 975.85553,630.297 "
        "820.86708,840.85007 Z\""
        "     "
        "style=\"fill:#ff0000;fill-opacity:1;stroke:#416b92;stroke-width:11."
        "0943px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.02 4044.686 330.142 143.581 35.895 168.045 "
        "0 1@fdee07bb\""
        "     class=\"visited\" />");
  }

  if (button == "up" and !state) {
    mouse_svg.replace(

        "  <path"
        "     id=\"wheel-up\""
        "     d=\"M 780.9929,800.71842 935.98137,590.16542 876.16941,529.96747 "
        "1116.5946,391.95268 1035.6675,690.49495 975.85553,630.297 "
        "820.86708,840.85007 Z\""
        "     "
        "style=\"fill:#ff0000;fill-opacity:1;stroke:#416b92;stroke-width:11."
        "0943px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.02 4044.686 330.142 143.581 35.895 168.045 "
        "0 1@fdee07bb\""
        "     class=\"visited\" />",
        "  <path"
        "     id=\"wheel-up\""
        "     d=\"M 780.9929,800.71842 935.98137,590.16542 876.16941,529.96747 "
        "1116.5946,391.95268 1035.6675,690.49495 975.85553,630.297 "
        "820.86708,840.85007 Z\""
        "     "
        "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;stroke-width:11."
        "0943px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.02 4044.686 330.142 143.581 35.895 168.045 "
        "0 1@fdee07bb\""
        "     class=\"visited\" />");
  }

  if (button == "down" and state) {
    mouse_svg.replace(

        "  <path"
        "     id=\"wheel-down\""
        "     d=\"m 471.89446,1335.9125 -143.24233,221.6357 62.96013,55.6357 "
        "-232.4457,155.5869 64.55242,-303.9486 62.96012,55.6358 "
        "143.24233,-221.6355 z\""
        "     "
        "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;stroke-width:11."
        "0944px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.026 4044.696 330.142 143.581 35.895 "
        "168.045 0 1@dbf8f4e0\""
        "     class=\"visited\" />",
        "  <path"
        "     id=\"wheel-down\""
        "     d=\"m 471.89446,1335.9125 -143.24233,221.6357 62.96013,55.6357 "
        "-232.4457,155.5869 64.55242,-303.9486 62.96012,55.6358 "
        "143.24233,-221.6355 z\""
        "     "
        "style=\"fill:#ff0000;fill-opacity:1;stroke:#416b92;stroke-width:11."
        "0944px;stroke-linejoin:bevel\""
        "     bx:shape=\"arrow 2535.026 4044.696 330.142 143.581 35.895 "
        "168.045 0 1@dbf8f4e0\""
        "     class=\"visited\" />");
  }

  if (button == "down" and !state) {
    mouse_svg.replace("  <path"
                      "     id=\"wheel-down\""
                      "     d=\"m 471.89446,1335.9125 -143.24233,221.6357 "
                      "62.96013,55.6357 -232.4457,155.5869 64.55242,-303.9486 "
                      "62.96012,55.6358 143.24233,-221.6355 z\""
                      "     "
                      "style=\"fill:#ff0000;fill-opacity:1;stroke:#416b92;"
                      "stroke-width:11.0944px;stroke-linejoin:bevel\""
                      "     bx:shape=\"arrow 2535.026 4044.696 330.142 143.581 "
                      "35.895 168.045 0 1@dbf8f4e0\""
                      "     class=\"visited\" />",
                      "  <path"
                      "     id=\"wheel-down\""
                      "     d=\"m 471.89446,1335.9125 -143.24233,221.6357 "
                      "62.96013,55.6357 -232.4457,155.5869 64.55242,-303.9486 "
                      "62.96012,55.6358 143.24233,-221.6355 z\""
                      "     "
                      "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;"
                      "stroke-width:11.0944px;stroke-linejoin:bevel\""
                      "     bx:shape=\"arrow 2535.026 4044.696 330.142 143.581 "
                      "35.895 168.045 0 1@dbf8f4e0\""
                      "     class=\"visited\" />");
  }

  if (button == "left" and state) {
    mouse_svg.replace("     id=\"mouse-1\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"",
                      "     id=\"mouse-1\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"");
  }

  if (button == "left" and !state) {
    mouse_svg.replace("     id=\"mouse-1\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"",
                      "     id=\"mouse-1\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"");
  }

  if (button == "right" and state) {
    mouse_svg.replace("     id=\"mouse-2\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"",
                      "     id=\"mouse-2\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"");
  }

  if (button == "right" and !state) {
    mouse_svg.replace("     id=\"mouse-2\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"",
                      "     id=\"mouse-2\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"");
  }

  if (button == "left" and state) {
    mouse_svg.replace("     id=\"mouse-1\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"",
                      "     id=\"mouse-1\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"");
  }

  if (button == "left" and !state) {
    mouse_svg.replace("     id=\"mouse-1\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"",
                      "     id=\"mouse-1\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"");
  }

  if (button == "middle" and state) {
    mouse_svg.replace("     id=\"mouse-3\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"",
                      "     id=\"mouse-3\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"");
  }

  if (button == "middle" and !state) {
    mouse_svg.replace("     id=\"mouse-3\""
                      "     style=\"fill:#ff0000;fill-opacity:1\"",

                      "     id=\"mouse-3\""
                      "     style=\"fill:#009e9e;fill-opacity:0.33\"");
  }

  svg_widget->load(mouse_svg.toLatin1());
  svg_widget->repaint();
}

void MainWindow::calloutsActionVisibility(int index) {

  if (loadingFiles)
    return;

  Q_UNUSED(index)
  if (ui->tabWidget->currentWidget()->objectName() == "Statistics") {
    ui->actiontogglecallouts->setVisible(false);
    ui->actionsetZeroX->setVisible(false);
    return;
  }

  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);

  for (auto callout_list : view->intPoints.values()) {
    if (callout_list.size() > 0) {
      ui->actiontogglecallouts->setVisible(true);
      ui->actionsetZeroX->setVisible(true);
      return;
    } else {
      ui->actiontogglecallouts->setVisible(false);
      ui->actionsetZeroX->setVisible(false);
    }
  }
}

void MainWindow::rescale_chart() {
  if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
    return;

  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);

  if (sender()->objectName() == "actionRescale_X") {
    for (auto axis : view->initialScale.keys()) {
      if (axis->orientation() == Qt::Horizontal) {
        axis->setMin(view->initialScale.value(axis).x());
        axis->setMax(view->initialScale.value(axis).y());
        emit axis->rangeChanged(view->initialScale.value(axis).x(),
                                view->initialScale.value(axis).y());
      }
    }
  }

  if (sender()->objectName() == "actionRescale_Y") {
    for (auto axis : view->initialScale.keys()) {
      if (axis->orientation() == Qt::Vertical) {
        axis->setMin(view->initialScale.value(axis).x());
        axis->setMax(view->initialScale.value(axis).y());
      }
    }
  }

  if (sender()->objectName() == "actionRescale_X_Y") {
    for (auto axis : view->initialScale.keys()) {
      axis->setMin(view->initialScale.value(axis).x());
      axis->setMax(view->initialScale.value(axis).y());
      if (axis->orientation() == Qt::Horizontal)
        emit axis->rangeChanged(view->initialScale.value(axis).x(),
                                view->initialScale.value(axis).y());
    }
  }

  view->updateCallouts();
  view->m_chart->update();
  QApplication::processEvents();
}

void MainWindow::importFiles() {
  loadingFiles = true;
  QString folder = "";
  auto paths = QFileDialog::getOpenFileNames(nullptr, "Open files:", folder,
                                             "(*.CSV *.ZS2 *.TXT)");

  for (auto path : paths) {
    qDebug() << "path:" << path;
    this->addTab(path);
  }

  loadingFiles = false;
  refreshStatistics();
}

void MainWindow::tenTicks() {

  int currentTab = ui->tabWidget->currentIndex();
  if (ui->tabWidget->widget(currentTab)->objectName() == "Statistics")
    return;

  if (linkedGraphs) {

    for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
      if (ui->tabWidget->widget(i)->objectName() == "Statistics")
        continue;

      auto tab = ui->tabWidget->widget(i)->children();
      auto graph = tab.at(1);

      View *view = qobject_cast<View *>(graph);
      view->tenTicks();
    }

  } else {

    auto tab = ui->tabWidget->widget(currentTab)->children();
    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);
    view->tenTicks();
  }
}

void MainWindow::markerToggled(QString name) {
  //    qDebug () << "marker toggled:" << name;

  if (!linkedGraphs) {
    return;
  }

  int currentTab = ui->tabWidget->currentIndex();
  auto current_view =
      qobject_cast<View *>(ui->tabWidget->widget(currentTab)->children().at(1));
  auto current_view_markers = current_view->m_chart->legend()->markers();

  bool state = false;

  for (auto m_marker : current_view_markers) {
    if (m_marker->label().contains(name)) {
      if (m_marker->series()->isVisible())
        state = true;
      break;
    }
  }

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == currentTab)
      continue;
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;
    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);

    view->toggleLegendMarker(name, state);
  }
}

void sendTcpRequest() {
  QTcpSocket socket;
  socket.connectToHost("192.168.1.9",
                       1234); // Replace with the correct IP and port

  if (!socket.waitForConnected(5000)) {
    qDebug() << "Error: Could not connect to the server";
    return;
  }

  QByteArray data = "Hello from Qt"; // The raw data you want to send

  socket.write(data);
  if (!socket.waitForBytesWritten(5000)) {
    qDebug() << "Error: Could not write data to the server";
    return;
  }

  QElapsedTimer timer;
  timer.start();

  if (!socket.waitForReadyRead(5000)) {
    qDebug() << "Error: No response from the server";
    return;
  }

  qint64 elapsedTime = timer.elapsed();
  QByteArray responseData = socket.readAll();

  qDebug() << "Time taken for the request:" << elapsedTime << "milliseconds";
  qDebug() << "Response from server:" << responseData;

  socket.disconnectFromHost();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  // qDebug () << event->text() << " key pressed over main window!";
  //    qDebug () << event->matches(QKeySequence::Copy) << "copy";

  if ((ui->tabWidget->widget(ui->tabWidget->currentIndex())->objectName() ==
       "Statistics") and
      (event->text() == "")) {
    qDebug() << "copy stat html";

    QMimeData *md = new QMimeData;
    md->setData(QLatin1String("text/html"), statistics_HTML.toUtf8());
    QApplication::clipboard()->setMimeData(md, QClipboard::Clipboard);
    ui->statusbar->showMessage("Statistics table copied in clipbpoard", 3000);
  }

  if (event->text() == 'g') {
    return;
    sendTcpRequest();
  }

  if (event->text() == 'j') {
    return;
    auto manager = new QNetworkAccessManager();
    QUrl url("http://MW18007277/api/data_points/");
    QNetworkRequest request(url);
    qDebug() << "posting 100 dummy points";

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonArray dataArray;

    for (int i = 0; i < 100; ++i) {
      QJsonObject json;
      json.insert("instrument_number", 0);

      QDateTime currentDateTime = QDateTime::currentDateTime();
      QString currentTimestamp =
          currentDateTime.toString("yyyy-MM-ddTHH:mm:ss") + "." +
          QString::number(currentDateTime.time().msec() * 1000);
      json.insert("timestamp", currentTimestamp);

      json.insert("value", 33.3 + i); // Example: different value for each point

      dataArray.append(json);
    }

    QJsonDocument doc(dataArray);
    QByteArray data = doc.toJson();

    QElapsedTimer timer;
    timer.start();
    QNetworkReply *reply = manager->post(request, data);

    QObject::connect(
        reply, &QNetworkReply::finished, this, [reply, manager, timer]() {
          if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error:" << reply->errorString();
            return;
          }

          qint64 elapsedTime = timer.elapsed();
          qDebug() << "Time taken to post:" << elapsedTime << "milliseconds";

          QByteArray response_data = reply->readAll();
          QJsonDocument jsonDoc(QJsonDocument::fromJson(response_data));
          QJsonObject jsonObj = jsonDoc.object();
          QString message = jsonObj["message"].toString();

          qDebug() << "Response:" << message;

          reply->deleteLater();
          manager->deleteLater();
        });
  }
}

void MainWindow::refreshStatistics() {
  // qDebug() << "void MainWindow::refreshStatistics()";

  if (ui->tabWidget->count() == 1) {
    if ("Statistics" == ui->tabWidget->widget(0)->objectName()) {
      int statisticsTab = ui->tabWidget->currentIndex();
      auto tab = ui->tabWidget->widget(statisticsTab)->children();
      auto graph = tab.at(1);

      QTextBrowser *table = qobject_cast<QTextBrowser *>(graph);
      table->clear();
    }
    return;
  }
  if (loadingFiles)
    return;

  // qDebug() << "tabs count:" << ui->tabWidget->count ();

  QList<QString> parametersList;
  // QList<QString> samplesList;
  QMap<QString, QMap<QString, QString>> sample_param_value_Map;
  QMap<QString, QString> units_map;

  int statisticsTab = ui->tabWidget->currentIndex();

  for (int i = 0; i < ui->tabWidget->count(); ++i) {
    if ("Statistics" == ui->tabWidget->widget(i)->objectName()) {
      statisticsTab = i;
      // qDebug() << "Statistics tab:" << i;
      break;
    }
  }

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == statisticsTab)
      continue;

    auto tab = ui->tabWidget->widget(i)->children();
    // qDebug() << "tab:" << ui->tabWidget->tabText (i);
    QString sample = ui->tabWidget->tabText(i);

    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);

    QMap<QString, QString> currentSampleParameters;

    for (auto raw_serie : view->m_chart->series()) {

      qDebug() << "serie name:" << raw_serie->name() << raw_serie->name();
      qDebug() << "serie type:" << raw_serie->property("my_type").toString();

      if (raw_serie->name() == "")
        continue;
      if ((!(raw_serie->property("my_type") == "current")) and
          (!(raw_serie->property("my_type") == "error")) and
          (!(raw_serie->property("my_type") == "voltage")))
        continue;

      auto serie = qobject_cast<QXYSeries *>(raw_serie)->points();
      if (serie.size() < 1)
        continue;

      // double average = 0;
      // double min = serie.at(0).y();
      // double max = serie.at(0).y();
      // double sum = 0;

      // for (QPointF point : serie) {
      //   min > point.y() ? min = point.y() : min;
      //   max < point.y() ? max = point.y() : max;
      //   sum += point.y();
      // }
      // average = sum / serie.size();

      // // Second pass to calculate the variance
      // double variance = 0;
      // for (const QPointF &point : serie) {
      //   double diff = point.y() - average;
      //   variance += diff * diff;
      // }
      // variance = variance / serie.size();
      // double stddev = std::sqrt(variance);

      double sum = 0;
      double average = 0;
      double min = serie.at(0).y();
      double max = serie.at(0).y();

      // qDebug() << "prin statistici";

      for (const QPointF &point : serie) {
        min > point.y() ? min = point.y() : min;
        max < point.y() ? max = point.y() : max;
        sum += point.y();
      }

      average = sum / serie.size();

      // Second pass to calculate the variance
      double variance = 0;
      for (const QPointF &point : serie) {
        double diff = point.y() - average;
        variance += diff * diff;
      }
      variance = variance / serie.size();
      double stddev = std::sqrt(variance);

      // qDebug() << "prin statistici dupa bucle";

      QString unit = "";
      QString sname = raw_serie->name();

      if (!raw_serie->property("unit").isValid()) {

        if ((sname.contains("[A]")) or (sname.contains("Curr")) or
            (sname.contains("curr")) or (sname.contains("ström")) or
            (sname.contains("Strom")) or (sname.contains("strom")) or
            (sname.contains("Ström")))
          unit = "A";

        if ((sname.contains("[V]")) or (sname.contains("Volt")) or
            (sname.contains("volt")) or (sname.contains("spann")) or
            (sname.contains("Spann")))
          unit = "V";

        if ((sname.contains("[°C]")) or (sname.contains("Tempe")) or
            (sname.contains("tempe")) or (sname.contains("[C]")))
          unit = "°C";

        if ((sname.contains("[%rh]")) or (sname.contains("Humid")) or
            (sname.contains("humid")) or (sname.contains("Feuch")) or
            (sname.contains("feuch")) or (sname.contains("[%]")))
          unit = "%rh";
      } else {
        unit = raw_serie->property("unit").toString();
      }

      // QString text =  "Measurements: " +
      //         QString::number (serie.size()) + "\nMin: " +
      //         view->engineering_Format (min) + unit +
      //         "\nMax: " + view->engineering_Format (max) + unit +
      //         "\nAvg: " + view->engineering_Format (average) + unit;

      // qDebug() << text << Qt::endl <<
      // raw_serie->property("format").toString() << Qt::endl;

      if (raw_serie->property("my_type") == "current" or
          raw_serie->property("my_type") == "voltage") {

        // currentSampleParameters.insert(sname + " |min", "  " +
        // view->engineering_Format(min) + "");
        // currentSampleParameters.insert(sname + " |max", "  " +
        // view->engineering_Format(max) + "");
        // currentSampleParameters.insert(sname + " |avg", "  " +
        // view->engineering_Format(average) + "");

        currentSampleParameters.insert(sname + "|min",
                                       QString::number(min, 'd'));
        currentSampleParameters.insert(sname + "|max",
                                       QString::number(max, 'd'));
        currentSampleParameters.insert(sname + "|avg",
                                       QString::number(average, 'd'));
        currentSampleParameters.insert(sname + "|stddev",
                                       QString::number(stddev, 'd'));
        currentSampleParameters.insert(sname + "|count",
                                       QString::number(serie.size()));

        units_map.insert(sname + "|min", unit);
        units_map.insert(sname + "|max", unit);
        units_map.insert(sname + "|avg", unit);
        units_map.insert(sname + "|stddev", unit);
        units_map.insert(sname + "|count", "");

        //                    currentSampleParameters.insert (sname + " [" +
        //                    unit + "] min", "  " + QString::number (min) + "
        //                    "); currentSampleParameters.insert (sname + " [" +
        //                    unit
        //                    +  "] max", "  " + QString::number (max) + "  ");
        //                    currentSampleParameters.insert (sname + " [" +
        //                    unit +  "] avg", "  " + QString::number (average)
        //                    + "  ");

        if (!parametersList.contains(sname + "|min"))
          parametersList.append(sname + "|min");

        if (!parametersList.contains(sname + "|max"))
          parametersList.append(sname + "|max");

        if (!parametersList.contains(sname + "|avg"))
          parametersList.append(sname + "|avg");

        // new

        if (!parametersList.contains(sname + "|stddev"))
          parametersList.append(sname + "|stddev");

        if (!parametersList.contains(sname + "|count"))
          parametersList.append(sname + "|count");
      }

      else if (raw_serie->property("my_type") == "error") {
        QString key = sname.split(" (").at(0) + "|count";
        QString value = "  " + QString::number(serie.size()) + "  ";
        currentSampleParameters.insert(key, value);

        if (!parametersList.contains(key)) {
          parametersList.append(key);
        }
      }

      sample_param_value_Map.insert(sample, currentSampleParameters);
    }
  }

  // qDebug() << Qt::endl << sample_param_value_Map;
  //                      "writing-mode: vertical-lr;"

  QString str_html = ("\n<!DOCTYPE html>\n"
                      "<html>\n"
                      "<head>\n\n"
                      "<style type=\"text/css\">\n\n"

                      "table {border-collapse: collapse;}\n"

                      "th {\n"
                      "border: 1.5px solid black;\n"
                      "padding: 1.5px;\n"
                      "font-family: \"Franklin Gothic Book\";\n"
                      "color: white;\n"
                      "text-align:left;\n"
                      "vertical-align:middle;\n"
                      "font-size: 15px;\n"
                      "font-weight: bold;\n"
                      "white-space: nowrap;\n"
                      "}\n\n"

                      ".th2 {\n"
                      "border: 1.5px solid black;\n"
                      "padding: 1.5px;\n"
                      "font-family: \"Franklin Gothic Book\";\n"
                      "color: white;\n"
                      "text-align:left;\n"
                      "vertical-align:middle;\n"
                      "font-size: 12px;\n"
                      "font-weight: bold;\n"
                      "white-space: nowrap;\n"
                      "}\n\n"

                      "td {\n"
                      "border: 1.5px solid black;\n"
                      "padding: 1.5px;\n"
                      "font-family: \"Franklin Gothic Book\";\n"
                      "text-align:center;\n"
                      "vertical-align:middle;\n"
                      "font-weight: bold;\n"
                      "font-size: 12px;\n"
                      "white-space: nowrap;\n"
                      "}\n\n"

                      "</style>\n"
                      "</head>\n\n"

                      "<table>\n"
                      "<thead>\n\n"
                      "<tr>\n"
                      "<td style=\"border: none;\">\n<b></b>\n</td>\n"
                      "<td style=\"border: none;\">\n<b></b>\n</td>\n");

  //    qDebug () << " sample list:" << sample_param_value_Map.keys();

  // add sample names to table header
  for (auto sample : sample_param_value_Map.keys()) {
    str_html.append("\n<th  bgcolor=#009E9E>  " + sample + "  </th>\n");
  }

  // end table header
  str_html.append("</tr>\n\n"
                  "</thead>\n"
                  " <tbody>\n\n");

  bool even = false;
  QString color = "#ccc";
  bool hadMin = false;

  // add parameters and values to table
  for (auto const &param : parametersList) {
    auto paramSplit = param.split("|");

    if (paramSplit.size() < 2)
      continue;

    QString paramHead = paramSplit.at(0);

    if (paramSplit.at(1) == "min") {
      str_html.append("<tr>\n"
                      "<th rowspan=\"5\" bgcolor=#009E9E style=\"color: white;"
                      "border: 1.5px solid black;\">  " +
                      paramHead + "  </th>\n");

      str_html.append("<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
                      "border: 1.5px solid black;\">min" +
                      QString(" [" + units_map.value(param) + "]") + "</th>\n");
      hadMin = true;
    } else if (paramSplit.at(1) == "max") {
      str_html.append(
          "<tr>\n<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
          "border: 1.5px solid black;\">max" +
          QString(" [" + units_map.value(param) + "]") + " </th>\n");
    } else if (paramSplit.at(1) == "avg") {
      str_html.append(
          "<tr>\n<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
          "border: 1.5px solid black;\">avg" +
          QString(" [" + units_map.value(param) + "]") + "</th>\n");
      // new 26.03.24
    } else if (paramSplit.at(1) == "stddev") {
      str_html.append(
          "<tr>\n<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
          "border: 1.5px solid black;\">stddev" +
          QString(" [" + units_map.value(param) + "]") + "</th>\n");

      //

    } else if (paramSplit.at(1) == "count" && hadMin == true) {
      str_html.append(
          "<tr>\n<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
          "border: 1.5px solid black;\">count</th>\n");
      hadMin = false;
    } else if (paramSplit.at(1) == "count" && hadMin == false) {
      str_html.append("<tr>"
                      "<th bgcolor=#009E9E style=\"color: white;"
                      "border: 1.5px solid black;\">  " +
                      paramHead + "  </th>\n");
      hadMin = false;

      str_html.append("<th class=\"th2\" bgcolor=#009E9E style=\"color: white;"
                      "border: 1.5px solid black;\">count</th>\n");
    }

    if (even) {
      color = "#eeeeee";
      even = false;
    } else {
      color = "#ffffff";
      even = true;
    }

    // add values for each sample
    for (auto const &sample : sample_param_value_Map.keys()) {
      auto sample_params = sample_param_value_Map.value(sample).keys();

      QString value = "0";
      if (sample_params.contains(param)) {
        value = sample_param_value_Map.value(sample).value(param);
      }

      str_html.append("\n<td bgcolor=" + color + ">" + value +
                      " </td>"); //+ units_map.value(param)

      // qDebug() << sample << " contains " << param <<
      // sample_params.contains(param) << units_map.value(param);
    }

    str_html.append("\n</tr>\n\n");
  }

  // table end
  str_html.append(

      "</tbody>"
      "</table>"

      "</body>"
      "</html>");

  auto tab = ui->tabWidget->widget(statisticsTab)->children();
  auto graph = tab.at(1);

  QTextBrowser *table = qobject_cast<QTextBrowser *>(graph);

  table->clear();
  table->setHtml(str_html);
  statistics_HTML = str_html;

  //    QMimeData *md = new QMimeData;
  //    md->setData(QLatin1String("text/html"),
  //    str_html.toUtf8());//table->toHtml ()
  //    QApplication::clipboard()->setMimeData(md, QClipboard::Clipboard);

  // get the tab names into a map of name->index
  QMap<QString, int> tabs;
  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == statisticsTab)
      continue;
    auto tab = ui->tabWidget->widget(i)->children();
    QString sample = ui->tabWidget->tabText(i);
    tabs.insert(sample, i);
  }

  // get a string list of tab names, for sorting
  QStringList v = tabs.keys();

  // sort the tab names naturally, using QCollator in numeric mode as compare
  // function, so 10 will sit after 2
  QCollator *collator = new QCollator;
  collator->setNumericMode(true);
  std::sort(v.begin(), v.end(), *collator);

  // add the statistics tab to the list
  v.append("Statistics");

  // move the tabs according to their sorted position
  qint32 i = 0, j = 0, k = v.count();
  for (i = 0; i < k; i++) {
    for (j = 0; j < ui->tabWidget->tabBar()->count(); j++) {
      if (ui->tabWidget->tabBar()->tabText(j) == v[i]) {
        ui->tabWidget->tabBar()->moveTab(j, i);
      }
    }
  }
}

void MainWindow::messageHandler(quint32 instanceId, QByteArray message) {
  auto args = message.split('|');
  qDebug() << "message reached handler:" << instanceId << args;
  //    QMessageBox::information(0, "Message!:", args.last());
  this->addTab(args.last());
}

void MainWindow::copyStatistics(const char *key) {
  // qDebug () << key;
}

void MainWindow::addStatisticsTableHTML() {

  auto tab = new QWidget(ui->tabWidget);
  auto gridLayout = new QGridLayout(tab);
  tab->setObjectName(QString::fromUtf8("Statistics"));

  auto tableWidget = new QTextBrowser(tab);
  tableWidget->setContextMenuPolicy(Qt::NoContextMenu);
  tableWidget->setFocusPolicy(Qt::NoFocus);

  //    QShortcut *shortcut = new QShortcut(QKeySequence::Copy, tab);
  //    QObject::connect(shortcut, SIGNAL(activated()), this,
  //    SLOT(&MainWindow::copyStatistics));

  //    auto tableWidget = new QLabel ( tab);

  QString str_html = "";

  tableWidget->setHtml(str_html);
  //    tableWidget->setText (str_html);

  gridLayout->addWidget(tableWidget, 0, 0, 1, 1); //, Qt::AlignTop
  tab->setLayout(gridLayout);

  ui->tabWidget->addTab(tab, "Statistics");

  //    QPixmap pixmap(tableWidget->size());
  //    tableWidget->render(&pixmap);
  //    QClipboard *clipboard = QApplication::clipboard();
  //    clipboard->setPixmap (pixmap);
}

void MainWindow::closeTabHandler(int index) {
  // qDebug() << "tab" << index << "want to close!";
  if (ui->tabWidget->widget(index)->objectName() == "Statistics")
    return;

  ui->tabWidget->widget(index)->deleteLater();
  ui->tabWidget->removeTab(index);
  if (ui->tabWidget->count() < 1)
    exit(1);
  refreshStatistics();
}

bool MainWindow::addTab(QString file_path) {
  // qDebug() << "add tab:" << file_path;

  //    argFile = true;
  //    fileNameArg = file_path;
  QFileInfo fi(file_path);

  this->statusBar()->showMessage(statusBar()->currentMessage() +
                                 " Loading: " + fi.baseName());

  auto graph = new View(ui->tabWidget, file_path);
  if (!graph->good) {
    this->statusBar()->showMessage(statusBar()->currentMessage() + "Failed!",
                                   2500);
    return false;
  }

  graph->setObjectName(QString::fromUtf8("graphView"));

  auto new_tab = new QWidget(ui->tabWidget);
  auto gridLayout = new QGridLayout(new_tab);
  gridLayout->addWidget(graph, 0, 0, 1, 1);
  new_tab->setLayout(gridLayout);

  ui->tabWidget->addTab(new_tab,
                        fi.fileName().remove(fi.fileName().size() - 4, 4));
  ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
  this->statusBar()->showMessage(statusBar()->currentMessage() + "Done!", 2500);

  QObject::connect(graph, &View::fileDropped, this, &MainWindow::addTab,
                   Qt::QueuedConnection);
  QObject::connect(graph, &View::rangeChanged, this,
                   &MainWindow::graphRangeChanged, Qt::QueuedConnection);
  QObject::connect(graph, &View::saved, this, &MainWindow::graph_saved,
                   Qt::QueuedConnection);
  QObject::connect(graph, &View::propertyChanged, this,
                   &MainWindow::graphPropertyChanged, Qt::QueuedConnection);
  QObject::connect(graph, &View::I_cut_my_graph, this, &MainWindow::cutGraphs,
                   Qt::QueuedConnection);
  QObject::connect(graph, &View::markerToggled, this,
                   &MainWindow::markerToggled, Qt::QueuedConnection);

  QObject::connect(graph, &View::mouseEvent, this,
                   &MainWindow::mouse_svg_transform, Qt::QueuedConnection);

  QObject::connect(graph, &View::dataChanged, this,
                   &MainWindow::refreshStatistics, Qt::QueuedConnection);

  graph->cutMode = ui->actionCut->isChecked();
  graph->setScaleAxis(ui->actionToggle_scale_axis->isChecked());

  refreshStatistics();

  // if (labViewEval)
  //   addTab(file_path);

  // if (graph->data_loader != nullptr){
  //     if ((graph->data_loader->type == dataLoader::LabView)
  //             and !this->has_toleranceinput)
  //     {
  //         qDebug() << "trying to add a spinbox...";
  //         auto input_min_factor = new QDoubleSpinBox();
  //         input_min_factor->setSingleStep (0.1);
  //         auto label1 = new QLabel();
  //         label1->setText (" Min Tolerance:");
  //         auto label2 = new QLabel();
  //         label2->setText ("x P +");
  //         auto input_min_offset = new QDoubleSpinBox();
  //         input_min_offset->setSingleStep (0.1);

  //         auto input_max_factor = new QDoubleSpinBox();
  //         input_min_factor->setSingleStep (0.1);
  //         auto label3 = new QLabel();
  //         label3->setText (" Max Tolerance:");
  //         auto label4 = new QLabel();
  //         label4->setText ("x P +");
  //         auto input_max_offset = new QDoubleSpinBox();
  //         input_min_offset->setSingleStep (0.1);

  //         QObject::connect(input_min_factor,
  //                          QOverload<double>::of(&QDoubleSpinBox::valueChanged),
  //                          this,
  //                          [=](double d){ this->Tolerance.min_factor = d;
  //                          emit toleranceChanged();}, Qt::QueuedConnection);

  //         QObject::connect(input_max_factor,
  //                          QOverload<double>::of(&QDoubleSpinBox::valueChanged),
  //                          this,
  //                          [=](double d){ this->Tolerance.max_factor = d;emit
  //                          toleranceChanged();}, Qt::QueuedConnection);

  //         QObject::connect(input_min_offset,
  //                          QOverload<double>::of(&QDoubleSpinBox::valueChanged),
  //                          this,
  //                          [=](double d){ this->Tolerance.min_offset = d;emit
  //                          toleranceChanged();}, Qt::QueuedConnection);

  //         QObject::connect(input_max_offset,
  //                          QOverload<double>::of(&QDoubleSpinBox::valueChanged),
  //                          this,
  //                          [=](double d){ this->Tolerance.max_offset = d;emit
  //                          toleranceChanged();}, Qt::QueuedConnection);

  //         this->statusBar ()->insertPermanentWidget (11, label1);
  //         this->statusBar ()->insertPermanentWidget (12, input_min_factor);
  //         this->statusBar ()->insertPermanentWidget (13, label2);
  //         this->statusBar ()->insertPermanentWidget (14, input_min_offset);

  //         this->statusBar ()->insertPermanentWidget (15, label3);
  //         this->statusBar ()->insertPermanentWidget (16, input_max_factor);
  //         this->statusBar ()->insertPermanentWidget (17, label4);
  //         this->statusBar ()->insertPermanentWidget (18, input_max_offset);

  //         this->has_toleranceinput = true;
  //     }
  // }

  return true;
}

void MainWindow::toleranceChanged() {
  qDebug() << "Tolerance changed!" << this->Tolerance.min_factor;
  if (this->Tolerance.min_factor == 33.1) {
    qDebug() << "Deleting!";
    //        for (auto child : this->statusBar ()->children ())
    //        child->deleteLater ();

    for (int i = 11; i <= 18; i++)
      this->statusBar()->children().at(i)->deleteLater();
  }
}

void MainWindow::graphRangeChanged(qreal min, qreal max, QString name) {
  if (!linkedGraphs)
    return;
  //    qDebug() << "Main window: range changed " << min << max << name <<
  //    ui->tabWidget->currentWidget ()->objectName ();
  int currentTab = ui->tabWidget->currentIndex();

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == currentTab)
      continue;
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;

    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);

    //        qDebug() << currentTab << name << i;

    View *view = qobject_cast<View *>(graph);
    view->userInteracted = false;

    // if ((name == "axis_errors") and (view->axis_errors != nullptr))
    //   view->axis_errors->setRange(min, max);
    // if ((name == "axis_temperature") and (view->axis_temperature != nullptr))
    //   view->axis_temperature->setRange(min, max);
    // if ((name == "axis_humidity") and (view->axis_humidity != nullptr))
    //   view->axis_humidity->setRange(min, max);
    if ((name == "axisX") and (view->axisX != nullptr))
      view->axisX->setRange(min, max);
    if ((name == "axisX2") and (view->axisX2 != nullptr))
      view->axisX2->setRange(min, max);
    // if ((name == "axis_current") and (view->axis_current != nullptr))
    //   view->axis_current->setRange(min, max);
    // if ((name == "axis_sleep_current") and
    //     (view->axis_sleep_current != nullptr))
    //   view->axis_sleep_current->setRange(min, max);
    // if ((name == "axis_voltage") and (view->axis_voltage != nullptr))
    //   view->axis_voltage->setRange(min, max);
    // if ((name == "axis_opmode") and (view->axis_opmode != nullptr))
    //   view->axis_opmode->setRange(min, max);
    // if ((name == "axis_durability_signals") and
    //     (view->axis_durability_signals != nullptr))
    //   view->axis_durability_signals->setRange(min, max);

    // if ((name == "axis_force") and (view->axis_force != nullptr))
    //   view->axis_force->setRange(min, max);
    // if ((name == "axis_travel") and (view->axis_travel != nullptr))
    //   view->axis_travel->setRange(min, max);

    view->updateCallouts();
  }
}

void MainWindow::graph_saved() {
  if (!linkedGraphs)
    return;
  qDebug() << "Main window: graph saved ";
  int currentTab = ui->tabWidget->currentIndex();

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == currentTab)
      continue;
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;
    ui->tabWidget->setCurrentIndex(i);
    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);
    view->savePNG(view->serieName);
  }

  ui->tabWidget->setCurrentIndex(currentTab);
}

void MainWindow::graphPropertyChanged() {
  if (linkedGraphs)
    axes_link(true);
}

void MainWindow::axes_link(bool state) {
  // linkedGraphs = state;

  // if (!state)
  //   return;
  // if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
  //   return;

  // int currentTab = ui->tabWidget->currentIndex();

  // auto tab = ui->tabWidget->widget(currentTab)->children();
  // auto current_graph = tab.at(1);
  // View *current_view = qobject_cast<View *>(current_graph);

  // for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
  //   if (i == currentTab)
  //     continue;
  //   if (ui->tabWidget->widget(i)->objectName() == "Statistics")
  //     continue;

  //   auto tab = ui->tabWidget->widget(i)->children();
  //   auto graph = tab.at(1);

  //   View *view = qobject_cast<View *>(graph);
  //   view->userInteracted = false;

  //   if ((view->axis_errors != nullptr) and
  //       (current_view->axis_errors != nullptr)) {
  //     view->axis_errors->setRange(current_view->axis_errors->min(),
  //                                 current_view->axis_errors->max());
  //     view->axis_errors->setTickCount(current_view->axis_errors->tickCount());
  //   }

  //   if ((view->axis_temperature != nullptr) and
  //       (current_view->axis_temperature != nullptr)) {
  //     view->axis_temperature->setRange(current_view->axis_temperature->min(),
  //                                      current_view->axis_temperature->max());
  //     view->axis_temperature->setTickCount(
  //         current_view->axis_temperature->tickCount());
  //     view->axis_temperature->setLabelsColor(
  //         current_view->axis_temperature->labelsColor());
  //     view->axis_temperature->setLinePen(
  //         current_view->axis_temperature->linePen());
  //     view->temperature->setColor(current_view->temperature->color());
  //   }

  //   if ((view->axis_humidity != nullptr) and
  //       (current_view->axis_humidity != nullptr)) {
  //     view->axis_humidity->setRange(current_view->axis_humidity->min(),
  //                                   current_view->axis_humidity->max());
  //     view->axis_humidity->setTickCount(
  //         current_view->axis_humidity->tickCount());
  //     view->axis_humidity->setLabelsColor(
  //         current_view->axis_humidity->labelsColor());
  //     view->axis_humidity->setLinePen(current_view->axis_humidity->linePen());
  //     view->humidity->setColor(current_view->humidity->color());
  //   }

  //   if ((view->axisX != nullptr) and (current_view->axisX != nullptr))
  //     view->axisX->setRange(current_view->axisX->min(),
  //                           current_view->axisX->max());

  //   if ((view->axisX2 != nullptr) and (current_view->axisX2 != nullptr))
  //     view->axisX2->setRange(current_view->axisX2->min(),
  //                            current_view->axisX2->max());

  //   if ((view->axis_current != nullptr) and
  //       (current_view->axis_current != nullptr)) {
  //     view->axis_current->setRange(current_view->axis_current->min(),
  //                                  current_view->axis_current->max());
  //     view->axis_current->setTickCount(current_view->axis_current->tickCount());
  //     view->axis_current->setLabelsColor(
  //         current_view->axis_current->labelsColor());
  //     view->axis_current->setLinePen(current_view->axis_current->linePen());
  //     view->current_activ->setColor(current_view->current_activ->color());
  //   }

  //   if ((view->axis_sleep_current != nullptr) and
  //       (current_view->axis_sleep_current != nullptr)) {
  //     view->axis_sleep_current->setRange(
  //         current_view->axis_sleep_current->min(),
  //         current_view->axis_sleep_current->max());
  //     view->axis_sleep_current->setTickCount(
  //         current_view->axis_sleep_current->tickCount());
  //     view->axis_sleep_current->setLabelsColor(
  //         current_view->axis_sleep_current->labelsColor());
  //     view->axis_sleep_current->setLinePen(
  //         current_view->axis_sleep_current->linePen());
  //     view->current_sleep->setColor(current_view->current_sleep->color());
  //   }

  //   if ((view->axis_voltage != nullptr) and
  //       (current_view->axis_voltage != nullptr)) {
  //     view->axis_voltage->setRange(current_view->axis_voltage->min(),
  //                                  current_view->axis_voltage->max());
  //     view->axis_voltage->setTickCount(current_view->axis_voltage->tickCount());
  //     view->axis_voltage->setLabelsColor(
  //         current_view->axis_voltage->labelsColor());
  //     view->axis_voltage->setLinePen(current_view->axis_voltage->linePen());
  //     view->voltage->setColor(current_view->voltage->color());
  //   }

  //   if ((view->axis_force != nullptr) and
  //       (current_view->axis_force != nullptr)) {
  //     view->axis_force->setRange(current_view->axis_force->min(),
  //                                current_view->axis_force->max());
  //     view->axis_force->setTickCount(current_view->axis_force->tickCount());
  //     view->axis_force->setLabelsColor(current_view->axis_force->labelsColor());
  //     view->axis_force->setLinePen(current_view->axis_force->linePen());
  //     //            view->f ->setColor (current_view->voltage->color ());
  //   }

  //   if ((view->axis_opmode != nullptr) and
  //       (current_view->axis_opmode != nullptr)) {
  //     view->axis_opmode->setRange(current_view->axis_opmode->min(),
  //                                 current_view->axis_opmode->max());
  //     view->axis_opmode->setLabelFormat(
  //         current_view->axis_opmode->labelFormat());
  //   }

  //   if ((view->axis_durability_signals != nullptr) and
  //       (current_view->axis_durability_signals != nullptr)) {
  //     view->axis_durability_signals->setRange(
  //         current_view->axis_durability_signals->min(),
  //         current_view->axis_durability_signals->max());

  //     //            view->axis_durability_signals->setRange
  //     //            (current_view->axis_durability_signals->min (),
  //     //                                                     current_view->axis_durability_signals->max
  //     //                                                     ());
  //   }

  //   for (auto serie : view->m_chart->series()) {

  //     //            if(serie->name () == "Operating Mode") continue;

  //     if (serie->name() == "")
  //       continue;

  //     for (auto current_serie : current_view->m_chart->series()) {

  //       if (!current_serie->property("my_symbol").isValid())
  //         continue;
  //       if (current_serie->name() == "Operating Mode")
  //         continue;
  //       if (current_serie->property("my_symbol") == "")
  //         continue;

  //       if (current_serie->name().section('(', 0, 0) ==
  //           serie->name().section('(', 0, 0)) {

  //         if (current_serie->isVisible() != serie->isVisible())
  //           view->toggleLegendMarker(current_serie->name().section('(', 0, 0),
  //                                    current_serie->isVisible());

  //         // qDebug() << serie->property ("my_symbol").isValid () <<
  //         //             serie->property ("my_symbol") <<
  //         //             current_serie->name ().section ('(', 0,0);

  //         QColor color;

  //         auto p_size = qobject_cast<QXYSeries *>(current_serie)
  //                           ->brush()
  //                           .textureImage()
  //                           .size();
  //         auto image =
  //             qobject_cast<QXYSeries *>(current_serie)->brush().textureImage();

  //         bool found = false;
  //         for (int x = 0; x < p_size.width(); x++) {
  //           for (int y = 0; y < p_size.height(); y++) {
  //             if (image.pixelColor(x, y).alpha() == 255) {
  //               //                            qDebug() << "tried to get color"
  //               //                            << image.pixelColor (x, y) <<
  //               //                            image.pixelColor (x, y).alpha ();
  //               color = image.pixelColor(x, y);
  //               found = true;
  //               break;
  //             }
  //           }
  //           if (found)
  //             break;
  //         }

  //         QString text = current_serie->property("my_symbol").toString();
  //         int size = current_serie->property("my_symbol_size").toInt();

  //         //                    qDebug() << serie->name () << text << size <<
  //         //                    color;

  //         const auto mytest = text;
  //         auto font = QFont("Arial", size, QFont::Bold);
  //         auto fm = QFontMetrics(font);
  //         auto rectangle = fm.boundingRect(text);
  //         auto w = fm.horizontalAdvance(text);
  //         auto h = rectangle.height();
  //         //                    qDebug() << rectangle;

  //         auto s = w > h ? w : h;

  //         QImage star(s, s, QImage::Format_ARGB32);
  //         star.fill(Qt::transparent);
  //         QPainter painter(&star);
  //         painter.setPen(color);
  //         painter.setFont(font);
  //         painter.drawText(star.rect(), Qt::AlignCenter, text);
  //         painter.end();

  //         static_cast<QScatterSeries *>(serie)->setBrush(star);
  //         static_cast<QScatterSeries *>(serie)->setPen(QColor(Qt::transparent));

  //         serie->setProperty("my_symbol", QVariant(text));
  //         serie->setProperty("my_symbol_size", QVariant(size));

  //         static_cast<QScatterSeries *>(serie)->setMarkerSize(s);
  //       } else if (current_serie->name() == serie->name()) {
  //       }
  //     }

  //     for (auto current_serie : current_view->m_chart->series()) {

  //       if (!current_serie->property("my_symbol").isValid()) {

  //         if (current_serie->name().section('(', 0, 0) ==
  //             serie->name().section('(', 0, 0)) {

  //           //                        qDebug() << "aiiiici" << serie->name () <<
  //           //                        current_serie->name ();

  //           if (current_serie->isVisible() != serie->isVisible()) {
  //             view->toggleLegendMarker(current_serie->name().section('(', 0, 0),
  //                                      current_serie->isVisible());
  //           }

  //           if (current_serie->name() == "Operating Mode") {
  //             continue;
  //           }

  //           static_cast<QScatterSeries *>(serie)->setColor(
  //               static_cast<QScatterSeries *>(current_serie)->color());
  //         }
  //       }
  //     }
  //   }
  // }
}

void MainWindow::current_NiceNumbers() {
  //    qDebug() << "current_NiceNumbers";
  if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
    return;
  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);
  view->applyNiceNumbers();
  //    qDebug() << graph;
}

void MainWindow::cutGraphs(double start, double end) {

  if (!linkedGraphs) {
    refreshStatistics();
    return;
  }

  int currentTab = ui->tabWidget->currentIndex();

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (i == currentTab)
      continue;
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;
    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);
    view->cutGraph(start, end);
  }

  refreshStatistics();
}

void MainWindow::setCutMode(bool state) {

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {

    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;
    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);
    View *view = qobject_cast<View *>(graph);
    view->cutMode = state;
  }
}

void MainWindow::current_ScaleAxis(bool state) {
  //    if(ui->tabWidget->currentWidget ()->objectName () == "Statistics")
  //    return; auto childs = ui->tabWidget->currentWidget ()->children (); auto
  //    graph = childs.at (1); View* view = qobject_cast<View*>(graph);
  //    view->toggleScaleAxis ();
  //    qDebug() << graph << graph;

  int currentTab = ui->tabWidget->currentIndex();
  if (ui->tabWidget->widget(currentTab)->objectName() == "Statistics")
    return;

  //    if(linkedGraphs)
  //    {

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;

    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);

    View *view = qobject_cast<View *>(graph);
    view->setScaleAxis(state);

    view->update();
    QApplication::processEvents();
  }

  //    }
  //    else
  //    {

  //        auto tab = ui->tabWidget->widget (currentTab)->children ();
  //        auto graph = tab.at (1);
  //        View* view = qobject_cast<View*>(graph);
  //        view->setScaleAxis (state);
  //    }
}

void MainWindow::current_NamedSave() {

  for (int i = ui->tabWidget->count() - 1; i >= 0; i--) {
    if (ui->tabWidget->widget(i)->objectName() == "Statistics")
      continue;

    auto tab = ui->tabWidget->widget(i)->children();
    auto graph = tab.at(1);

    View *view = qobject_cast<View *>(graph);
    // if (view->is_robot)
    //   view->exportAction();

    view->savePNG(view->serieName);

    QApplication::processEvents();
  }

  if (ui->tabWidget->currentWidget()->objectName() == "Statistics") {
    QMimeData *md = new QMimeData;
    md->setData(QLatin1String("text/html"),
                statistics_HTML.toUtf8()); // table->toHtml ()
    QApplication::clipboard()->setMimeData(md, QClipboard::Clipboard);

    ui->statusbar->showMessage("Statistics table copied in clipbpoard", 3000);

    return;
  }

  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);

  View *view = qobject_cast<View *>(graph);
  view->exportAction();
}

void MainWindow::current_ChangeOpMNames() {
  //    qDebug() << "current_NamedSave"  ;
  if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
    return;
  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);
  view->actionOpmEdit();
  //    qDebug() << graph << graph;
}

void MainWindow::dropEvent(QDropEvent *event) {

  for (auto url : event->mimeData()->urls()) {
    QString filePath;
    if (!url.toString().contains("///")) {
      filePath = url.toString().remove(0, 5);
    } else {
      filePath = url.path(QUrl::FullyDecoded).remove(0, 1);
    }

    //        QFileInfo fi(filePath);
    //        if (fi.exists () and
    //                (fi.suffix().contains ("csv")
    //                 or fi.suffix().contains ("CSV")
    //                 or fi.suffix().contains ("zs2")
    //                 or fi.suffix().contains ("ZS2")
    //                 or fi.suffix().contains ("txt")))
    //        {

    //            qDebug() << "Valid File dropped:" << fi.baseName ();
    addTab(filePath);
    //        } else {
    //            qDebug() << "the file does not contain a recognized suffix!"
    //            << fi;
    ////            return;
    //        }
  }

  event->acceptProposedAction();
  event->acceptProposedAction();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  //    if (event->mimeData()->hasFormat("text/plain"))
  event->acceptProposedAction();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event);
  if (svg_widget != nullptr)
    svg_widget->setGeometry(QRect(this->width() - 75, 0, 75, 60));
}

void MainWindow::setMouseSVG_data() {
  mouse_svg =
      "<svg"
      "   id=\"mouse\""
      "   viewBox=\"0.917 0.331 2000 2000\""
      "   width=\"2000\""
      "   height=\"2000\""
      "   version=\"1.1\""
      "   sodipodi:docname=\"mouse.svg\""
      "   inkscape:version=\"1.0.1 (3bc2e813f5, 2020-09-07)\">"
      "  <metadata"
      "     id=\"metadata298\">"
      "    <rdf:RDF>"
      "      <cc:Work"
      "         rdf:about=\"\">"
      "        <dc:format>image/svg+xml</dc:format>"
      "        <dc:type"
      "           rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />"
      "        <dc:title></dc:title>"
      "      </cc:Work>"
      "    </rdf:RDF>"
      "  </metadata>"
      "  <defs"
      "     id=\"defs296\" />"
      "  <sodipodi:namedview"
      "     pagecolor=\"#ffffff\""
      "     bordercolor=\"#666666\""
      "     borderopacity=\"1\""
      "     objecttolerance=\"10\""
      "     gridtolerance=\"10\""
      "     guidetolerance=\"10\""
      "     inkscape:pageopacity=\"0\""
      "     inkscape:pageshadow=\"2\""
      "     inkscape:window-width=\"1680\""
      "     inkscape:window-height=\"987\""
      "     id=\"namedview294\""
      "     showgrid=\"false\""
      "     inkscape:zoom=\"0.3675\""
      "     inkscape:cx=\"925.17007\""
      "     inkscape:cy=\"1108.8435\""
      "     inkscape:window-x=\"-8\""
      "     inkscape:window-y=\"-8\""
      "     inkscape:window-maximized=\"1\""
      "     inkscape:current-layer=\"Layer_2\" />"
      "  <polygon"
      "     style=\"fill:#ffffff\""
      "     points=\"979.666,414.532 1106.875,384.189 1209.716,377.437 "
      "1336.374,393.357 1409.585,417.127 1476.487,452.71 1541.071,500.431 "
      "1593.42,556.291 1627.7,603.9 1668.307,684.194 1690.246,795.184 "
      "1685.87,883.063 1669.446,951.085 1639.021,1018.645 1594.49,1077.762 "
      "1391.745,1167.637 1293.837,1222.122 1236.579,1262.911 1104.818,1380.372 "
      "985.532,1496.323 844.926,1513.217 551.153,1425.339 374.566,1163.246 "
      "365.254,1124.956 361.399,958.497 446.68,838.481 549.264,720.013 "
      "664.397,607.513 762.901,533.207 837.19,479.736 \""
      "     id=\"polygon155\""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <polygon"
      "     id=\"mouse-1\""
      "     style=\"fill:#009e9e;fill-opacity:0.33\""
      "     points=\"1005.542,773.958 1078.992,695.296 1190.432,660.797 "
      "1294.852,814.227 1348.032,924.339 982.406,1370.02 817.005,1420.91 "
      "605.79,1360.91 610.236,1317 792.977,1058.66 \""
      "     class=\"visited\""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <polygon"
      "     id=\"mouse-2\""
      "     style=\"fill:#009e9e;fill-opacity:0.33\""
      "     points=\"621.698,1030.215 837.618,782.876 949.843,672.314 "
      "998.034,635.495 1015.703,557.266 789.408,529.271 740.344,546.192 "
      "622.891,645.612 508.993,762.294 413.651,882.575 361.77,959.442 "
      "365.604,1133.119 459.432,1258.302 458.32,1234.725 \""
      "     class=\"visited\""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <polygon"
      "     id=\"mouse-3\""
      "     style=\"fill:#009e9e;fill-opacity:0.33\""
      "     points=\"693.549,943.689 751.401,889.029 789.846,869.447 "
      "831.479,851.575 892.346,888.88 887.499,911.834 715.323,1122.092 "
      "694.379,1130.985 677.718,1123.949 662.842,1106.245 630.999,1090.223 "
      "646.047,1021.603 \""
      "     class=\"visited\""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <polygon"
      "     id=\"mouse-4\""
      "     style=\"fill:#ffffff\""
      "     points=\"1393.892,882.938 1377.961,925.571 1285.369,1053.932 "
      "1260.276,1036.151 1395.09,858.858 \""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <polygon"
      "     id=\"mouse-5\""
      "     style=\"fill:#ffffff\""
      "     points=\"1274.192,1074.556 1158.893,1231.773 1093.133,1241.791 "
      "1243.645,1054.638 1262.324,1062.141 \""
      "     transform=\"matrix(1.4656815,0,0,1.71383,-494.05335,-611.69059)\" "
      "/>"
      "  <g"
      "     id=\"Layer_2\""
      "     transform=\"matrix(4.9193884,0,0,5.75227,-491.73263,-611.96076)\">"
      "    <path"
      "       class=\"st0\""
      "       d=\"m 107.7,335 -1,-49.5 c 0,0 132,-217 301.5,-165 0,0 67,19 "
      "88,88 21,69 -22.5,112 -22.5,112 0,0 -54.1,22.6 -73.8,34 -10.8,6.3 "
      "-25,14 -46.3,33 -22.4,20 -61.5,58 -61.5,58 l -42,5 -87.4,-26.5 "
      "-52,-78.5 z\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:3px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path162\" />"
      "    <path"
      "       class=\"st0\""
      "       d=\"m 473.6,165.7 c 0,0 -3.3,-0.7 -9.3,9 -6,9.7 -44,74.3 "
      "-62.3,98.3 -18.3,24 -109.7,137 -109.7,137 0,0 -21.7,3.8 -45.2,11.6 "
      "-2.1,0.7 -4.3,0.8 -6.5,0.2 -15,-3.8 -76.3,-19.6 -92.3,-27.5 L "
      "107.7,335\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:3px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path164\" />"
      "    <path"
      "       class=\"st1\""
      "       d=\"m 272.4,414.4 10.7,23.3 13.2,-2.9 61.2,-56.8 c 0,0 "
      "23.6,-20.4 50.6,-32.8 24,-11 71,-31.3 71,-31.3\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-miterlimit:10\""
      "       id=\"path166\" />"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 324.5,370.1 18.9,-2.9 61.8,-85.5 c 0,0 11,-16 8.9,-25.9\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path168\" />"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 369.3,314 c 0,0 5.6,0.6 8.5,5.1\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path170\" />"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 373.4,308.9 c 0,0 5.3,0.3 8.3,5.3\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path172\" />"
      "    <path"
      "       class=\"st3\""
      "       d=\"m 354.7,196.8 c 0,0 30.7,43.6 32.5,48.5 4.3,11.5 13.9,29 "
      "13.9,29\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:3px;stroke-miterlimit:10\""
      "       id=\"path174\" />"
      "    <path"
      "       class=\"st3\""
      "       d=\"m 180.1,405.3 v -7.7 c 0,0 97.1,-146.3 138.6,-188.7 1.5,-1.5 "
      "3.3,-2.6 5.3,-3.2 l 30.7,-9 c 0,0 27.7,-46.5 64,-61.8 2.2,-0.9 2.9,-3.8 "
      "1.4,-5.6 -3.7,-4.4 -12.4,-10 -32.1,-5.9 -29.8,6.3 -56.4,17.9 -86.9,42.2 "
      "0,0 -2.2,-2 -13.4,-2.2 -11.2,-0.2 -36.7,-4.5 -43.5,-5 -6.8,-0.5 "
      "-11.6,-1.8 -23.1,3.1\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:3px;stroke-miterlimit:10\""
      "       id=\"path176\" />"
      "    <path"
      "       class=\"st3\""
      "       d=\"M 135,374.9 V 369 c 0,0 97.6,-129.2 157.5,-176.9 2.2,-1.8 "
      "3.7,-4.2 4.3,-6.9 l 4.2,-19.5\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:3px;stroke-miterlimit:10\""
      "       id=\"path178\" />"
      "    <g"
      "       id=\"g182\">"
      "      <path"
      "         class=\"st4\""
      "         d=\"m 113.7,338.8 c 3.2,3.9 6.3,7.9 9.4,11.9 l 4.7,6 c 0.8,1 "
      "1.6,2 2.3,3 l 2.2,3.1 17.9,24.5 -0.5,-0.4 62.5,17.9 15.6,4.5 7.8,2.2 c "
      "0.6,0.2 1.3,0.4 1.9,0.5 0.6,0.1 1.2,0.2 1.8,0.2 1.2,0 2.5,-0.2 3.6,-0.6 "
      "10,-3.9 20,-8.2 30.3,-11.7 l 0.2,-0.1 0.2,0.1 c 2.7,1 5.4,2.1 8.1,3.3 "
      "2.7,1.2 5.3,2.4 7.9,3.8 -2.7,-1.2 -5.4,-2.2 -8.2,-3.2 -2.7,-1 -5.5,-1.9 "
      "-8.3,-2.7 h 0.4 c -5,2.2 -9.9,4.5 -14.9,6.4 l -15.1,6 c -1.4,0.5 "
      "-2.9,0.7 -4.4,0.7 -0.7,0 -1.5,-0.1 -2.2,-0.3 -0.8,-0.2 -1.3,-0.4 "
      "-2,-0.6 l -7.8,-2.2 -15.6,-4.5 -62.5,-17.9 -0.3,-0.1 -0.2,-0.3 "
      "-17.9,-24.5 -2.3,-3 c -0.7,-1 -1.4,-2.1 -2.1,-3.1 l -4.2,-6.3 c "
      "-2.7,-4.2 -5.5,-8.4 -8.3,-12.6 z\""
      "         style=\"fill:#43708f\""
      "         id=\"path180\" />"
      "    </g>"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 187.7,324.4 c 0,0 2,-28.6 23,-48.6 17.8,-16.9 31.8,-21.1 "
      "35.7,-22 0.7,-0.2 1.4,0 2,0.3 l 16.6,10.4 c 0,0 -46.9,11.9 -56.1,71.1\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path184\" />"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 262.1,262.9 c 0,0 -17.7,0.1 -37.4,24.6 C 205,312 203.2,336 "
      "203.2,336\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path186\" />"
      "    <path"
      "       class=\"st2\""
      "       d=\"m 236.2,257.3 c 0,0 2.5,-4.5 4.5,-6 2,-1.5 6.3,-3.5 "
      "11.5,-0.3 5.2,3.2 12.3,7.3 13.3,12.5 1,5.2 -7,13 -7.8,13.5 -0.8,0.5 "
      "-31.5,39.5 -43,54.5 0,0 -6,6.8 -10.5,4.8 -4.5,-2 -6.3,-7.3 -11,-9 "
      "-4.7,-1.7 -8.3,-4.3 -8.3,-6.8 0,-2.5 1.8,-5 4.5,-7\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-width:2px;stroke-linecap:round;"
      "stroke-linejoin:round;stroke-miterlimit:10\""
      "       id=\"path188\" />"
      "    <g"
      "       id=\"g192\">"
      "      <path"
      "         class=\"st4\""
      "         d=\"m 189.4,332.5 c -1.3,2.1 -2.8,4.2 -4.1,6.3 l -4.5,6 "
      "-9.1,11.9 -9.1,11.9 -4.5,6 c -1.6,1.9 -3.3,3.8 -4.9,5.7 1.3,-2.1 "
      "2.8,-4.2 4.1,-6.3 l 4.5,-6 9.1,-11.9 9.1,-11.9 4.5,-6 c 1.6,-1.9 "
      "3.2,-3.8 4.9,-5.7 z\""
      "         style=\"fill:#43708f\""
      "         id=\"path190\" />"
      "    </g>"
      "    <line"
      "       class=\"st5\""
      "       x1=\"197\""
      "       y1=\"302\""
      "       x2=\"207.7\""
      "       y2=\"308.70001\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line194\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"201.7\""
      "       y1=\"292.79999\""
      "       x2=\"212.3\""
      "       y2=\"299.5\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line196\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"207.8\""
      "       y1=\"284.20001\""
      "       x2=\"218.5\""
      "       y2=\"290.79999\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line198\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"214.3\""
      "       y1=\"277\""
      "       x2=\"225\""
      "       y2=\"283.70001\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line200\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"220.8\""
      "       y1=\"270.29999\""
      "       x2=\"231.5\""
      "       y2=\"277\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line202\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"228.3\""
      "       y1=\"265\""
      "       x2=\"237.5\""
      "       y2=\"270.5\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line204\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"236\""
      "       y1=\"261.20001\""
      "       x2=\"243.8\""
      "       y2=\"266.20001\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line206\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"242.5\""
      "       y1=\"258.20001\""
      "       x2=\"249.7\""
      "       y2=\"262.79999\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line208\" />"
      "    <g"
      "       id=\"g212\">"
      "      <path"
      "         class=\"st4\""
      "         d=\"m 272.6,399.9 c 26.2,-33.8 52.1,-68 77.4,-102.5 12.6,-17.3 "
      "25.1,-34.7 37.4,-52.2 l 9.1,-13.3 c 3,-4.4 5.9,-8.9 8.9,-13.4 l "
      "2.2,-3.4 2.1,-3.4 4.3,-6.8 c 1.5,-2.2 2.8,-4.6 4.1,-6.9 l 4,-7 -3.9,7 c "
      "-1.3,2.3 -2.6,4.7 -4,6.9 l -4.2,6.8 -2.1,3.4 -2.2,3.4 c -2.9,4.5 -5.8,9 "
      "-8.8,13.5 l -9,13.3 c -12.1,17.7 -24.5,35.2 -37,52.6 -25.1,34.7 "
      "-50.7,69.1 -76.6,103.2 -0.3,0.4 -1,0.5 -1.4,0.2 -0.5,-0.3 -0.6,-1 "
      "-0.3,-1.4 z\""
      "         style=\"fill:#43708f\""
      "         id=\"path210\" />"
      "    </g>"
      "    <g"
      "       id=\"g216\">"
      "      <path"
      "         class=\"st4\""
      "         d=\"m 243.3,424.5 c 1.5,3.7 2.7,7.5 3.8,11.3 1.1,3.8 2.1,7.6 "
      "2.9,11.6 -1.5,-3.7 -2.7,-7.5 -3.8,-11.3 -1.1,-3.9 -2.1,-7.7 -2.9,-11.6 "
      "z\""
      "         style=\"fill:#43708f\""
      "         id=\"path214\" />"
      "    </g>"
      "    <g"
      "       id=\"g280\""
      "       style=\"image-rendering:auto;fill:#e6e6e6\">"
      "      <defs"
      "         id=\"defs219\">"
      "        <path"
      "           id=\"SVGID_1_\""
      "           d=\"m 337.4,375 c -0.5,-0.5 -0.8,6.3 -0.8,6.3 0,0 -1,5.5 "
      "4.3,4 5.3,-1.5 23.3,-27 69,-45.8 45.7,-18.8 63.9,-34.8 72.8,-49.3 "
      "4.9,-8 18.5,-39.8 3.3,-69.5 -15.2,-29.7 -29.7,-6.8 -32.7,-2.3 -3,4.5 "
      "-22.7,34.8 -41.5,62.8 -19.3,28.8 -52.2,73.6 -61.8,84.8 -8.8,10.2 "
      "-10.8,10.8 -12.6,9 z\" />"
      "      </defs>"
      "      <clipPath"
      "         id=\"SVGID_2_\">"
      "        <path"
      "           d=\"m 337.4,375 c -0.5,-0.5 -0.8,6.3 -0.8,6.3 0,0 -1,5.5 "
      "4.3,4 5.3,-1.5 23.3,-27 69,-45.8 45.7,-18.8 63.9,-34.8 72.8,-49.3 "
      "4.9,-8 18.5,-39.8 3.3,-69.5 -15.2,-29.7 -29.7,-6.8 -32.7,-2.3 -3,4.5 "
      "-22.7,34.8 -41.5,62.8 -19.3,28.8 -52.2,73.6 -61.8,84.8 -8.8,10.2 "
      "-10.8,10.8 -12.6,9 z\""
      "           style=\"overflow:visible\""
      "           id=\"path221\" />"
      "      </clipPath>"
      "      <g"
      "         class=\"st6\""
      "         clip-path=\"url(#SVGID_2_)\""
      "         id=\"g278\""
      "         style=\"fill:#e6e6e6\">"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 471.5,201.3 c 0,0 17.2,46.3 22.7,56.5\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path224\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 466,200.5 c 0,0 7.5,24 10.7,32.5 3.2,8.5 15.2,34.2 "
      "16.7,37\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path226\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 460.8,201 c 0,0 10.1,30.7 15,43.7 4.8,13 12.1,28.1 "
      "14.2,34.6\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path228\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 457.4,204.1 c 0,0 3.4,12.5 6.5,23.8 3.1,11.3 7.6,25.7 "
      "12.1,34.8 4.5,9.1 9.1,19.2 10.8,24.4\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path230\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 453.5,208.8 c 0,0 6.1,29.4 13.8,48.2 7.6,18.8 "
      "13.2,30.2 15.5,36.3\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path232\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 449.6,214 c 0,0 5,27.9 10.1,41.2 5,13.2 16,35.2 "
      "19.2,44.3\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path234\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 445.7,218.7 c 0,0 3.2,24.6 9.9,43.9 6.7,19.4 14.3,33.9 "
      "18.1,42.3\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path236\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 442.1,223.5 c 0,0 2.8,28.9 9.5,48 6.7,19.2 18.8,37.4 "
      "19,38\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path238\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 437.7,229.5 c 0,0 2.4,27.6 7.8,43.9 5.4,16.4 15.3,32.6 "
      "19.4,39.8\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path240\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 433.6,235.8 c 0,0 1.5,28.9 7.1,45.1 7.4,21.8 15.6,33.1 "
      "17.3,36.5\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path242\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 429.3,241.7 c 0,0 0.6,28.9 5.6,45.8 5,16.9 16.9,33.7 "
      "16.9,34.3\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path244\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 425.4,247.9 c 0,0 0.3,26.3 3.4,41.3 4.5,21.8 12.5,33.1 "
      "15.5,37.1\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path246\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 420.5,254 c 0,0 -1.7,23.5 2.2,40.4 5.6,23.8 14.9,35.4 "
      "14.9,35.4\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path248\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 415.5,262 c 0,0 -2.3,19.1 0.2,34.1 3.2,19.2 9.9,31.5 "
      "11.9,38.7\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path250\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 409.4,276.4 c -0.9,11.2 -1.4,22.5 0.9,33.5 0.7,3.3 "
      "1.6,6.5 2.6,9.6 2,6.9 4.1,13.8 6.1,20.7\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path252\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 403.9,285 c -1.3,9.4 -2.6,19.1 -0.8,28.5 0.5,2.9 "
      "1.4,5.7 2.1,8.6 1.7,6.9 2.8,14 3.8,21.1\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path254\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 396.6,295.5 c -0.8,8.6 0.5,17.2 1.8,25.7 1.3,8.3 "
      "2.6,16.6 3.9,24.9\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path256\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 390.9,305.8 c -3.2,14.8 2,29.9 2.5,45\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path258\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 384.9,315.3 c -0.4,13.1 0.3,26.2 2.1,39.1\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path260\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 378.6,324.4 c -0.8,12 -0.9,24 -0.2,35.9\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path262\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 373.7,331.7 c -0.8,10.7 -1,21.3 -0.6,32\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path264\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 368.6,338 c -0.4,9.6 -0.8,19.1 -1.2,28.7\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path266\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 364.1,344.4 c -0.3,8.9 -1.3,17.8 -3.1,26.6\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path268\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 359.4,350.7 c -1.7,9.2 -3.5,18.5 -5.2,27.7\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path270\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 353.2,358 c -1.1,8.2 -2.1,16.4 -3.2,24.6\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path272\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 349.2,362.8 c -0.8,7.7 -2.1,15.4 -3.7,22.9\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path274\" />"
      "        <path"
      "           class=\"st2\""
      "           d=\"m 344.4,366.4 c -0.4,7.8 -3.1,15.3 -4.1,23\""
      "           "
      "style=\"fill:#e6e6e6;stroke:#43708f;stroke-width:2px;stroke-linecap:"
      "round;stroke-linejoin:round;stroke-miterlimit:10\""
      "           id=\"path276\" />"
      "      </g>"
      "    </g>"
      "    <g"
      "       id=\"g284\">"
      "      <path"
      "         class=\"st4\""
      "         d=\"m 293.2,413.3 c 0.7,3 1.1,6.1 1.4,9.2 0.3,3.1 0.5,6.2 "
      "0.4,9.3 -0.7,-3 -1.1,-6.1 -1.4,-9.2 -0.3,-3.1 -0.5,-6.2 -0.4,-9.3 z\""
      "         style=\"fill:#43708f\""
      "         id=\"path282\" />"
      "    </g>"
      "    <line"
      "       class=\"st5\""
      "       x1=\"195\""
      "       y1=\"312.5\""
      "       x2=\"203.39999\""
      "       y2=\"317.39999\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line286\" />"
      "    <line"
      "       class=\"st5\""
      "       x1=\"193.3\""
      "       y1=\"320.79999\""
      "       x2=\"200.3\""
      "       y2=\"324.79999\""
      "       "
      "style=\"fill:none;stroke:#43708f;stroke-linecap:round;stroke-linejoin:"
      "round;stroke-miterlimit:10\""
      "       id=\"line288\" />"
      "  </g>"

      "  <path"
      "     id=\"wheel-down\""
      "     d=\"m 471.89446,1335.9125 -143.24233,221.6357 62.96013,55.6357 "
      "-232.4457,155.5869 64.55242,-303.9486 62.96012,55.6358 "
      "143.24233,-221.6355 z\""
      "     "
      "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;stroke-width:11."
      "0944px;stroke-linejoin:bevel\""
      "     bx:shape=\"arrow 2535.026 4044.696 330.142 143.581 35.895 168.045 "
      "0 1@dbf8f4e0\""
      "     class=\"visited\" />"

      "  <path"
      "     id=\"wheel-up\""
      "     d=\"M 780.9929,800.71842 935.98137,590.16542 876.16941,529.96747 "
      "1116.5946,391.95268 1035.6675,690.49495 975.85553,630.297 "
      "820.86708,840.85007 Z\""
      "     "
      "style=\"fill:#009e9e;fill-opacity:0.33;stroke:#416b92;stroke-width:11."
      "0943px;stroke-linejoin:bevel\""
      "     bx:shape=\"arrow 2535.02 4044.686 330.142 143.581 35.895 168.045 0 "
      "1@fdee07bb\""
      "     class=\"visited\" />"

      "</svg>";
}

void MainWindow::help() {
  QDesktopServices::openUrl("file:" + QCoreApplication::applicationDirPath() +
                            "\\help\\help.html");
}

void MainWindow::toggleCallouts() {
  if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
    return;
  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);
  view->actionToggleCallouts();
}

void MainWindow::on_actionDataManagerDialog_triggered() {

  if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
    return;
  auto childs = ui->tabWidget->currentWidget()->children();
  auto graph = childs.at(1);
  View *view = qobject_cast<View *>(graph);
  view->actionShowDataManager();
}

void MainWindow::on_actionsetZeroX_triggered() {
  // //
  // if (ui->tabWidget->currentWidget()->objectName() == "Statistics")
  //   return;

  // auto childs = ui->tabWidget->currentWidget()->children();
  // auto graph = childs.at(1);
  // View *view = qobject_cast<View *>(graph);

  // qDebug() << "manual X set";
  // // return;
  // for (auto serie : view->m_chart->series()) {
  //   auto XY_serie = qobject_cast<QXYSeries *>(serie);
  //   auto offset = view->axis_travel->min();
  //   qDebug() << "offset" << offset;
  //   auto points_copy = XY_serie->points();
  //   for (auto &point : points_copy) {
  //     point.setX(point.x() - offset);
  //   }
  //   XY_serie->replace(points_copy);
  //   for (auto haptic_point_name : view->data_loader->hapticPoints.keys()) {
  //     // replace the point in the map
  //     auto haptic_point = view->data_loader->hapticPoints[haptic_point_name];
  //     haptic_point.setX(haptic_point.x() - offset);
  //     view->data_loader->hapticPoints[haptic_point_name] = haptic_point;
  //   }
  // }
}

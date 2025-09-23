/****************************************************************************
**
**
****************************************************************************/

#include "view.h"
#include "ui_data_dialog.h"
#include "view_utilities.h" 
#include <cmath>

#include <QtCharts/QChartGlobal>

extern bool calloutDragging;

void checkAndCreateIniFileWithUUID(const QString &UUID);

View::View(QWidget *parent, QString s_path)
    : QGraphicsView(new QGraphicsScene, parent) {

  this->setVisible(false);
  this->setRenderHint(QPainter::Antialiasing);
  setDragMode(QGraphicsView::NoDrag);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_chart->setParent(this);
  this->folder = s_path;
  m_chart->setTheme(QChart::ChartThemeBlueNcs);


  QString path;
  if (argFile) {
    path = fileNameArg;
    argFile = false;
    this->folder = fileNameArg;
    this->serieName = fileNameArg;
  } else if (s_path != "") {
    this->folder = s_path;
    this->serieName = s_path;
    path = s_path;
    fileNameArg = path;
    argFile = false;
  }

  QFileInfo fi(serieName);

  if (fi.exists() and fi.suffix().contains("blf")) {
    qDebug() << "Valid file loaded in view :" << fi;
  } else {
    qDebug() << "the file does not contain a recognized suffix!" << fi;

  }


if (serieName.contains(".blf")) {

    qDebug() << "blf file here!" << serieName;
    this->data.fi = fi;

    // QFileInfo settingsfi(fi.path() + "\\settings.ini");

    if (this->data.readBlfUUID(fi))
      checkAndCreateIniFileWithUUID(this->data.BlfFileParser->loggingBlockUUID);

    // QFileInfo settingsfi(serieName + ".ini");
    // if (settingsfi.exists()) {
    //   this->data.DataManager->settings =
    //       new QSettings(settingsfi.absoluteFilePath(),
    //       QSettings::IniFormat);

    //   qDebug() << "local settings found";
    // } else if (this->data.readBlfUUID(fi)) {
    //   // this->data.DataManager->settings = new QSettings();
    //   qDebug() << "searching for remote settings";
    //   checkAndCreateIniFileWithUUID(
    //       this->data.BlfFileParser->loggingBlockUUID);

    //   qDebug() << this->data.BlfFileParser->loggingBlockUUID;
    // }

    this->data.loadDBC();

    qDebug() << data.DataManager->settings->status();

    this->data.parseBLF(fi);
    this->good = true;

    connect(&data, &Data::dataReady, this, &View::constructDataChart);
    connect(&data, &Data::parsingProgress, this, &View::handleParsingProgress);

    connect(data.DataManager, &data_Dialog::cosmeticReset, this,
            &View::constructDataChart); //&View::handleCosmeticReset

    m_coordX->hide();
    m_coordY->hide();

    qDebug() << "done here!";
  }

  if (!this->good)
    return;

  this->setVisible(true);
  setAcceptDrops(true);

  // m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));
  m_chart->setTitle(fi.baseName());
  m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
  m_chart->setAnimationOptions(QChart::NoAnimation);
  this->setRenderHint(QPainter::Antialiasing);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  scene()->addItem(m_chart);

  qDebug() << "View constructor return!";
  //    QObject::connect(this, &View::mouseDoubleClickEvent,
  //    &View::handle_mouseDoubleClickEvent);
}





void View::constructSerie(QList<QPointF> &points, QString name, QString symbol,
                          QColor color, QValueAxis *axis) {
  // qDebug() << "View::constructSerie" << name << symbol << (symbol == "─")
          //  << color.name() << points.size() << points.first() << points.last();
  static int drawOrder = 100;

  QLineSeries *lineSerie = nullptr;
  QScatterSeries *scatterSerie = nullptr;

  if (symbol == "─") {
    if (lineSerie == nullptr)
      lineSerie = new QLineSeries();

    lineSerie->replace(points);

    lineSerie->setPen(QPen(color, 2, Qt::SolidLine));
    lineSerie->setColor(color);
    lineSerie->setUseOpenGL(true);
    lineSerie->setName(name);
    lineSerie->setObjectName(name);
    lineSerie->setProperty("my_type", QVariant(QObject::tr("current")));
    lineSerie->setProperty("my_order", QVariant(++drawOrder));
    lineSerie->setProperty("my_color", QVariant(color.rgba()));
    lineSerie->setProperty("my_symbol_size", QVariant(20));
    lineSerie->setProperty("unit", QVariant(QObject::tr("current")));

    axis->show();
    m_chart->addSeries(lineSerie);
    lineSerie->attachAxis(axisX);
    lineSerie->attachAxis(axis);
    lineSerie->deselectAllPoints();

  } else {

    if (scatterSerie == nullptr)
      scatterSerie = new QScatterSeries();

    scatterSerie->replace(points);
    scatterSerie->setPointsVisible(true);

    // newserie->setPen(QPen(color, 2, Qt::SolidLine));
    scatterSerie->setColor(color);

    // newserie->setColor(QColor(255, 150, 0));
    scatterSerie->setMarkerSize(6);
    // newserie->setBorderColor("transparent");
    scatterSerie->setUseOpenGL(true);
    scatterSerie->setName(name);
    scatterSerie->setObjectName(name);
    scatterSerie->setProperty("my_type", QVariant(QObject::tr("current")));
    scatterSerie->setProperty("my_order", QVariant(++drawOrder));

    if (symbol != "•") {
      auto font = QFont("Arial", 20, QFont::Bold);
      auto fm = QFontMetrics(font);
      auto rectangle = fm.boundingRect(symbol);
      auto w = fm.horizontalAdvance(symbol);
      auto h = rectangle.height();
      auto s = w > h ? w : h;
      QImage sprite(s, s, QImage::Format_ARGB32);
      sprite.fill(Qt::transparent);
      QPainter painter(&sprite);
      painter.setOpacity(1.0);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.setRenderHint(QPainter::Antialiasing, false);
      painter.setPen(QPen(color));
      painter.setFont(QFont("Arial", 20, QFont::Bold));
      painter.drawText(sprite.rect(), Qt::AlignCenter, symbol);
      painter.end();

      scatterSerie->setProperty("my_symbol", QVariant(sprite));
      scatterSerie->setProperty("my_color", QVariant(color.rgba()));
      scatterSerie->setProperty("my_symbol_size", QVariant(20));
      // newserie->setObjectName("mfu_error_" + QString::number(index + 1));
      scatterSerie->setProperty("sprite", QVariant(sprite));
      scatterSerie->setBrush(sprite);
      scatterSerie->setPen(QColor(Qt::transparent));
      scatterSerie->setMarkerSize(30.0);
    }

    scatterSerie->setProperty("unit", axis->property("unit"));
    scatterSerie->setProperty("format", axis->property("format"));
    axis->show();
    m_chart->addSeries(scatterSerie);
    scatterSerie->attachAxis(axisX);

    scatterSerie->attachAxis(axis);

    // if (name.contains("current", Qt::CaseInsensitive)) {
    //   QLineSeries *current_sleep_avg = new QLineSeries();
    //   // current_sleep_avg->setMarkerSize(6);
    //   current_sleep_avg->setColor(QColor(10, 200, 20));
    //   // set line width
    //   current_sleep_avg->setPen(QPen(QColor(10, 200, 20), 4));
    //   // current_sleep_avg->setBorderColor("transparent");

    //   current_sleep_avg->setName("Current [A] 41ms MovingAvg");
    //   current_sleep_avg->setObjectName("current_sleep_avg");
    //   current_sleep_avg->setProperty("my_type",
    //                                  QVariant(QObject::tr("current")));
    //   current_sleep_avg->setProperty("my_order", QVariant(97));
    //   m_chart->addSeries(current_sleep_avg);

    //   calculateMovingAverage(scatterSerie, current_sleep_avg);

    //   current_sleep_avg->attachAxis(axisX);
    //   current_sleep_avg->attachAxis(axis);
    // }
  }
}

void View::checkAndCreateIniFileWithUUID(const QString &UUID) {
  QDir templatesDir(QCoreApplication::applicationDirPath() + "/templates");

  // Check if the directory exists, create it if it does not
  if (!templatesDir.exists()) {
    qDebug() << "The templates directory does not exist. Creating it now.";
    if (!templatesDir.mkpath(".")) { // Creates the directory and any necessary
                                     // parent directories
      qDebug() << "Failed to create the templates directory.";
      return;
    }
  }

  // Set filter for *.ini files
  templatesDir.setNameFilters(QStringList() << "*.ini");
  templatesDir.setFilter(QDir::Files);
  QFileInfoList fileList = templatesDir.entryInfoList();

  // Search through each .ini file for the blockUUID

  foreach (QFileInfo fileInfo, fileList) {
    QSettings settings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
    if (settings.value("blockUUID").toString() == UUID && UUID != "") {
      qDebug() << "remote settings found in" << fileInfo.fileName()
               << fileInfo.absoluteFilePath();
      templateSettingsFound = true;
      QFile::copy(fileInfo.absoluteFilePath(), serieName + ".ini");

      this->data.DataManager->settingsTemplate =
          new QSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
      this->data.DataManager->settingsTemplate->setValue("blockUUID", UUID);
      this->data.DataManager->settingsTemplate->sync();

      // copy the template file fileInfo.absoluteFilePath() as serieName +
      // ".ini"

      break;
    }
  }

  QFileInfo settingsfi(serieName + ".ini");
  // if (settingsfi.exists()) {
  this->data.DataManager->settings =
      new QSettings(settingsfi.absoluteFilePath(), QSettings::IniFormat);
  this->data.DataManager->settings->setValue("blockUUID", UUID);
  this->data.DataManager->settings->sync();
  // }
  // If UUID not found, create a new .ini file
  if (!templateSettingsFound) {
    QString newFilePath = templatesDir.absolutePath() + "/" + UUID + ".ini";
    this->data.DataManager->settingsTemplate =
        new QSettings(newFilePath, QSettings::IniFormat);

    // this->data.DataManager->settingsTemplate->beginGroup("General");
    this->data.DataManager->settingsTemplate->setValue("blockUUID", UUID);
    this->data.DataManager->settingsTemplate->endGroup();
    this->data.DataManager->settingsTemplate->sync();

    qDebug() << "New .ini file created with UUID:" << UUID << newFilePath;
  }
}

void View::constructDataChart() {
  if (this->constructionInProgress)
    return;
  constructionInProgress = true;
  qDebug() << "constructDataChart";

  resetView();
  initialScale.clear();

  qDebug() << this->data.DataManager->settings->fileName();
  if (this->data.DataManager->settings == nullptr) {
    qDebug() << "Settings is nullptr";
  } else {
    qDebug() << "Settings is not nullptr";
    templateSettingsFound = true;
  }

  // checkAndCreateIniFileWithUUID(this->data.BlfFileParser->loggingBlockUUID);

  if (!templateSettingsFound) // QFile::exists(this->data.DataManager->settings->fileName())
  {

    QString msgText = "Settings file not found for logging block UUID:" +
                      this->data.BlfFileParser->loggingBlockUUID;
    msgText +=
        "\n You have to config the BLF settings before plotting the data.";
    msgText += "\n Below is a list with the DBC files in the CANoe simulation.";
    msgText += "\n You have to drop the DBC files containing the bus "
               "communication definition in the setting dialog. \n";

    QStringList dbcFilesList = this->data.BlfFileParser->blfHeader.split(";");
    QString infoText;

    for (int i = 0; i < dbcFilesList.size(); i += 2) {
      QString dbcPath = dbcFilesList[i];

      if ((dbcPath.contains("ueberwach", Qt::CaseInsensitive)) or
          (dbcPath.contains("monitori", Qt::CaseInsensitive)) or
          (dbcPath.contains("general", Qt::CaseInsensitive)) or
          (dbcPath.contains("allgemein", Qt::CaseInsensitive)) or
          (dbcPath.contains("loggin", Qt::CaseInsensitive)))
        continue;

      infoText += dbcPath + '\n';
    }

    // QString folderPath = "file:///C:/logsbuffer";
    // infoText += QString("Click <a href=\"%1\">here</a> to open the
    // folder.") .arg(folderPath);

    qDebug() << "Settings file not found!" << dbcFilesList << infoText;
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);

    msgBox.setText(msgText);
    msgBox.setInformativeText(infoText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);

    this->data.DataManager->show();
    msgBox.exec();
    constructionInProgress = false;
    templateSettingsFound = true;
    return;
  }

  // qDebug() << "inainte de tabele si chestii";

  auto axesTable = this->data.DataManager->ui->axesTable;
  auto seriesTable = this->data.DataManager->ui->seriesTable;
  QMap<QString, QValueAxis *> axes;

  // qDebug() << "inainte de bucla";

  for (int row = 0; row < axesTable->rowCount() - 1; row++) {
    auto axisName = axesTable->item(row, 0)->text();
    auto axisUnit = axesTable->item(row, 2)->text();
    auto axisFmt =
        this->data.DataManager->axisTypes.value(axesTable->item(row, 1)->text())
            .toUtf8();
    qDebug() << "axisFmt" << axisFmt;
    auto celsius = QString::fromLatin1("°C").toUtf8();
    auto ohm = QString::fromLatin1("Ω").toUtf8();
    if (axisUnit == "°C")
      axisUnit = celsius;
    else if (axisUnit == "Ω")
      axisUnit = ohm;

    auto axisformat = axisFmt + axisUnit;
    auto axisColor = axesTable->item(row, 3)->background().color();
    auto axisAlignment = this->data.DataManager->axisPositions.value(
        axesTable->item(row, 4)->text());

    QValueAxis *axis = nullptr;
    if (row == 0) {
      this->axisX = constructValueAxis(axisName, axisName, axisformat,
                                       axisColor, axisAlignment);
      // qDebug() << "X axis inserted. format:" << axisFmt;
      axis = axisX;
    } else
      axis = constructValueAxis(axisName, axisName, axisformat, axisColor,
                                axisAlignment);

    axis->setProperty("unit", axisUnit);
    axis->setProperty("format",
                      axisformat); // axesTable->item(row, 1)->text()
    axes.insert(axisName, axis);
    axis->hide();
    // qDebug() << "axisName" << axisName << axisAlignment << axisColor
    //          << axisFmt << axisUnit;
  }

  this->axisX2 = new QValueAxis(m_chart);
  axisX2->setObjectName("axisX2");
  axisX2->setTickCount(11);
  axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
  axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
  axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
  axisX2->setMinorTickCount(3);
  axisX2->setLabelFormat("%4.1Td");
  axisX2->setTickAnchor(0);
  m_chart->addAxis(axisX2, Qt::AlignBottom);

  // qDebug() << this->data.SysVars->keys ();
  // qDebug() << this->data.DBC->keys ();

  for (int row = 0; row < seriesTable->rowCount(); row++) {
    // qDebug() << seriesTable->item(row, 0)->checkState()
    //          << seriesTable->item(row, 2)->text()
    //          << seriesTable->item(row, 3)->text();

    if (seriesTable->item(row, 0)->checkState() &&
        ((seriesTable->item(row, 2) ? seriesTable->item(row, 2)->text().toInt() > 0: false))) {
      auto yAxisName = seriesTable->item(row, 4)->text();

      if (yAxisName == "none") {
        continue;
      }

      auto symbol = seriesTable->item(row, 5)->text();
      auto color = seriesTable->item(row, 6)->background().color();
      QString serieName = seriesTable->item(row, 3)->text();
      QString fullName = seriesTable->item(row, 9)->text();

      QString channel = fullName.contains("(ch_")
                            ? stringBetween(fullName, "(ch_", ")")
                            : fullName.mid(0, fullName.indexOf("::"));
      QString caption = fullName.contains("SysVar")
                            ? fullName.mid(fullName.lastIndexOf("::") + 2)
                            : fullName;

      if (seriesTable->item(row, 1)->text().contains("SysVar") || seriesTable->item(row, 1)->text().contains("CSV")) {
        // qDebug() << "SYSVAR" << seriesTable->item(row, 9)->text()
        //          << "Y axis:" << yAxisName;

        auto sysVarName = fullName.replace("SysVar", "");
        constructSerie(this->data.SysVars->operator[](sysVarName).points,
                       serieName, symbol, color, axes.value(yAxisName));

        //

        //

      } else {
        // constructSerie(this->data->DBC->, serieName, symbol, color,
        //                axes.value(yAxisName));
        // qDebug() << "fullName" << fullName;

        for (auto &message : *this->data.DBC) {
          for (auto &signal : message) {
            // Extract base name from fullName (before '(')
            QString baseName = fullName.left(fullName.indexOf('('));
            if (baseName == signal.name) {
              qDebug() << signal.name << channel
                       << signal.series->contains(channel.toUInt())
                       << signal.series->keys();
              if (signal.series->contains(channel.toUInt())) {
                auto seriesHash = signal.series;
                auto points = seriesHash->value(channel.toUInt());
                constructSerie(points, serieName, symbol, color,
                               axes.value(yAxisName));
              }
            }
          }
        }
      }
    }
  }

  // for (auto abstractSerie : m_chart->series()) {
  //   auto xySerie = qobject_cast<QXYSeries *>(abstractSerie);
  //   if (xySerie != nullptr) {
  //     auto delta = xySerie->at(0);
  //     if (delta.x() == 0)
  //       continue;
  //     for (auto point : xySerie->points()) {
  //       xySerie->replace(point, QPointF(point.x() - delta.x(), point.y()));
  //     }
  //   }
  // }

  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
  m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

  // m_chart->legend()->setInteractive(true);
  // m_chart->legend()->detachFromChart();
  // m_chart->legend()->setBackgroundVisible();

  m_chart->legend()->update();
  m_chart->legend()->setVisible(true);
  m_chart->setVisible(true);

  adjustAxesRange(m_chart);

  QObject::disconnect(this->axisX);
  QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                   &View::handleTimeRangeChanged, Qt::QueuedConnection);

  // QObject::disconnect(this->axisX2);
  // QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
  //                  &View::handleTime2RangeChanged);

  const auto markers = m_chart->legend()->markers();
  for (QLegendMarker *marker : markers) {
    QObject::disconnect(marker);
    QObject::connect(marker, &QLegendMarker::clicked, this,
                     &View::handleMarkerClicked, Qt::QueuedConnection);
  }

  // for (auto axis : m_chart->axes())
  // {
  //     auto valueAxis = qobject_cast<QValueAxis *>(axis);

  //     valueAxis->setVisible(true);
  //     adjustAxisRange(valueAxis);

  // }

  if (axisX2 != nullptr)
    axisX2->setVisible(false);

  //        applyNiceNumbers();

  for (auto axis : m_chart->axes()) {

    auto valueAxis = qobject_cast<QValueAxis *>(axis);

    valueAxis->setVisible(true);
    adjustAxisRange(valueAxis);

    if (axis->orientation() == Qt::Horizontal)
      continue;
    QObject::disconnect(qobject_cast<QValueAxis *>(axis));
    QObject::connect(qobject_cast<QValueAxis *>(axis),
                     &QValueAxis::rangeChanged, this, &View::proc_rangeChanged,
                     Qt::QueuedConnection);
  }

  hideUnusedAxes(m_chart);
  axisX->setVisible();

  emit dataChanged();
  this->data.DataManager->initOngoing = false;
  this->constructionInProgress = false;
  // scroll the chart a bit to trigger the x axis lable change
  m_chart->scroll(1, 1);
  m_chart->scroll(-1, -1);
}


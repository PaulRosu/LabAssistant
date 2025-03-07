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
bool labViewEval;
dataLoader *tempDataLoader;

QMap<QString, QColor> View::m_colors;
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

  qDebug() << "labViewEval state" << labViewEval;

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

  this->data_loader = new dataLoader(path);
  if (this->data_loader->type == dataLoader::BLF_Export) {

    construct_BLF_export();

    if (!this->good)
      return;
    this->setVisible(true);
    setAcceptDrops(true);
    return;
  }

  QFileInfo fi(serieName);

  if (fi.exists() and
      (fi.suffix().contains("csv") or fi.suffix().contains("CSV") or
       fi.suffix().contains("zs2") or fi.suffix().contains("ZS2") or
       fi.suffix().contains("txt") or fi.suffix().contains("blf"))) {
    qDebug() << "Valid file loaded in view :" << fi;
  } else {
    qDebug() << "the file does not contain a recognized suffix!" << fi;

    if (!labViewEval) {

      this->data_loader = new dataLoader(path);
      if (!data_loader->good)
        return;

      if (data_loader->type == data_loader->LabView) {
        // construct_LabView_Chart (path);
        // construct_LabView_Chart_eval (path);
        tempDataLoader = this->data_loader;
        construct_LabView_Chart(path);
      }
    } else {
      qDebug() << "tempDataLoader->type" << tempDataLoader->type;
      construct_LabView_Chart_eval(path);
    }

    //        return;
  }

  init_colors();

  if ((serieName.contains(".txt")) or (serieName.contains(".txt"))) {

    this->data_loader = new dataLoader(path);
    if (!data_loader->good)
      return;

    if (data_loader->type == data_loader->LabView) {
      construct_LabView_Chart(path);
    } else {
      construct_robot_haptic_chart(path);
    }
  }

  else if ((serieName.contains(".zs2")) or (serieName.contains(".ZS2"))) {

    // construct_robot_haptic_chart (path);
    construct_haptic_chart(path);
  }

  else if (serieName.contains(".csv", Qt::CaseInsensitive)) {

    // if (isGraphtec(path)) {

    //   qDebug() << "graphtec file dropped!";
    // } else if (isIllum(path)) {

    //   qDebug() << "Illum file dropped!";

    // } else {

    //   construct_MFU_chart(path);
    // }

  
  
   // test the new csv parser
  qDebug() << "CSV file dropped: using new CSV parser";

    // Disconnect existing connections to prevent duplicates
    // disconnect(&data, &Data::dataReady, this, &View::constructDataChart);
    // disconnect(&data, &Data::parsingProgress, this, &View::handleParsingProgress);
    
    // Connect signals before parsing
    // connect(&data, &Data::dataReady, this, &View::constructDataChart);
    // connect(&data, &Data::parsingProgress, this, &View::handleParsingProgress);
    
    // Set file info and parse
    this->data.fi = fi;

    // Initialize CSV settings before parsing
    this->data.initializeCsvSettings(fi.filePath());

    connect(&data, &Data::dataReady, this, &View::constructDataChart);
    connect(&data, &Data::parsingProgress, this, &View::handleParsingProgress);

    connect(data.DataManager, &data_Dialog::cosmeticReset, this,
            &View::constructDataChart); //&View::handleCosmeticReset


    this->data.parseCSV(fi);
    
    this->good = true;


    m_coordX->hide();
    m_coordY->hide();

  
    
  
  } else if (serieName.contains(".blf")) {

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
  qDebug() << "View::constructSerie" << name << symbol << (symbol == "─")
           << color.name() << points.size() << points.first() << points.last();
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
            if (fullName.contains(signal.name)) {
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

void View::construct_robot_haptic_chart(QString path) {
  qDebug() << "ROBOT TXT IN CONSTRUCTION: " << path;

  //    if (load_robottxt_opt(path) < 0) return;
  //    qDebug()<<  "after:" << durability_errors.size();
  //    return;

  // hapticSerie* serie = new hapticSerie(this->m_chart);

  this->data_loader = new dataLoader(path);

  if (!data_loader->good)
    return;

  qDebug() << "serie.robotData.size ()" << data_loader->y_dataMap.size();

  if (data_loader->y_dataMap.size() > 0) {

    this->folder = path;

    m_chart = new QChart();
    m_chart->setParent(this);
    this->setRenderHint(QPainter::Antialiasing);

    m_chart->setTheme(QChart::ChartThemeBlueNcs);
    QFileInfo fi(serieName);
    m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

    m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
    m_chart->setAnimationOptions(QChart::NoAnimation);
    this->setRenderHint(QPainter::Antialiasing);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight);

    this->axis_travel = new QValueAxis(m_chart);
    m_chart->addAxis(axis_travel, Qt::AlignBottom);
    axis_travel->setObjectName("axisX"); // axis_travel
    axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_travel->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axis_travel->setMinorTickCount(1);
    axis_travel->setTickCount(11);
    axis_travel->setTitleText(
        "Travel [N]"); // Angle [arcdeg]//Travel [N]Running Time [days]
    axis_travel->setTitleFont(QFont("Verdana", 12, QFont::Bold));
    axis_travel->setLabelFormat("%4.1f"); //("");%4.0f%4.1f
    axis_travel->setLabelsEditable(true);

    //        this->axisX = axis_travel;

    //        this->axisX2 = new QValueAxis(m_chart);
    //        axisX2->setObjectName ("axisX2");
    //        axisX2->setTickCount (11);
    //    axisX2->setTitleText("chart time scale");
    //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
    //        axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    //        axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    //        axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine ) );
    //        axisX2->setMinorTickCount(3);
    //        axisX2->setLabelFormat("%4.1Td");
    //        axisX2->setTickAnchor (0);
    //        m_chart->addAxis(axisX2, Qt::AlignBottom);

    this->axis_force = new QValueAxis(m_chart);
    m_chart->addAxis(axis_force, Qt::AlignLeft);
    axis_force->setObjectName("axis_force");
    axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axis_force->setMinorTickCount(1);
    axis_force->setTickCount(11);
    axis_force->setTitleText("Force [N]");
    axis_force->setTitleFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLabelFormat("%4.1f");
    axis_force->setLabelsEditable(true);

    this->axis_voltage = new QValueAxis(m_chart);
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setObjectName("axis_voltage");
    //        axis_voltage->setLabelFormat("%4.1RV ");
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    //        axis_voltage->setLabelsColor(voltage->color());
    //        axis_voltage->setLinePenColor (voltage->color());
    axis_voltage->setMinorTickCount(1);
    axis_voltage->setTickCount(11);
    axis_voltage->setLabelsEditable(true);

    //        temperature->replace (*data_loader->y_dataMap["Temperature"]);
    //        data_loader->y_dataMap.remove ("Temperature");

    if (temperature->count() > 3) {
      temperature->replace(
          temperature->count() - 1,
          QPointF(temperature->at(temperature->count() - 1).x(),
                  temperature->at(temperature->count() - 1).y() - 0.0001));

      temperature->setPen(QPen(QColor(0, 100, 255), 2, Qt::SolidLine));
      if (m_colors.value("temperature").isValid())
        temperature->setPen(
            QPen(m_colors.value("temperature"), 2, Qt::SolidLine));
      // temperature->setUseOpenGL (true);
      temperature->setName("Temperature");
      temperature->setObjectName("Temperature");
      temperature->setProperty("my_type", QVariant(QObject::tr("temperature")));
      temperature->setProperty("my_order", QVariant(99));
      //          mfu_error->setProperty ("my_symbol",
      //          QVariant(QObject::tr("×")));

      QColor my_color = temperature->color();
      temperature->setProperty("my_color", QVariant(my_color.rgba()));
      temperature->setProperty("my_symbol_size", QVariant(20));
      m_chart->addSeries(temperature);

      //            QObject::connect(temperature, &QLineSeries::pressed, this,
      //            &View::handleSerieClick);
    }

    if (temperature->count() > 3) {

      this->axis_temperature = new QValueAxis(m_chart);

      axis_temperature->setObjectName("axis_temperature");

      //        axis_temperature->setTitleText("Temperature");
      //        axis_temperature->setTitleFont(QFont("Verdana", 14,
      //        QFont::Bold));
      axis_temperature->setTickCount(11);
      axis_temperature->setLabelFormat(
          QString("%4.0f" + QString::fromLatin1("°C ")).toUtf8());
      axis_temperature->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_temperature->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_temperature->setMinorTickCount(1);
      axis_temperature->setLabelsEditable(true);
      m_chart->addAxis(axis_temperature, Qt::AlignLeft);

      axis_temperature->setLabelsColor(temperature->color());
      axis_temperature->setLinePenColor(temperature->color());
      temperature->attachAxis(axis_travel);
      temperature->attachAxis(axis_temperature);
      axis_temperature->setVisible(true);
    }

    m_chart->addAxis(axis_voltage, Qt::AlignRight);

    QString s_name;
    QScatterSeries *m_hapticSerie;

    qDebug() << data_loader->y_dataMap.keys();

    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {

      QString s_name = data_loader->y_dataMap.key(data_vector);

      qDebug() << "prima bucla:" << s_name;

      if (!(s_name.contains("[N")))
        continue;

      qDebug() << "prima bucla dupa filtru:" << s_name;

      QScatterSeries *chart_serie;

      chart_serie = new QScatterSeries(m_chart);
      chart_serie->replace(*data_vector);

      connect(chart_serie, &QScatterSeries::clicked, this, &View::keepCallout);
      connect(chart_serie, &QScatterSeries::hovered, this, &View::tooltip);

      axis_travel->setTitleText(data_loader->x_dataMap.keys().at(0));

      m_hapticSerie = chart_serie;

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);
      chart_serie->setMarkerSize(6);
      chart_serie->setBorderColor("transparent");

      //            chart_serie->setName (s_name + " (" + QString::number
      //            (chart_serie->points ().size ()) + ")");
      chart_serie->setName(s_name);
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axis_travel);

      double maxforce{0.0};
      int maxforcelocation{0};

      if (!s_name.contains("[N")) {
        qDebug() << "prima bucla dupa if:" << s_name;

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        chart_serie->attachAxis(axis_voltage);

        double min = chart_serie->points().first().y();
        double max = chart_serie->points().first().y();
        for (QPointF point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage->min() << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage->max() << max;

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);

        if (s_name.contains("[V]"))
          axis_voltage->setLabelFormat("%4.1RV ");
      } else {
        qDebug() << "prima bucla dupa else:" << s_name;
        chart_serie->attachAxis(axis_force);
      }
    }

    auto timestamp1 = QDateTime::currentMSecsSinceEpoch();
    //        QList<QFuture<void>> flist;
    //        for (auto serie : m_chart->series ()){

    //            auto temp_serie = qobject_cast<QScatterSeries*>(serie);

    //            qDebug () << "here!" << temp_serie->name () <<
    //            temp_serie->hapticPoints;

    //            if (temp_serie->name ().contains ("[N]"))
    //                flist.append (QtConcurrent::run(temp_serie,
    //                &hapticSerie::findHapticPoints));

    //        }

    //        for (auto f : flist){
    //            while(!f.isFinished ()) QApplication::processEvents ();
    //        }

    auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "interest points in view found in ms"
             << timestamp2 - timestamp1;

    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {
      QString s_name = data_loader->y_dataMap.key(data_vector);

      qDebug() << "a doua bucla:" << s_name;

      if (s_name.contains("[N]"))
        continue;
      if (s_name.contains("[Nm]"))
        continue;

      qDebug() << "a doua bucla dupa filtru:" << s_name;

      QScatterSeries *chart_serie;

      chart_serie = new QScatterSeries(this->m_chart);

      chart_serie->replace(*data_vector);

      connect(chart_serie, &QScatterSeries::clicked, this, &View::keepCallout);
      connect(chart_serie, &QScatterSeries::hovered, this, &View::tooltip);

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);
      chart_serie->setMarkerSize(6);
      chart_serie->setBorderColor("transparent");

      if (s_name.contains("position"))
        s_name.replace("[]", "[LIN %]");

      //            chart_serie->setName (s_name + " (" + QString::number
      //            (chart_serie->points ().size ()) + ")");
      chart_serie->setName(s_name);
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axis_travel);

      if (!s_name.contains("[N]")) {

        qDebug() << "a doua bucla dupa if:" << s_name;

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        chart_serie->attachAxis(axis_voltage);

        double min = chart_serie->points().first().y();
        double max = chart_serie->points().first().y();
        for (QPointF point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage->min() << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage->max() << max;

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);

        if (s_name.contains("[V]"))
          axis_voltage->setLabelFormat("%4.1RV ");
      } else {
        chart_serie->attachAxis(axis_force);
      }
      /*
      //            }

      //            QString fileContents;
      //            QFile csvFile;

      //            fileContents.append ("Travel [mm];"
      //                                 "Force [N];"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";"
      //                                 ";" +

      //                                 QLocale().toString(maxforcelocation) +
      //                                 ";;"
      //                                 "\n");

      //            for (int i = 0; i < dura_signal->points ().size (); i++) {

      //                fileContents.append (QLocale().toString (dura_signal->at
      (i).x ()) + ";" +
      //                                     QLocale().toString (dura_signal->at
      (i).y ()) + "\r\n");

      //            }




      //            csvFile.setFileName (this->serieName + ".csv");
      //            qDebug() << "CSV File: " << this->serieName + ".csv";
      //            csvFile.open (QIODevice::WriteOnly);

      //            csvFile.write(fileContents.toUtf8 ());
      //            csvFile.close ();
      //            fileContents.clear ();
*/
    }

    if (data_loader->hapticPoints.keys().contains("max force")) {
      axis_travel->setMax(data_loader->hapticPoints.value("max force").x() *
                          1.15);
      axis_force->setMax(data_loader->hapticPoints.value("max force").y() *
                         1.15);
    }

    qDebug() << "data_loader->hapticPoints.keys ()"
             << data_loader->hapticPoints.keys();

    for (auto serie : m_chart->series()) {
      auto temp_serie = qobject_cast<QScatterSeries *>(serie);

      // to do: implement SYSTEC analog signal points
      if (data_loader->type == dataLoader::SYSTEC)
        if (serie->name() == "Signal [V]")
          continue;

      for (auto pointName : data_loader->hapticPoints.keys()) {
        tooltip_ex(data_loader->hapticPoints.value(pointName), true, Qt::red,
                   pointName);
        keepCallout();
      }

      intPoints.insert(temp_serie->name(), m_callouts);
      m_callouts.clear();

      qDebug() << " and here!" << temp_serie->name()
               << data_loader->hapticPoints;
    }

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
    m_chart->legend()->update();

    axis_force->setMin(0);
    axis_travel->setMin(0);

    initialScale.insert(axis_travel, QPointF(0, axis_travel->max()));
    initialScale.insert(axis_force, QPointF(0, axis_force->max()));

    // m_chart->setAcceptHoverEvents(true);

    setRenderHint(QPainter::Antialiasing);
    scene()->addItem(m_chart);

    m_coordX = new QGraphicsSimpleTextItem(m_chart);
    m_coordX->setPos(m_chart->size().width() / 2 - 50,
                     m_chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(m_chart);
    m_coordY->setPos(m_chart->size().width() / 2 + 50,
                     m_chart->size().height());
    m_coordY->setText("Y: ");

    const auto markers = m_chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
      // Disconnect possible existing connection to avoid multiple connections
      QObject::disconnect(marker, &QLegendMarker::clicked, this,
                          &View::handleMarkerClicked);

      QObject::connect(marker, &QLegendMarker::clicked, this,
                       &View::handleMarkerClicked);
    }

    for (auto axis : m_chart->axes())
      adjustAxisRange(qobject_cast<QValueAxis *>(axis));

    applyNiceNumbers();

    for (auto axis : m_chart->axes())
      QObject::connect(qobject_cast<QValueAxis *>(axis),
                       &QValueAxis::rangeChanged, this,
                       &View::proc_rangeChanged);

    //        this->serieName = fileNameArg;

    //        qDebug() << "heere!";

    //        QObject::connect(this->axis_travel, &QValueAxis::rangeChanged,
    //        this, &View::handleTimeRangeChanged);
    //        QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
    //        &View::handleTime2RangeChanged);

    qDebug() << "heere!";
    if (m_tooltip != nullptr)
      m_tooltip->hide();
    this->setMouseTracking(true);
    this->good = true;
    this->is_robot = true;

    {

      //        exportDurability();

      //        {

      //            QString fileContents;
      //            QFile csvFile;

      //            fileContents.append ("Travel [mm];"
      //                                 "Force [N];"
      //                                 "Haptic Points;\n"
      //                                 );

      //            auto dataVector = m_hapticSerie->pointsVector ();

      //            qDebug() << "serie->hapticPoints before points conversion:"
      //            << data_loader->hapticPoints;

      //            for (int i = 0; i < dataVector.size (); i++) {
      //                for (auto h_point_name : data_loader->hapticPoints.keys
      //                ()){
      //                    if(data_loader->hapticPoints.value (h_point_name) ==
      //                    dataVector[i]){
      //                        data_loader->hapticPointsRow.insert
      //                        (h_point_name, i); qDebug() << "Point row:" <<
      //                        h_point_name << dataVector[i] << i;
      //                    }
      //                }
      //            }

      //            for (int i = 0; i < dataVector.size (); i++) {

      //                fileContents.append (QLocale().toString (dataVector[i].x
      //                ()) + ";" +
      //                                     QLocale().toString (dataVector[i].y
      //                                     ()) + ";");

      //                if(!data_loader->hapticPointsRow.isEmpty ()){
      //                    auto p_name = data_loader->hapticPointsRow.lastKey
      //                    (); auto p_row = data_loader->hapticPointsRow.take
      //                    (p_name); fileContents.append (p_name + ";" +
      //                    QString::number (p_row + 2));
      //                }

      //                fileContents.append("\r\n");
      //            }

      //            csvFile.setFileName (this->serieName + ".csv");
      //            qDebug() << "CSV File: " << this->serieName + ".csv";
      //            csvFile.open (QIODevice::WriteOnly);

      //            QDir dir = QFileInfo(path).absoluteDir();
      //            QString absdir = dir.absolutePath();

      //            csvFile.write(fileContents.toUtf8 ());
      //            csvFile.close ();
      //            fileContents.clear ();

      //            if(!QFile::exists (absdir + "\\" + dir.dirName () +
      //            "_Graphs.xlsm"))
      //                QFile::copy(QCoreApplication::applicationDirPath() +
      //                "\\Graphs.XLSM",
      //                            absdir + "\\" + dir.dirName () +
      //                            "_Graphs.xlsm");

      //        }
    }
  }
}

void View::construct_BLF_export() {

  qDebug() << "in constructie!";

  this->folder = path;

  m_chart = new QChart();
  m_chart->setParent(this);
  this->setRenderHint(QPainter::Antialiasing);

  m_chart->setTheme(QChart::ChartThemeBlueNcs);
  QFileInfo fi(serieName);
  m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

  m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
  m_chart->setAnimationOptions(QChart::NoAnimation);
  this->setRenderHint(QPainter::Antialiasing);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);

  this->axisX = new QValueAxis(m_chart);
  m_chart->addAxis(axisX, Qt::AlignBottom);
  axisX->setObjectName("axisX");
  axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axisX->setMinorTickCount(1);
  axisX->setTickCount(11);
  axisX->setTitleText(
      "Running Time"); // Angle [arcdeg]//Travel [N]Running Time [days]
  axisX->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axisX->setLabelFormat("%4.1Td"); //("");%4.0f%4.1f
  axisX->setLabelsEditable(true);

  this->axisX2 = new QValueAxis(m_chart);
  axisX2->setObjectName("axisX2");
  axisX2->setTickCount(11);
  //    axisX2->setTitleText("chart time scale");
  //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
  axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
  axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
  axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
  axisX2->setMinorTickCount(3);
  axisX2->setLabelFormat("%4.1Td");
  axisX2->setTickAnchor(0);
  m_chart->addAxis(axisX2, Qt::AlignBottom);

  this->axis_voltage = new QValueAxis(m_chart);
  m_chart->addAxis(axis_voltage, Qt::AlignLeft);
  axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_voltage->setObjectName("axis_voltage");
  axis_voltage->setLabelFormat("%4.1RV ");
  axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_voltage->setLinePen(QPen(Qt::blue, 4, Qt::SolidLine));
  //        axis_voltage->setTitleText ("Voltage [V]");
  axis_voltage->setMinorTickCount(1);
  axis_voltage->setTickCount(11);
  axis_voltage->setLabelsEditable(true);

  auto axis_voltage_HV = new QValueAxis(m_chart);
  m_chart->addAxis(axis_voltage_HV, Qt::AlignRight);
  axis_voltage_HV->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_voltage_HV->setObjectName("axis_HV_voltage");
  axis_voltage_HV->setLabelFormat("%4.3RV ");
  axis_voltage_HV->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_voltage_HV->setLinePen(QPen(Qt::red, 2, Qt::SolidLine));
  //        axis_voltage->setTitleText ("Voltage [V]");
  axis_voltage_HV->setMinorTickCount(1);
  axis_voltage_HV->setTickCount(11);
  axis_voltage_HV->setLabelsEditable(true);

  this->axis_current = new QValueAxis(m_chart);
  axis_current->setObjectName("axis_current");
  //    axis_current->setTitleText("Current Consumption");
  //    axis_current->setTitleFont(QFont("Verdana", 14, QFont::Bold));
  //    axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_current->setLabelFormat("%4.1RA ");
  axis_current->setTickCount(11);
  axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_current->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_current->setMinorTickCount(1);
  axis_current->setLabelsEditable(true);
  //    m_chart->addAxis(axis_current, Qt::AlignLeft);

  auto shunt_axis_current = new QValueAxis(m_chart);
  shunt_axis_current->setObjectName("axis_HV_current");
  //    axis_current->setTitleText("Current Consumption");
  //    axis_current->setTitleFont(QFont("Verdana", 14, QFont::Bold));
  //    axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  shunt_axis_current->setLabelFormat("%4.1RA ");
  shunt_axis_current->setTickCount(11);
  shunt_axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  shunt_axis_current->setLinePen(QPen(Qt::red, 2, Qt::SolidLine));
  shunt_axis_current->setMinorTickCount(1);
  shunt_axis_current->setLabelsEditable(true);
  m_chart->addAxis(shunt_axis_current, Qt::AlignRight);

  this->axis_temperature = new QValueAxis(m_chart);

  axis_temperature->setObjectName("axis_temperature");

  //        axis_temperature->setTitleText("Temperature");
  //        axis_temperature->setTitleFont(QFont("Verdana", 14, QFont::Bold));
  axis_temperature->setTickCount(11);
  axis_temperature->setLabelFormat(
      QString("%4.0f" + QString::fromLatin1("°C ")).toUtf8());
  axis_temperature->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_temperature->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_temperature->setMinorTickCount(1);
  axis_temperature->setLabelsEditable(true);
  m_chart->addAxis(axis_temperature, Qt::AlignLeft);

  //    QMap<QString, int> tabs;
  //    for(int i = ui->tabWidget->count () -1; i >= 0; i--){
  //        if (i == statisticsTab) continue;
  //        auto tab = ui->tabWidget->widget (i)->children ();
  //        QString sample = ui->tabWidget->tabText (i);
  //        tabs.insert (sample, i);
  //    }

  //    //get a string list of tab names, for sorting
  //    QStringList v = tabs.keys();

  //    //sort the tab names naturally, using QCollator in numeric mode as
  //    compare function, so 10 will sit after 2 QCollator* collator = new
  //    QCollator; collator->setNumericMode (true); std::sort(v.begin(),
  //    v.end(), *collator);

  //    //add the statistics tab to the list
  //    v.append ("Statistics");

  //    //move the tabs according to their sorted position
  //    qint32 i = 0, j = 0, k = v.count();
  //    for(i=0; i<k; i++)
  //    {
  //        for (j=0; j<ui->tabWidget->tabBar ()->count(); j++)
  //        {
  //            if (ui->tabWidget->tabBar ()->tabText(j) == v[i])
  //            {
  //                ui->tabWidget->tabBar ()->moveTab(j, i);
  //            }
  //        }
  //    }

  QStringList y_series_list = data_loader->y_dataMap.keys();
  QCollator *collator = new QCollator;
  collator->setNumericMode(true);
  std::sort(y_series_list.begin(), y_series_list.end(), *collator);

  //    for(QVector<QPointF>* data_vector : data_loader->y_dataMap.values ())
  for (const QString &s_name : y_series_list) {

    if (s_name.contains("Sup1")) {

      QLineSeries *chart_serie;
      chart_serie = new QLineSeries(m_chart);
      chart_serie->replace(
          *data_loader->y_dataMap.value(s_name)); //*data_vector

      //            QScatterSeries* chart_serie;
      //            chart_serie = new QScatterSeries(m_chart);
      //            chart_serie->replace (*data_vector);

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);

      chart_serie->setName(s_name);
      chart_serie->setColor(QColor(193, 97, 5));
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      chart_serie->setPointsVisible(true);
      //            chart_serie->setMarkerSize (6);
      //            chart_serie->setBorderColor ("transparent");

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axisX);

      chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
      chart_serie->attachAxis(axis_voltage_HV);
      //            axis_voltage->setLinePen((QPen(chart_serie->color(), 2,
      //            Qt::SolidLine)));
    } else

        if (s_name.contains("Sup2")) {

      QLineSeries *chart_serie;
      chart_serie = new QLineSeries(m_chart);
      chart_serie->replace(
          *data_loader->y_dataMap.value(s_name)); //*data_vector

      //            QScatterSeries* chart_serie;
      //            chart_serie = new QScatterSeries(m_chart);
      //            chart_serie->replace (*data_vector);

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);

      chart_serie->setName(s_name);
      chart_serie->setColor(QColor(193, 97, 5));
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      chart_serie->setPointsVisible(true);
      //            chart_serie->setMarkerSize (6);
      //            chart_serie->setBorderColor ("transparent");

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axisX);

      chart_serie->setProperty("my_type", QVariant(QObject::tr("current")));
      chart_serie->attachAxis(shunt_axis_current);
      // axis_voltage->setLinePen((QPen(chart_serie->color(), 2,
      // Qt::SolidLine)));
    } else

      //        QString s_name = data_loader->y_dataMap.key(data_vector);
      if (s_name.contains("Supply Voltage") or s_name.contains("Volt DC")) {

        QLineSeries *chart_serie;
        chart_serie = new QLineSeries(m_chart);
        chart_serie->replace(
            *data_loader->y_dataMap.value(s_name)); //*data_vector

        //            QScatterSeries* chart_serie;
        //            chart_serie = new QScatterSeries(m_chart);
        //            chart_serie->replace (*data_vector);

        chart_serie->setUseOpenGL(true);
        chart_serie->setPointsVisible(true);

        chart_serie->setName(s_name);
        chart_serie->setColor(QColor(193, 97, 5));
        chart_serie->setProperty("my_order",
                                 QVariant(m_chart->series().size() + 1));

        chart_serie->setPointsVisible(true);
        //            chart_serie->setMarkerSize (6);
        //            chart_serie->setBorderColor ("transparent");

        m_chart->addSeries(chart_serie);
        chart_serie->attachAxis(axisX);

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        chart_serie->attachAxis(axis_voltage);
        axis_voltage->setLinePen(
            (QPen(chart_serie->color(), 2, Qt::SolidLine)));

        double min = chart_serie->points().constFirst().y();
        double max = chart_serie->points().constFirst().y();

        for (QPointF &point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage->min() << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage->max() << max;

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);
      } else

          if (s_name.contains("HV Voltage") or s_name.contains("Voltage DUT") or
              s_name.contains("Cell")) {

        QScatterSeries *chart_serie;
        chart_serie = new QScatterSeries(m_chart);

        //            for(int i = 0; i < data_vector->size (); i++){
        //                data_vector[i].data ()->setY (data_vector->at (i).y
        //                ()/ 1000);
        //            }

        chart_serie->replace(
            *data_loader->y_dataMap.value(s_name)); //*data_vector

        chart_serie->setUseOpenGL(true);
        chart_serie->setPointsVisible(true);

        chart_serie->setName(s_name);
        chart_serie->setProperty("my_order",
                                 QVariant(m_chart->series().size() + 1));

        chart_serie->setPointsVisible(true);
        chart_serie->setMarkerSize(6);
        chart_serie->setBorderColor("transparent");
        //            chart_serie->setColor(QColor(5, 232, 107));

        m_chart->addSeries(chart_serie);
        chart_serie->attachAxis(axisX);

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        axis_voltage_HV->setLinePen(
            (QPen(Qt::gray, 2, Qt::SolidLine))); // chart_serie->color ()
        chart_serie->attachAxis(axis_voltage_HV);

        double min = chart_serie->points().constFirst().y();
        double max = chart_serie->points().constFirst().y();

        for (QPointF &point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage_HV->min()
                 << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage_HV->max()
                 << max;

        if (axis_voltage_HV->min() > min)
          axis_voltage_HV->setMin(min);
        if (axis_voltage_HV->max() < max)
          axis_voltage_HV->setMax(max);
      } else

        //        QScatterSeries* as_volt = new QScatterSeries(this->m_chart);
        //        as_volt->replace (*as_volt_vect);

        //        QString s_name = AS_volts.key(as_volt_vect);//as_volt
        //        //            qDebug() << "here, line 576";

        //        as_volt->setPointsVisible (true);
        //        as_volt->setMarkerSize (6);
        //        as_volt->setBorderColor ("transparent");
        //        as_volt->setColor(QColor(5, 232, 107));

        if (s_name.contains("Current Consumption") or
            s_name.contains("Amp DC") or s_name.contains("Amp AC") or
            (s_name.contains("Supply") and s_name.contains("[A]"))) {
          m_chart->addAxis(axis_current, Qt::AlignRight);
          QScatterSeries *chart_serie;
          chart_serie = new QScatterSeries(m_chart);
          chart_serie->replace(
              *data_loader->y_dataMap.value(s_name)); //*data_vector

          chart_serie->setUseOpenGL(true);
          chart_serie->setPointsVisible(true);

          chart_serie->setName(s_name);
          chart_serie->setColor(QColor(50, 130, 50));
          chart_serie->setProperty("my_order",
                                   QVariant(m_chart->series().size() + 1));

          chart_serie->setPointsVisible(true);
          chart_serie->setMarkerSize(6);
          chart_serie->setBorderColor("transparent");
          //            chart_serie->setColor(QColor(5, 232, 107));

          m_chart->addSeries(chart_serie);
          chart_serie->attachAxis(axisX);

          chart_serie->setProperty("my_type", QVariant(QObject::tr("current")));
          axis_current->setLinePen(
              (QPen(chart_serie->color(), 2, Qt::SolidLine)));
          chart_serie->attachAxis(axis_current);

          double min = chart_serie->points().first().y();
          double max = chart_serie->points().first().y();

          for (QPointF point : chart_serie->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          qDebug() << " init axis_voltage->min ()" << axis_voltage->min()
                   << min;
          qDebug() << " init axis_voltage->max ()" << axis_voltage->max()
                   << max;

          if (axis_current->min() > min)
            axis_current->setMin(min);
          if (axis_current->max() < max)
            axis_current->setMax(max);
        } else

            if (s_name.contains("Shunt Current") or
                s_name.contains("Current")) {
          QScatterSeries *chart_serie;
          chart_serie = new QScatterSeries(m_chart);
          chart_serie->replace(
              *data_loader->y_dataMap.value(s_name)); //*data_vector

          chart_serie->setUseOpenGL(true);
          chart_serie->setPointsVisible(true);

          chart_serie->setName(s_name);
          chart_serie->setColor(QColor(255, 150, 0));
          chart_serie->setProperty("my_order",
                                   QVariant(m_chart->series().size() + 1));

          chart_serie->setPointsVisible(true);
          chart_serie->setMarkerSize(6);
          chart_serie->setBorderColor("transparent");
          chart_serie->setColor(Qt::red);

          m_chart->addSeries(chart_serie);
          chart_serie->attachAxis(axisX);

          chart_serie->setProperty("my_type", QVariant(QObject::tr("current")));
          shunt_axis_current->setLinePen(
              (QPen(chart_serie->color(), 2, Qt::SolidLine)));
          chart_serie->attachAxis(shunt_axis_current);

          double min = chart_serie->points().first().y();
          double max = chart_serie->points().first().y();

          for (QPointF point : chart_serie->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          qDebug() << " init axis_voltage->min ()" << axis_voltage->min()
                   << min;
          qDebug() << " init axis_voltage->max ()" << axis_voltage->max()
                   << max;

          if (shunt_axis_current->min() > min)
            shunt_axis_current->setMin(min);
          if (shunt_axis_current->max() < max)
            shunt_axis_current->setMax(max);
        } else

            if (s_name.contains("Temperature")) {
          QLineSeries *chart_serie;
          chart_serie = new QLineSeries(m_chart);
          chart_serie->replace(
              *data_loader->y_dataMap.value(s_name)); //*data_vector

          chart_serie->setUseOpenGL(true);
          chart_serie->setPointsVisible(true);

          chart_serie->setName(s_name);
          chart_serie->setColor(QColor(255, 150, 0));
          chart_serie->setProperty("my_order",
                                   QVariant(m_chart->series().size() + 1));

          chart_serie->setPointsVisible(true);
          //            chart_serie->setMarkerSize (6);
          //            chart_serie->setBorderColor ("transparent");
          chart_serie->setColor(Qt::blue);

          m_chart->addSeries(chart_serie);
          chart_serie->attachAxis(axisX);

          //            axis_temperature->setLabelsColor(temperature->color());
          //            axis_temperature->setLinePenColor
          //            (temperature->color());
          //            temperature->attachAxis(axis_travel);
          //            temperature->attachAxis(axis_temperature);
          //            axis_temperature->setVisible (true);

          chart_serie->setProperty("my_type",
                                   QVariant(QObject::tr("temperature")));
          shunt_axis_current->setLinePen(
              (QPen(chart_serie->color(), 2, Qt::SolidLine)));
          chart_serie->attachAxis(axis_temperature);

          double min = chart_serie->points().first().y();
          double max = chart_serie->points().first().y();

          for (QPointF point : chart_serie->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          if (axis_temperature->min() > min)
            axis_temperature->setMin(min);
          if (axis_temperature->max() < max)
            axis_temperature->setMax(max);
        }
  }

  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
  m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
  m_chart->legend()->update();

  initialScale.insert(axisX, QPointF(0, axisX->max()));

  // m_chart->setAcceptHoverEvents(true);

  setRenderHint(QPainter::Antialiasing);
  scene()->addItem(m_chart);

  m_coordX = new QGraphicsSimpleTextItem(m_chart);
  m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
  m_coordX->setText("X: ");
  m_coordY = new QGraphicsSimpleTextItem(m_chart);
  m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());
  m_coordY->setText("Y: ");

  const auto markers = m_chart->legend()->markers();
  for (QLegendMarker *marker : markers) {
    // Disconnect possible existing connection to avoid multiple connections
    QObject::disconnect(marker, &QLegendMarker::clicked, this,
                        &View::handleMarkerClicked);

    QObject::connect(marker, &QLegendMarker::clicked, this,
                     &View::handleMarkerClicked);
  }

  QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                   &View::handleTimeRangeChanged);
  QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
                   &View::handleTime2RangeChanged);

  for (auto axis : m_chart->axes())
    adjustAxisRange(qobject_cast<QValueAxis *>(axis));

  applyNiceNumbers();

  for (auto axis : m_chart->axes())
    QObject::connect(qobject_cast<QValueAxis *>(axis),
                     &QValueAxis::rangeChanged, this, &View::proc_rangeChanged);

  for (auto axis : m_chart->axes()) {
    axis->setVisible(false);
    for (auto serie : m_chart->series()) {
      if (serie->attachedAxes().contains(axis))
        axis->setVisible(true);
    }
  }

  if (axisX2 != nullptr)
    axisX2->setVisible(!axisX2->isVisible());
  if (axisX != nullptr)
    axisX->setMin(0.0);

  this->setMouseTracking(true);
  this->good = true;
}

void View::construct_LabView_Chart(QString path) {
  qDebug() << "LAB_VIEW IN CONSTRUCTION:" << path;

  if (data_loader->y_dataMap.size() > 0) {

    this->folder = path;

    m_chart = new QChart();
    m_chart->setParent(this);
    this->setRenderHint(QPainter::Antialiasing);

    m_chart->setTheme(QChart::ChartThemeBlueNcs);
    QFileInfo fi(serieName);
    m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

    m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
    m_chart->setAnimationOptions(QChart::NoAnimation);
    this->setRenderHint(QPainter::Antialiasing);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight);

    this->axisX = new QValueAxis(m_chart);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    axisX->setObjectName("axisX");
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axisX->setMinorTickCount(1);
    axisX->setTickCount(11);
    axisX->setTitleText(
        "Running Time"); // Angle [arcdeg]//Travel [N]Running Time [days]
    axisX->setTitleFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLabelFormat("%4.1Td"); //("");%4.0f%4.1f
    axisX->setLabelsEditable(true);

    this->axisX2 = new QValueAxis(m_chart);
    axisX2->setObjectName("axisX2");
    axisX2->setTickCount(11);
    //    axisX2->setTitleText("chart time scale");
    //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
    axisX2->setMinorTickCount(3);
    axisX2->setLabelFormat("%4.1Td");
    axisX2->setTickAnchor(0);
    m_chart->addAxis(axisX2, Qt::AlignBottom);

    this->axis_force = new QValueAxis(m_chart);
    m_chart->addAxis(axis_force, Qt::AlignLeft);
    axis_force->setObjectName("axis_force");
    axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axis_force->setMinorTickCount(1);
    axis_force->setTickCount(11);
    axis_force->setTitleText("Real Pressure [bar]");
    axis_force->setTitleFont(QFont("Verdana", 12, QFont::Bold));
    axis_force->setLabelFormat("%4.1f");
    axis_force->setLabelsEditable(true);

    this->axis_voltage = new QValueAxis(m_chart);
    m_chart->addAxis(axis_voltage, Qt::AlignRight);
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setObjectName("axis_voltage");
    axis_voltage->setLabelFormat("%4.1RV ");
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    //        axis_voltage->setTitleText ("Voltage [V]");
    axis_voltage->setMinorTickCount(1);
    axis_voltage->setTickCount(11);
    axis_voltage->setLabelsEditable(true);

    QString s_name;
    QLineSeries *m_hapticSerie;

    qDebug() << data_loader->y_dataMap.keys();

    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {

      QString s_name = data_loader->y_dataMap.key(data_vector);

      qDebug() << "prima bucla:" << s_name;

      if (!(s_name.contains("[bar")))
        continue; // filter

      qDebug() << "prima bucla dupa filtru:" << s_name;

      QLineSeries *chart_serie;

      chart_serie = new QLineSeries(m_chart);
      chart_serie->replace(*data_vector);

      //            connect(chart_serie, &QScatterSeries::clicked, this,
      //            &View::keepCallout); connect(chart_serie,
      //            &QScatterSeries::hovered, this, &View::tooltip);

      m_hapticSerie = chart_serie;

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);
      //            chart_serie->setMarkerSize (6);
      //            chart_serie->setBorderColor ("transparent");

      // chart_serie->setName (s_name + " (" + QString::number
      // (chart_serie->points ().size ()) + ")");

      chart_serie->setName(s_name);
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axisX);

      if (!s_name.contains("[bar")) {
        qDebug() << "prima bucla dupa if:" << s_name;

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        chart_serie->attachAxis(axis_voltage);

        double min = chart_serie->points().first().y();
        double max = chart_serie->points().first().y();

        for (QPointF point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage->min() << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage->max() << max;

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);

        if (s_name.contains("[bar]"))
          axis_voltage->setLabelFormat("%4.1Rbar ");
      } else {
        qDebug() << "prima bucla dupa else:" << s_name;
        chart_serie->attachAxis(axis_force);
      }
    }

    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {
      QString s_name = data_loader->y_dataMap.key(data_vector);

      qDebug() << "a doua bucla:" << s_name;

      if (s_name.contains("[bar]"))
        continue;
      if (s_name.contains("[mbar]"))
        continue;

      qDebug() << "a doua bucla dupa filtru:" << s_name;

      QLineSeries *chart_serie;

      chart_serie = new QLineSeries(this->m_chart);

      chart_serie->replace(*data_vector);

      //            connect(chart_serie, &QScatterSeries::clicked, this,
      //            &View::keepCallout); connect(chart_serie,
      //            &QScatterSeries::hovered, this, &View::tooltip);

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);
      //            chart_serie->setMarkerSize (6);
      //            chart_serie->setBorderColor ("transparent");

      if (s_name.contains("position"))
        s_name.replace("[]", "[LIN %]");

      //            chart_serie->setName (s_name + " (" + QString::number
      //            (chart_serie->points ().size ()) + ")");
      chart_serie->setName(s_name);
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axisX);

      if (!s_name.contains("[bar]")) {

        qDebug() << "a doua bucla dupa if:" << s_name;

        chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
        chart_serie->attachAxis(axis_voltage);

        double min = chart_serie->points().first().y();
        double max = chart_serie->points().first().y();
        for (QPointF point : chart_serie->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        qDebug() << " init axis_voltage->min ()" << axis_voltage->min() << min;
        qDebug() << " init axis_voltage->max ()" << axis_voltage->max() << max;

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);

        axis_voltage->setLabelFormat("%4.1RV ");
      } else {
        chart_serie->attachAxis(axis_force);
      }
    }

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight);
    m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
    m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    m_chart->legend()->update();

    axis_force->setMin(0);
    axisX->setMin(0);

    initialScale.insert(axisX, QPointF(0, axisX->max()));
    initialScale.insert(axis_force, QPointF(0, axis_force->max()));

    // m_chart->setAcceptHoverEvents(true);

    setRenderHint(QPainter::Antialiasing);
    scene()->addItem(m_chart);

    m_coordX = new QGraphicsSimpleTextItem(m_chart);
    m_coordX->setPos(m_chart->size().width() / 2 - 50,
                     m_chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(m_chart);
    m_coordY->setPos(m_chart->size().width() / 2 + 50,
                     m_chart->size().height());
    m_coordY->setText("Y: ");

    const auto markers = m_chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
      // Disconnect possible existing connection to avoid multiple connections
      QObject::disconnect(marker, &QLegendMarker::clicked, this,
                          &View::handleMarkerClicked);

      QObject::connect(marker, &QLegendMarker::clicked, this,
                       &View::handleMarkerClicked);
    }

    for (auto axis : m_chart->axes())
      adjustAxisRange(qobject_cast<QValueAxis *>(axis));

    applyNiceNumbers();

    for (auto axis : m_chart->axes())
      QObject::connect(qobject_cast<QValueAxis *>(axis),
                       &QValueAxis::rangeChanged, this,
                       &View::proc_rangeChanged);

    QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                     &View::handleTimeRangeChanged);
    QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
                     &View::handleTime2RangeChanged);

    if (m_tooltip != nullptr)
      m_tooltip->hide();
    this->setMouseTracking(true);
    this->good = true;
    this->is_robot = true;

    //        bool labView_eval = true;
    //        this->setProperty ("labView_eval", QVariant(labView_eval));
    //        emit fileDropped (path);
    //        labView_eval = false;
    //        this->setProperty ("labView_eval", QVariant(labView_eval));

    labViewEval = true;
  }
}

void View::construct_LabView_Chart_eval(QString path) {
  qDebug() << "LAB_VIEW EVAL IN CONSTRUCTION:" << path;

  labViewEval = false;

  this->data_loader = tempDataLoader;

  if (data_loader->y_dataMap.size() > 0) {

    this->folder = path;

    m_chart = new QChart();
    m_chart->setParent(this);
    this->setRenderHint(QPainter::Antialiasing);

    m_chart->setTheme(QChart::ChartThemeBlueNcs);
    QFileInfo fi(serieName);
    m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

    m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
    m_chart->setAnimationOptions(QChart::NoAnimation);
    this->setRenderHint(QPainter::Antialiasing);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight);

    this->axisX = new QValueAxis(m_chart);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    axisX->setObjectName("sgdh"); // axisX
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axisX->setMinorTickCount(1);
    axisX->setTickCount(11);
    axisX->setTitleText(
        "Real Pressure [bar]"); // Angle [arcdeg]//Travel [N]Running Time [days]
    axisX->setTitleFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLabelFormat("%4.1f"); //("");%4.0f%4.1fTd
    axisX->setLabelsEditable(true);

    this->axis_voltage = new QValueAxis(m_chart);
    m_chart->addAxis(axis_voltage, Qt::AlignLeft);
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setObjectName("axis_voltage");
    axis_voltage->setLabelFormat("%4.1RV ");
    axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axis_voltage->setTitleText("Voltage [V]");
    axis_voltage->setMinorTickCount(1);
    axis_voltage->setTickCount(11);
    axis_voltage->setLabelsEditable(true);
    axis_voltage->setVisible(true);

    QString s_name;

    qDebug() << data_loader->y_dataMap.keys();

    QVector<QPointF> *pressure_vector = nullptr;
    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {
      s_name = data_loader->y_dataMap.key(data_vector);
      if ((s_name.contains("[bar"))) {
        pressure_vector = data_vector;
        break;
      }
    }

    for (QVector<QPointF> *data_vector : data_loader->y_dataMap.values()) {
      s_name = data_loader->y_dataMap.key(data_vector);
      if ((s_name.contains("[bar")))
        continue;
      if ((s_name.contains("_Tolerance")))
        continue;

      qDebug() << "converting serie:" << s_name;
      for (int i = 0; i < data_vector->size(); i++) {
        //                    qDebug() << "replacing:" << data_vector->at (i) <<
        //                    "to" << pressure_vector->at (i);
        data_vector->replace(
            i, QPointF(pressure_vector->at(i).y(), data_vector->at(i).y()));
      }

      QScatterSeries *chart_serie;
      chart_serie = new QScatterSeries(m_chart);
      chart_serie->replace(*data_vector);

      chart_serie->setUseOpenGL(true);
      chart_serie->setPointsVisible(true);
      chart_serie->setMarkerSize(6);
      chart_serie->setBorderColor("transparent");

      chart_serie->setName(s_name);
      chart_serie->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      m_chart->addSeries(chart_serie);
      chart_serie->attachAxis(axisX);
      chart_serie->attachAxis(axis_voltage);
      chart_serie->setProperty("my_type", QVariant(QObject::tr("voltage")));
    }

    QLineSeries *min_tolerance;
    min_tolerance = new QLineSeries(m_chart);
    min_tolerance->append(QPointF(0.0, 1.02));
    min_tolerance->append(QPointF(5.0, 3.62));
    min_tolerance->setUseOpenGL(true);
    min_tolerance->setName("Min Tolerance");
    min_tolerance->setProperty("my_order", QVariant(10000));
    m_chart->addSeries(min_tolerance);
    min_tolerance->attachAxis(axisX);
    min_tolerance->attachAxis(axis_voltage);

    QLineSeries *max_tolerance;
    max_tolerance = new QLineSeries(m_chart);
    max_tolerance->append(QPointF(0.0, 1.23));
    max_tolerance->append(QPointF(5.0, 4.13));
    max_tolerance->setUseOpenGL(true);
    max_tolerance->setName("Max Tolerance");
    max_tolerance->setProperty("my_order", QVariant(10001));
    m_chart->addSeries(max_tolerance);
    max_tolerance->attachAxis(axisX);
    max_tolerance->attachAxis(axis_voltage);

    {

      //        for(QVector<QPointF>* data_vector :
      //        data_loader->y_dataMap.values ())
      //        {

      //            s_name = data_loader->y_dataMap.key(data_vector);
      //            qDebug() << "prima bucla:" << s_name;

      //            if(! (s_name.contains ("[bar"))) continue; //filter

      //            qDebug() << "prima bucla dupa filtru:" << s_name;

      //            QLineSeries* chart_serie;

      //            chart_serie = new QLineSeries(m_chart);
      //            chart_serie->replace (*data_vector);

      //            //            connect(chart_serie, &QScatterSeries::clicked,
      //            this, &View::keepCallout);
      //            //            connect(chart_serie, &QScatterSeries::hovered,
      //            this, &View::tooltip);

      //            m_hapticSerie = chart_serie;

      //            chart_serie->setUseOpenGL (true);
      //            chart_serie->setPointsVisible (true);
      //            //            chart_serie->setMarkerSize (6);
      //            //            chart_serie->setBorderColor ("transparent");

      //            //chart_serie->setName (s_name + " (" + QString::number
      //            (chart_serie->points ().size ()) + ")");

      //            chart_serie->setName (s_name);
      //            chart_serie->setProperty ("my_order",
      //            QVariant(m_chart->series ().size () + 1));

      //            m_chart->addSeries(chart_serie);
      //            chart_serie->attachAxis(axisX);

      //            if (!s_name.contains("[bar")){
      //                qDebug() << "prima bucla dupa if:" << s_name;

      //                chart_serie->setProperty ("my_type",
      //                QVariant(QObject::tr("voltage")));
      //                chart_serie->attachAxis(axis_voltage);

      //                double min = chart_serie->points ().first ().y ();
      //                double max = chart_serie->points ().first ().y ();

      //                for (QPointF point : chart_serie->points ()){
      //                    min = point.y() < min ? point.y() : min;
      //                    max = point.y() > max ? point.y() : max;
      //                }

      //                qDebug() << " init axis_voltage->min ()" <<
      //                axis_voltage->min () << min; qDebug() << " init
      //                axis_voltage->max ()" << axis_voltage->max () << max;

      //                if( axis_voltage->min () > min) axis_voltage->setMin
      //                (min); if( axis_voltage->max () < max)
      //                axis_voltage->setMax (max);

      //                if (s_name.contains("[bar]"))
      //                axis_voltage->setLabelFormat("%4.1Rbar ");

      //            }else{
      //                qDebug() << "prima bucla dupa else:" << s_name;
      //                chart_serie->attachAxis(axis_force);

      //            }

      //        }

      //        for(QVector<QPointF>* data_vector :
      //        data_loader->y_dataMap.values ())
      //        {
      //            QString s_name = data_loader->y_dataMap.key(data_vector);

      //            qDebug() << "a doua bucla:" << s_name;

      //            if(s_name.contains ("[bar]")) continue;
      //            if(s_name.contains ("[mbar]")) continue;

      //            qDebug() << "a doua bucla dupa filtru:" << s_name;

      //            QLineSeries* chart_serie;

      //            chart_serie = new QLineSeries(this->m_chart);

      //            chart_serie->replace (*data_vector);

      //            //            connect(chart_serie, &QScatterSeries::clicked,
      //            this, &View::keepCallout);
      //            //            connect(chart_serie, &QScatterSeries::hovered,
      //            this, &View::tooltip);

      //            chart_serie->setUseOpenGL (true);
      //            chart_serie->setPointsVisible (true);
      //            //            chart_serie->setMarkerSize (6);
      //            //            chart_serie->setBorderColor ("transparent");

      //            if(s_name.contains ("position"))
      //                s_name.replace ("[]", "[LIN %]");

      //            //            chart_serie->setName (s_name + " (" +
      //            QString::number (chart_serie->points ().size ()) + ")");
      //            chart_serie->setName (s_name);
      //            chart_serie->setProperty ("my_order",
      //            QVariant(m_chart->series ().size () + 1));

      //            m_chart->addSeries(chart_serie);
      //            chart_serie->attachAxis(axisX);

      //            if (!s_name.contains("[bar]")){

      //                qDebug() << "a doua bucla dupa if:" << s_name;

      //                chart_serie->setProperty ("my_type",
      //                QVariant(QObject::tr("voltage")));
      //                chart_serie->attachAxis(axis_voltage);

      //                double min = chart_serie->points ().first ().y ();
      //                double max = chart_serie->points ().first ().y ();
      //                for (QPointF point : chart_serie->points ())
      //                {
      //                    min = point.y() < min ? point.y() : min;
      //                    max = point.y() > max ? point.y() : max;
      //                }

      //                qDebug() << " init axis_voltage->min ()" <<
      //                axis_voltage->min () << min; qDebug() << " init
      //                axis_voltage->max ()" << axis_voltage->max () << max;

      //                if( axis_voltage->min () > min) axis_voltage->setMin
      //                (min); if( axis_voltage->max () < max)
      //                axis_voltage->setMax (max);

      //                axis_voltage->setLabelFormat("%4.1RV ");

      //            }else{
      //                chart_serie->attachAxis(axis_force);
      //            }
      //        }
    }

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight);
    m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
    m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    m_chart->legend()->update();

    axisX->setMin(0);
    axis_voltage->setMin(0);

    initialScale.insert(axis_voltage, QPointF(0, axis_voltage->max()));
    initialScale.insert(axisX, QPointF(0, axisX->max()));

    // m_chart->setAcceptHoverEvents(true);

    setRenderHint(QPainter::Antialiasing);
    scene()->addItem(m_chart);

    m_coordX = new QGraphicsSimpleTextItem(m_chart);
    m_coordX->setPos(m_chart->size().width() / 2 - 50,
                     m_chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(m_chart);
    m_coordY->setPos(m_chart->size().width() / 2 + 50,
                     m_chart->size().height());
    m_coordY->setText("Y: ");

    const auto markers = m_chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
      // Disconnect possible existing connection to avoid multiple connections
      QObject::disconnect(marker, &QLegendMarker::clicked, this,
                          &View::handleMarkerClicked);

      QObject::connect(marker, &QLegendMarker::clicked, this,
                       &View::handleMarkerClicked);
    }

    for (auto axis : m_chart->axes())
      adjustAxisRange(qobject_cast<QValueAxis *>(axis));

    applyNiceNumbers();

    for (auto axis : m_chart->axes())
      QObject::connect(qobject_cast<QValueAxis *>(axis),
                       &QValueAxis::rangeChanged, this,
                       &View::proc_rangeChanged);

    //                QObject::connect(this->axisX, &QValueAxis::rangeChanged,
    //                this, &View::handleTimeRangeChanged);
    //                QObject::connect(this->axisX2, &QValueAxis::rangeChanged,
    //                this, &View::handleTime2RangeChanged);

    if (m_tooltip != nullptr)
      m_tooltip->hide();
    this->setMouseTracking(true);
    this->good = true;
    this->is_robot = true;
  }
}

void View::construct_haptic_chart(QString path) {

  // MeasSerie loadData(QString fileName)

  QFuture<MeasSerie> future =
      QtConcurrent::run([&](QString path) { return loadData(path); }, path);

  while (!future.isFinished())
    QApplication::processEvents();

  MeasSerie serie = future.result();

  qDebug() << "serie size :" << serie.specimenList.size();

  if (!serie.status) {
    QMessageBox::information(this, tr("Error:"),
                             "The file at\n" + path + "\ncould not be loaded!");
    return;
  }

  //        MeasSerie serie = loadData();
  //        if (!serie.status) return;

  serieName = serie.name;
  this->folder = serie.folder;
  this->path = path;

  m_chart = new QChart;
  m_chart->setParent(this);

  m_chart->setTheme(QChart::ChartThemeBlueNcs);

  QFileInfo fi(fileNameArg);
  m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4) +
                    " haptic measurements");
  m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));

  m_chart->setAnimationOptions(QChart::NoAnimation);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);

  this->axis_travel = new QValueAxis(m_chart);
  m_chart->addAxis(axis_travel, Qt::AlignBottom);
  axis_travel->setObjectName("axis_travel");
  axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_travel->setMinorTickCount(1);
  axis_travel->setTickCount(11);
  axis_travel->setTitleText("Travel [mm]");
  axis_travel->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLabelFormat("%4.3f");
  axis_travel->setLabelsEditable(true);
  connect(axis_travel, &QValueAxis::rangeChanged, this, &View::updateCallouts);

  this->axis_force = new QValueAxis(m_chart);
  m_chart->addAxis(axis_force, Qt::AlignLeft);
  axis_force->setObjectName("axis_force");
  axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_force->setMinorTickCount(1);
  axis_force->setTickCount(11);
  axis_force->setTitleText("Force [N]");
  axis_force->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLabelFormat("%4.3f");
  axis_force->setLabelsEditable(true);
  connect(axis_force, &QValueAxis::rangeChanged, this, &View::updateCallouts);

  // QScatterSeries *series;
  hapticSerie *series;

  auto m_colors_haptic_curve =
      m_colors.keys().filter(QStringLiteral("haptic_curve"));
  int index = 0;

  double maxChartForce = 0, maxChartTravel = 0;

  //    m_tooltip= new Callout(m_chart);

  for (const MeasSpecimen &specimen : serie.specimenList) {
    // series = new QScatterSeries(m_chart);
    series = new hapticSerie(m_chart);

    float approachDistance = specimen.approachDistance;

    //        approachDistance = 0.0;

    qDebug() << specimen.name << "approachDistance" << approachDistance;
    for (uint l = 0; l < specimen.samples; l++) // l = 12
    {
      // travel serie for normal tests
      series->append((qreal)specimen.travel.at(l) - approachDistance,
                     (qreal)specimen.force.at(l));
      // time serie for misuse tests
      // series->append ((qreal)serie.time.at (l) - serie.time.at
      // (serie.approachRow), (qreal) serie.force.at (l));
    }

    series->setPointsVisible(true);
    series->setMarkerSize(4);
    series->setBorderColor("transparent");
    series->setUseOpenGL(true);

    series->setName(specimen.name);

    //****** add interest points callout in serie
    //        qDebug() << specimen.name << "Interesting points Nr:" <<
    //        specimen.InterestPointsList.size(); for (int p = 0; p <
    //        specimen.InterestPointsList.size(); p++)
    //        {
    //            if(specimen.IP_ForceList.at (p) == 0) continue;
    //            tooltip(    QPointF((qreal) specimen.IP_TravelList.at (p),
    //                                (qreal) specimen.IP_ForceList.at
    //                                (p)),true);
    //            keepCallout();
    //        }

    //        // add max force point callout
    //        tooltip(    QPointF((qreal) specimen.travel.at
    //        (specimen.maxForce_row) - specimen.approachDistance,
    //                            (qreal) specimen.force.at
    //                            (specimen.maxForce_row)),true);

    if ((specimen.travel.at(specimen.maxForce_row) -
         specimen.approachDistance) > maxChartTravel) {
      maxChartTravel = (specimen.travel.at(specimen.maxForce_row) -
                        specimen.approachDistance);
    }

    if ((specimen.force.at(specimen.maxForce_row)) > maxChartForce) {
      maxChartForce = (specimen.force.at(specimen.maxForce_row));
    }

    //        //        tooltip(    QPointF((qreal) serie.time.at
    //        (serie.maxForce_row) - serie.time.at (serie.approachRow) ,
    //        //                            (qreal) serie.force.at
    //        (serie.maxForce_row)),true);

    //        keepCallout();

    //        //   add BMW_F3
    //        if (specimen.BMW_F3_row != 0)
    //        {
    //            tooltip(    QPointF((qreal) specimen.travel.at
    //            (specimen.BMW_F3_row) - specimen.approachDistance,
    //                                (qreal) specimen.force.at
    //                                (specimen.BMW_F3_row)),true);
    //            keepCallout();
    //        }

    //        if (specimen.SW1_row.size () > 0)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW1_row.at (0)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW1_row.at (0))),true,
    //                            specimen.SW1_state.at (0) == 1, "Contact
    //                            1");
    //            keepCallout();
    //        }

    //        if (specimen.SW1_row.size () > 1)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW1_row.at (1)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW1_row.at (1))),true,
    //                            specimen.SW1_state.at (1) == 1, "Contact
    //                            1");
    //            keepCallout();
    //        }

    //        if (specimen.SW2_row.size () > 0)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW2_row.at (0)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW2_row.at (0))),true,
    //                            specimen.SW2_state.at (0) == 1, "Contact
    //                            2");
    //            keepCallout();
    //        }

    //        if (specimen.SW2_row.size () > 1)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW2_row.at (1)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW2_row.at (1))),true,
    //                            specimen.SW2_state.at (1) == 1, "Contact
    //                            2");
    //            keepCallout();
    //        }

    m_chart->addSeries(series);

    //        series->findHapticPoints ();

    //        for (auto pointName : series->hapticPoints.keys ()){

    //            tooltip_ex(series->hapticPoints.value (pointName), true,
    //            Qt::red, pointName); keepCallout();
    //        }

    //        intPoints[series->name()].append(series->callouts);
    //        intPoints.insert (series->name(), series->callouts);

    intPoints.insert(series->name(), m_callouts);
    m_callouts.clear();

    if (m_colors_haptic_curve.size() > index) {
      if (m_colors.value(m_colors_haptic_curve.at(index)).isValid())
        series->setColor(m_colors.value(m_colors_haptic_curve.at(index)));
    }
    index++;

    series->attachAxis(axis_travel);
    series->attachAxis(axis_force);

    connect(series, &QScatterSeries::clicked, this, &View::keepCallout);
    connect(series, &QScatterSeries::hovered, this, &View::tooltip);

    //        specimen.force.clear ();
    //        specimen.force.squeeze ();
    //        specimen.travel.clear ();
    //        specimen.travel.squeeze ();
    //        specimen.time.clear ();
    //        specimen.time.squeeze ();
    //        specimen.contact1.clear ();
    //        specimen.contact1.squeeze ();
    //        specimen.cycle.clear ();
    //        specimen.cycle.squeeze ();
  }

  //    auto timestamp1 = QDateTime::currentMSecsSinceEpoch();
  //    QList<QFuture<void>> flist;
  //    for (auto serie : m_chart->series())
  //    {
  //        auto temp_serie = qobject_cast<hapticSerie *>(serie);
  //        flist.append(QtConcurrent::run(temp_serie,
  //        &hapticSerie::findHapticPoints));
  //    }

  //    for (auto f : flist)
  //    {
  //        while (!f.isFinished())
  //            QApplication::processEvents();
  //    }

  //    auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << "interest points in view found in ms" << timestamp2 -
  //    timestamp1;

  auto timestamp1 = QDateTime::currentMSecsSinceEpoch();
  QList<QFuture<void>> flist;

  for (auto serie : m_chart->series()) {
    auto temp_serie = qobject_cast<hapticSerie *>(serie);
    if (temp_serie) {
      flist.append(
          QtConcurrent::run([=]() { temp_serie->findHapticPoints(); }));
    }
  }

  for (auto &f : flist) {
    f.waitForFinished();
  }

  m_tooltip = nullptr;

  auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
  qDebug() << "interest points in view found in ms" << timestamp2 - timestamp1;
  qDebug() << "tooltip" << (m_tooltip == nullptr) << m_tooltip;

  for (auto serie : m_chart->series()) {
    auto temp_serie = qobject_cast<hapticSerie *>(serie);
    for (auto pointName : temp_serie->hapticPoints.keys()) {
      tooltip_ex(temp_serie->hapticPoints.value(pointName), true, Qt::red,
                 pointName);
      keepCallout();
    }

    intPoints.insert(temp_serie->name(), m_callouts);
    m_callouts.clear();
  }

  for (MeasSpecimen specimen : serie.specimenList) {

    qDebug() << specimen.maxForce_row;

    if (specimen.maxForce_row > 0) {
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          h_serie->hapticPointsRow.insert(QStringLiteral("max force"),
                                          specimen.maxForce_row + 1);
        }
      }
    }

    qDebug() << "iterating serie.specimenList: " << specimen.name
             << " specimen.SW1_row.size(): " << specimen.SW1_row.size();
    if (specimen.SW1_row.size() > 0) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW1_row.at(0)), true,
                     specimen.SW1_state.at(0) == 1,
                     QStringLiteral("Contact 1"));
          keepCallout();

          if (specimen.SW1_state.at(0) == 1) {
            h_serie->hapticPointsRow.insert(QStringLiteral("D1 pressed"),
                                            specimen.SW1_row.at(0) + 1);
          } else {
            h_serie->hapticPointsRow.insert(QStringLiteral("D1 released"),
                                            specimen.SW1_row.at(0) + 1);
          }
        }
      }
    }

    if (specimen.SW1_row.size() > 1) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW1_row.at(1)), true,
                     specimen.SW1_state.at(1) == 1,
                     QStringLiteral("Contact 1"));
          keepCallout();

          if (specimen.SW1_state.at(1) == 1) {
            h_serie->hapticPointsRow.insert(QStringLiteral("D1 pressed"),
                                            specimen.SW1_row.at(1) + 1);
          } else {
            h_serie->hapticPointsRow.insert(QStringLiteral("D1 released"),
                                            specimen.SW1_row.at(1) + 1);
          }
        }
      }
    }

    if (specimen.SW2_row.size() > 0) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {

        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW2_row.at(0)), true,
                     specimen.SW2_state.at(0) == 1,
                     QStringLiteral("Contact 1"));
          keepCallout();

          if (specimen.SW2_state.at(0) == 1) {

            h_serie->hapticPointsRow.insert(QStringLiteral("D2 pressed"),
                                            specimen.SW2_row.at(0) + 1);
          } else {
            h_serie->hapticPointsRow.insert(QStringLiteral("D2 released"),
                                            specimen.SW2_row.at(0) + 1);
          }
        }
      }
    }

    if (specimen.SW2_row.size() > 1) {
      qreal this_offset = 0;

      for (auto s : m_chart->series()) {

        if (s->name() == specimen.name) {

          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW2_row.at(1)), true,
                     specimen.SW2_state.at(1) == 1,
                     QStringLiteral("Contact 2"));
          keepCallout();

          if (specimen.SW2_state.at(1) == 1) {
            h_serie->hapticPointsRow.insert(QStringLiteral("D2 pressed"),
                                            specimen.SW2_row.at(1) + 1);
          } else {
            h_serie->hapticPointsRow.insert(QStringLiteral("D2 released"),
                                            specimen.SW2_row.at(1) + 1);
          }
        }
      }
    }

    intPoints[specimen.name].append(m_callouts);
    m_callouts.clear();
  }

  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
  m_chart->legend()->update();

  axis_force->setMin(0);
  axis_travel->setMin(0);

  axis_travel->setMax(maxChartTravel * 1.15);
  axis_force->setMax(maxChartForce * 1.15);

  initialScale.insert(axis_travel, QPointF(0, axis_travel->max()));
  initialScale.insert(axis_force, QPointF(0, axis_force->max()));

  m_chart->setAcceptHoverEvents(true);
  setRenderHint(QPainter::Antialiasing);
  scene()->addItem(m_chart);

  m_coordX = new QGraphicsSimpleTextItem(m_chart);
  m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
  m_coordX->setText("X: ");
  m_coordY = new QGraphicsSimpleTextItem(m_chart);
  m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());
  m_coordY->setText("Y: ");

  const auto markers = m_chart->legend()->markers();
  for (QLegendMarker *marker : markers) {
    // Disconnect possible existing connection to avoid multiple connections
    QObject::disconnect(marker, &QLegendMarker::clicked, this,
                        &View::handleMarkerClicked);
    QObject::connect(marker, &QLegendMarker::clicked, this,
                     &View::handleMarkerClicked);
  }

  for (auto axis : m_chart->axes())
    adjustAxisRange(qobject_cast<QValueAxis *>(axis));

  //    applyNiceNumbers();

  for (auto axis : m_chart->axes())
    QObject::connect(qobject_cast<QValueAxis *>(axis),
                     &QValueAxis::rangeChanged, this, &View::proc_rangeChanged);

  if (m_tooltip)
    m_tooltip->hide();
  this->setMouseTracking(true);

  this->good = true;
  this->is_durability = true;

  qDeleteAll(errors);

  /* ZS2 */
}

void View::construct_haptic_chart_l(QString path) {

  // MeasSerie loadData(QString fileName)

  QFuture<MeasSerie> future =
      QtConcurrent::run([&](QString path) { return loadData(path); }, path);

  while (!future.isFinished())
    QApplication::processEvents();

  MeasSerie serie = future.result();

  qDebug() << "serie size chartlll:" << serie.specimenList.size();

  if (!serie.status) {
    QMessageBox::information(this, tr("Error:"),
                             "The file at\n" + path + "\ncould not be loaded!");
    return;
  }

  //        MeasSerie serie = loadData();
  //        if (!serie.status) return;

  serieName = serie.name;
  this->folder = serie.folder;
  this->path = path;

  m_chart = new QChart;
  m_chart->setParent(this);

  m_chart->setTheme(QChart::ChartThemeBlueNcs);

  QFileInfo fi(fileNameArg);
  m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4) +
                    " haptic measurements");
  m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));

  m_chart->setAnimationOptions(QChart::NoAnimation);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);

  this->axis_travel = new QValueAxis(m_chart);
  m_chart->addAxis(axis_travel, Qt::AlignBottom);
  axis_travel->setObjectName("axis_travel");
  axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_travel->setMinorTickCount(1);
  axis_travel->setTickCount(11);
  axis_travel->setTitleText("Travel [mm]");
  axis_travel->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis_travel->setLabelFormat("%4.3f");
  axis_travel->setLabelsEditable(true);
  connect(axis_travel, &QValueAxis::rangeChanged, this, &View::updateCallouts);

  this->axis_force = new QValueAxis(m_chart);
  m_chart->addAxis(axis_force, Qt::AlignLeft);
  axis_force->setObjectName("axis_force");
  axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_force->setMinorTickCount(1);
  axis_force->setTickCount(11);
  axis_force->setTitleText("Force [N]");
  axis_force->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis_force->setLabelFormat("%4.3f");
  axis_force->setLabelsEditable(true);
  connect(axis_force, &QValueAxis::rangeChanged, this, &View::updateCallouts);

  // QScatterSeries *series;
  hapticSerie *series;

  auto m_colors_haptic_curve = m_colors.keys().filter("haptic_curve");
  int index = 0;

  double maxChartForce = 0, maxChartTravel = 0;

  //    m_tooltip= new Callout(m_chart);

  for (MeasSpecimen specimen : serie.specimenList) {
    // series = new QScatterSeries(m_chart);
    series = new hapticSerie(m_chart);

    // dataLoader loader =

    float approachDistance = specimen.approachDistance;

    for (uint l = 12; l < specimen.samples; l++) {
      // travel serie for normal tests
      series->append((qreal)specimen.travel.at(l) - approachDistance,
                     (qreal)specimen.force.at(l));
      // time serie for misuse tests
      // series->append ((qreal)serie.time.at (l) - serie.time.at
      // (serie.approachRow), (qreal) serie.force.at (l));
    }

    series->setPointsVisible(true);
    series->setMarkerSize(4);
    series->setBorderColor("transparent");
    series->setUseOpenGL(true);

    series->setName(specimen.name);

    //****** add interest points callout in serie
    //        qDebug() << specimen.name << "Interesting points Nr:" <<
    //        specimen.InterestPointsList.size(); for (int p = 0; p <
    //        specimen.InterestPointsList.size(); p++)
    //        {
    //            if(specimen.IP_ForceList.at (p) == 0) continue;
    //            tooltip(    QPointF((qreal) specimen.IP_TravelList.at (p),
    //                                (qreal) specimen.IP_ForceList.at
    //                                (p)),true);
    //            keepCallout();
    //        }

    //        // add max force point callout
    //        tooltip(    QPointF((qreal) specimen.travel.at
    //        (specimen.maxForce_row) - specimen.approachDistance,
    //                            (qreal) specimen.force.at
    //                            (specimen.maxForce_row)),true);

    if ((specimen.travel.at(specimen.maxForce_row) -
         specimen.approachDistance) > maxChartTravel) {
      maxChartTravel = (specimen.travel.at(specimen.maxForce_row) -
                        specimen.approachDistance);
    }

    if ((specimen.force.at(specimen.maxForce_row)) > maxChartForce) {
      maxChartForce = (specimen.force.at(specimen.maxForce_row));
    }

    //        //        tooltip(    QPointF((qreal) serie.time.at
    //        (serie.maxForce_row) - serie.time.at (serie.approachRow) ,
    //        //                            (qreal) serie.force.at
    //        (serie.maxForce_row)),true);

    //        keepCallout();

    //        //   add BMW_F3
    //        if (specimen.BMW_F3_row != 0)
    //        {
    //            tooltip(    QPointF((qreal) specimen.travel.at
    //            (specimen.BMW_F3_row) - specimen.approachDistance,
    //                                (qreal) specimen.force.at
    //                                (specimen.BMW_F3_row)),true);
    //            keepCallout();
    //        }

    //        if (specimen.SW1_row.size () > 0)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW1_row.at (0)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW1_row.at (0))),true,
    //                            specimen.SW1_state.at (0) == 1, "Contact 1");
    //            keepCallout();
    //        }

    //        if (specimen.SW1_row.size () > 1)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW1_row.at (1)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW1_row.at (1))),true,
    //                            specimen.SW1_state.at (1) == 1, "Contact 1");
    //            keepCallout();
    //        }

    //        if (specimen.SW2_row.size () > 0)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW2_row.at (0)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW2_row.at (0))),true,
    //                            specimen.SW2_state.at (0) == 1, "Contact 2");
    //            keepCallout();
    //        }

    //        if (specimen.SW2_row.size () > 1)
    //        {
    //            tooltip_el (    QPointF((qreal) specimen.travel.at
    //            (specimen.SW2_row.at (1)) - specimen.approachDistance,
    //                                    (qreal) specimen.force.at
    //                                    (specimen.SW2_row.at (1))),true,
    //                            specimen.SW2_state.at (1) == 1, "Contact 2");
    //            keepCallout();
    //        }

    m_chart->addSeries(series);

    //        series->findHapticPoints ();

    //        for (auto pointName : series->hapticPoints.keys ()){

    //            tooltip_ex(series->hapticPoints.value (pointName), true,
    //            Qt::red, pointName); keepCallout();
    //        }

    //        intPoints[series->name()].append(series->callouts);
    //        intPoints.insert (series->name(), series->callouts);

    intPoints.insert(series->name(), m_callouts);
    m_callouts.clear();

    if (m_colors_haptic_curve.size() > index) {
      if (m_colors.value(m_colors_haptic_curve.at(index)).isValid())
        series->setColor(m_colors.value(m_colors_haptic_curve.at(index)));
    }
    index++;

    series->attachAxis(axis_travel);
    series->attachAxis(axis_force);

    connect(series, &QScatterSeries::clicked, this, &View::keepCallout);
    connect(series, &QScatterSeries::hovered, this, &View::tooltip);

    specimen.force.clear();
    specimen.force.squeeze();
    specimen.travel.clear();
    specimen.travel.squeeze();
    specimen.time.clear();
    specimen.time.squeeze();
    specimen.contact1.clear();
    specimen.contact1.squeeze();
    specimen.cycle.clear();
    specimen.cycle.squeeze();
  }

  //    auto timestamp1 = QDateTime::currentMSecsSinceEpoch();
  //    QList<QFuture<void>> flist;
  //    for (auto serie : m_chart->series())
  //    {
  //        auto temp_serie = qobject_cast<hapticSerie *>(serie);
  //        flist.append(QtConcurrent::run(temp_serie,
  //        &hapticSerie::findHapticPoints));
  //    }

  //    for (auto f : flist)
  //    {
  //        while (!f.isFinished())
  //            QApplication::processEvents();
  //    }

  //    auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << "interest points in viewlllll found in ms" << timestamp2 -
  //    timestamp1;

  auto timestamp1 = QDateTime::currentMSecsSinceEpoch();
  QList<QFuture<void>> flist;

  for (auto serie : m_chart->series()) {
    auto temp_serie = qobject_cast<hapticSerie *>(serie);
    if (temp_serie) {
      flist.append(
          QtConcurrent::run([=]() { temp_serie->findHapticPoints(); }));
    }
  }

  for (auto &f : flist) {
    f.waitForFinished();
  }

  auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
  qDebug() << "interest points in viewlllll found in ms"
           << timestamp2 - timestamp1;

  //

  for (auto serie : m_chart->series()) {
    auto temp_serie = qobject_cast<hapticSerie *>(serie);
    for (auto pointName : temp_serie->hapticPoints.keys()) {
      tooltip_ex(temp_serie->hapticPoints.value(pointName), true, Qt::red,
                 pointName);
      keepCallout();
    }

    intPoints.insert(temp_serie->name(), m_callouts);
    m_callouts.clear();
  }

  for (MeasSpecimen specimen : serie.specimenList) {

    if (specimen.SW1_row.size() > 0) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW1_row.at(0)), true,
                     specimen.SW1_state.at(0) == 1, "Contact 1");
          keepCallout();

          if (specimen.SW1_state.at(0) == 1) {
            h_serie->hapticPointsRow.insert("D1 pressed",
                                            specimen.SW1_row.at(0) + 1);
          } else {
            h_serie->hapticPointsRow.insert("D1 released",
                                            specimen.SW1_row.at(0) + 1);
          }
        }
      }
    }

    if (specimen.SW1_row.size() > 1) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW1_row.at(1)), true,
                     specimen.SW1_state.at(1) == 1, "Contact 1");
          keepCallout();

          if (specimen.SW1_state.at(1) == 1) {
            h_serie->hapticPointsRow.insert("D1 pressed",
                                            specimen.SW1_row.at(1) + 1);
          } else {
            h_serie->hapticPointsRow.insert("D1 released",
                                            specimen.SW1_row.at(1) + 1);
          }
        }
      }
    }

    if (specimen.SW2_row.size() > 0) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW2_row.at(0)), true,
                     specimen.SW1_state.at(0) == 1, "Contact 1");
          keepCallout();

          if (specimen.SW1_state.at(0) == 1) {
            h_serie->hapticPointsRow.insert(QStringLiteral("D2 pressed"),
                                            specimen.SW2_row.at(0) + 1);
          } else {
            h_serie->hapticPointsRow.insert("D2 released",
                                            specimen.SW2_row.at(0) + 1);
          }
        }
      }
    }

    if (specimen.SW2_row.size() > 1) {
      qreal this_offset = 0;
      for (auto s : m_chart->series()) {
        if (s->name() == specimen.name) {
          auto h_serie = qobject_cast<hapticSerie *>(s);
          this_offset = h_serie->offset;

          tooltip_el(h_serie->at(specimen.SW2_row.at(1)), true,
                     specimen.SW1_state.at(1) == 1, "Contact 2");
          keepCallout();

          if (specimen.SW1_state.at(1) == 1) {
            h_serie->hapticPointsRow.insert("D2 pressed",
                                            specimen.SW2_row.at(1) + 1);
          } else {
            h_serie->hapticPointsRow.insert("D2 released",
                                            specimen.SW2_row.at(1) + 1);
          }
        }
      }
    }

    intPoints[specimen.name].append(m_callouts);
    m_callouts.clear();
  }

  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
  m_chart->legend()->update();

  axis_force->setMin(0);
  axis_travel->setMin(0);

  axis_travel->setMax(maxChartTravel * 1.15);
  axis_force->setMax(maxChartForce * 1.15);

  initialScale.insert(axis_travel, QPointF(0, axis_travel->max()));
  initialScale.insert(axis_force, QPointF(0, axis_force->max()));

  m_chart->setAcceptHoverEvents(true);
  setRenderHint(QPainter::Antialiasing);
  scene()->addItem(m_chart);

  m_coordX = new QGraphicsSimpleTextItem(m_chart);
  m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
  m_coordX->setText("X: ");
  m_coordY = new QGraphicsSimpleTextItem(m_chart);
  m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());
  m_coordY->setText("Y: ");

  const auto markers = m_chart->legend()->markers();
  for (QLegendMarker *marker : markers) {
    // Disconnect possible existing connection to avoid multiple connections
    QObject::disconnect(marker, &QLegendMarker::clicked, this,
                        &View::handleMarkerClicked);
    QObject::connect(marker, &QLegendMarker::clicked, this,
                     &View::handleMarkerClicked);
  }

  for (auto axis : m_chart->axes())
    adjustAxisRange(qobject_cast<QValueAxis *>(axis));

  //    applyNiceNumbers();

  for (auto axis : m_chart->axes())
    QObject::connect(qobject_cast<QValueAxis *>(axis),
                     &QValueAxis::rangeChanged, this, &View::proc_rangeChanged);

  if (m_tooltip)
    m_tooltip->hide();
  this->setMouseTracking(true);

  this->good = true;
  this->is_durability = true;

  qDeleteAll(errors);

  /* ZS2 */
}

void View::exportDurability() {

  QDir dir = QFileInfo(this->folder + "\\" + "test" + ".csv").absoluteDir();
  QString absdir = dir.absolutePath();

  auto export_folder = this->folder + "_export\\";
  qDebug() << "dir.dirName ()" << dir.dirName() << "export_folder"
           << export_folder;
  QDir().mkdir(export_folder);

  if (!QFile::exists(export_folder + dir.dirName() + "_Graphs.xlsm"))
    QFile::copy(QCoreApplication::applicationDirPath() + "\\Graphs.XLSM",
                export_folder + dir.dirName() + "_Graphs.xlsm");

  qDebug() << "applicationDirPath:" << QCoreApplication::applicationDirPath();

  for (auto s : m_chart->series()) {

    auto serie = qobject_cast<hapticSerie *>(s);

    QString fileContents;
    QFile csvFile;

    fileContents.append("Travel [mm];"
                        "Force [N];"
                        "Haptic Points;\n");

    auto dataVector = serie->points();

    qDebug() << "serie->hapticPoints before points conversion:"
             << serie->hapticPoints << serie->hapticPointsRow;

    for (int i = 0; i < dataVector.size(); i++) {
      //
      for (auto h_point_name : serie->hapticPoints.keys()) {

        if (serie->hapticPoints.value(h_point_name) == dataVector[i]) {

          serie->hapticPointsRow.insert(h_point_name,
                                        i - 1); // try to fix 1 point offset
          qDebug() << serie->name() << "Point row:" << h_point_name
                   << dataVector[i] << i;
        }
      }
    }

    for (int i = 0; i < dataVector.size(); i++) {

      fileContents.append(QLocale().toString(dataVector[i].x()) + ";" +
                          QLocale().toString(dataVector[i].y()) + ";");

      if (!serie->hapticPointsRow.isEmpty()) {
        auto p_name = serie->hapticPointsRow.lastKey();
        auto p_row = serie->hapticPointsRow.take(p_name);
        fileContents.append(p_name + ";" + QString::number(p_row + 2));
      }
      fileContents.append("\r\n");
    }

    QDir dir = QFileInfo(path).absoluteDir();
    QString absdir = dir.absolutePath();

    csvFile.setFileName(export_folder + serie->name() + ".csv");
    qDebug() << "CSV File: "
             << absdir + "\\" + serieName + "\\" + serie->name() + ".csv";
    qDebug() << "folder" << export_folder;
    csvFile.open(QIODevice::WriteOnly);

    csvFile.write(fileContents.toUtf8());
    csvFile.close();
    fileContents.clear();
  }

  QFileInfo fileinfo(absdir);
  QDesktopServices::openUrl("file:" + export_folder.replace("/", "\\"));
}

void View::exportRobot() {

  QDir dir = QFileInfo(this->folder + "\\" + "test" + ".csv").absoluteDir();
  QString absdir = dir.absolutePath();

  QFileInfo fi(absdir);

  qDebug() << "fi:" << Qt::endl
           << " 1:" << fi << Qt::endl
           << " 2:" << fi.isDir() << Qt::endl
           << " 3:" << fi.isFile() << Qt::endl
           << " 4:" << fi.exists() << Qt::endl
           << " 5:" << fi.absoluteDir() << Qt::endl
           << " 6:" << fi.absoluteFilePath() << Qt::endl
           << " 7:" << fi.absolutePath() << Qt::endl
           << " 8:" << fi.baseName() << Qt::endl
           << " 9:" << fi.completeBaseName() << Qt::endl
           << "10:" << fi.dir() << Qt::endl
           << "11:" << fi.fileName() << Qt::endl
           << "12:" << fi.suffix() << Qt::endl
           << "13:" << fi.completeSuffix() << Qt::endl
           << "14:" << fi.filePath() << Qt::endl
           << "15:" << fi.size() << Qt::endl
           << Qt::endl
           << "15:" << fi.size() << Qt::endl
           << Qt::endl;

  auto export_folder = fi.absolutePath() + "\\";
  qDebug() << "absdir" << absdir;
  //    QDir().mkdir(export_folder);

  //    if(!QFile::exists (export_folder + dir.dirName () + "_Graphs.xlsm"))
  //        QFile::copy(QCoreApplication::applicationDirPath() +
  //        "\\Graphs.XLSM",
  //                    export_folder  + dir.dirName () + "_Graphs.xlsm");

  if (!QFile::exists(export_folder + "Graphs.xlsm"))
    QFile::copy(QCoreApplication::applicationDirPath() + "\\Graphs.XLSM",
                export_folder + "Graphs.xlsm");

  //    for (auto s : m_chart->series ()){

  //        auto serie = qobject_cast<QScatterSeries*>(s);

  auto serie = qobject_cast<QScatterSeries *>(m_chart->series().first());

  QString fileContents;
  QFile csvFile;

  fileContents.append("Travel [mm];"
                      "Force [N];"
                      "Haptic Points;\n");

  auto dataVector = serie->points();

  qDebug() << "serie->hapticPoints before points conversion:"
           << this->data_loader->hapticPoints;

  for (int i = 0; i < dataVector.size(); i++) {
    for (auto h_point_name : this->data_loader->hapticPoints.keys()) {
      if (this->data_loader->hapticPoints.value(h_point_name) ==
          dataVector[i]) {
        this->data_loader->hapticPointsRow.insert(h_point_name, i);
        qDebug() << "Point row:" << h_point_name << dataVector[i] << i;
      }
    }
  }

  for (int i = 0; i < dataVector.size(); i++) {

    fileContents.append(QLocale().toString(dataVector[i].x()) + ";" +
                        QLocale().toString(dataVector[i].y()) + ";");

    if (!this->data_loader->hapticPointsRow.isEmpty()) {
      auto p_name = this->data_loader->hapticPointsRow.lastKey();
      auto p_row = this->data_loader->hapticPointsRow.take(p_name);
      fileContents.append(p_name + ";" + QString::number(p_row + 1)); // 2
    }
    fileContents.append("\r\n");
  }

  //    QDir dir = QFileInfo(path).absoluteDir();
  //    QString absdir = dir.absolutePath();

  csvFile.setFileName(export_folder + fi.baseName() + ".csv");
  qDebug() << "CSV File: "
           << absdir + "\\" + serieName + "\\" + serie->name() + ".csv";
  qDebug() << "folder" << export_folder;
  csvFile.open(QIODevice::WriteOnly);

  csvFile.write(fileContents.toUtf8());
  csvFile.close();
  fileContents.clear();
  //    }

  //    QFileInfo fileinfo(absdir);
  //    QDesktopServices::openUrl("file:" + export_folder.replace ("/", "\\"));
}

void View::construct_ParamChart(QString path) {
  qDebug() << "PARAM CHART IN CONSTRUCTION:" << path;

  /*! \var m_chart a new chart is created as a child of this view */

  m_chart = new QChart();
  m_chart->setParent(this);
  this->setRenderHint(QPainter::Antialiasing);

  m_chart->setTheme(QChart::ChartThemeBlueNcs);
  m_chart->setTitle("Parameter test chart prototype");

  m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
  m_chart->setAnimationOptions(QChart::NoAnimation);
  this->setRenderHint(QPainter::Antialiasing);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);

  QCategoryAxis *axis_x = new QCategoryAxis(m_chart);
  m_chart->addAxis(axis_x, Qt::AlignBottom);
  axis_x->setObjectName("sgdh"); // axisX
  axis_x->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_x->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_x->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_x->setMinorTickCount(1);
  axis_x->setTickCount(11);
  axis_x->setTitleText(
      "Parameter tests"); // Angle [arcdeg]//Travel [N]Running Time [days]
  axis_x->setTitleFont(QFont("Verdana", 12, QFont::Bold));
  axis_x->setLabelFormat("%4.1f"); //("");%4.0f%4.1fTd
  axis_x->setLabelsEditable(true);

  this->axis_current = new QValueAxis(m_chart);
  m_chart->addAxis(axis_current, Qt::AlignLeft);
  axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_current->setObjectName("axis_current");
  axis_current->setLabelFormat("%4.1RV ");
  axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
  axis_current->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
  axis_current->setTitleText("Current [A]");
  axis_current->setMinorTickCount(1);
  axis_current->setTickCount(11);
  axis_current->setLabelsEditable(true);
  axis_current->setVisible(true);

  //        QString s_name;

  //        qDebug() << data_loader->y_dataMap.keys ();

  //        QVector<QPointF>* pressure_vector = nullptr;
  //        for(QVector<QPointF>* data_vector : data_loader->y_dataMap.values
  //        ())
  //        {
  //            s_name = data_loader->y_dataMap.key(data_vector);
  //            if((s_name.contains ("[bar"))){
  //                pressure_vector = data_vector;
  //                break;
  //            }
  //        }

  //        for(QVector<QPointF>* data_vector : data_loader->y_dataMap.values
  //        ())
  //        {
  //            s_name = data_loader->y_dataMap.key(data_vector);
  //            if((s_name.contains ("[bar"))) continue;
  //            if((s_name.contains ("_Tolerance"))) continue;

  //            qDebug() << "converting serie:" << s_name;
  //            for (int i = 0; i < data_vector->size (); i++){
  //                //                    qDebug() << "replacing:" <<
  //                data_vector->at (i) << "to" << pressure_vector->at (i);
  //                data_vector->replace (i, QPointF(pressure_vector->at (i).y
  //                (),data_vector->at (i).y ()));
  //            }

  //            QScatterSeries* chart_serie;
  //            chart_serie = new QScatterSeries(m_chart);
  //            chart_serie->replace (*data_vector);

  //            chart_serie->setUseOpenGL (true);
  //            chart_serie->setPointsVisible (true);
  //            chart_serie->setMarkerSize (6);
  //            chart_serie->setBorderColor ("transparent");

  //            chart_serie->setName (s_name);
  //            chart_serie->setProperty ("my_order", QVariant(m_chart->series
  //            ().size () + 1));

  //            m_chart->addSeries(chart_serie);
  //            chart_serie->attachAxis(axisX);
  //            chart_serie->attachAxis(axis_voltage);
  //            chart_serie->setProperty ("my_type",
  //            QVariant(QObject::tr("voltage")));
  //        }

  //        QLineSeries* min_tolerance;
  //        min_tolerance = new QLineSeries(m_chart);
  //        min_tolerance->append (QPointF(0.0, 1.02));
  //        min_tolerance->append (QPointF(5.0, 3.62));
  //        min_tolerance->setUseOpenGL (true);
  //        min_tolerance->setName ("Min Tolerance");
  //        min_tolerance->setProperty ("my_order", QVariant(10000));
  //        m_chart->addSeries(min_tolerance);
  //        min_tolerance->attachAxis(axisX);
  //        min_tolerance->attachAxis(axis_voltage);

  //        QLineSeries* max_tolerance;
  //        max_tolerance = new QLineSeries(m_chart);
  //        max_tolerance->append (QPointF(0.0, 1.23));
  //        max_tolerance->append (QPointF(5.0, 4.13));
  //        max_tolerance->setUseOpenGL (true);
  //        max_tolerance->setName ("Max Tolerance");
  //        max_tolerance->setProperty ("my_order", QVariant(10001));
  //        m_chart->addSeries(max_tolerance);
  //        max_tolerance->attachAxis(axisX);
  //        max_tolerance->attachAxis(axis_voltage);

  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignRight);
  m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
  m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
  m_chart->legend()->update();

  axis_x->setMin(0);
  axis_current->setMin(0);

  initialScale.insert(axis_x, QPointF(0, axis_voltage->max()));
  initialScale.insert(axis_current, QPointF(0, axisX->max()));

  // m_chart->setAcceptHoverEvents(true);

  setRenderHint(QPainter::Antialiasing);
  scene()->addItem(m_chart);

  m_coordX = new QGraphicsSimpleTextItem(m_chart);
  m_coordX->setPos(m_chart->size().width() / 2 - 50, m_chart->size().height());
  m_coordX->setText("X: ");
  m_coordY = new QGraphicsSimpleTextItem(m_chart);
  m_coordY->setPos(m_chart->size().width() / 2 + 50, m_chart->size().height());
  m_coordY->setText("Y: ");

  const auto markers = m_chart->legend()->markers();
  for (QLegendMarker *marker : markers) {
    // Disconnect possible existing connection to avoid multiple connections
    QObject::disconnect(marker, &QLegendMarker::clicked, this,
                        &View::handleMarkerClicked);

    QObject::connect(marker, &QLegendMarker::clicked, this,
                     &View::handleMarkerClicked);
  }

  for (auto axis : m_chart->axes())
    adjustAxisRange(qobject_cast<QValueAxis *>(axis));

  applyNiceNumbers();

  for (auto axis : m_chart->axes())
    QObject::connect(qobject_cast<QValueAxis *>(axis),
                     &QValueAxis::rangeChanged, this, &View::proc_rangeChanged);

  //                QObject::connect(this->axisX, &QValueAxis::rangeChanged,
  //                this, &View::handleTimeRangeChanged);
  //                QObject::connect(this->axisX2, &QValueAxis::rangeChanged,
  //                this, &View::handleTime2RangeChanged);

  if (m_tooltip != nullptr)
    m_tooltip->hide();
  this->setMouseTracking(true);
  this->good = true;
  this->is_robot = true;
}

void loadCSVinDataManager(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  QDataStream in(&file);
  QList<QByteArray> lines;
  int maxLineLength = 2000;
  int lineCount = 200;

  for (int linedRead = 0; linedRead < lineCount; linedRead++) {
    if (file.atEnd())
      break;
    lines << file.readLine(maxLineLength);

    qDebug() << lines.back();
  }

  int maxSegments = 0;
  char semicolon = ';';
  int max_semicolon;
  char comma = ',';
  int max_comma;
  char space = ' ';
  int max_space;
  char tab = '\t';
  int max_tab;

  for (auto &line : lines) {
    QList<QByteArray> segments = line.split(comma);
    max_comma = qMax(max_comma, segments.size());
    segments.clear();

    segments = line.split(semicolon);
    max_semicolon = qMax(max_semicolon, segments.size());
    segments.clear();

    segments = line.split(space);
    max_space = qMax(max_space, segments.size());
    segments.clear();

    segments = line.split(tab);
    max_tab = qMax(max_tab, segments.size());
    segments.clear();
  }

  file.close();
}

double engineering_value(double value) {

  value = qAbs(value);

  if (value >= 3000000) {
    value /= 1000000;
  } else if (value >= 3000) {
    value /= 1000;
  } else if (value >= 3) {
    // No unit needed
  } else if (value >= 0.003) {
    value *= 1000;
  } else if (value >= 0.000003) {
    value *= 1000000;
  } else {
    value *= 1000000000;
  }

  return value;
}

void View::adjustAxisFormat(QValueAxis *axis, qreal min, qreal max) {
  int ticks = axis->tickCount();
  if (ticks < 2)
    return;

  min = engineering_value(min);
  max = engineering_value(max);

  double interval = (max - min) / (ticks - 1);

  // Compute minimal decimals ensuring unique tick labels
  int decimals = 0;
  bool uniqueLabels = false;
  while (!uniqueLabels && decimals < 10) {
    uniqueLabels = true;
    QString prev = QString::number(min, 'f', decimals);
    for (int i = 1; i < ticks; ++i) {
      double value = min + i * interval;
      QString current = QString::number(value, 'f', decimals);
      if (current == prev) {
        uniqueLabels = false;
        break;
      }
      prev = current;
    }
    if (!uniqueLabels)
      ++decimals;
  }

  // Build a new label format string that preserves the engineering marker 'R'
  // and the unit 'A' Note: "%%" escapes a literal '%' in the QString format.
  QString formatStr = QString("4.%1RA ").arg(decimals);
  formatStr = "%" + formatStr;
  qDebug() << "Axis format:" << formatStr;
  axis->setLabelFormat(formatStr);
}

void View::construct_MFU_chart(QString path) {
  loadCSVinDataManager(path);

  {

    this->folder = path;

    int timestamp1 = QDateTime::currentMSecsSinceEpoch();

    m_chart = new QChart();
    m_chart->setParent(this);
    this->setRenderHint(QPainter::Antialiasing);

    QFuture<int> future = QtConcurrent::run(
        [&](QString path) { return this->loadCSV_opt(path); }, path);

    while (!future.isFinished())
      QApplication::processEvents();

    int res = future.result();

    if (res < 0) {
      //            QMessageBox::information(this,  tr("Error:"),  "The file
      //            at\n" + path + "\ncould not be loaded!");
      return;
    }

    //        this->serieName = fileNameArg;

    int timestamp2 = QDateTime::currentMSecsSinceEpoch();

    qDebug() << "File processing time" << timestamp2 - timestamp1;

    //        this->loadCSV (path);

    m_chart->setTheme(QChart::ChartThemeBlueNcs);
    QFileInfo fi(serieName);

    m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));
    m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
    m_chart->setAnimationOptions(QChart::NoAnimation);
    this->setRenderHint(QPainter::Antialiasing);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    qDebug() << "temperature->count ()" << temperature->count();
    qDebug() << "temperature_sv->count ()" << temperature_sv->count();
    qDebug() << "humidity->count ()" << humidity->count();
    qDebug() << "humidity_sv->count ()" << humidity_sv->count();

    if (temperature->count() > 1) {
      temperature->replace(
          temperature->count() - 1,
          QPointF(temperature->at(temperature->count() - 1).x(),
                  temperature->at(temperature->count() - 1).y() - 0.0001));

      temperature->setPen(QPen(QColor(0, 100, 255), 2, Qt::SolidLine));
      if (m_colors.value("temperature").isValid())
        temperature->setPen(
            QPen(m_colors.value("temperature"), 2, Qt::SolidLine));
      // temperature->setUseOpenGL (true);
      temperature->setName("Temperature");
      temperature->setObjectName("Temperature");
      temperature->setProperty("my_type", QVariant(QObject::tr("temperature")));
      temperature->setProperty("my_order", QVariant(99));
      //          mfu_error->setProperty ("my_symbol",
      //          QVariant(QObject::tr("×")));

      QColor my_color = temperature->color();
      temperature->setProperty("my_color", QVariant(my_color.rgba()));
      temperature->setProperty("my_symbol_size", QVariant(20));
      m_chart->addSeries(temperature);

      //            QObject::connect(temperature, &QLineSeries::pressed, this,
      //            &View::handleSerieClick);
    }

    if (temperature_sv->count() > 1) {
      temperature_sv->replace(
          temperature_sv->count() - 1,
          QPointF(temperature_sv->at(temperature_sv->count() - 1).x(),
                  temperature_sv->at(temperature_sv->count() - 1).y() -
                      0.0001));

      temperature_sv->setPen(QPen(QColor(94, 196, 255), 2, Qt::SolidLine));
      //            if (m_colors.value ("temperature").isValid ())
      //                temperature_sv->setPen(QPen(Qt::gray, 2, Qt::SolidLine
      //                ));
      //            temperature_sv->setPen(QPen(m_colors.value ("temperature"),
      //            2, Qt::SolidLine ));
      // temperature->setUseOpenGL (true);
      temperature_sv->setName("Temperature_SetVal");
      temperature_sv->setObjectName("Temperature");
      temperature_sv->setProperty("my_type",
                                  QVariant(QObject::tr("temperature")));
      temperature_sv->setProperty("my_order", QVariant(10));
      //          mfu_error->setProperty ("my_symbol",
      //          QVariant(QObject::tr("×")));

      QColor my_color = temperature_sv->color();
      temperature_sv->setProperty("my_color", QVariant(my_color.rgba()));
      temperature_sv->setProperty("my_symbol_size", QVariant(20));
      m_chart->addSeries(temperature_sv);

      //            QObject::connect(temperature, &QLineSeries::pressed, this,
      //            &View::handleSerieClick);
    }

    if (voltage->count() > 1) {
      voltage->replace(voltage->count() - 1,
                       QPointF(voltage->at(voltage->count() - 1).x(),
                               voltage->at(voltage->count() - 1).y() - 0.0001));

      voltage->setPen(QPen(QColor(193, 97, 0), 2, Qt::SolidLine));
      if (m_colors.value("voltage").isValid())
        voltage->setPen(QPen(m_colors.value("voltage"), 2, Qt::SolidLine));
      // temperature->setUseOpenGL (true);
      voltage->setName("Voltage");
      voltage->setObjectName("voltage");
      voltage->setProperty("my_type", QVariant(QObject::tr("voltage")));
      voltage->setProperty("my_order", QVariant(98));
      m_chart->addSeries(voltage);
    }

    if (humidity->count() > 1) {
      double max = humidity->at(0).y();
      for (QPointF point : humidity->points())
        max < point.y() ? max = point.y() : max;
      if (max == 0) {
        humidity->clear();
        humidity_sv->clear();
        qDebug() << "max humid is zero!";
      } else {
        humidity->setPen(QPen(QColor(255, 0, 255), 2, Qt::SolidLine));
        if (m_colors.value("humidity").isValid())
          humidity->setPen(QPen(m_colors.value("humidity"), 2, Qt::SolidLine));
        humidity->setName("Humidity");
        humidity->setObjectName("humidity");
        humidity->setProperty("my_type", QVariant(QObject::tr("humidity")));
        humidity->setProperty("my_order", QVariant(97));
        m_chart->addSeries(humidity);
      }
    }

    if (humidity_sv->count() > 1) {
      double max = humidity_sv->at(0).y();
      for (QPointF point : humidity_sv->points())
        max < point.y() ? max = point.y() : max;
      if (max == 0) {
        humidity_sv->clear();
      } else {
        humidity_sv->setPen(QPen(QColor(209, 140, 255), 2, Qt::SolidLine));
        //                if (m_colors.value ("humidity").isValid ())
        //                humidity_sv->setPen(QPen(m_colors.value ("humidity"),
        //                2, Qt::SolidLine ));
        //                humidity_sv->setPen(QPen(Qt::gray, 2, Qt::SolidLine
        //                ));
        humidity_sv->setName("Humidity_SetVal");
        humidity_sv->setObjectName("humidity");
        humidity_sv->setProperty("my_type", QVariant(QObject::tr("humidity")));
        humidity_sv->setProperty("my_order", QVariant(11));
        m_chart->addSeries(humidity_sv);
      }
    }

    if (current_activ->count() > 1) {
      current_activ->setPointsVisible(true);
      current_activ->setColor(QColor(255, 150, 0));
      if (m_colors.value("current_active").isValid())
        current_activ->setColor(m_colors.value("current_active"));
      current_activ->setMarkerSize(6);
      current_activ->setBorderColor("transparent");
      // current_activ->setUseOpenGL (true);
      current_activ->setName("Current active");
      current_activ->setObjectName("current_active");
      current_activ->setProperty("my_type", QVariant(QObject::tr("current")));
      current_activ->setProperty("my_order", QVariant(96));
      m_chart->addSeries(current_activ);
    }

    if (current_KL30C->count() > 1) {
      current_KL30C->setPointsVisible(true);
      current_KL30C->setColor(QColor(255, 150, 0));
      if (m_colors.value("current_active").isValid())
        current_KL30C->setColor(m_colors.value("current_active"));
      current_KL30C->setMarkerSize(6);
      current_KL30C->setBorderColor("transparent");
      // current_activ->setUseOpenGL (true);
      current_KL30C->setName("Current KL30C");
      current_KL30C->setObjectName("current_active");
      current_KL30C->setProperty("my_type", QVariant(QObject::tr("current")));
      current_KL30C->setProperty("my_order", QVariant(26));
      m_chart->addSeries(current_KL30C);
    }

    if (current_sleep->count() > 1) {
      current_sleep->setPointsVisible(true);
      current_sleep->setColor(QColor(50, 130, 50));
      if (m_colors.value("current_sleep").isValid())
        current_sleep->setColor(m_colors.value("current_sleep"));
      current_sleep->setMarkerSize(6);
      current_sleep->setBorderColor("transparent");
      // current_sleep->setUseOpenGL (true);
      current_sleep->setName("Current sleep");
      current_sleep->setObjectName("current_sleep");
      current_sleep->setProperty("my_type", QVariant(QObject::tr("current")));
      current_sleep->setProperty("my_order", QVariant(95));
      m_chart->addSeries(current_sleep);
    }

    if (OM->count() > 1) {
      OM->setPen(QPen(Qt::black, 1, Qt::SolidLine));
      // OM->setUseOpenGL (true);
      OM->setName("Operating Mode");
      OM->setObjectName("om");
      OM->setProperty("my_type", QVariant(QObject::tr("om")));
      OM->setProperty("my_order", QVariant(150));
      m_chart->addSeries(OM);
    }

    this->axisX = new QValueAxis(m_chart); // QValueAxis *axisX
    m_chart->addAxis(axisX, Qt::AlignBottom);
    axisX->setObjectName("axisX");
    axisX->setTickCount(11);
    //    axisX->setTitleText("Running Time");
    //    axisX->setTitleFont(QFont("Verdana", 14, QFont::Bold));
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axisX->setMinorTickCount(1);
    axisX->setLabelFormat("%4.1Td");
    axisX->setLabelsEditable(true);

    this->axisX2 = new QValueAxis(m_chart);
    axisX2->setObjectName("axisX2");
    axisX2->setTickCount(11);
    //    axisX2->setTitleText("chart time scale");
    //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
    axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
    axisX2->setMinorTickCount(3);
    axisX2->setLabelFormat("%4.1Td");
    axisX2->setTickAnchor(0);
    m_chart->addAxis(axisX2, Qt::AlignBottom);

    //        QLineSeries *dummy_serie = new QLineSeries(m_chart);
    //        m_chart->addSeries(dummy_serie);

    if ((current_activ->count() > 1) or (AS_currents.size() > 0)) {
      //            qDebug() << "creating curr axis";
      this->axis_current = new QValueAxis(m_chart);
      axis_current->setObjectName("axis_current");
      //    axis_current->setTitleText("Current Consumption");
      //    axis_current->setTitleFont(QFont("Verdana", 14, QFont::Bold));
      //    axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_current->setLabelFormat("%4.1RA ");
      axis_current->setTickCount(11);
      axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_current->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_current->setMinorTickCount(1);
      axis_current->setLabelsEditable(true);
      m_chart->addAxis(axis_current, Qt::AlignRight);

      connect(axis_current, &QValueAxis::rangeChanged, this,
              [this](qreal min, qreal max) {
                adjustAxisFormat(this->axis_current, min, max);
              });
    }
    if (current_activ->count() > 1) {
      axis_current->setLabelsColor(current_activ->color());
      axis_current->setLinePenColor(current_activ->color());
      current_activ->attachAxis(axisX);
      current_activ->attachAxis(axis_current);
    }

    if (current_KL30C->count() > 1) {
      current_KL30C->attachAxis(axisX);
      current_KL30C->attachAxis(axis_current);
    }

    if (current_sleep->count() > 1) {
      this->axis_sleep_current = new QValueAxis(m_chart);
      axis_sleep_current->setObjectName("axis_sleep_current");
      //    axis_current->setTitleText("Current Consumption");
      //    axis_current->setTitleFont(QFont("Verdana", 14, QFont::Bold));
      //    axis_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold))
      axis_sleep_current->setLabelFormat("%4.1RA ");
      if (current_activ->count() > 1)
        axis_sleep_current->setLabelFormat("%4.1RA ");

      axis_sleep_current->setTickCount(11);
      axis_sleep_current->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_sleep_current->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_sleep_current->setMinorTickCount(1);
      axis_sleep_current->setLabelsEditable(true);
      m_chart->addAxis(axis_sleep_current, Qt::AlignRight);

      connect(axis_sleep_current, &QValueAxis::rangeChanged, this,
              [this](qreal min, qreal max) {
                adjustAxisFormat(this->axis_sleep_current, min, max);
              });

      //        qDebug() << "current_serie_sleep" <<
      //        mfu_sample.current_sleep.size ();
      axis_sleep_current->setLabelsColor(current_sleep->color());
      axis_sleep_current->setLinePenColor(current_sleep->color());
      current_sleep->attachAxis(axisX);
      current_sleep->attachAxis(axis_sleep_current);
    }

    if (!AS_volts.values().isEmpty() or !voltage->points().isEmpty()) {
      axis_voltage = new QValueAxis(m_chart);
      axis_voltage->setObjectName("axis_voltage");
      if (!voltage->points().isEmpty())
        axis_voltage->setLabelsColor(voltage->color());
      //        axis_voltage->setTitleText("Voltage");
      //        axis_voltage->setTitleFont(QFont("Verdana", 14, QFont::Bold));
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setLabelFormat("%4.3RV ");
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_voltage->setLabelsColor(voltage->color());
      axis_voltage->setLinePenColor(voltage->color());
      axis_voltage->setMinorTickCount(1);
      axis_voltage->setTickCount(11);
      axis_voltage->setLabelsEditable(true);

      m_chart->addAxis(axis_voltage, Qt::AlignRight);

      if (!voltage->points().isEmpty()) {
        voltage->attachAxis(axisX);
        voltage->attachAxis(axis_voltage);
      }
    }

    if (humidity->count() > 1) {
      qDebug() << "Creating the humidity axis.....";
      this->axis_humidity = new QValueAxis(m_chart);
      this->axis_humidity->setObjectName("axis_humidity");
      this->axis_humidity->setTickCount(11);
      this->axis_humidity->setLabelFormat(
          QString("%4.0f" + QString::fromLatin1("%rh ")).toUtf8());
      this->axis_humidity->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      this->axis_humidity->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      this->axis_humidity->setMinorTickCount(1);
      this->axis_humidity->setLabelsEditable(true);
      m_chart->addAxis(this->axis_humidity, Qt::AlignLeft);

      this->axis_humidity->setLabelsColor(humidity->color());
      this->axis_humidity->setLinePenColor(humidity->color());
      humidity->attachAxis(this->axisX);
      humidity->attachAxis(this->axis_humidity);
    }

    if (humidity_sv->count() > 1) {

      humidity_sv->attachAxis(this->axisX);
      qDebug() << "humidity->count()" << humidity->count();
      humidity_sv->attachAxis(this->axis_humidity);
    }

    if (temperature->count() > 1) {
      qDebug() << "Creating the temperature axis.....";
      this->axis_temperature = new QValueAxis(m_chart);

      axis_temperature->setObjectName("axis_temperature");

      //        axis_temperature->setTitleText("Temperature");
      //        axis_temperature->setTitleFont(QFont("Verdana", 14,
      //        QFont::Bold));
      axis_temperature->setTickCount(11);
      axis_temperature->setLabelFormat(
          QString("%4.0f" + QString::fromLatin1("°C ")).toUtf8());
      axis_temperature->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_temperature->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_temperature->setMinorTickCount(1);
      axis_temperature->setLabelsEditable(true);
      m_chart->addAxis(axis_temperature, Qt::AlignLeft);

      axis_temperature->setLabelsColor(temperature->color());
      axis_temperature->setLinePenColor(temperature->color());
      temperature->attachAxis(axisX);
      temperature->attachAxis(axis_temperature);
      axis_temperature->setVisible(true);
    }

    if (temperature_sv->count() > 1) {

      temperature_sv->attachAxis(axisX);
      temperature_sv->attachAxis(axis_temperature);
    }

    if (OM->count() > 1) {
      axis_opmode = new QValueAxis(m_chart);
      axis_opmode->setObjectName("axis_opmode");
      axis_opmode->setTickCount(3);
      QString fmt = "%0.0fOpM|" + OM1_name + "|" + OM2_name + "|" + OM3_name;
      axis_opmode->setLabelFormat(fmt.toUtf8());
      axis_opmode->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_opmode->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_opmode->setMinorTickCount(0);
      axis_opmode->setTickAnchor(0);
      axis_opmode->setTickInterval(1);
      axis_opmode->setRange(0.9, 3.1);
      axis_opmode->setTickType(QValueAxis::TicksDynamic);
      axis_opmode->setLabelsEditable(true);
      m_chart->addAxis(axis_opmode, Qt::AlignLeft);
      OM->attachAxis(axisX);
      OM->attachAxis(axis_opmode);
      OM->setProperty("my_type", QVariant(QObject::tr("om")));
      axis_opmode->setVisible(true);
    }

    auto axis_dura_signals = new QValueAxis(m_chart);
    axis_dura_signals->setObjectName("axis_dura_signals");
    axis_dura_signals->setTickCount(6);

    axis_dura_signals->setLabelFormat("%0.0f");
    axis_dura_signals->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
    axis_dura_signals->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    axis_dura_signals->setMinorTickCount(0);
    //        axis_dura_signals->setTickAnchor(0);
    //        axis_dura_signals->setTickInterval(1);

    //        axis_dura_signals->setTickType(QValueAxis::TicksDynamic);

    axis_dura_signals->setLabelsEditable(true);
    m_chart->addAxis(axis_dura_signals, Qt::AlignRight);
    axis_dura_signals->setVisible(true);

    QList<QColor> colors;
    colors.append(Qt::red); // 0
    colors.append(Qt::magenta);
    colors.append(QColor(254, 182, 2));
    colors.append(QColor(255, 143, 0));
    colors.append(QColor(255, 1, 146));
    colors.append(QColor(194, 46, 210)); // 5

    colors.append(QColor(5, 232, 107)); // 6
    colors.append(QColor(5, 223, 161));
    colors.append(QColor(147, 223, 7));
    colors.append(QColor(140, 140, 222));
    colors.append(QColor(14, 118, 229));
    colors.append(QColor(213, 213, 0)); // 11

    colors.append(QColor(249, 5, 5)); // 12
    colors.append(QColor(249, 133, 17));
    colors.append(QColor(238, 123, 70));
    colors.append(QColor(238, 44, 86));
    colors.append(QColor(239, 2, 234));
    colors.append(QColor(213, 0, 0)); // 17

    int index = 0;
    auto m_colors_as_volt = m_colors.keys().filter("as_volt");

    index = 6;
    for (QVector<QPointF> *as_volt_vect : AS_volts.values()) {
      if (as_volt_vect->size() < 3)
        continue;

      // QScatterSeries* as_volt : AS_volts.values ()){
      QScatterSeries *as_volt = new QScatterSeries(this->m_chart);
      as_volt->replace(*as_volt_vect);

      QString s_name = AS_volts.key(as_volt_vect); // as_volt
      //            qDebug() << "here, line 576";

      as_volt->setPointsVisible(true);
      as_volt->setMarkerSize(6);
      as_volt->setBorderColor("transparent");
      as_volt->setColor(QColor(5, 232, 107));
      if ((index <= 11) and (colors.size() > index))
        as_volt->setColor(colors.at(index));
      //            AS_volts_serie->setUseOpenGL (true);

      if (m_colors_as_volt.size() > index - 6) {
        if (m_colors.value(m_colors_as_volt.at(index - 6)).isValid())
          as_volt->setColor(m_colors.value(m_colors_as_volt.at(index - 6)));
      }

      as_volt->setName(s_name);
      m_chart->addSeries(as_volt);
      //            qDebug() << "here, line 588" << as_volt->parent ();
      as_volt->attachAxis(axisX);
      as_volt->attachAxis(axis_voltage);
      as_volt->setProperty("my_type", QVariant(QObject::tr("voltage")));
      as_volt->setProperty("my_order", QVariant(m_chart->series().size() + 1));
      as_volt->setObjectName("as_volt_" + QString::number(index - 5));
      index++;

      double min = as_volt->points().first().y();
      double max = as_volt->points().first().y();
      for (QPointF point : as_volt->points()) {
        min = point.y() < min ? point.y() : min;
        max = point.y() > max ? point.y() : max;
      }
      if (axis_voltage->min() > min)
        axis_voltage->setMin(min);
      if (axis_voltage->max() < max)
        axis_voltage->setMax(max);
    }

    auto m_colors_as_curr = m_colors.keys().filter("as_curr");
    index = 12;
    for (QVector<QPointF> *as_current_vect : AS_currents.values()) {
      if (as_current_vect->size() < 3)
        continue;

      QScatterSeries *as_current = new QScatterSeries(this->m_chart);

      as_current->replace(*as_current_vect);

      QString s_name = AS_currents.key(as_current_vect);
      qDebug() << "AS_currents.key(as_current)" << s_name;
      as_current->setParent(this);
      qDebug() << "as_current->count()" << as_current->count();

      as_current->setPointsVisible(true);
      as_current->setMarkerSize(6);
      as_current->setBorderColor("transparent");
      as_current->setColor(QColor(249, 5, 5));
      if ((index <= 17) and (colors.size() > index))
        as_current->setColor(colors.at(index));
      // as_current->setUseOpenGL (true);

      if (m_colors_as_curr.size() > index - 12) {
        if (m_colors.value(m_colors_as_curr.at(index - 12)).isValid())
          as_current->setColor(m_colors.value(m_colors_as_curr.at(index - 12)));
      }

      as_current->setName(s_name);
      m_chart->addSeries(as_current);

      as_current->attachAxis(axisX);
      as_current->attachAxis(axis_current);
      as_current->setProperty("my_type", QVariant(QObject::tr("current")));
      as_current->setProperty("my_order",
                              QVariant(m_chart->series().size() + 1));
      as_current->setObjectName("as_current_" + QString::number(index - 11));
      index++;

      double min = as_current->points().first().y();
      double max = as_current->points().first().y();
      for (QPointF point : as_current->points()) {
        min = point.y() < min ? point.y() : min;
        max = point.y() > max ? point.y() : max;
      }
      if (axis_current->min() > min)
        axis_current->setMin(min);
      if (axis_current->max() < max)
        axis_current->setMax(max);
    }

    for (auto ser : m_chart->series())
      ser->setUseOpenGL(true);

    index = 0;
    auto m_colors_mfu_err = m_colors.keys().filter("mfu_err");
    // qDebug() << "errors list:" << errors.keys();
    // for (QVector<QPointF> *mfu_error_vect : errors.values())
    for (auto *mfu_error_vect : errors.values()) {

      // qDebug() << "Handling mfu_error " << errors.key(mfu_error_vect)
      //          << mfu_error_vect;
      QScatterSeries *mfu_error = new QScatterSeries(this->m_chart);
      mfu_error->replace(*mfu_error_vect);
      // qDebug() << "Here...";

      QString s_name = errors.key(mfu_error_vect);

      QImage star(30, 30, QImage::Format_ARGB32);
      star.fill(Qt::transparent);
      QPainter painter(&star);
      painter.setOpacity(1.0);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.setRenderHint(QPainter::Antialiasing, false);

      painter.setPen(QPen(Qt::red));
      mfu_error->setColor(Qt::red);

      if ((index <= 5) and (colors.size() > index))
        painter.setPen(colors.at(index));
      if (colors.size() > index)
        mfu_error->setColor(colors.at(index));

      if (m_colors_mfu_err.size() > index) {
        if (m_colors.value(m_colors_mfu_err.at(index)).isValid())
          painter.setPen(m_colors.value(m_colors_mfu_err.at(index)));
        mfu_error->setColor(m_colors.value(m_colors_mfu_err.at(index)));
      }

      painter.setFont(QFont("Arial", 20, QFont::Bold));
      painter.drawText(star.rect(), Qt::AlignCenter, "×");
      painter.end();
      mfu_error->setProperty("my_symbol", QVariant(QObject::tr("×")));
      QColor my_color = mfu_error->color();
      //                my_color.setAlpha (200);
      mfu_error->setProperty("my_color", QVariant(my_color.rgba()));
      mfu_error->setProperty("my_symbol_size", QVariant(20));
      mfu_error->setObjectName("mfu_error_" + QString::number(index + 1));

      mfu_error->setProperty("sprite", QVariant(star));
      mfu_error->setBrush(star);
      mfu_error->setPen(QColor(Qt::transparent));
      mfu_error->setMarkerSize(30.0);
      mfu_error->setUseOpenGL(true);
      qDebug() << "sent color:" << mfu_error->color();

      //            }

      mfu_error->setName(s_name + " (" +
                         QString::number(mfu_error->points().size()) + ")");
      mfu_error->setProperty("my_order",
                             QVariant(m_chart->series().size() + 1));
      m_chart->addSeries(mfu_error);
      mfu_error->attachAxis(axisX);

      if ((axis_temperature == nullptr) or temperature->points().count() < 2) {

        mfu_error->attachAxis(axis_current);
      } else {
        mfu_error->attachAxis(axis_temperature);
      }

      mfu_error->setProperty("my_type", QVariant(QObject::tr("error")));
      index++;
    }

    qDebug() << "current_active->points().count (): "
             << temperature->points().count()
             << "current_activ->points().count (): "
             << current_activ->points().count()
             << "OM->points().count (): " << OM->points().count() << "m_chart"
             << m_chart->axes();

    index = 0;
    for (QVector<QPointF> *dura_signal_vect : durability_errors.values()) {
      QScatterSeries *dura_signal = new QScatterSeries(this->m_chart);
      dura_signal->replace(*dura_signal_vect);

      QString s_name = durability_errors.key(dura_signal_vect);
      qDebug() << s_name << dura_signal->points().size();

      QImage star(30, 30, QImage::Format_ARGB32);
      star.fill(Qt::transparent);
      QPainter painter(&star);

      painter.setPen(QPen(Qt::red));
      if ((index <= 5) and (colors.size() > index))
        painter.setPen(colors.at(index));

      if (m_colors_mfu_err.size() > index) {
        if (m_colors.value(m_colors_mfu_err.at(index)).isValid())
          painter.setPen(m_colors.value(m_colors_mfu_err.at(index)));
      }

      painter.setFont(QFont("Arial", 20, QFont::Bold));
      painter.drawText(star.rect(), Qt::AlignCenter, "×");
      dura_signal->setColor(painter.pen().color());
      painter.end();

      dura_signal->setProperty("my_symbol", QVariant(QObject::tr("×")));
      dura_signal->setProperty("my_symbol_size", QVariant(QObject::tr("20")));
      dura_signal->setObjectName("dura_error_" + QString::number(index + 1));

      QColor my_color = dura_signal->color();
      //                my_color.setAlpha (200);
      dura_signal->setProperty("my_color", QVariant(my_color.rgba()));
      dura_signal->setProperty("my_symbol_size", QVariant(20));
      dura_signal->setProperty("sprite", QVariant(star));

      dura_signal->setBrush(star);
      dura_signal->setPen(QColor(Qt::transparent));
      dura_signal->setMarkerSize(30.0);
      dura_signal->setUseOpenGL(true);
      //            }

      dura_signal->setName(s_name + " (" +
                           QString::number(dura_signal->points().size()) + ")");
      dura_signal->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));
      m_chart->addSeries(dura_signal);
      dura_signal->attachAxis(axisX);

      //            dura_signal->attachAxis(axis_temperature);

      dura_signal->attachAxis(axis_dura_signals);

      dura_signal->setProperty("my_type", QVariant(QObject::tr("error")));
      index++;
    }

    for (QVector<QPointF> *dura_signal_vect : durability_signals.values()) {
      QLineSeries *dura_signal = new QLineSeries(this->m_chart);
      dura_signal->replace(*dura_signal_vect);

      QString s_name = durability_signals.key(dura_signal_vect);
      qDebug() << s_name << dura_signal->points().size();

      dura_signal->setPen(QPen(Qt::black, 1, Qt::SolidLine));
      dura_signal->setColor(Qt::black);

      dura_signal->setObjectName("dura_signal");

      dura_signal->setName(s_name + " (" +
                           QString::number(((dura_signal->points().size() / 2) /
                                            durability_signals.size())) +
                           ")");

      dura_signal->setProperty("my_order",
                               QVariant(m_chart->series().size() + 1));

      dura_signal->setUseOpenGL(true);
      m_chart->addSeries(dura_signal);

      dura_signal->attachAxis(axisX);
      dura_signal->attachAxis(axis_dura_signals);
      dura_signal->setProperty("my_type", QVariant(QObject::tr("signal")));
    }

    if (!durability_signals.isEmpty()) {
      axis_dura_signals->setMin(-5);
      axis_dura_signals->setMax(durability_signals.size() * 2);
      qDebug() << "dura signals size:" << (durability_signals.size());
      this->initialScale.insert(
          axis_dura_signals,
          QPointF(axis_dura_signals->min(), axis_dura_signals->max()));
    }

    qDeleteAll(AS_volts);
    qDeleteAll(durability_errors);
    qDeleteAll(durability_signals);
    qDeleteAll(AS_currents);
    qDeleteAll(errors);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setFont(
        QFont(QStringLiteral("Verdana"), 10, QFont::Bold));

    if (m_chart->legend()->markers().size() < 5)
      m_chart->legend()->setAlignment(Qt::AlignBottom);
    else
      m_chart->legend()->setAlignment(Qt::AlignRight);

    for (auto marker : m_chart->legend()->markers())
      if (marker->label() == "")
        marker->setVisible(false);

    m_chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    m_chart->legend()->update();

    //        m_chart->setAcceptHoverEvents(true);
    setRenderHint(QPainter::Antialiasing);
    scene()->addItem(m_chart);

    m_coordX = new QGraphicsSimpleTextItem(m_chart);
    m_coordX->setPos(m_chart->size().width() / 2 - 50,
                     m_chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(m_chart);
    m_coordY->setPos(m_chart->size().width() / 2 + 50,
                     m_chart->size().height());
    m_coordY->setText("Y: ");

    m_coordX->hide();
    m_coordY->hide();

    QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                     &View::handleTimeRangeChanged);
    QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
                     &View::handleTime2RangeChanged);

    for (auto ser : m_chart->series()) {
      auto m_ser = qobject_cast<QXYSeries *>(ser);
      QObject::connect(m_ser, &QXYSeries::clicked, this,
                       &View::handleSerieClick);
      QObject::connect(m_ser, &QXYSeries::released, this,
                       &View::handleSerieRelease);
    }
    //        handleSerieClick

    if (axis_opmode != nullptr)
      QObject::connect(this->axis_opmode, &QValueAxis::rangeChanged, this,
                       &View::handleOpMRangeChanged);

    emit this->handleTimeRangeChanged(axisX->min(), axisX->max());

    const auto markers = m_chart->legend()->markers();
    for (QLegendMarker *marker : markers) {
      // Disconnect possible existing connection to avoid multiple connections
      QObject::disconnect(marker, &QLegendMarker::clicked, this,
                          &View::handleMarkerClicked);
      QObject::connect(marker, &QLegendMarker::clicked, this,
                       &View::handleMarkerClicked);
    }

    // m_tooltip->hide();

    if (axisX2 != nullptr)
      axisX2->setVisible(!axisX2->isVisible());
    if (axisX != nullptr)
      axisX->setMin(0.0);
    if ((axisX != nullptr) and (axis_temperature != nullptr) and
        (axis_temperature->isVisible()) and
        (temperature->points().size() > 3)) {
      axisX->setMax(temperature->points().constLast().x());
    } else if ((axisX != nullptr) and (current_activ != nullptr) and
               (current_activ->isVisible()) and
               (current_activ->points().size() > 3)) {
      axisX->setMax(current_activ->points().constLast().x());
    }

    for (auto axis : m_chart->axes())
      adjustAxisRange(qobject_cast<QValueAxis *>(axis));

    //        applyNiceNumbers();

    for (auto axis : m_chart->axes())
      QObject::connect(qobject_cast<QValueAxis *>(axis),
                       &QValueAxis::rangeChanged, this,
                       &View::proc_rangeChanged);

    //        this->serieName = fileNameArg;

    //        m_chart->setAcceptHoverEvents(false);
  }

  this->good = true;
}

bool View::isIllum(QString path) {

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return false;
  }

  int fileSize = file.size();
  qDebug() << "Illum file size:" << fileSize;

  if (file.readLine().contains("Runtime [s];")) {
    qDebug() << "This is a Illum file!" << Qt::endl;

    this->serieName = path;

    QDataStream in(&file);

    QDir d = QFileInfo(path).absoluteDir();
    QString absolute = d.absolutePath();

    QString separator = ";";
    QString headerseparator;

    char ch;
    char carriage_return = '\r';
    char new_line = '\n';

    QString header, current_line;

    // get the header
    in.device()->seek(0);
    while (!in.atEnd()) {
      in.readRawData(&ch, 1);

      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (ch == new_line) {
          header.replace("\"", "");
          // qDebug() << "header:" << header << header.size() << Qt::endl;
          break;
        }
      } else {
        header += ch;
      }
    }

    QList<QString> headerlist = header.split(separator);
    header.clear();

    qDebug() << "headerlist:" << headerlist;

    QList<QString> unitslist = header.split(separator);
    header.clear();

    char word[256];
    int word_index = 0;
    int word_count = -1;

    bool skip_line = false;

    int res = 0;
    int curr_line = 0;

    double this_runningtime = 0.0;

    while (!file.atEnd()) {
      res = file.getChar(&ch);

      if (res < 1) {
        qDebug() << "File read error:" << res;
        //            QMessageBox::critical(this,  tr("Error:"), "File read
        //            error!");
        return -1;
      }

      if (ch ==
          '\0') // bullshit, probably memory leak in file during windows crash
      {
        word_index = 0;
        word_count = -1;
        skip_line = true;
        continue;
      }

      if (word_index > 255) // buffer overrun
      {
        qDebug() << "Buffer exceeded! clearing..." << curr_line;
        word_index = 0;
        word_count = -1;
        skip_line = true;
        continue;
      }

      if ((skip_line) and !(ch == new_line))
        continue;

      if ((ch == separator) or (ch == carriage_return)) {
        word_count += 1;

        if (word_index > 0) {
          word[word_index] = '\0';
          word_index = 0;

          // qDebug() << word_count << headerlist[word_count] << word;

          if (word_count == 0) {
            this_runningtime =
                static_cast<double>(fast_atof(word)) / (60 * 60 * 24);
          } else if (word_count > 2) {

            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            double dtemp = fast_atof(word);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, dtemp));
          }
        }
      } else {
        word[word_index] = ch;
        word_index += 1;
      }

      if (ch == new_line) {
        word_index = 0;
        word_count = -1;
        curr_line++;
        skip_line = false;
      }
    }

    qDebug() << "result container size:" << durability_errors.size();
    qDebug() << "first serie size:" << durability_errors.first()->size();

    file.close();

    if (this->durability_errors.size() > 0) {

      this->folder = path;

      m_chart = new QChart();
      m_chart->setParent(this);
      this->setRenderHint(QPainter::Antialiasing);

      m_chart->setTheme(QChart::ChartThemeBlueNcs);
      QFileInfo fi(serieName);
      m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

      m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
      m_chart->setAnimationOptions(QChart::NoAnimation);
      this->setRenderHint(QPainter::Antialiasing);
      m_chart->legend()->setVisible(true);
      m_chart->legend()->setAlignment(Qt::AlignRight);

      this->axis_temperature = new QValueAxis(m_chart);

      this->axisX = new QValueAxis(m_chart); // QValueAxis *axisX
      m_chart->addAxis(axisX, Qt::AlignBottom);
      axisX->setObjectName("axisX");
      axisX->setTickCount(11);
      //    axisX->setTitleText("Running Time");
      //    axisX->setTitleFont(QFont("Verdana", 14, QFont::Bold));
      axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axisX->setMinorTickCount(1);
      axisX->setLabelFormat("%4.1Td");
      axisX->setLabelsEditable(true);

      this->axisX2 = new QValueAxis(m_chart);
      axisX2->setObjectName("axisX2");
      axisX2->setTickCount(11);
      //    axisX2->setTitleText("chart time scale");
      //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
      axisX2->setMinorTickCount(3);
      axisX2->setLabelFormat("%4.1Td");
      axisX2->setTickAnchor(0);
      m_chart->addAxis(axisX2, Qt::AlignBottom);

      this->axis_voltage = new QValueAxis(m_chart);
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setObjectName("axis_voltage");
      axis_voltage->setLabelFormat("%4.1R "); // RV
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_voltage->setMinorTickCount(1);
      axis_voltage->setTickCount(11);
      axis_voltage->setLabelsEditable(true);
      m_chart->addAxis(axis_voltage, Qt::AlignLeft);
      axis_voltage->setVisible(false);

      this->axis_humidity = new QValueAxis(m_chart);

      QString s_name;

      qDebug() << durability_errors.keys();

      //            QString folder = QString(this->serieName).mid
      //            (0,QString(this->serieName).length () - 4);
      QString fname = QString(this->serieName).split('/').last();
      QString folder = QString(this->serieName).replace(fname, "");

      qDebug() << "folder" << folder;
      QFile txtfile(folder + "config.txt");
      QList<QString> txtfilenames;
      QMap<QString, QString> namesMap;
      if (txtfile.exists()) {
        if (!txtfile.open(QIODevice::ReadOnly)) {
          qDebug() << "txt File Open Failed!";
        } else {

          QTextStream txtin(&txtfile);
          while (!txtin.atEnd()) {
            txtfilenames.append(txtin.readLine());
          }
          qDebug() << txtfilenames;
          txtfile.close();
        }

        for (auto line : txtfilenames) {
          auto line_name = line.split(":");
          if (line_name.size() > 1) {
            auto new_name_l = line_name.at(1).split(QString("\""));

            if (new_name_l.size() >= 3) {
              auto new_name = new_name_l.at(1);
              auto old_name = line_name.first();

              for (auto key : durability_errors.keys()) {
                if (key.contains(old_name + " ")) {

                  auto new_name_complete = key;
                  new_name_complete.replace(old_name, new_name);
                  //                                    qDebug() << "atreplace:"
                  //                                    << old_name << new_name
                  //                                    << key <<
                  //                                    new_name_complete;
                  namesMap.insert(key, new_name_complete);
                }
              }
            }
          }
          line_name.clear();
        }
      }

      qDebug() << "namesMap" << namesMap;

      for (QVector<QPointF> *dura_signal_vect : durability_errors.values()) {

        QLineSeries *dura_signal = new QLineSeries(this->m_chart);

        dura_signal->replace(*dura_signal_vect);

        QString s_name = durability_errors.key(dura_signal_vect);

        dura_signal->setUseOpenGL(true);
        dura_signal->setPointsVisible(true);

        if (namesMap.contains(s_name)) {
          dura_signal->setName(namesMap.value(s_name) + " (" +
                               QString::number(dura_signal->points().size()) +
                               ")");
        } else {
          dura_signal->setName(s_name + " (" +
                               QString::number(dura_signal->points().size()) +
                               ")");
        }

        dura_signal->setProperty("my_order",
                                 QVariant(m_chart->series().size() + 1));

        m_chart->addSeries(dura_signal);
        dura_signal->attachAxis(axisX);

        qDebug() << "s_name" << s_name << s_name.contains(QString("°C"));

        dura_signal->setProperty("my_type", QVariant(QObject::tr("voltage")));
        dura_signal->attachAxis(axis_voltage);

        double min = dura_signal->points().first().y();
        double max = dura_signal->points().first().y();
        for (QPointF point : dura_signal->points()) {
          min = point.y() < min ? point.y() : min;
          max = point.y() > max ? point.y() : max;
        }

        if (axis_voltage->min() > min)
          axis_voltage->setMin(min);
        if (axis_voltage->max() < max)
          axis_voltage->setMax(max);
        axis_voltage->setVisible(true);
      }

      m_chart->legend()->setVisible(true);
      m_chart->legend()->setAlignment(Qt::AlignRight);
      m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
      m_chart->legend()->update();

      setRenderHint(QPainter::Antialiasing);
      scene()->addItem(m_chart);

      m_coordX = new QGraphicsSimpleTextItem(m_chart);
      m_coordX->setPos(m_chart->size().width() / 2 - 50,
                       m_chart->size().height());
      m_coordX->setText("X: ");
      m_coordY = new QGraphicsSimpleTextItem(m_chart);
      m_coordY->setPos(m_chart->size().width() / 2 + 50,
                       m_chart->size().height());
      m_coordY->setText("Y: ");

      QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                       &View::handleTimeRangeChanged);
      QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
                       &View::handleTimeRangeChanged);

      const auto markers = m_chart->legend()->markers();
      for (QLegendMarker *marker : markers) {
        // Disconnect possible existing connection to avoid multiple connections
        QObject::disconnect(marker, &QLegendMarker::clicked, this,
                            &View::handleMarkerClicked);

        QObject::connect(marker, &QLegendMarker::clicked, this,
                         &View::handleMarkerClicked);
      }

      for (auto axis : m_chart->axes())
        adjustAxisRange(qobject_cast<QValueAxis *>(axis));

      //        applyNiceNumbers();

      for (auto axis : m_chart->axes())
        QObject::connect(qobject_cast<QValueAxis *>(axis),
                         &QValueAxis::rangeChanged, this,
                         &View::proc_rangeChanged);
    }

    emit this->handleTimeRangeChanged(axisX->min(), axisX->max());

    this->axis_temperature->setLabelFormat(
        QString("%4.1f" + QString::fromLatin1("°C ")).toUtf8());

    this->setMouseTracking(true);
    this->good = true;

    return true;
  } else {
    return false;
  }
}

bool View::isGraphtec(QString path) {

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return false;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  if (file.readLine().contains("GRAPHTEC")) {
    qDebug() << "This is a GRAPHTEC file!" << Qt::endl;

    this->serieName = path;

    QDataStream in(&file);

    QDir d = QFileInfo(path).absoluteDir();
    QString absolute = d.absolutePath();

    QString separator = ",";
    QString headerseparator;

    char ch;
    char carriage_return = '\r';
    char new_line = '\n';

    QString header, current_line;

    double samplingInterval{0};

    // seek trough the file until the header begin flag is found
    for (int i = 0; i < 30; i++) {
      while (!in.atEnd()) {
        in.readRawData(&ch, 1);
        current_line += ch;
        if (ch == carriage_return) {
          in.readRawData(&ch, 1);
          if (ch == new_line)
            break;
        }
      }
      //            qDebug() << "current_line.indexOf (Data)" <<
      //            current_line.indexOf ("Data");

      if (current_line.contains("Sampling interval")) {

        QString s_samplingInterval = current_line.split(",").at(1);
        qDebug() << "s_samplingInterval" << s_samplingInterval;
        if (s_samplingInterval.contains("min")) {
          samplingInterval = s_samplingInterval.replace("min", "")
                                 .replace(carriage_return, "")
                                 .toDouble();
          samplingInterval = (samplingInterval / 60.0 / 24.0);
        } else if (s_samplingInterval.contains("ms")) {
          samplingInterval = s_samplingInterval.replace("ms", "")
                                 .replace(carriage_return, "")
                                 .toDouble();
          samplingInterval = (samplingInterval / 1000.0 / 60 / 60.0 / 24.0);
        } else if (s_samplingInterval.contains("s")) {
          s_samplingInterval =
              s_samplingInterval.replace("s", "").replace(carriage_return, "");
          qDebug() << "seconds" << s_samplingInterval
                   << s_samplingInterval.toDouble();
          samplingInterval = s_samplingInterval.toDouble();
          samplingInterval = (samplingInterval / 60 / 60.0 / 24.0);
          qDebug() << "double scaled" << samplingInterval;
        }
      }

      if (current_line.indexOf("Data") == 0)
        break;

      //        qDebug() << "current_line" << current_line;
      current_line.clear();
    }

    // get the header
    while (!in.atEnd()) {
      in.readRawData(&ch, 1);

      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (ch == new_line) {
          header.replace("\"", "");
          qDebug() << "header:" << header << header.size() << Qt::endl;
          break;
        }
      } else {
        header += ch;
      }
    }

    QList<QString> headerlist = header.split(separator);
    header.clear();

    while (!in.atEnd()) {
      in.readRawData(&ch, 1);

      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (ch == new_line) {
          header.replace("\"", "");
          header.replace("degC", "°C");
          qDebug() << "units header:" << header << header.size() << Qt::endl;
          break;
        }
      } else {
        header += ch;
      }
    }

    QList<QString> unitslist = header.split(separator);
    header.clear();

    qDebug() << "Header size:" << headerlist.size();
    qDebug() << "unitslist size:" << unitslist.size();

    for (int i = 0; i < unitslist.size(); i++) {
      headerlist.replace(i, headerlist.at(i) + " [" + unitslist.at(i) + "]");
    }

    qDebug() << "unitslist:" << unitslist;
    qDebug() << "final header:" << headerlist;
    unitslist.clear();

    if (samplingInterval == 0.0)
      samplingInterval = (1.0 / 3600.0 / 60.0 / 24.0);

    qDebug() << "samplingInterval:" << samplingInterval;

    char word[256];
    int word_index = 0;
    int word_count = -1;

    bool skip_line = false;

    int res = 0;
    int curr_line = 0;

    double this_runningtime = 0.0;

    while (!file.atEnd()) {
      res = file.getChar(&ch);

      if (res < 1) // file read error
      {
        qDebug() << "File read error:" << res;
        //            QMessageBox::critical(this,  tr("Error:"), "File read
        //            error!");
        return -1;
      }

      if (ch ==
          '\0') // bullshit, probably memory leak in file during windows crash
      {
        word_index = 0;
        word_count = -1;
        skip_line = true;
        continue;
      }

      if (word_index > 255) // buffer overrun
      {
        qDebug() << "Buffer exceeded! clearing..." << curr_line;
        word_index = 0;
        word_count = -1;
        skip_line = true;
        continue;
      }

      if ((skip_line) and !(ch == new_line))
        continue;

      if ((ch == separator) or (ch == carriage_return)) {
        word_count += 1;

        if (word_index > 0) {
          word[word_index] = '\0';
          word_index = 0;

          //                qDebug() << word_count << headerlist[word_count] <<
          //                word ; qDebug() << headerlist[word_count];

          //                if (word_count == 0) continue;

          if (word[0] == 'B')
            continue;
          if (word_count > 12)
            continue;
          if (word_count == 0)
            continue;
          if (word_count == 2)
            continue;

          if (word_count == 1) {
            this_runningtime += samplingInterval;
          } else { // if (word_count == 3)  (word_count > 8) and (word_count <
                   // 16)

            //                    if (this_runningtime > 0){

            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            double dtemp = fast_atof(word);

            //                        qDebug() << this_runningtime << dtemp <<
            //                        word;
            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, dtemp));
          }
        }
      } else {
        word[word_index] = ch;
        word_index += 1;
      }

      if (ch == new_line) {
        word_index = 0;
        word_count = -1;
        curr_line++;
        skip_line = false;
      }
    }

    qDebug() << "result container size:" << durability_errors.size();
    qDebug() << "first serie size:" << durability_errors.first()->size();

    //    qDebug() << durability_signals[headerlist[15]];

    //    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
    //    //    qDebug() << timestamp2 - timestamp1;
    file.close();

    if (this->durability_errors.size() > 0) {

      this->folder = path;

      m_chart = new QChart();
      m_chart->setParent(this);
      this->setRenderHint(QPainter::Antialiasing);

      m_chart->setTheme(QChart::ChartThemeBlueNcs);
      QFileInfo fi(serieName);
      m_chart->setTitle(fi.fileName().remove(fi.fileName().size() - 4, 4));

      m_chart->setTitleFont(QFont("Verdana", 16, QFont::Bold));
      m_chart->setAnimationOptions(QChart::NoAnimation);
      this->setRenderHint(QPainter::Antialiasing);
      m_chart->legend()->setVisible(true);
      m_chart->legend()->setAlignment(Qt::AlignRight);

      this->axis_temperature = new QValueAxis(m_chart);
      this->axis_temperature->setObjectName("axis_temperature");
      this->axis_temperature->setTickCount(11);
      //            this->axis_temperature->setLabelFormat(QString("%4.0f" +
      //            QString::fromLatin1("°C ")).toUtf8());
      this->axis_temperature->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      this->axis_temperature->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      this->axis_temperature->setMinorTickCount(1);
      this->axis_temperature->setLabelsEditable(true);
      m_chart->addAxis(this->axis_temperature, Qt::AlignLeft);
      this->axis_temperature->setVisible(false);

      //            auto dummyserie = new QLineSeries;
      //            m_chart->addSeries(dummyserie);
      //            dummyserie->attachAxis (this->axis_temperature);

      this->axisX = new QValueAxis(m_chart); // QValueAxis *axisX
      m_chart->addAxis(axisX, Qt::AlignBottom);
      axisX->setObjectName("axisX");
      axisX->setTickCount(11);
      //    axisX->setTitleText("Running Time");
      //    axisX->setTitleFont(QFont("Verdana", 14, QFont::Bold));
      axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axisX->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axisX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axisX->setMinorTickCount(1);
      axisX->setLabelFormat("%4.1Td");
      axisX->setLabelsEditable(true);

      this->axisX2 = new QValueAxis(m_chart);
      axisX2->setObjectName("axisX2");
      axisX2->setTickCount(11);
      //    axisX2->setTitleText("chart time scale");
      //    axisX2->setTitleFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLabelsFont(QFont("Verdana", 10, QFont::Bold));
      axisX2->setLinePen(QPen(Qt::gray, 1, Qt::SolidLine));
      axisX2->setMinorTickCount(3);
      axisX2->setLabelFormat("%4.1Td");
      axisX2->setTickAnchor(0);
      m_chart->addAxis(axisX2, Qt::AlignBottom);

      this->axis_voltage = new QValueAxis(m_chart);
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setObjectName("axis_voltage");
      axis_voltage->setLabelFormat("%4.1RV ");
      axis_voltage->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_voltage->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_voltage->setMinorTickCount(1);
      axis_voltage->setTickCount(11);
      axis_voltage->setLabelsEditable(true);
      m_chart->addAxis(axis_voltage, Qt::AlignRight);
      axis_voltage->setVisible(false);

      this->axis_humidity = new QValueAxis(m_chart);
      axis_humidity->setObjectName("axis_humidity");
      axis_humidity->setTickCount(11);
      axis_humidity->setLabelFormat(
          QString("%4.0f" + QString::fromLatin1("%rh ")).toUtf8());
      axis_humidity->setLabelsFont(QFont("Verdana", 12, QFont::Bold));
      axis_humidity->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
      axis_humidity->setMinorTickCount(1);
      axis_humidity->setLabelsEditable(true);
      m_chart->addAxis(axis_humidity, Qt::AlignRight);
      axis_humidity->setVisible(false);

      QString s_name;

      qDebug() << durability_errors.keys();

      //            QString folder = QString(this->serieName).mid
      //            (0,QString(this->serieName).length () - 4);
      QString fname = QString(this->serieName).split('/').last();
      QString folder = QString(this->serieName).replace(fname, "");

      qDebug() << "folder" << folder;
      QFile txtfile(folder + "config.txt");
      QList<QString> txtfilenames;
      QMap<QString, QString> namesMap;
      if (txtfile.exists()) {
        if (!txtfile.open(QIODevice::ReadOnly)) {
          qDebug() << "txt File Open Failed!";
        } else {

          QTextStream txtin(&txtfile);
          while (!txtin.atEnd()) {
            txtfilenames.append(txtin.readLine());
          }
          qDebug() << txtfilenames;
          txtfile.close();
        }

        for (auto line : txtfilenames) {
          auto line_name = line.split(":");
          if (line_name.size() > 1) {
            auto new_name_l = line_name.at(1).split(QString("\""));

            if (new_name_l.size() >= 3) {
              auto new_name = new_name_l.at(1);
              auto old_name = line_name.first();

              for (auto key : durability_errors.keys()) {
                if (key.contains(old_name + " ")) {

                  auto new_name_complete = key;
                  new_name_complete.replace(old_name, new_name);
                  //                                    qDebug() << "atreplace:"
                  //                                    << old_name << new_name
                  //                                    << key <<
                  //                                    new_name_complete;
                  namesMap.insert(key, new_name_complete);
                }
              }
            }
          }
          line_name.clear();
        }
      }

      qDebug() << "namesMap" << namesMap;

      for (QVector<QPointF> *dura_signal_vect : durability_errors.values()) {

        QLineSeries *dura_signal =
            new QLineSeries(this->m_chart); // QScatterSeries

        dura_signal->replace(*dura_signal_vect);

        QString s_name = durability_errors.key(dura_signal_vect);

        dura_signal->setUseOpenGL(true);
        dura_signal->setPointsVisible(true);
        //                dura_signal->setMarkerSize (6);
        //                dura_signal->setBorderColor ("transparent");

        if (namesMap.contains(s_name)) {
          dura_signal->setName(namesMap.value(s_name) + " (" +
                               QString::number(dura_signal->points().size()) +
                               ")");
        } else {
          dura_signal->setName(s_name + " (" +
                               QString::number(dura_signal->points().size()) +
                               ")");
        }

        dura_signal->setProperty("my_order",
                                 QVariant(m_chart->series().size() + 1));

        m_chart->addSeries(dura_signal);
        dura_signal->attachAxis(axisX);

        qDebug() << "s_name" << s_name << s_name.contains(QString("°C"));
        if (s_name.contains(QString("°C"))) {
          dura_signal->setProperty("my_type",
                                   QVariant(QObject::tr("temperature")));
          dura_signal->attachAxis(axis_temperature);

          double min = dura_signal->points().first().y();
          double max = dura_signal->points().first().y();
          for (QPointF point : dura_signal->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          if (axis_temperature->min() > min)
            axis_temperature->setMin(min);
          if (axis_temperature->max() < max)
            axis_temperature->setMax(max);
          axis_temperature->setVisible(true);
        } else if (s_name.contains("%")) {

          dura_signal->setProperty("my_type",
                                   QVariant(QObject::tr("humidity")));
          dura_signal->attachAxis(axis_humidity);

          double min = dura_signal->points().first().y();
          double max = dura_signal->points().first().y();
          for (QPointF point : dura_signal->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          if (axis_humidity->min() > min)
            axis_humidity->setMin(min);
          if (axis_humidity->max() < max)
            axis_humidity->setMax(max);
          axis_humidity->setVisible(true);
        } else {
          dura_signal->setProperty("my_type", QVariant(QObject::tr("voltage")));
          dura_signal->attachAxis(axis_voltage);

          double min = dura_signal->points().first().y();
          double max = dura_signal->points().first().y();
          for (QPointF point : dura_signal->points()) {
            min = point.y() < min ? point.y() : min;
            max = point.y() > max ? point.y() : max;
          }

          if (axis_voltage->min() > min)
            axis_voltage->setMin(min);
          if (axis_voltage->max() < max)
            axis_voltage->setMax(max);
          axis_voltage->setVisible(true);
        }
      }

      m_chart->legend()->setVisible(true);
      m_chart->legend()->setAlignment(Qt::AlignRight);
      m_chart->legend()->setFont(QFont("Verdana", 11, QFont::Bold));
      m_chart->legend()->update();

      setRenderHint(QPainter::Antialiasing);
      scene()->addItem(m_chart);

      m_coordX = new QGraphicsSimpleTextItem(m_chart);
      m_coordX->setPos(m_chart->size().width() / 2 - 50,
                       m_chart->size().height());
      m_coordX->setText("X: ");
      m_coordY = new QGraphicsSimpleTextItem(m_chart);
      m_coordY->setPos(m_chart->size().width() / 2 + 50,
                       m_chart->size().height());
      m_coordY->setText("Y: ");

      QObject::connect(this->axisX, &QValueAxis::rangeChanged, this,
                       &View::handleTimeRangeChanged);
      QObject::connect(this->axisX2, &QValueAxis::rangeChanged, this,
                       &View::handleTimeRangeChanged);

      const auto markers = m_chart->legend()->markers();
      for (QLegendMarker *marker : markers) {
        // Disconnect possible existing connection to avoid multiple connections
        QObject::disconnect(marker, &QLegendMarker::clicked, this,
                            &View::handleMarkerClicked);

        QObject::connect(marker, &QLegendMarker::clicked, this,
                         &View::handleMarkerClicked);
      }

      for (auto axis : m_chart->axes())
        adjustAxisRange(qobject_cast<QValueAxis *>(axis));

      //        applyNiceNumbers();

      for (auto axis : m_chart->axes())
        QObject::connect(qobject_cast<QValueAxis *>(axis),
                         &QValueAxis::rangeChanged, this,
                         &View::proc_rangeChanged);
    }

    emit this->handleTimeRangeChanged(axisX->min(), axisX->max());

    this->axis_temperature->setLabelFormat(
        QString("%4.1f" + QString::fromLatin1("°C ")).toUtf8());

    this->setMouseTracking(true);
    this->good = true;

    return true;
  } else {
    return false;
  }
}

void View::init_colors() {
  QFile m_colors_file(QCoreApplication::applicationDirPath() + "\\colors.txt");
  QList<QString> txtfilenames;

  if (m_colors_file.exists() and m_colors.size() == 0) {
    if (!m_colors_file.open(QIODevice::ReadOnly)) {
      qDebug() << "colors.txt File Open Failed!";
    } else {

      QTextStream txtin(&m_colors_file);

      QStringList line_color, line_color_rgb;
      int r, g, b;

      while (!txtin.atEnd()) {
        QString line = txtin.readLine();
        if (line.isEmpty())
          continue;
        line_color.clear();
        line_color = line.split(":");

        if (line_color.size() == 2) {
          line_color_rgb.clear();
          line_color_rgb = line_color.at(1).split(",");
        }

        if (line_color_rgb.size() == 3) {
          r = -1;
          g = -1;
          b = -1;
          r = line_color_rgb.at(0).toInt();
          g = line_color_rgb.at(1).toInt();
          b = line_color_rgb.at(2).toInt();

          if ((r <= 255 && !(r < 0)) and (g <= 255 && !(r < 0)) and
              (b <= 255 && !(r < 0))) {

            m_colors.insert(line_color.at(0), QColor(r, g, b));
            //                        qDebug() << line << line_color.at(0) <<
            //                        QColor(r, g, b);
          }
        }
      }
      m_colors_file.close();
    }
    //        qDebug() << "m_colors: " << this->m_colors;
  }
}

void View::adjustAxisRange(QValueAxis *axis) {
  if (axis == nullptr)
    return;
  // if (!axis->isVisible())
  //     return;
  // if (axis->orientation () == Qt::Horizontal)
  //     return;
  if (axis->objectName() == "axis_dura_signals")
    return;

  QString format = axis->labelFormat();

  // qDebug() << "initial:" << axis->objectName() << "axis format" << format
  //          << "min-max" << axis->min() << axis->max();

  // if (axis->min() < -10000000) {
  //   axis->setMin(-10000000);
  // }
  // if (axis->max() > 10000000) {
  //   axis->setMax(100000000);
  // }

  if (axis->min() == axis->max()) {
    axis->setMin(axis->min() - 1);
    axis->setMax(axis->min() + 1);
  }

  qDebug() << axis->objectName() << "axis format" << format << "min-max"
           << axis->min() << axis->max();

  if (format.contains("RA") or format.contains("RV")) {

    if (axis->max() < 0)
      ; // axis->setMax (1)
    else if (axis->max() < 0.00001)
      axis->setMax(0.00001);
    else if (axis->max() < 0.00001)
      axis->setMax(0.00001);
    else if (axis->max() < 0.00005)
      axis->setMax(0.00005);
    else if (axis->max() < 0.0001)
      axis->setMax(0.0001);
    else if (axis->max() < 0.0002)
      axis->setMax(0.0002);
    else if (axis->max() < 0.0003)
      axis->setMax(0.0003);
    else if (axis->max() < 0.0004)
      axis->setMax(0.0004);
    else if (axis->max() < 0.0005)
      axis->setMax(0.0005);
    else if (axis->max() < 0.0008)
      axis->setMax(0.0008);
    else if (axis->max() < 0.001)
      axis->setMax(0.001);
    else if (axis->max() < 0.01)
      axis->setMax(0.01);
    else if (axis->max() < 0.03)
      axis->setMax(0.03);
    else if (axis->max() < 0.05)
      axis->setMax(0.05);
    else if (axis->max() < 0.08)
      axis->setMax(0.08);
    else if (axis->max() < 0.1)
      axis->setMax(0.1);
    else if (axis->max() < 0.13)
      axis->setMax(0.13);
    else if (axis->max() < 0.15)
      axis->setMax(0.15);
    else if (axis->max() < 0.2)
      axis->setMax(0.2);
    else if (axis->max() < 0.3)
      axis->setMax(0.3);
    else if (axis->max() < 0.5)
      axis->setMax(0.5);
    else if (axis->max() < 0.8)
      axis->setMax(0.8);
    else if (axis->max() < 1)
      axis->setMax(1);
    else if (axis->max() < 1.3)
      axis->setMax(1.3);
    else if (axis->max() < 1.5)
      axis->setMax(1.5);
    else if (axis->max() < 2)
      axis->setMax(2);
    else if (axis->max() < 5)
      axis->setMax(5);
    else if (axis->max() < 10)
      axis->setMax(10);
    else if (axis->max() < 15)
      axis->setMax(15);
    else if (axis->max() < 20)
      axis->setMax(20);
    else if (axis->max() < 25)
      axis->setMax(25);
    else if (axis->max() < 30)
      axis->setMax(30);
    else if (axis->max() < 35)
      axis->setMax(35);
    else if (axis->max() < 40)
      axis->setMax(40);
    else if (axis->max() < 50)
      axis->setMax(50);
    else if (axis->max() < 60)
      axis->setMax(60);

    if (axis->min() < -35)
      ; // axis->setMin (0)
    else if ((axis->min() < -30))
      axis->setMin(-35);
    else if ((axis->min() < -20))
      axis->setMin(-30);
    else if ((axis->min() < -15))
      axis->setMin(-20);
    else if ((axis->min() < -10))
      axis->setMin(-15);
    else if ((axis->min() < -5))
      axis->setMin(-10);
    else if ((axis->min() < -1))
      axis->setMin(-5);
    else if ((axis->min() < -0.5))
      axis->setMin(-1);
    else if ((axis->min() < -0.1))
      axis->setMin(-0.5);
    else if ((axis->min() < -0.05))
      axis->setMin(-0.1);
    else if ((axis->min() < -0.01))
      axis->setMin(-0.05);
    else if ((axis->min() < -0.005))
      axis->setMin(-0.01);
    else if ((axis->min() < -0.001))
      axis->setMin(-0.005);
    else if ((axis->min() < -0.0005))
      axis->setMin(-0.001);
    else if ((axis->min() < 0))
      axis->setMin(-0.0001);
    else if ((axis->min() < 0.000005))
      axis->setMin(0);
    else if ((axis->min() < 0.00001))
      axis->setMin(0);
    else if ((axis->min() < 0.00003))
      axis->setMin(0.00001);
    else if ((axis->min() < 0.00005))
      axis->setMin(0.00003);
    else if ((axis->min() < 0.00008))
      axis->setMin(0.00005);
    else if ((axis->min() < 0.0001))
      axis->setMin(0.00008);
    else if ((axis->min() < 0.0002))
      axis->setMin(0.0001);
    else if ((axis->min() < 0.0003))
      axis->setMin(0.0002);
    else if ((axis->min() < 0.0005))
      axis->setMin(0.0003);
    else if ((axis->min() < 0.0008))
      axis->setMin(0.0005);
    else if ((axis->min() < 0.001))
      axis->setMin(0.0008);
    else if ((axis->min() < 0.002))
      axis->setMin(0.001);
    else if ((axis->min() < 0.003))
      axis->setMin(0.002);
    else if ((axis->min() < 0.005))
      axis->setMin(0.003);
    else if ((axis->min() < 0.01))
      axis->setMin(0.005);
    else if ((axis->min() < 0.02))
      axis->setMin(0.01);
    else if ((axis->min() < 0.03))
      axis->setMin(0.02);
    else if ((axis->min() < 0.05))
      axis->setMin(0.03);
    else if ((axis->min() < 0.08))
      axis->setMin(0.05);
    else if ((axis->min() < 0.1))
      axis->setMin(0.08);
    else if ((axis->min() < 0.15))
      axis->setMin(0.1);
    else if ((axis->min() < 0.5))
      axis->setMin(0.1);
    else if ((axis->min() < 1))
      axis->setMin(0);
    else if ((axis->min() < 5))
      axis->setMin(0);
    else if ((axis->min() < 10))
      axis->setMin(5);
    else if ((axis->min() < 15))
      axis->setMin(10);
    else if ((axis->min() < 20))
      axis->setMin(15);
    else if ((axis->min() < 25))
      axis->setMin(20);
    else if ((axis->min() < 30))
      axis->setMin(25);
    else if ((axis->min() < 35))
      axis->setMin(30);
    else if ((axis->min() < 40))
      axis->setMin(35);
    else if ((axis->min() < 45))
      axis->setMin(40);
    else if ((axis->min() < 50))
      axis->setMin(45);
    else if ((axis->min() < 55))
      axis->setMin(50);

    if (axis->min() == axis->max())
      axis->setMin(axis->min() - axis->min());

    qDebug() << "adjusted min-max" << axis->min() << axis->max();

    axis->setVisible(true);
  }

  else if (format.contains("%rh") or format.contains("C")) {

    if (axis->max() > 150)
      axis->setMax(axis->max() +
                   (axis->max() * 0.1)); // should not happen, do nothing;
    else if (axis->max() > 120)
      axis->setMax(150);
    else if (axis->max() > 100)
      axis->setMax(120);
    else if (axis->max() > 85)
      axis->setMax(100);
    else if (axis->max() > 60)
      axis->setMax(85);
    else if (axis->max() > 40)
      axis->setMax(60);
    else if (axis->max() > 20)
      axis->setMax(40);
    else if (axis->max() > 0)
      axis->setMax(20);
    else if (axis->max() > -10)
      axis->setMax(0);
    else if (axis->max() > -20)
      axis->setMax(-10);
    else if (axis->max() > -40)
      axis->setMax(-20);
    else if (axis->max() > -50)
      axis->setMax(-40);
    else if (axis->max() > -80)
      axis->setMax(-50);

    if (axis->min() < -80) {
      // qDebug() << "axis min set here:" << axis->min()
      //          << axis->min() + (axis->min() * 0.1);
      axis->setMin(axis->min() + (axis->min() * 0.1));
    } else if (axis->min() < -50)
      axis->setMin(-80);
    else if (axis->min() < -30)
      axis->setMin(-50);
    else if (axis->min() < -20)
      axis->setMin(-30);
    else if (axis->min() < -10)
      axis->setMin(-20);
    else if (axis->min() < 0)
      axis->setMin(-10);
    else if (axis->min() < 10)
      axis->setMin(0);
    else if (axis->min() < 20)
      axis->setMin(10);
    else if (axis->min() < 40)
      axis->setMin(20);
    else if (axis->min() < 60)
      axis->setMin(40);
    else if (axis->min() < 80)
      axis->setMin(60);
    else if (axis->min() < 100)
      axis->setMin(80);
    else if (axis->min() < 120)
      axis->setMin(100);
    else if (axis->min() < 150)
      axis->setMin(120);

    qDebug() << "adjusted min-max" << axis->min() << axis->max();
  } else if (axis->orientation() == Qt::Vertical) {
    // Axes with less-specific formatting

    if (axis->max() < 1) {
      axis->setMax(3); // Extend the maximum upwards
    } else if (axis->max() < 10) {
      axis->setMax(axis->max() + 2); // Add a margin above
    } else {
      // For larger values, add a 10% margin
      axis->setMax(axis->max() * 1.1);
    }

    if (axis->min() > 0) {
      axis->setMin(-1); // Extend minimum downwards
    } else if (axis->min() > -10) {
      axis->setMin(axis->min() - 2); // Add a margin below
    } else {
      // For smaller values, add a 10% margin
      axis->setMin(axis->min() * 0.9);
    }
  }

  if (axis->orientation() == Qt::Vertical) {
    axis->applyNiceNumbers();
    int desiredTickCount = std::max(1, (int)abs(axis->max() - axis->min()) + 1);
    int maxAllowedTicks = 11;
    desiredTickCount = std::min(desiredTickCount, maxAllowedTicks);
    axis->setTickCount(desiredTickCount);
    axis->setVisible(true);
  }
  this->initialScale.insert(axis, QPointF(axis->min(), axis->max()));

  return;
}

void View::actionOpmEdit() {
  bool ok;
  if (axis_opmode == nullptr)
    return;

  auto current_format = axis_opmode->labelFormat().split("|");
  QString new_format = current_format.at(0);

  QString OpM_C =
      QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM C name:"),
                            QLineEdit::Normal, current_format.at(1), &ok);
  if (ok && !OpM_C.isEmpty())
    new_format += "|" + OpM_C;
  else
    new_format += "|" + current_format.at(1);

  QFileInfo fi(fileNameArg);
  QString OpM_D =
      QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM D name:"),
                            QLineEdit::Normal, current_format.at(2), &ok);
  if (ok && !OpM_D.isEmpty())
    new_format += "|" + OpM_D;
  else
    new_format += "|" + current_format.at(2);

  QString OpM_E =
      QInputDialog::getText(this, tr("Change OpM Name"), tr("OpM E name:"),
                            QLineEdit::Normal, current_format.at(3), &ok);
  if (ok && !OpM_E.isEmpty())
    new_format += "|" + OpM_E;
  else
    new_format += "|" + current_format.at(3);

  axis_opmode->setLabelFormat(new_format.toUtf8());

  emit propertyChanged();
}

void View::actionTitleEdit() {
  bool ok;
  QFileInfo fi(fileNameArg);
  QString text =
      QInputDialog::getText(this, tr("Change chart title"), tr("Chart title:"),
                            QLineEdit::Normal, m_chart->title(), &ok);
  if (ok && !text.isEmpty())
    m_chart->setTitle(text);
}

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

void View::tooltip_ex(QPointF point, bool state, QColor color,
                      QString caption) {
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

void View::applyNiceNumbers() {
  // if ((axis_current != nullptr) and (axis_current->isVisible()))
  //     axis_current->applyNiceNumbers();
  // if ((axis_sleep_current != nullptr) and (axis_sleep_current->isVisible()))
  //     axis_sleep_current->applyNiceNumbers();
  // if ((axis_temperature != nullptr) and (axis_temperature->isVisible()))
  //     axis_temperature->applyNiceNumbers();
  // if ((axis_humidity != nullptr) and (axis_humidity->isVisible()))
  //     axis_humidity->applyNiceNumbers();
  // if ((axis_voltage != nullptr) and (axis_voltage->isVisible()))
  //     axis_voltage->applyNiceNumbers();

  // if (axis_travel != nullptr)
  //     axis_travel->applyNiceNumbers();
  // if (axis_force != nullptr)
  //     axis_force->applyNiceNumbers();

  // for (auto ax : m_chart->axes())
  // {
  //     QValueAxis *axis = qobject_cast<QValueAxis *>(ax);
  //     if (axis->objectName() == "axis_HV_voltage")
  //         axis->applyNiceNumbers();
  //     if (axis->objectName() == "axis_HV_current")
  //         axis->applyNiceNumbers();
  // }

  for (QAbstractAxis *axis : m_chart->axes()) {
    if (axis->orientation() == Qt::Vertical) {
      auto valueAxis = qobject_cast<QValueAxis *>(axis);
      // if (valueAxis->isVisible())
      valueAxis->applyNiceNumbers();
    }
  }

  updateCallouts();
  this->m_chart->update();
  QApplication::processEvents();
}

void View::tenTicks() {
  for (auto abstract_axis : m_chart->axes()) {
    QValueAxis *axis = qobject_cast<QValueAxis *>(abstract_axis);
    axis->setTickCount(11);
  }
  this->m_chart->update();
  QApplication::processEvents();
}

void View::setScaleAxis(bool state) {
  if (axisX2 != nullptr) {
    axisX2->setVisible(state);
  }
  this->m_chart->update();
  QApplication::processEvents();
}

void View::exportAction() {

  if (this->is_durability) {
    exportDurability();
    return;
  }

  if (this->is_robot) {
    exportRobot();
    return;
  }

  bool ok;
  QFileInfo fi(serieName);
  QString text = QInputDialog::getText(this, tr("Export current view"),
                                       tr("Picture name:"), QLineEdit::Normal,
                                       fi.baseName(), &ok);
  if (ok && !text.isEmpty())
    savePNG(fi.absolutePath() + "/" + text);
}

void View::updateCallouts() {

  for (auto serie : m_chart->series()) {
    //        qDebug() << intPoints << serie->name();
    if (serie->isVisible())
      for (Callout *callout : intPoints[serie->name()])
        callout->updateGeometry();
  }
}

void View::cutGraph(double start, double stop) {
  if (!cutMode)
    return;

  if (stop > 0) {

    QPointF temp_point;
    QVector<QPointF> new_vector;
    QVector<QPointF> m_points_vector;

    qDebug() << "stop" << stop;

    if ((temperature != nullptr) and temperature->points().size() > 5) {

      if (stop > temperature->points().last().x())
        stop = temperature->points().last().x();
    } else if ((OM != nullptr) and (OM->points().size() > 1)) {

      if (stop > OM->points().last().x())
        stop = OM->points().last().x();
    }

    qDebug() << "stop" << stop;

    for (auto m_serie : m_chart->series()) {

      auto m_serie_xy = qobject_cast<QXYSeries *>(m_serie);
      m_points_vector = m_serie_xy->points();

      for (auto m_point : m_points_vector) {
        if (m_point.x() < 0)
          continue;
        if (m_point.x() > start) {

          if (m_point.x() > stop) {
            temp_point = m_point;
            temp_point.setX(m_point.x() - (stop - start));
            new_vector.append(temp_point);
          }
        } else {
          new_vector.append(m_point);
        }
      }

      m_serie_xy->replace(new_vector);
      new_vector.clear();
      m_points_vector.clear();

      if (m_serie_xy->property("my_type") == "error") {
        if (m_serie_xy->points().size() == 0) {
          m_chart->removeSeries(m_serie_xy);
          m_serie_xy->deleteLater();
        } else {
          m_serie_xy->setName(m_serie_xy->name().section('(', 0, 0) + "(" +
                              QString::number(m_serie_xy->points().size()) +
                              ")");
        }
      }
    }

    // if (axisX != nullptr)
    // {
    //     auto init_X_Max = this->initialScale.value(axisX).y();
    //     this->initialScale.insert(axisX, QPointF(0, init_X_Max - (stop -
    //     start)));
    // }

    // if (axis_travel != nullptr)
    // {
    //     auto init_X_Max = this->initialScale.value(axis_travel).y();
    //     this->initialScale.insert(axis_travel, QPointF(0, init_X_Max -
    //     (stop - start)));
    // }

    for (auto axis : m_chart->axes(Qt::Horizontal)) {
      QValueAxis *valueAxis = qobject_cast<QValueAxis *>(axis);
      if (valueAxis->objectName() == "axisX2")
        continue;
      auto init_X_Max = this->initialScale.value(valueAxis).y();
      this->initialScale.insert(valueAxis,
                                QPointF(0, init_X_Max - (stop - start)));
    }

    new_vector.clear();
    m_points_vector.clear();
    new_vector.squeeze();
    m_points_vector.squeeze();

    m_chart->update();

    updateCallouts();

    View::m_chart->update();
    View::update();
    QApplication::processEvents();
  }
}

void showStatistics(const QVector<QPointF> *serie, QString name) {
  if (!serie || serie->isEmpty())
    return;

  // Calculate statistics
  double average = 0, min = serie->at(0).y(), max = serie->at(0).y(), sum = 0;
  for (const QPointF &point : *serie) {
    double y = point.y();
    min = std::min(min, y);
    max = std::max(max, y);
    sum += y;
  }
  average = sum / serie->size();

  double variance = 0;
  for (const QPointF &point : *serie) {
    double diff = point.y() - average;
    variance += diff * diff;
  }
  variance /= serie->size();
  double stddev = std::sqrt(variance);

  // Copy statistics to clipboard
  QString statsString =
      QString("Min: %1\nMax: %2\nAverage: %3\nStandard Deviation: %4")
          .arg(min)
          .arg(max)
          .arg(average)
          .arg(stddev);
  QGuiApplication::clipboard()->setText(statsString);

  // Setup Dialog
  QDialog dialog;
  dialog.setWindowTitle(name + " - Statistics & Distribution");
  dialog.resize(1024, 768); // Set initial dialog size
  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  // Customize font for statistics text
  QFont statsFont;
  statsFont.setBold(true);
  statsFont.setPointSize(12);

  // Text for statistics with customized font
  QLabel *statsLabel = new QLabel(statsString);
  statsLabel->setFont(statsFont);
  layout->addWidget(statsLabel);

  // Setup chart
  QChart *chart = new QChart();
  QBarSeries *series = new QBarSeries();

  // Histogram calculation with percentage
  int numBins = 10;
  QVector<double> bins(numBins, 0);
  double binWidth = (max - min) / numBins;
  int totalPoints = serie->size();

  for (const QPointF &point : *serie) {
    int binIndex =
        std::min(static_cast<int>((point.y() - min) / binWidth), numBins - 1);
    bins[binIndex]++;
  }

  QBarSet *set = new QBarSet("Distribution");
  for (double bin : bins) {
    double percent = (bin / totalPoints) * 100;
    *set << percent;
  }
  series->append(set);

  chart->addSeries(series);
  chart->createDefaultAxes();
  chart->legend()->hide();

  // Customize axes
  QFont labelFont;
  labelFont.setBold(true);
  labelFont.setPointSize(12);

  QBarCategoryAxis *categoryAxis = new QBarCategoryAxis();
  for (int i = 0; i < numBins; ++i) {
    categoryAxis->append(QString("[%1, %2)")
                             .arg(min + i * binWidth)
                             .arg(min + (i + 1) * binWidth));
  }
  categoryAxis->setTitleText("Bin Range");
  categoryAxis->setTitleFont(labelFont);
  chart->addAxis(categoryAxis, Qt::AlignBottom);
  series->attachAxis(categoryAxis);

  QValueAxis *axisY = new QValueAxis();
  axisY->setRange(0, 100);
  axisY->setTitleText("Percentage (%)");
  axisY->setTitleFont(labelFont);
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  QChartView *chartView = new QChartView(chart);
  chartView->setMinimumSize(640, 480);
  layout->addWidget(chartView);

  dialog.exec();
}

void View::handleMarkerClicked() {
  // qDebug() << "handleMarkerClicked";

  this->userInteracted = true;
  QLegendMarker *marker = qobject_cast<QLegendMarker *>(sender());
  Q_ASSERT(marker);

  //    qDebug() << "marker clicked!";

  switch (marker->type()) {
  case QLegendMarker::LegendMarkerTypeXY: {

    if (mid_clicked) {
      auto serie = qobject_cast<QXYSeries *>(marker->series())->points();
      showStatistics(&serie, marker->series()->name());
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

      // QString unit = "";
      // QString sname = marker->series()->name();
      // if ((sname.contains("[A]")) or (sname.contains("Curr")) or
      //     (sname.contains("curr")) or (sname.contains("ström")) or
      //     (sname.contains("Strom")) or (sname.contains("strom")) or
      //     (sname.contains("Ström")))
      //   unit = "A";

      // if ((sname.contains("[V]")) or (sname.contains("Volt")) or
      //     (sname.contains("volt")) or (sname.contains("spann")) or
      //     (sname.contains("Spann")))
      //   unit = "V";

      // if ((sname.contains("[°C]")) or (sname.contains("Tempe")) or
      //     (sname.contains("tempe")) or (sname.contains("[C]")))
      //   unit = "°C";

      // if ((sname.contains("[%rh]")) or (sname.contains("Humid")) or
      //     (sname.contains("humid")) or (sname.contains("Feuch")) or
      //     (sname.contains("feuch")) or (sname.contains("[%]")))
      //   unit = "%rh";

      // QString text = "Measurements: " + QString::number(serie.size()) +
      //                "\nMin: " + engineering_Format(min) + unit +
      //                "\nMax: " + engineering_Format(max) + unit +
      //                "\nAvg: " + engineering_Format(average) + unit +
      //                "\nStdDev: " + QString::number(stddev);

      // //            qDebug() << text;

      // QMessageBox::information(this, sname + tr(" statistics:"), text);
      // QClipboard *clipboard = QApplication::clipboard();
      // clipboard->setText(marker->series()->name() + tr(" statistics:\n") +
      //                    text);
      break;
    } else if (rmb_clicked) {

      QString sname = qobject_cast<QXYSeries *>(marker->series())->name();
      QColor scolor = qobject_cast<QXYSeries *>(marker->series())->color();

      if (qobject_cast<QXYSeries *>(marker->series())->name() ==
          "Operating Mode")
        return;

      if (qobject_cast<QXYSeries *>(marker->series())->brush().style() ==
          Qt::TexturePattern) {
        auto size = qobject_cast<QXYSeries *>(marker->series())
                        ->brush()
                        .textureImage()
                        .size();
        auto image =
            qobject_cast<QXYSeries *>(marker->series())->brush().textureImage();
        bool found = false;
        for (int x = 0; x < size.width(); x++) {
          for (int y = 0; y < size.height(); y++) {
            if (image.pixelColor(x, y).alpha() == 255) {
              // qDebug() << "tried to get color" << image.pixelColor (x, y) <<
              // image.pixelColor (x, y).alpha ();
              scolor = image.pixelColor(x, y);
              found = true;
              break;
            }
          }
          if (found)
            break;
        }
      }

      QColor color =
          QColorDialog::getColor(scolor, this, "Select " + sname + " Color");

      qDebug() << sname;

      if (color.isValid()) {
        if (QXYSeries *lineseries =
                qobject_cast<QXYSeries *>(marker->series())) {
          if (lineseries->brush().style() == Qt::TexturePattern) // error
          {

            bool ok;
            QStringList items;
            items << tr("×") << tr("#") << tr("§") << tr("*") << tr("☼")
                  << tr("↑") << tr("↓") << tr("↨") << tr("¤") << tr("Θ")
                  << tr("√") << tr("•") << tr("♦") << tr("■") << tr("▲")
                  << tr("▼");

            QInputDialog symbolInput;
            symbolInput.setParent(nullptr);
            symbolInput.setWindowTitle(sname); // sname + tr(" error symbol")
            symbolInput.setLabelText(tr("Symbol:")); // Choose/insert symbol:
            symbolInput.setInputMode(QInputDialog::TextInput);
            symbolInput.setComboBoxItems(items);
            symbolInput.setFont(QFont("Arial", 18, QFont::Bold));
            symbolInput.setComboBoxEditable(true);
            ok = symbolInput.exec();
            QString text = symbolInput.textValue();
            if (ok && !text.isEmpty())
              text = text.at(0);
            else
              text = "x";

            int size = QInputDialog::getInt(this, tr("Error Symbol size"),
                                            sname + " error symbol size:", 20,
                                            4, 60, 1, &ok);
            if (ok)
              ;
            else
              size = 20;

            const auto mytest = text;
            auto font = QFont("Arial", size, QFont::Bold);
            auto fm = QFontMetrics(font);
            auto rectangle = fm.boundingRect(text);
            auto w = fm.horizontalAdvance(text);
            auto h = rectangle.height();
            qDebug() << rectangle;

            auto s = w > h ? w : h;

            QImage star(s, s, QImage::Format_ARGB32);
            star.fill(Qt::transparent);
            QPainter painter(&star);
            painter.setPen(color);
            painter.setFont(font);
            painter.drawText(star.rect(), Qt::AlignCenter, text);
            painter.end();

            lineseries->setBrush(star);
            lineseries->setPen(QColor(Qt::transparent));

            lineseries->setProperty("my_symbol", QVariant(text));
            lineseries->setProperty("my_symbol_size", QVariant(size));
            lineseries->setProperty("sprite", QVariant(star));

            QColor my_color = color;
            my_color.setAlpha(200);
            lineseries->setProperty("my_color", QVariant(my_color.rgba()));

            static_cast<QScatterSeries *>(lineseries)->setMarkerSize(s);
          } else // other
          {
            if (color != QColor(0, 0, 0)) {
              lineseries->setColor(color);
              lineseries->setBrush(color);
            } else {
              lineseries->setColor(QColor(65, 65, 65));
              lineseries->setBrush(QColor(65, 65, 65));
            }
          }

          /* replaced with universal solution below
          //also color the axes if applies
          if (lineseries->name () == "Temperature")
          {
              this->axis_temperature->setLinePenColor (color);
              this->axis_temperature->setLabelsColor (color);
              //               qobject_cast<QLineSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Current active")
          {
              this->axis_current->setLinePenColor (color);
              this->axis_current->setLabelsColor (color);
              //                    qobject_cast<QScatterSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Current sleep")
          {
              this->axis_sleep_current->setLinePenColor (color);
              this->axis_sleep_current->setLabelsColor (color);
              //                        qobject_cast<QScatterSeries *>(sender
          ())->setUseOpenGL (false);
          }
          else if (lineseries->name () == "Humidity")
          {
              this->axis_humidity->setLinePenColor (color);
              this->axis_humidity->setLabelsColor (color);
          }
          else if (lineseries->name () == "Voltage")
          {
              this->axis_voltage->setLinePenColor (color);
              this->axis_voltage->setLabelsColor (color);
          }
          */

          for (auto attachedaxis : lineseries->attachedAxes()) {
            if (attachedaxis->alignment() != Qt::AlignBottom) {
              attachedaxis->setLinePenColor(color);
              attachedaxis->setLabelsColor(color);
            }
          }
        }
        m_chart->legend()->update();
      }
      emit propertyChanged();
      break;
    }

    // Toggle visibility of series
    marker->series()->setVisible(!marker->series()->isVisible());
    if (marker->label().contains("(")) {
      emit markerToggled(marker->label().section('(', 0, 0));
    } else {
      emit markerToggled(marker->label());
    }

    // Turn legend marker back to visible, since hiding series also hides the
    // marker and we don't want it to happen now.
    marker->setVisible(true);

    // Dim the marker, if series is not visible
    qreal alpha = 1.0;

    // qDebug() << "serie name:" << marker->series()->name ();
    if (!marker->series()->isVisible()) // hidden
    {
      // dim the marker label
      alpha = 0.5;

      // hide the callouts
      for (Callout *callout : intPoints[marker->series()->name()]) {
        if (!callout->supressed)
          callout->hide();
      }

      //--> hide the corresponding normal MFU axis
      if (marker->series()->name() == "Operating Mode")
        axis_opmode->setVisible(false);
      else if (marker->series()->name() == "Current sleep")
        axis_sleep_current->setVisible(false);
      else if (marker->series()->name() == "Current active")
        axis_current->setVisible(false);
      else if (marker->series()->name() == "Humidity")
        axis_humidity->setVisible(false);
      else if (marker->series()->name() == "Temperature")
        axis_temperature->setVisible(false);
      //<--
      else {
        //-->if this is the last MFU AS serie on axis, hide the axis
        bool voltage_visible = false;
        bool current_visible = false;
        for (auto my_marker : m_chart->legend()->markers()) {
          if (my_marker->series()->property("my_type") == "current")
            if (my_marker->series()->isVisible())
              current_visible = true;

          if (my_marker->series()->property("my_type") == "voltage")
            if (my_marker->series()->isVisible())
              voltage_visible = true;
        }
        if (axis_current != nullptr)
          axis_current->setVisible(current_visible);
        if (axis_voltage != nullptr)
          axis_voltage->setVisible(voltage_visible);
        //<--
      }
    } else // visible
    {
      // show callouts
      for (Callout *callout : intPoints[marker->series()->name()]) {
        if (!callout->supressed)
          callout->show();
      }

      //--> show the corresponding normal MFU axis
      if (marker->series()->name() == "Operating Mode")
        axis_opmode->setVisible(true);
      else if (marker->series()->name() == "Current sleep")
        axis_sleep_current->setVisible(true);
      else if (marker->series()->name() == "Current active")
        axis_current->setVisible(true);
      else if (marker->series()->name() == "Humidity")
        axis_humidity->setVisible(true);
      else if (marker->series()->name() == "Temperature")
        axis_temperature->setVisible(true);
      //<--
      else {
        //--> if any MFU AS serie is visible, show it's corresponding axis
        bool voltage_visible = false;
        bool current_visible = false;
        for (auto my_marker : m_chart->legend()->markers()) {

          if (my_marker->series()->property("my_type") == "current")
            if (my_marker->series()->isVisible())
              current_visible = true;

          if (my_marker->series()->property("my_type") == "voltage")
            if (my_marker->series()->isVisible())
              voltage_visible = true;
        }
        if (axis_current != nullptr)
          axis_current->setVisible(current_visible);
        if (axis_voltage != nullptr)
          axis_voltage->setVisible(voltage_visible);
        //<--
      }
    }

    updateCallouts();

    QColor color;
    QBrush brush = marker->labelBrush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setLabelBrush(brush);

    brush = marker->brush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setBrush(brush);

    QPen pen = marker->pen();
    color = pen.color();
    color.setAlphaF(alpha);
    pen.setColor(color);
    marker->setPen(pen);

    // Start new logic for axis hide/show
    QAbstractSeries *series = marker->series();
    QList<QAbstractAxis *> axes = series->attachedAxes();
    QList<QAbstractSeries *> seriesList = m_chart->series();
    QValueAxis *yAxis = nullptr;
    for (QAbstractAxis *axis : axes) {
      if (axis->orientation() == Qt::Vertical) {
        yAxis = qobject_cast<QValueAxis *>(axis);
        break;
      }
    }

    if (yAxis != nullptr) {
      bool hasVisibleSeries = false;
      for (QAbstractSeries *otherSeries : seriesList) {
        if (otherSeries == series)
          continue;
        if (!otherSeries->isVisible())
          continue;

        QList<QAbstractAxis *> otherAxes = otherSeries->attachedAxes();
        if (otherAxes.contains(yAxis)) {
          hasVisibleSeries = true;
          break;
        }
      }
      yAxis->setVisible(hasVisibleSeries || series->isVisible());
    }
    // End new logic for axis hide/show

    View::update();
    break;
  }
  default: {
    qDebug() << "Unknown marker type";
    break;
  }
  }
}

void View::toggleLegendMarker(QString name, bool state) {
  qDebug() << "TGM";
  QLegendMarker *marker = nullptr;

  for (auto m_marker : m_chart->legend()->markers()) {
    if (m_marker->label().contains(name)) {
      marker = m_marker;
      break;
    }
  }

  if (marker == nullptr)
    return;

  if (marker->series()->isVisible() == state)
    return;

  // Toggle visibility of series
  marker->series()->setVisible(!marker->series()->isVisible());

  // Turn legend marker back to visible, since hiding series also hides the
  // marker and we don't want it to happen now.
  marker->setVisible(true);

  // Dim the marker, if series is not visible
  qreal alpha = 1.0;

  // qDebug() << "serie name:" << marker->series()->name ();
  if (!marker->series()->isVisible()) // hidden
  {
    // dim the marker label
    alpha = 0.5;

    // hide the callouts
    for (Callout *callout : intPoints[marker->series()->name()]) {
      callout->hide();
    }

    //--> hide the corresponding normal MFU axis
    if (marker->series()->name() == "Operating Mode")
      axis_opmode->setVisible(false);
    else if (marker->series()->name() == "Current sleep")
      axis_sleep_current->setVisible(false);
    else if (marker->series()->name() == "Current active")
      axis_current->setVisible(false);
    else if (marker->series()->name() == "Humidity")
      axis_humidity->setVisible(false);
    else if (marker->series()->name() == "Temperature")
      axis_temperature->setVisible(false);
    //<--
    else {
      //-->if this is the last MFU AS serie on axis, hide the axis
      bool voltage_visible = false;
      bool current_visible = false;
      for (auto my_marker : m_chart->legend()->markers()) {
        if (my_marker->series()->property("my_type") == "current")
          if (my_marker->series()->isVisible())
            current_visible = true;

        if (my_marker->series()->property("my_type") == "voltage")
          if (my_marker->series()->isVisible())
            voltage_visible = true;
      }
      if (axis_current != nullptr)
        axis_current->setVisible(current_visible);
      if (axis_voltage != nullptr)
        axis_voltage->setVisible(voltage_visible);
      //<--
    }
  } else // visible
  {
    // show callouts
    for (Callout *callout : intPoints[marker->series()->name()]) {
      callout->show();
    }

    //--> show the corresponding normal MFU axis
    if (marker->series()->name() == "Operating Mode")
      axis_opmode->setVisible(true);
    else if (marker->series()->name() == "Current sleep")
      axis_sleep_current->setVisible(true);
    else if (marker->series()->name() == "Current active")
      axis_current->setVisible(true);
    else if (marker->series()->name() == "Humidity")
      axis_humidity->setVisible(true);
    else if (marker->series()->name() == "Temperature")
      axis_temperature->setVisible(true);
    //<--
    else {
      //--> if any MFU AS serie is visible, show it's corresponding axis
      bool voltage_visible = false;
      bool current_visible = false;
      for (auto my_marker : m_chart->legend()->markers()) {

        if (my_marker->series()->property("my_type") == "current")
          if (my_marker->series()->isVisible())
            current_visible = true;

        if (my_marker->series()->property("my_type") == "voltage")
          if (my_marker->series()->isVisible())
            voltage_visible = true;
      }
      if (axis_current != nullptr)
        axis_current->setVisible(current_visible);
      if (axis_voltage != nullptr)
        axis_voltage->setVisible(voltage_visible);
      //<--
    }
  }

  updateCallouts();

  QColor color;
  QBrush brush = marker->labelBrush();
  color = brush.color();
  color.setAlphaF(alpha);
  brush.setColor(color);
  marker->setLabelBrush(brush);

  brush = marker->brush();
  color = brush.color();
  color.setAlphaF(alpha);
  brush.setColor(color);
  marker->setBrush(brush);

  QPen pen = marker->pen();
  color = pen.color();
  color.setAlphaF(alpha);
  pen.setColor(color);
  marker->setPen(pen);

  View::update();
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

void View::handleMarkerToggle(QLegendMarker *marker) {
  //    QLegendMarker* marker = qobject_cast<QLegendMarker*> (sender());
  //    Q_ASSERT(marker);

  switch (marker->type())

  {
  case QLegendMarker::LegendMarkerTypeXY: {

    // Toggle visibility of series
    marker->series()->setVisible(!marker->series()->isVisible());

    // Turn legend marker back to visible, since hiding series also hides the
    // marker and we don't want it to happen now.
    marker->setVisible(true);

    // Dim the marker, if series is not visible
    qreal alpha = 1.0;

    if (!marker->series()->isVisible()) {

      alpha = 0.5;

      for (Callout *callout : intPoints[marker->series()->name()]) {
        callout->hide();
      }

      if (marker->series()->name() == "Operating Mode")
        axis_opmode->setVisible(false);
      else if (marker->series()->name() == "Current sleep")
        axis_sleep_current->setVisible(false);
      else if (marker->series()->name() == "Current active")
        axis_current->setVisible(false);
      else if (marker->series()->name() == "Humidity")
        axis_humidity->setVisible(false);
      else if (marker->series()->name() == "Temperature")
        axis_temperature->setVisible(false);
    } else {
      for (Callout *callout : intPoints[marker->series()->name()]) {
        callout->show();
      }

      if (marker->series()->name() == "Operating Mode")
        axis_opmode->setVisible(true);
      else if (marker->series()->name() == "Current sleep")
        axis_sleep_current->setVisible(true);
      else if (marker->series()->name() == "Current active")
        axis_current->setVisible(true);
      else if (marker->series()->name() == "Humidity")
        axis_humidity->setVisible(true);
      else if (marker->series()->name() == "Temperature")
        axis_temperature->setVisible(true);
    }

    for (auto serie : m_chart->series()) {
      if (serie->isVisible())
        for (Callout *callout : intPoints[serie->name()])
          callout->updateGeometry();
    }

    QColor color;
    QBrush brush = marker->labelBrush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setLabelBrush(brush);

    brush = marker->brush();
    color = brush.color();
    color.setAlphaF(alpha);
    brush.setColor(color);
    marker->setBrush(brush);

    QPen pen = marker->pen();
    color = pen.color();
    color.setAlphaF(alpha);
    pen.setColor(color);
    marker->setPen(pen);
    View::update();
    break;
  }
  default: {
    qDebug() << "Unknown marker type";
    break;
  }
  }
}

void View::savePNG(QString name) {

  for (auto legend_item : m_chart->legend()->markers()) {
    if (!legend_item->series()->isVisible()) {
      legend_item->setVisible(false);
    }
  }

  m_chart->legend()->layout()->invalidate();
  this->update();
  QApplication::processEvents();

  QPixmap p = this->grab(
      m_chart->rect().toAlignedRect().marginsRemoved(m_chart->margins() + 10));
  QOpenGLWidget *glWidget = View::findChild<QOpenGLWidget *>();
  if (glWidget) {
    QPainter painter(&p);
    //                    QPoint d = glWidget->mapToGlobal(QPoint()) -
    //                    View::mapToGlobal(QPoint());
    //                    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    //                    painter.drawImage(d, glWidget->grabFramebuffer());

    painter.end();
  }
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setPixmap(p);

  qDebug() << name;
  p.save(name + "_" + QString::number(exportNr) + ".png", "PNG");
  exportNr += 1;

  for (auto legend_item : m_chart->legend()->markers()) {
    legend_item->setVisible(true);
  }

  m_chart->legend()->layout()->invalidate();
  this->update();
  QApplication::processEvents();
}

double View::pow10(int n) {
  double ret = 1.0;
  double r = 10.0;

  // Handle negative exponent by taking reciprocal
  if (n < 0) {
    n = -n;
    r = 0.1;
  }

  // Exponentiation by squaring
  while (n) {
    if (n & 1) // If least significant bit is set
    {
      ret *= r;
    }
    r *= r;  // Square the base
    n >>= 1; // Halve the exponent
  }
  return ret;
}

double View::niceTickRange(double range, int tickCount) {
  double unroundedTickSize = range / (tickCount - 1);
  double x = ceil(log10(unroundedTickSize) - 1);
  double pow10x = pow10(x);
  double roundedTickRange = ceil(unroundedTickSize / pow10x) * pow10x;
  return roundedTickRange;
}

int View::translateOM(QString om) {
  // qDebug() << "Operating mode:" << om;

  if (om == u"")
    return -33;

  if (om.contains(u"IIa") or om.contains(u"II.a"))
    return 1;
  if (om.contains(u"IIb") or om.contains(u"II.b"))
    return 2;
  if (om.contains(u"IIc") or om.contains(u"II.c"))
    return 3;

  if (om.contains(u"H"))
    return 1;

  if (om.contains(u"lla") or om.contains(u"ll.a"))
    return 1;
  if (om.contains(u"llb") or om.contains(u"ll.b"))
    return 2;
  if (om.contains(u"llc") or om.contains(u"ll.c"))
    return 3;

  if (om.contains(u"11a") or om.contains(u"11.a"))
    return 1;
  if (om.contains(u"11b") or om.contains(u"11.b"))
    return 2;
  if (om.contains(u"11c") or om.contains(u"11.c"))
    return 3;

  if (om.contains(u"IVa1") or om.contains(u"IVa0"))
    return 1;
  if (om.contains(u"IVc0"))
    return 2;
  if (om.contains(u"IVc1"))
    return 3;

  if (om.contains(u"OPmin", Qt::CaseInsensitive))
    return 1;
  if (om.contains(u"OPmax", Qt::CaseInsensitive))
    return 3;

  if (om.contains(u"2,1") or om.contains(u"2.1"))
    return 1;
  if (om.contains(u"2,2") or om.contains(u"2.2"))
    return 2;
  if (om.contains(u"2,3") or om.contains(u"2.3"))
    return 3;

  if (om.contains(u"2,4") or om.contains(u"2.4"))
    return 3; // florea
  if (om.contains(u"2,3") or om.contains(u"2.5"))
    return 2;

  if (om.contains(u"2h"))
    return 3;
  if (om.contains(u"1b"))
    return 1;

  if (om.contains(u"2.4"))
    return 3;

  if (om.contains(u"OM3A", Qt::CaseInsensitive))
    return 1;
  if (om.contains(u"OM3B", Qt::CaseInsensitive))
    return 2;
  if (om.contains(u"OM3C", Qt::CaseInsensitive))
    return 3;

  if (om.contains(u"2A", Qt::CaseInsensitive))
    return 1;
  if (om.contains(u"2B", Qt::CaseInsensitive))
    return 3;
  if (om.contains(u"3A", Qt::CaseInsensitive))
    return 1;
  if (om.contains(u"3B", Qt::CaseInsensitive))
    return 3;

  if (om.contains(u"U_Nominal", Qt::CaseInsensitive))
    return 3;

  if (om.contains(u"Ua", Qt::CaseInsensitive))
    return 3;
  if (om.contains(u"OFF", Qt::CaseInsensitive))
    return 1;

  //    if (om.contains("1a") ) return -1;
  if (om.contains(u"2a"))
    return 1;
  if (om.contains(u"2b"))
    return 2;
  if (om.contains(u"2c"))
    return 3;

  if (om.contains("lab_1") or om.contains("lab1"))
    return 1;
  if (om.contains("lab_2") or om.contains("lab2"))
    return 2;
  if (om.contains("lab_3") or om.contains("lab3"))
    return 3;

  if (om.contains("iia") or om.contains("ii.a"))
    return 1;
  if (om.contains("iib") or om.contains("ii.b"))
    return 2;
  if (om.contains("iic") or om.contains("ii.c"))
    return 3;

  if (om.contains("II1") or om.contains("II.1"))
    return 1;
  if (om.contains("II2") or om.contains("II.2"))
    return 2;
  if (om.contains("II3") or om.contains("II.3"))
    return 3;

  if (om.contains("2,a") or om.contains("2.a") or om.contains("2a"))
    return 1;
  if (om.contains("2,b") or om.contains("2.b") or om.contains("2b"))
    return 2;
  if (om.contains("2,c") or om.contains("2.c") or om.contains("2c")) {

    return 3;
  }
  // cata
  if (om.contains("3,1") or om.contains("3.1"))
    return 1;
  if (om.contains("3,2") or om.contains("3.2"))
    return 2;
  if (om.contains("3,3") or om.contains("3.3"))
    return 3;

  if (om.contains("3,2") or om.contains("3.2"))
    return 3;

  if (om.contains("3,1") or om.contains("3.1"))
    return 4;
  if (om.contains("3,2") or om.contains("3.2"))
    return 5;
  if (om.contains("3,3") or om.contains("3.3"))
    return 6;

  if (om.contains("1,1") or om.contains("1.1"))
    return -3;
  if (om.contains("1,2") or om.contains("1.2"))
    return 1;
  if (om.contains("1,3") or om.contains("1.3"))
    return -1;

  if (om.contains(u"N-Op"))
    return 1;
  //    if (om.contains("Ib") or om.contains("I.b")) return 1;
  if (om.contains(u"Op"))
    return 3;

  if (om.contains("Ia") or om.contains("I.a"))
    return -3;
  if (om.contains("Ib") or om.contains("I.b"))
    return 1;
  if (om.contains("Ic") or om.contains("I.c"))
    return -1;

  if (om.contains(u"D2"))
    return -33;

  if (om.contains(u"C") or om.contains(u"c"))
    return 1;
  if (om.contains(u"D") or om.contains(u"d"))
    return 2;
  if (om.contains(u"E") or om.contains(u"e"))
    return 3;

  if (om.contains("1") or om.contains("2"))
    return 3;
  if (om.contains("3") or om.contains("4"))
    return 3;
  if (om.contains("5") or om.contains("6"))
    return 3;
  if (om.contains("7") or om.contains("8"))
    return 3;
  if (om.contains("9") or om.contains("10"))
    return 3;

  return -33;
}

double View::fast_atof(const char *num) // implement not-ok flag!!
{
  if (!num || !*num) {
    return -333333333.33;
  }

  int sign = 1;
  double integerPart = 0.0;
  double fractionPart = 0.0;
  bool hasFraction = false;
  bool hasExpo = false;
  bool hasDigit = false;

  // Take care of +/- sign, skip leading white space
  while (*num == ' ') {
    ++num;
  }

  if (*num == '-') {
    ++num;
    sign = -1;
  } else if (*num == '+') {
    ++num;
  }

  while (*num != '\0')

  {
    //        qDebug() << "*num:" << *num << (*num == ' ');
    if (*num >= '0' && *num <= '9') {
      integerPart = integerPart * 10 + (*num - '0');
      hasDigit = true;
    } else if (*num == '.') {
      hasFraction = true;
      ++num;
      break;
    } else if (*num == ',') {
      hasFraction = true;
      ++num;
      break;
    } else if (*num == 'E') {
      hasExpo = true;
      ++num;
      break;
    } else if (*num == 'e') {
      hasExpo = true;
      ++num;
      break;
    } else if (*num == ' ') {
      // skip white spaces
    } else {
      if (!hasDigit) {
        //               qDebug() <<  num;
        return -333333333.33;
      }
      return sign * integerPart;
    }
    ++num;
  }

  if (hasFraction) {
    double fractionExpo = 0.1;

    while (*num != '\0') {
      if (*num >= '0' && *num <= '9') {
        fractionPart += fractionExpo * (*num - '0');
        fractionExpo *= 0.1;
      } else if (*num == 'E') {
        hasExpo = true;
        ++num;
        break;
      } else if (*num == 'e') {
        hasExpo = true;
        ++num;
        break;
      } else {
        return sign * (integerPart + fractionPart);
      }
      ++num;
    }
  }

  // parsing exponet part
  double expPart = 1.0;
  if (*num != '\0' && hasExpo) {
    int expSign = 1;
    if (*num == '-') {
      expSign = -1;
      ++num;
    } else if (*num == '+') {
      ++num;
    }

    int e = 0;
    while (*num != '\0' && *num >= '0' && *num <= '9') {
      e = e * 10 + *num - '0';
      ++num;
    }

    expPart = pow10(expSign * e);
  }

  return sign * (integerPart + fractionPart) * expPart;
}

int View::loadCSV_opt(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  QDataStream in(&file);

  //    uint8_t fileBOM;
  //    in.readRawData(reinterpret_cast<char*>(&fileBOM), 1);
  //    if ((fileBOM != 239) and (fileBOM != 68) and (fileBOM != 42) and
  //    (fileBOM != 80))
  //    {
  //        qDebug() << "Invalid file encoding!" << fileBOM;
  //        return -2;
  //    }
  //    in.readRawData(reinterpret_cast<char*>(&fileBOM), 1);
  //    if ((fileBOM != 187) and (fileBOM != 97) and (fileBOM != 42) and
  //    (fileBOM != 114))
  //    {
  //        qDebug() << "Invalid file encoding!" << fileBOM;
  //        return -3;
  //    }
  //    in.readRawData(reinterpret_cast<char*>(&fileBOM), 1);
  //    if ((fileBOM != 191)and (fileBOM != 116) and (fileBOM != 42) and
  //    (fileBOM != 252))
  //    {
  //        qDebug() << "Invalid file encoding!" << fileBOM;
  //        return -4;
  //    }
  //    qDebug() << "file encoding: UTF8 => correct!" << Qt::endl;

  //    qDebug() << "path" << path;

  QDir d = QFileInfo(path).absoluteDir();
  QString absolute = d.absolutePath();

  QString separator;
  QString headerseparator;

  char ch;

  //    char sep = ',';
  char carriage_return = '\r';
  char new_line = '\n';
  //    bool header_has_comma = false, header_has_dot = false,
  //    header_has_semicolon = false; bool firstl_has_comma = false,
  //    firstl_has_dot = false, firstl_has_semicolon = false;
  bool is_ccsp = false, is_ccsp2 = false, is_ccsp_service = false,
       is_tsc = false, is_tsc_service = false, is_mfu_as = false,
       is_mfu = false, is_mfu_ld = false, is_mfu_as_ld = false,
       is_mfu_old = false, is_mfu_ld_old = false, is_mfu_ld_err = false,
       is_picoammeter = false, is_key_special = false, is_neww_MFU = false,
       is_ld_ioni_special = false, is_vechitura_mircea = false;
  QString header, first_line;

  while (!in.atEnd()) // get the header
  {
    in.readRawData(&ch, 1);

    if (ch == carriage_return) {
      in.readRawData(&ch, 1);
      if (ch == new_line) {
        //            QMessageBox::information(this,  tr("header:"), '|' +
        //            header + '|');
        break;
      }
    } else {
      header += ch;
      //            if (ch == ',') header_has_comma = true;
      //            if (ch == '.') header_has_dot = true;
      //            if (ch == ';') header_has_semicolon = true;
    }
  }
  qDebug() << "header:" << header << Qt::endl;

  if (header.contains("Crad_SP"))
    is_tsc_service = true;

  //    else if (header.contains("fplatz-Nr.: \t\t")) is_picoammeter = true;
  else if (header.contains(
               "********************************************************"))
    is_picoammeter = true;

  else if (header.contains(
               "*****************************************************"))
    is_ld_ioni_special = true;

  else if (header.contains("cpcAV_HP"))
    is_ccsp_service = true;

  else if (header.contains("CV4_SP"))
    is_tsc = true;

  else if (header.contains("CV2_SP") and header.contains("Running time"))
    is_ccsp2 = true;

  else if (header.contains("CV2_SP"))
    is_ccsp = true;

  else if (header.contains("AV_Temp"))
    is_ccsp = true;

  else if (header.contains("current deviation [A]") and
           header.contains("current max [A]"))
    is_key_special = true;

  else if (header.contains("Laufzeit") and header.contains("Min. Strom"))
    is_neww_MFU = true;

  else if (header.contains("Running time") and header.contains("min. current"))
    is_neww_MFU = true;

  else if (header.contains("Voltage [V]") and
           (header.contains("TestStep-No.")) and header.contains("Tolerance"))
    return -1;

  else if (header.contains("Voltage [V]") and (header.contains("TestStep-No.")))
    is_mfu_ld = true;

  else if (header.contains("Testschritt-Nr.") and
           (header.contains("Testschritt-Name")))
    is_mfu_ld = true;

  else if (header.contains("Voltage [V]") and (header.contains("TestStep")))
    is_mfu_ld_old = true;

  else if ((header.contains("Voltage [V]") or
            header.contains("Spannung [V]")) and
           (!header.contains("Humidity") and !header.contains("Feuchtigkeit")))
    is_mfu_old = true;

  else if ((header.contains("Voltage [V]") or
            header.contains("Spannung [V]")) and
           (header.contains("Humidity") or header.contains("Feuchtigkeit")))
    is_mfu = true;

  else if (header.contains(u"Power supply 2 [V]") or
           header.contains(QStringLiteral("NetzgerÃ¤t 2[V]")) or
           header.contains(QStringLiteral("Netzgerät 2[V]")))
    is_mfu_as = true;

  else if (header.contains(u"PowerSupply 2 U [V]") and
           (header.contains(u"TestStep-No.")))
    is_mfu_as_ld = true;

  else if (header.contains(QStringLiteral("Netzgerät 2[V]")) or
           (header.contains(QStringLiteral("Netzgerät 2[V]")) and
            (header.contains(u"Testschritt-Nr."))))
    is_vechitura_mircea = true;

  else if (header.contains(QStringLiteral("Kommentar")))
    is_vechitura_mircea = true;

  else {

    qDebug() << "This log type is not implemented";

    //        QMessageBox::information(nullptr, QString::fromUtf8("Not
    //        implemented"),
    //                                 QString::fromUtf8("This log type is not
    //                                 implemented.\n"
    //                                                   "If you need to analyze
    //                                                   this log type, please
    //                                                   save it in the tool's
    //                                                   directory and fill the
    //                                                   feedback form."));
    return -1;
  }

  volatile int temp_index = 0, hum_index = 0, temp_sv_index = 0,
               hum_sv_index = 0, om_index = 0, curr_index = 0,
               voltage_index = 0; // id_index = 0,
  int volt1_index = 0, volt2_index = 0, volt3_index = 0, volt4_index = 0,
      volt5_index = 0, volt6_index = 0;
  int curr1_index = 0, curr2_index = 0, curr3_index = 0, curr4_index = 0;
  int can_io_err_index = 0;
  int picoammeter_time = 0;
  int KL30C_index = 0;

  if (header.contains(u"Strom KL30 [A]"))
    KL30C_index = 14;

  if (is_picoammeter) {
    temp_index = 3;
    hum_index = 5;
    curr_index = 11;
    picoammeter_time = 1;
  } else if (is_ccsp) {
    temp_sv_index = 1;
    temp_index = 2;

    hum_sv_index = 3;
    hum_index = 4;
  } else if (is_ccsp2) {
    temp_sv_index = 2;
    temp_index = 3;

    hum_sv_index = 4;
    hum_index = 5;
  } else if (is_ccsp_service) {
    temp_sv_index = 1;
    temp_index = 2;
    hum_sv_index = 3;
    hum_index = 4;
  } else if (is_tsc) {
    temp_sv_index = 3;
    temp_index = 4;
  } else if (is_tsc_service) {
    temp_sv_index = 3;
    temp_index = 4;
  } else if (is_mfu_old) {
    temp_index = 3;
    hum_index = 0;
    om_index = 6;
    curr_index = 9;
    can_io_err_index = 10;
    voltage_index = 4;
  } else if (is_mfu) {
    temp_index = 3;
    hum_index = 4;
    om_index = 7;
    curr_index = 10;
    can_io_err_index = 11;
    voltage_index = 5;
  } else if (is_neww_MFU) {
    temp_index = 3;
    hum_index = 4;
    om_index = 7;
    curr_index = 10;
    can_io_err_index = 18;
    voltage_index = 5;
  } else if (is_key_special) {
    temp_index = 3;
    hum_index = 4;
    om_index = 7;
    curr_index = 10;
    can_io_err_index = 0;
    voltage_index = 5;
  } else if (is_mfu_as) {
    temp_index = 3;
    hum_index = 4;
    om_index = 8;
    voltage_index = 5;

    curr1_index = 11;
    curr2_index = 12;
    curr3_index = 13;
    curr4_index = 14;

    volt1_index = 15;
    volt2_index = 16;
    volt3_index = 17;
    volt4_index = 18;
    volt5_index = 19;
    volt6_index = 20;
  } else if (is_mfu_ld) {
    temp_index = 4;
    voltage_index = 5;
  } else if (is_mfu_ld_old) {
    temp_index = 4;
    can_io_err_index = 10;
    voltage_index = 5;
  } else if (is_mfu_ld_err) {
    temp_index = 4;
    can_io_err_index = 11;
    is_mfu_ld_old = true;
    voltage_index = 5;
  } else if (is_mfu_as_ld) {
    temp_index = 4;
    voltage_index = 5;

    curr1_index = 23;
    curr2_index = 25;
    curr3_index = 27;
    curr4_index = 29;

    volt1_index = 11;
    volt2_index = 13;
    volt3_index = 15;
    volt4_index = 17;
    volt5_index = 19;
    volt6_index = 21;
  } else if (is_ld_ioni_special) {
    temp_index = 2;
  }

  else if (is_vechitura_mircea) {
    temp_index = 2;
    //        can_io_err_index = 4;
    //        voltage_index = 5;
    om_index = 3;
    curr_index = 6;
  }

  if (is_ccsp)
    qDebug() << "is_ccsp" << is_ccsp << Qt::endl;
  if (is_ccsp2)
    qDebug() << "is_ccsp2" << is_ccsp2 << Qt::endl;
  if (is_ccsp_service)
    qDebug() << "is_ccsp_service" << is_ccsp_service << Qt::endl;
  if (is_tsc)
    qDebug() << "is_ccsp" << is_tsc << Qt::endl;
  if (is_mfu_ld)
    qDebug() << "is_mfu_ld" << is_mfu_ld << Qt::endl;
  if (is_mfu)
    qDebug() << "is_mfu" << is_mfu << Qt::endl;
  if (is_mfu_old)
    qDebug() << "is_mfu_old" << is_mfu_old << Qt::endl;
  if (is_mfu_as)
    qDebug() << "is_mfu_as" << is_mfu_as << Qt::endl;
  if (is_mfu_as_ld)
    qDebug() << "is_mfu_as_ld" << is_mfu_as_ld << Qt::endl;
  if (is_mfu_ld_old)
    qDebug() << "is_mfu_ld_old" << is_mfu_ld_old << Qt::endl;
  if (is_picoammeter)
    qDebug() << "is_picoammeter" << is_picoammeter << Qt::endl;
  if (is_key_special)
    qDebug() << "is_key_special" << is_key_special << Qt::endl;
  if (is_neww_MFU)
    qDebug() << "is_neww_MFU" << is_neww_MFU << Qt::endl;
  if (is_ld_ioni_special)
    qDebug() << "is_ld_ioni_special" << is_ld_ioni_special << Qt::endl;

  // exit(0);

  // while (!in.atEnd()) { // skip one line, sometimes it contains a wrong time
  //   in.readRawData(&ch, 1);
  //   if (ch == carriage_return) {
  //     in.readRawData(&ch, 1);
  //     if (ch == new_line)
  //       break;
  //   }
  // }

  while (!in.atEnd()) // get first line
  {
    in.readRawData(&ch, 1);

    if (ch == carriage_return) {
      in.readRawData(&ch, 1);
      if (ch == new_line) {
        //            QMessageBox::information(this,  tr("first line:"), '|' +
        //            first_line + '|');
        break;
      }
    } else {
      first_line += ch;
      //            if (ch == ',') firstl_has_comma = true;
      //            if (ch == '.') firstl_has_dot = true;
      //            if (ch == ';') firstl_has_semicolon = true;
    }
  }

  bool double_separator_header = false;
  int initial_header_size = 0;
  int final_header_size = 0;

  if ((header.contains("CV1_SP")) and (first_line.contains(";"))) {
    separator = ";";
    headerseparator = ";";
    //        sep = '.';
  } else if (header.contains(
                 "*****************************************************")) {
    separator = ";";
    headerseparator = ";";
  } else if ((header.contains("SP_Temp")) and (header.contains(";")) and
             (first_line.contains(";")) and first_line.contains(".")) {
    separator = ";";
    headerseparator = ";";
  }

  else if ((header.contains(";")) and (header.contains(",")) and
           (first_line.contains(";"))) {
    separator = ";";
    headerseparator = ",";
    double_separator_header = true;
    //        sep = '.';
  }

  else if ((header.contains(";")) and (first_line.contains(";"))) {
    separator = ";";
    headerseparator = ";";
    //        sep = ',';
  } else if ((header.contains(",")) and (first_line.contains(";"))) {
    separator = ";";
    headerseparator = ",";
    //        sep = '.';
  } else if ((header.contains(";")) and (header.contains(",")) and
             (first_line.contains(","))) {
    separator = ",";
    headerseparator = ";";
    double_separator_header = true;
    //        sep = '.';
  } else if ((header.contains(",")) and (first_line.contains(","))) {
    separator = ",";
    headerseparator = ",";
    //        sep = '.';
  } else if (header.contains(QChar(0x09))) // Explicitly convert int to QChar
  {
    separator = QChar(0x09);       // Explicitly convert int to QChar
    headerseparator = QChar(0x09); // Explicitly convert int to QChar
  } else if (is_ld_ioni_special) {
    separator = ";";
  } else {

    qDebug() << "Unknown separator!";
    QMessageBox::information(this, tr("Error:"), "Unknown separator!");
    return -5;
  }

  double time_offset =
      0; //= fast_atof(first_line.split(separator).at(0).toUtf8());

  QStringList parts = first_line.trimmed().split(separator);
  if (!parts.isEmpty())
    double time_offset = fast_atof(parts.at(0).toUtf8());

  qDebug() << qSetRealNumberPrecision(15) << "time offset:" << time_offset
           << Qt::endl
           << Qt::endl;

  QList<QString> headerlist = header.split(headerseparator);
  initial_header_size = headerlist.size();
  qDebug() << "Header size:" << initial_header_size
           << "separator:" << separator;

  if (double_separator_header) {
    qDebug() << "Double separator header!";
    auto oldheaderlist = headerlist;
    headerlist.clear();
    for (auto partial : oldheaderlist) {
      if (partial.contains(",")) {
        headerlist.append(partial.split(","));
      }
      if (partial.contains(";")) {
        headerlist.append(partial.split(";"));
      } else {
        headerlist.append(partial);
      }
    }
    final_header_size = headerlist.size();
    qDebug() << "Final header size:" << final_header_size;
  }

  if (is_picoammeter) {
    while (!in.atEnd()) // get first line
    {
      in.readRawData(&ch, 1);

      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (first_line.contains("Uhrzeit"))
          break;
        if (ch == new_line) {
          //            QMessageBox::information(this,  tr("first line:"), '|' +
          //            first_line + '|');
          first_line.clear();
        }
      } else {
        first_line += ch;
      }
    }
  }

  char word[64];
  char old_word[64];
  int word_index = 0;
  //    int ret = 0;
  int word_count = -1;

  double this_runningtime = 0, this_temperature = -333333333.333,
         this_humid = 0, this_om = 0, this_current = 0;
  //    double this_volt1 = 0, this_volt2 = 0, this_volt3 = 0, this_volt4 = 0,
  //    this_volt5 = 0, this_volt6 = 0; double this_curr1 = 0, this_curr2 = 0,
  //    this_curr3 = 0, this_curr4 = 0; double tsc_runningtime = 0;
  const double one_min = (1.0 / 60 / 24);
  const double five_sec = ((5.1 / 60 + 5.0) / 60 / 60 / 24); //

  bool skip_line = false;
  // bool skip_word = false;

  int res = 0;
  int curr_line = 0;
  //    qDebug() << "temp_index" << temp_index;

  //    while (!in.atEnd ()){
  //    res = in.readRawData(&ch, 1);

  OM1_name = "";
  OM2_name = "";
  OM3_name = "";
  QString s_this_om = ""; // char s_this_om[8]
  QString s_this_ld_err = "";

  QList<int> ld_err_count_list;
  if (is_mfu_ld)
    for (int e = 0; e < headerlist.size(); e++)
      ld_err_count_list << 0;

  volatile double old_voltage = 0.0;

  // double old_voltage =
  //     fast_atof(first_line.split(separator).at(voltage_index).toUtf8());

  parts = first_line.trimmed().split(separator);
  if (voltage_index >= 0 && voltage_index < parts.size())
    old_voltage = fast_atof(parts[voltage_index].toUtf8());
  // old_voltage = fast_atof(parts.at(voltage_index).toUtf8());

  //
  double old_om = -1.0;
  double old_time = 0;

  QString fileContents;
  QFile csvFile;

  fileContents.append("Daily Current Average;\n");

  //        for (int k = 0 ; k < durability_errors.size (); k++){
  //            fileContents.append (durability_errors.keys ().at(k) + ";");
  //        }

  double actualday = 1;
  double actualhours = 0;
  double actualsamplecount = 0;
  double actualaccumulation = 0;

  bool otherError;
  // QList<bool> indexShouldBeGap;

  while (!file.atEnd()) {
    res = file.getChar(&ch);

    if (res < 1) // file read error
    {
      qDebug() << "File read error:" << res;
      // QMessageBox::critical(this,  tr("Error:"), "File read error!");
      return -1;
    }

    if (ch ==
        '\0') // bullshit, probably memory leak in file during windows crash
    {
      word_index = 0;
      word_count = -1;
      skip_line = true;
      continue;
    }

    if (word_index > 63) // buffer overrun
    {
      //            qDebug() << "Buffer exceeded! clearing..." << curr_line;
      word_index = 0;
      word_count = -1;
      skip_line = true;
      continue;
    }

    if ((skip_line) and !(ch == new_line))
      continue;

    if ((ch == separator) or (ch == carriage_return)) {
      word_count += 1;
      // qDebug() << word;

      //                    qDebug() << "sep:" << separator << word_count;

      if (word_index > 0) {
        word[word_index] = '\0';
        word_index = 0;
        //                QMessageBox::information(this,  tr("Deb:"), word);

        if (word_count == 0) {

          if (is_picoammeter or is_vechitura_mircea) {
            continue;
          } else if (is_tsc or is_ccsp or is_ccsp2) {
            this_runningtime += one_min;
          } else if (is_ccsp_service or is_tsc_service) {
            this_runningtime += five_sec;
          } else {
            double temp = fast_atof(word);
            if (temp <= 0.0) // header or non-number
            {
              skip_line = true;
              continue;
            }

            if (old_time == 0.00) {
              qDebug() << "First line, set time to zero" << temp << old_time;
              old_time = temp;
            }

            if ((temp - old_time) < 0.0) {
              qDebug() << "(temp - old_time)" << temp << old_time
                       << (temp - old_time) << word << old_word;

              if ((temp - old_time) >= -0.9997686 &&
                  (temp - old_time) <= -0.9993055) {
                qDebug() << "Skip this line";
                skip_line = true;
                continue;
              } else {
                skip_line = true;
                old_time = temp;
                continue;
              }
            }

            this_runningtime =
                this_runningtime + (temp - old_time); // temp - time_offset
            old_time = temp;

            //                        actualhours = actualhours +
            //                        this_runningtime; qDebug() <<
            //                        "this_runningtime: " << this_runningtime;

            // media zilnica pentru Andrei si Ioni
            if ((this_runningtime - actualday) > 0) {
              fileContents.append(
                  QString::number(actualday) + ";" +
                  QString::number((actualaccumulation / actualsamplecount) *
                                  1000000) +
                  "\r\n");
              qDebug() << "actualdays: " << actualday;
              actualday += 1;
              actualaccumulation = 0;
              actualsamplecount = 0;
              //                           actualhours = 0;
            }

            //                        for (int i = 0; i< 63; i++){
            //                          old_word[i] = word[i];
            //                        }

            //                        if (is_mfu_as_ld or is_mfu_ld or
            //                        is_mfu_ld_old) this_runningtime -=
            //                        time_offset; qDebug() << this_runningtime;
          }
        } else if (is_vechitura_mircea and word_count == 1) {
          double temp = fast_atof(word);
          if (temp <= 0.0) // header or non-number
          {
            skip_line = true;
            continue;
          }

          if (old_time == 0.00) {
            qDebug() << "First line, set time to zero" << temp << old_time;
            old_time = temp;
          }

          //                        if ((temp - old_time) < 0.0){
          //                            qDebug() << "(temp - old_time)" << temp
          //                            << old_time << word << old_word;
          //                            skip_line = true;
          //                            old_time = temp;
          //                            continue;
          //                        }

          this_runningtime =
              this_runningtime + (temp - old_time); // temp - time_offset
          old_time = temp;
        }

        else if (word_count == temp_sv_index) {
          this_temperature = fast_atof(word);
          if (this_temperature > -70)
            this->temperature_sv->append(
                QPointF(this_runningtime, this_temperature));
          //                    qDebug() <<  "insert" << this_runningtime <<
          //                    this_temperature;
        }

        else if (word_count == temp_index) {
          this_temperature = fast_atof(word);
          if (this_temperature > -70)
            this->temperature->append(
                QPointF(this_runningtime, this_temperature));
          //                    qDebug() <<  "insert" << this_runningtime <<
          //                    this_temperature;
        } else if (word_count == hum_sv_index) {
          this_humid = fast_atof(word);
          if ((this_humid > 0.0) and (this_humid < 100.0)) {
            this->humidity_sv->append(QPointF(this_runningtime, this_humid));
            // indexShouldBeGap.append(false);
          } else {

            this->humidity_sv->append(QPointF(this_runningtime, 0.0));
            // indexShouldBeGap.append(true);
          }
        } else if (word_count == hum_index) {
          this_humid = fast_atof(word);
          if ((this_humid > 0.0) and (this_humid < 100.0)) {
            this->humidity->append(QPointF(this_runningtime, this_humid));
            // indexShouldBeGap.append(false);
          } else {
            this->humidity->append(QPointF(this_runningtime, 0.0));
            // indexShouldBeGap.append(true);
          }
        } else if (word_count == om_index) {
          this_om = translateOM(word);
          s_this_om = word;
          // qDebug() << "OM: " << this_om << s_this_om << this_runningtime;

          if (this_om == 3) {
            if (OM3_name == "")
              OM3_name = s_this_om;
            if (old_om != this_om)
              OM->append(QPointF(this_runningtime, old_om));
            OM->append(QPointF(this_runningtime, this_om));
            old_om = this_om;
          } else if (this_om == 2) {
            if (OM2_name == "")
              OM2_name = s_this_om;
            if (old_om != this_om)
              OM->append(QPointF(this_runningtime, old_om));
            OM->append(QPointF(this_runningtime, this_om));
            old_om = this_om;
          } else if (this_om == 1) {
            if (OM1_name == "")
              OM1_name = s_this_om;
            if (old_om != this_om)
              OM->append(QPointF(this_runningtime, old_om));
            OM->append(QPointF(this_runningtime, this_om));
            old_om = this_om;
          }
        } else if (word_count == curr_index) {
          this_current = fast_atof(word);
          //                    qDebug() << "this_current"<< this_current;

          if (is_picoammeter) {
            if ((this_current < 10000000.0))
              this->current_sleep->append(
                  QPointF(this_runningtime, this_current / 1000000));
            //                        qDebug() << this_current;
          } else if (this_om == 3) {
            this->current_activ->append(
                QPointF(this_runningtime, this_current));
          } else if (this_om == 2) {
            this->current_activ->append(
                QPointF(this_runningtime, this_current));
          } else if (this_om == 1) {

            // if((this_temperature > 80) and (this_temperature < 90)
            // and (this_current < 0.001)) if (this_current < 4)
            this->current_sleep->append(
                QPointF(this_runningtime, this_current));

            actualaccumulation += this_current;
            actualsamplecount += 1;
          } else {
            // this->current_activ->append(
            //     QPointF(this_runningtime, this_current));
          }
        }

        // Magnus Hettich special logs
        else if (word_count == KL30C_index) {
          this_current = fast_atof(word);
          this->current_KL30C->append(QPointF(this_runningtime, this_current));
        }

        // MH

        else if (word_count == can_io_err_index) {
          //                 qDebug()<< "Error: " << QString(word) << " temp: "
          //                 << this_temperature;

          if (QString(word).contains("Cell Voltage 220"))
            continue;

          if (QString(word).contains("Cell Voltage 160"))
            continue;

          if (QString(word).contains("Cell Voltage ")) {
            word[12] = '\0';
          }

          if ((this->errors.size() <= 60) and
              ((this_temperature > -202) or
               (this_temperature == -333333333.333))) {
            if (this->errors[word] == nullptr)
              this->errors.insert(word, new QVector<QPointF>);

            if (this_temperature < -70) {
              errors[word]->append(QPointF(this_runningtime, this_current));
            } else {
              errors[word]->append(QPointF(this_runningtime, this_temperature));
            }
          }

          //     otherError = false;
          //     if (((this_temperature > -70) or
          //          (this_temperature == -333333333.333))) {

          //       if (!this->errors.contains(word)) {

          //         if (this->errors.size() < 90) {
          //           this->errors.insert(word, new
          //           QVector<QPointF>); qDebug()
          //               << "inserting error, errors count:"
          //               << this->errors.size() << errors[word] <<
          //               word;
          //         } else {
          //           otherError = true;
          //           if (this->errors[QString("Other errors")] ==
          //               nullptr) {
          //             this->errors.insert(QString("Other errors"),
          //                                 new QVector<QPointF>);
          //             qDebug() << "Other errors serie created!";
          //           }
          //         }
          //       }

          //       if (this_temperature < -70) {
          //         if (otherError == true) {
          //           if (errors[QString("Other errors")]->count() <
          //           1000)
          //             errors[QString("Other errors")]->append(
          //                 QPointF(this_runningtime, this_current));
          //         } else {
          //           errors[word]->append(
          //               QPointF(this_runningtime, this_current));
          //         }

          //       } else {
          //         if (otherError == true) {
          //           errors[QString("Other errors")]->append(
          //               QPointF(this_runningtime,
          //               this_temperature));
          //         } else {
          //           errors[word]->append(
          //               QPointF(this_runningtime,
          //               this_temperature));
          //         }
          //       }
          //     }

        } else if (word_count == picoammeter_time) {
          double temp = fast_atof(word);
          if (temp == 0.0) // header or non-number
          {
            skip_line = true;
            continue;
          }
          this_runningtime = temp;
        } else if (word_count == voltage_index) {

          auto this_voltage = fast_atof(word);

          if ((this_voltage > -150.0) and (this_voltage < 150.0)) {

            if (this_voltage != old_voltage) {
              this->voltage->append(QPointF(this_runningtime, old_voltage));
              old_voltage = this_voltage;
            }

            this->voltage->append(QPointF(this_runningtime, this_voltage));
          }
        }

        if (is_mfu_as or is_mfu_as_ld) {

          if (fast_atof(word) > -150) {
            if (word_count == volt1_index) {
              // if (this->AS_volts[headerlist.at(word_count)] == nullptr)
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == volt2_index) {
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == volt3_index) {
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == volt4_index) {
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == volt5_index) {
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == volt6_index) {
              if (this->AS_volts[headerlist[word_count]] == nullptr)
                this->AS_volts.insert(
                    headerlist[word_count],
                    new QVector<QPointF>); // QScatterSeries(this->m_chart)

              this->AS_volts[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == curr1_index) {
              if (this->AS_currents[headerlist[word_count]] == nullptr)
                this->AS_currents.insert(headerlist[word_count],
                                         new QVector<QPointF>);

              this->AS_currents[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == curr2_index) {
              if (this->AS_currents[headerlist[word_count]] == nullptr)
                this->AS_currents.insert(headerlist[word_count],
                                         new QVector<QPointF>);

              this->AS_currents[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == curr3_index) {
              if (this->AS_currents[headerlist[word_count]] == nullptr)
                this->AS_currents.insert(headerlist[word_count],
                                         new QVector<QPointF>);

              this->AS_currents[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            } else if (word_count == curr4_index) {
              if (this->AS_currents[headerlist[word_count]] == nullptr)
                this->AS_currents.insert(headerlist[word_count],
                                         new QVector<QPointF>);

              this->AS_currents[headerlist[word_count]]->append(
                  QPointF(this_runningtime, fast_atof(word)));
            }
          }
        }

        if (is_mfu_ld) {
          if (word_count > (9)) {
            if (!(word_count % 2 == 0)) {

              if (ld_err_count_list[word_count] < atoi(word)) {
                if (this->durability_errors.size() < 512) {
                  if (this->durability_errors[headerlist[word_count]] ==
                      nullptr)
                    this->durability_errors.insert(headerlist[word_count],
                                                   new QVector<QPointF>);

                  //                                    durability_errors[headerlist[word_count]]->append(QPointF(this_runningtime,
                  //                                    this_temperature));

                  durability_errors[headerlist[word_count]]->append(
                      QPointF(this_runningtime + (0.6 / 3600 / 24),
                              0.5 + (word_count - 11)));

                  ld_err_count_list[word_count] = atoi(word);
                }
              }
            } else {

              if (this->durability_signals.size() < 512) {
                if (this->durability_signals[headerlist[word_count]] ==
                    nullptr) {
                  this->durability_signals.insert(headerlist[word_count],
                                                  new QVector<QPointF>);
                }

                // durability_signals[headerlist[word_count]]->append(
                //     QPointF(
                //     this_runningtime,
                //     durability_signals[headerlist[word_count]]->last().y())
                //     );

                // durability_signals[headerlist[word_count]]->append(QPointF(
                //     this_runningtime,
                //     durability_signals[headerlist[word_count]]
                //         ->at(
                //             durability_signals[headerlist[word_count]]->size()
                //             - 1)
                //         .y()));
                // qDebug() <<
                QList<QPointF> &signalVector =
                    *(durability_signals[headerlist[word_count]]);

                int lastIndex = signalVector.size() - 1;

                if (lastIndex < 0)
                  lastIndex = 0;

                // qDebug() << "here, last index:" << lastIndex
                //          << durability_signals;

                if (signalVector.isEmpty())
                  signalVector.append(QPointF(0, 0));

                signalVector.append(
                    QPointF(this_runningtime, signalVector[lastIndex].y()));

                durability_signals[headerlist[word_count]]->append(
                    QPointF(this_runningtime, atoi(word) + word_count - 10));

                //                                ld_err_count_list[word_count]
                //                                = atoi(word);
              }
            }
          }
        }
      }
    } else {
      word[word_index] = ch;
      word_index += 1;
    }

    if (ch == new_line) {
      word_index = 0;
      word_count = -1;
      curr_line++;
      skip_line = false;
    }
  }

  //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << timestamp2 - timestamp1;
  file.close();

  //    QDir dir = QFileInfo(path).absoluteDir();
  //    QString absdir = dir.absolutePath();

  //    csvFile.setFileName (this->folder + ".csv");
  //    qDebug() << "CSV File: " << absdir + "\\" + serieName + "\\" +
  //    this->folder + ".csv";

  //    csvFile.open (QIODevice::WriteOnly);

  //    csvFile.write(fileContents.toUtf8 ());
  //    csvFile.close ();
  fileContents.clear();

  if (OM->count() > 1)
    OM->remove(0);

  // this->humidity->setProperty ("indexShouldBeGap", indexShouldBeGap);
  return 1;
}

int View::load_robottxt_opt(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  QDataStream in(&file);

  // check if the file is a valid robot .txt;
  // may pass with invalid file by luck
  uint8_t fileBOM;
  in.readRawData(reinterpret_cast<char *>(&fileBOM), 1);
  if ((fileBOM != 35)) {
    qDebug() << "Invalid file encoding!" << fileBOM;
    return -2;
  }
  in.readRawData(reinterpret_cast<char *>(&fileBOM), 1);
  if ((fileBOM != 82) and (fileBOM != 80)) {
    qDebug() << "Invalid file encoding!" << fileBOM;
    return -3;
  }
  in.readRawData(reinterpret_cast<char *>(&fileBOM), 1);
  if ((fileBOM != 70) and (fileBOM != 114)) {
    qDebug() << "Invalid file encoding!" << fileBOM;
    return -4;
  }
  qDebug() << "file encoding: robot => correct!" << Qt::endl;

  //    qDebug() << "path" << path;

  QDir d = QFileInfo(path).absoluteDir();
  QString absolute = d.absolutePath();

  QString separator = ";";
  QString headerseparator;

  char ch;
  char carriage_return = '\r';
  char new_line = '\n';

  QString header, current_line;

  // seek trough the file until the header begin flag is found
  for (int i = 0; i < 30; i++) {
    while (!in.atEnd()) {
      in.readRawData(&ch, 1);
      current_line += ch;
      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (ch == new_line)
          break;
      }
    }
    if (current_line.contains("BEGIN:{Header}"))
      break;
    //        qDebug() << "current_line" << current_line;
    current_line.clear();
  }

  // get the header
  while (!in.atEnd()) {
    in.readRawData(&ch, 1);

    if (ch == carriage_return) {
      in.readRawData(&ch, 1);
      if (ch == new_line) {
        qDebug() << "header:" << header << header.size() << Qt::endl;
        break;
      }
    } else {
      header += ch;
    }
  }

  QList<QString> headerlist = header.split(";");
  qDebug() << "Header size:" << headerlist.size();

  bool has_LIN_data = false;

  for (int i = 0; i < headerlist.size(); i++) {
    auto sl_parts = headerlist.at(i).split("&");

    if (sl_parts.size() < 3)
      continue;

    headerlist.replace(i, sl_parts.at(0) + " [" + sl_parts.at(1) + "]]");
    if (sl_parts.at(0).contains("VLIN"))
      has_LIN_data = true;
  }

  qDebug() << "final header:" << headerlist;

  char word[256];
  int word_index = 0;
  int word_count = -1;

  bool skip_line = false;

  int res = 0;
  int curr_line = 0;

  double this_runningtime = 0.0;

  int byte1, byte2;

  double this_runningtime_time = 0.0;

  while (!file.atEnd()) {
    res = file.getChar(&ch);

    if (res < 1) // file read error
    {
      qDebug() << "File read error:" << res;
      //            QMessageBox::critical(this,  tr("Error:"), "File read
      //            error!");
      return -1;
    }

    if (ch ==
        '\0') // bullshit, probably memory leak in file during windows crash
    {
      word_index = 0;
      word_count = -1;
      skip_line = true;
      continue;
    }

    if (word_index > 255) // buffer overrun
    {
      qDebug() << "Buffer exceeded! clearing..." << curr_line;
      word_index = 0;
      word_count = -1;
      skip_line = true;
      continue;
    }

    if ((skip_line) and !(ch == new_line))
      continue;

    if ((ch == separator) or (ch == carriage_return)) {
      word_count += 1;

      if (word_index > 0) {
        word[word_index] = '\0';
        word_index = 0;

        //                                                qDebug() << word_count
        //                                                <<
        //                                                headerlist[word_count]
        //                                                << word ;
        //                qDebug() << headerlist[word_count];

        //                if (word_count == 0) continue;

        if (word_count == 1) {
          this_runningtime = fast_atof(word);
        } else if (word_count == 0) {
          this_runningtime_time = fast_atof(word);
        } else if (word_count == 15) { //(word_count > 8) and (word_count < 16)

          //                    if (this_runningtime > 0){

          if (this->durability_errors[headerlist[word_count]] == nullptr)
            this->durability_errors.insert(headerlist[word_count],
                                           new QVector<QPointF>);

          double dtemp = fast_atof(word);

          //                                                                qDebug()
          //                                                                <<
          //                                                                this_runningtime
          //                                                                <<
          //                                                                dtemp
          //                                                                <<
          //                                                                word;
          durability_errors[headerlist[word_count]]->append(
              QPointF(this_runningtime, dtemp));

          //                    if (word_count == 15)  qDebug() <<
          //                    this_runningtime << dtemp << word ;

          //                ld_err_count_list[word_count] = atoi(word);
          //                    }
        } else if (word_count == 24) {

          // specific, generalisation needed!
          if (has_LIN_data)
            byte1 = fast_atof(word);
        } else if (word_count == 25) {

          if (has_LIN_data) {

            byte2 = fast_atof(word);

            QString hex;

            QString hexadecimal;
            hexadecimal.setNum(byte2, 16);

            hex = hexadecimal;

            hexadecimal.setNum(byte1, 16);

            hex += hexadecimal;
            int signal = hex.toInt(nullptr, 16);

            //                                            qDebug() << "hex:" <<
            //                                            hex << "dec:" <<
            //                                            hex.toInt (nullptr,
            //                                            16);

            if (this->durability_errors["Button_1 "] == nullptr)
              this->durability_errors.insert("Button_1 ", new QVector<QPointF>);

            durability_errors["Button_1 "]->append(
                QPointF(this_runningtime, signal));
            // qDebug() << QPointF(this_runningtime, signal);

            if (this->durability_errors["Button_1_time"] == nullptr)
              this->durability_errors.insert("Button_1_time",
                                             new QVector<QPointF>);

            durability_errors["Button_1_time"]->append(
                QPointF(this_runningtime_time, signal));
            // qDebug() << "hex:" << hex << "dec:" << hex.toInt (nullptr, 16);
          }
        } else if (word_count == 26) {
          if (has_LIN_data)
            byte1 = fast_atof(word);
        } else if (word_count == 27) {
          if (has_LIN_data) {

            byte2 = fast_atof(word);

            QString hex;

            QString hexadecimal;
            hexadecimal.setNum(byte2, 16);

            hex = hexadecimal;

            hexadecimal.setNum(byte1, 16);

            hex += hexadecimal;
            int signal = hex.toInt(nullptr, 16);

            //                                            qDebug() << "hex:" <<
            //                                            hex << "dec:" <<
            //                                            hex.toInt (nullptr,
            //                                            16);

            if (this->durability_errors["Button_2 "] == nullptr)
              this->durability_errors.insert("Button_2 ", new QVector<QPointF>);

            durability_errors["Button_2 "]->append(
                QPointF(this_runningtime, signal));

            //                        if
            //                        (this->durability_errors["Button_2_time"]
            //                        == nullptr)
            //                            this->durability_errors.insert
            //                            ("Button_2_time", new
            //                            QVector<QPointF>);

            //                        durability_errors["Button_2_time"]->append(QPointF(this_runningtime_time,
            //                        signal));
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        } else if (word_count == 28) {
          if (has_LIN_data) {
            byte1 = fast_atof(word);
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        } else if (word_count == 29) {

          if (has_LIN_data) {

            byte2 = fast_atof(word);

            QString hex;

            QString hexadecimal;
            hexadecimal.setNum(byte2, 16);

            hex = hexadecimal;

            hexadecimal.setNum(byte1, 16);

            hex += hexadecimal;
            int signal = hex.toInt(nullptr, 16);

            //                                            qDebug() << "hex:" <<
            //                                            hex << "dec:" <<
            //                                            hex.toInt (nullptr,
            //                                            16);

            if (this->durability_errors["Reference_1 "] == nullptr)
              this->durability_errors.insert("Reference_1 ",
                                             new QVector<QPointF>);

            durability_errors["Reference_1 "]->append(
                QPointF(this_runningtime, signal));

            //                        if
            //                        (this->durability_errors["Button_3_time"]
            //                        == nullptr)
            //                            this->durability_errors.insert
            //                            ("Button_3_time", new
            //                            QVector<QPointF>);

            //                        durability_errors["Button_3_time"]->append(QPointF(this_runningtime_time,
            //                        signal));
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        } else if (word_count == 30) {

          if (has_LIN_data) {
            byte1 = fast_atof(word);
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        } else if (word_count == 31) {
          byte2 = fast_atof(word);

          QString hex;

          QString hexadecimal;
          hexadecimal.setNum(byte2, 16);

          hex = hexadecimal;

          hexadecimal.setNum(byte1, 16);

          hex += hexadecimal;
          int signal = hex.toInt(nullptr, 16);

          //                                        qDebug() << "hex:" << hex <<
          //                                        "dec:" << hex.toInt
          //                                        (nullptr, 16);

          if (this->durability_errors["Reference_2 "] == nullptr)
            this->durability_errors.insert("Reference_2 ", new QVector<QPointF>);

          durability_errors["Reference_2 "]->append(
              QPointF(this_runningtime, signal));

          //                    if (this->durability_errors["Button_4_time"] ==
          //                    nullptr)
          //                        this->durability_errors.insert
          //                        ("Button_4_time", new QVector<QPointF>);

          //                    durability_errors["Button_4_time"]->append(QPointF(this_runningtime_time,
          //                    signal));
        } else if (word_count == 99) {
          if (has_LIN_data)
            byte1 = fast_atof(word);
        } else if (word_count == 100) {

          if (has_LIN_data) {

            byte2 = fast_atof(word);

            QString hex;

            QString hexadecimal;
            hexadecimal.setNum(byte2, 16);

            hex = hexadecimal;

            hexadecimal.setNum(byte1, 16);

            hex += hexadecimal;
            int signal = hex.toInt(nullptr, 16);

            //                                            qDebug() << "hex:" <<
            //                                            hex << "dec:" <<
            //                                            hex.toInt (nullptr,
            //                                            16);

            if (this->durability_errors["Slider_1 "] == nullptr)
              this->durability_errors.insert("Slider_1 ", new QVector<QPointF>);

            durability_errors["Slider_1 "]->append(
                QPointF(this_runningtime, signal));

            //                        if
            //                        (this->durability_errors["Slider_1_time"]
            //                        == nullptr)
            //                            this->durability_errors.insert
            //                            ("Slider_1_time", new
            //                            QVector<QPointF>);

            //                        durability_errors["Slider_1_time"]->append(QPointF(this_runningtime_time,
            //                        signal));
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        } else if (word_count == 101) {
          if (has_LIN_data)
            byte1 = fast_atof(word);
        } else if (word_count == 102) {
          if (has_LIN_data) {

            byte2 = fast_atof(word);

            QString hex;

            QString hexadecimal;
            hexadecimal.setNum(byte2, 16);

            hex = hexadecimal;

            hexadecimal.setNum(byte1, 16);

            hex += hexadecimal;
            int signal = hex.toInt(nullptr, 16);

            //                                            qDebug() << "hex:" <<
            //                                            hex << "dec:" <<
            //                                            hex.toInt (nullptr,
            //                                            16);

            if (this->durability_errors["Slider_2 "] == nullptr)
              this->durability_errors.insert("Slider_2 ", new QVector<QPointF>);

            durability_errors["Slider_2 "]->append(
                QPointF(this_runningtime, signal));

            //                        if
            //                        (this->durability_errors["Slider_2_time"]
            //                        == nullptr)
            //                            this->durability_errors.insert
            //                            ("Slider_2_time", new
            //                            QVector<QPointF>);

            //                        durability_errors["Slider_2_time"]->append(QPointF(this_runningtime_time,
            //                        signal));
          } else {
            if (this->durability_errors[headerlist[word_count]] == nullptr)
              this->durability_errors.insert(headerlist[word_count],
                                             new QVector<QPointF>);

            durability_errors[headerlist[word_count]]->append(
                QPointF(this_runningtime, fast_atof(word)));
          }
        }
      }
    } else {
      word[word_index] = ch;
      word_index += 1;
    }

    //        qDebug() << word << word_count;
    if (ch == new_line) {
      //            qDebug() << "new line!";
      word_index = 0;
      word_count = -1;
      curr_line++;
      skip_line = false;
    }
  }
  qDebug() << "result container size:" << durability_errors.size();
  qDebug() << "first serie size:" << durability_errors.first()->size();

  qDebug() << durability_signals[headerlist[15]];

  //        QDir dir = QFileInfo(this->folder + "\\" + "test" +
  //        ".csv").absoluteDir(); QString absdir = dir.absolutePath();

  //        auto export_folder = this->folder + "_export\\";
  //        qDebug() << "dir.dirName ()" << dir.dirName () << "export_folder" <<
  //        export_folder; QDir().mkdir(export_folder);

  {

    // for (auto s : durability_errors.keys ()){

    auto serie = durability_errors.first();

    qDebug() << durability_errors.key(serie);

    QString fileContents;
    QFile csvFile;

    fileContents.append("Travel [mm];"
                        "Time [s];"
                        "Force [N];"
                        "Sensor_1;"
                        "Reference_1;"
                        "Sensor_2;"
                        "Reference_2;"
                        "\n");

    //        for (int k = 0 ; k < durability_errors.size (); k++){
    //            fileContents.append (durability_errors.keys ().at(k) + ";");
    //        }

    fileContents.append("\r\n");

    auto dataVector = serie; // serie->pointsVector ();

    //            qDebug() << "serie->hapticPoints before points conversion:" <<
    //            serie->hapticPoints;

    //            for (int i = 0; i < dataVector->size (); i++) {
    //                for (auto h_point_name : serie->hapticPoints.keys ()){
    //                    if(serie->hapticPoints.value (h_point_name) ==
    //                    dataVector[i]){
    //                        serie->hapticPointsRow.insert (h_point_name, i);
    //                        qDebug() << "Point row:" << h_point_name <<
    //                        dataVector[i] << i;
    //                    }
    //                }
    //            }

    //*** zero offset pentru Silviu Petre ***//
    auto dataVector_Travel = durability_errors.values().at(5);
    auto dataVector_Force = durability_errors.values().at(5);
    double travel_offset = 0.0;
    for (int i = 0; i < dataVector_Travel->size(); i++) {
      if (dataVector_Force->at(i).y() > 0.01)
        break;
      travel_offset = dataVector_Travel->at(i).x();
    }

    for (int i = 0; i < dataVector_Travel->size(); i++) {
      dataVector_Travel->replace(
          i, QPointF(dataVector_Travel->at(i).x() - travel_offset,
                     dataVector_Travel->at(i).y()));
    }

    for (int i = 0; i < dataVector->size(); i++) {

      //            for (int h = 0 ; h < durability_errors.size (); h++){

      //                fileContents.append (QLocale().toString
      //                (durability_errors.values ().at (h)->at(i).x()) + ";" +
      //                                     QLocale().toString
      //                                     (durability_errors.values ().at
      //                                     (h)->at(i).y()) + ";");

      fileContents.append(
          QLocale().toString(durability_errors.values().at(5)->at(i).x()) +
          ";" + // travel
          QLocale().toString(durability_errors.values().at(1)->at(i).x()) +
          ";" + // time
          QLocale().toString(durability_errors.values().at(5)->at(i).y()) +
          ";" + // force
          QLocale().toString(durability_errors.values().at(0)->at(i).y()) +
          ";" + // Button_1
          QLocale().toString(durability_errors.values().at(3)->at(i).y()) +
          ";" + // Reference_1
          QLocale().toString(durability_errors.values().at(2)->at(i).y()) +
          ";" + // Button_2
          QLocale().toString(durability_errors.values().at(4)->at(i).y()) +
          ";" // Reference_2
      );

      //                fileContents.append (QLocale().toString (dataVector->at
      //                (i).x ()) + ";" +
      //                                     QLocale().toString (dataVector->at
      //                                     (i).y ()) + ";");
      //                if(!serie->hapticPointsRow.isEmpty ()){
      //                    auto p_name = serie->hapticPointsRow.lastKey ();
      //                    auto p_row = serie->hapticPointsRow.take (p_name);
      //                    fileContents.append (p_name + ";" + QString::number
      //                    (p_row + 2));
      //                }
      //            }

      fileContents.append("\r\n");
    }

    QDir dir = QFileInfo(path).absoluteDir();
    QString absdir = dir.absolutePath();

    csvFile.setFileName(this->folder + ".csv");
    qDebug() << "CSV File: "
             << absdir + "\\" + serieName + "\\" + this->folder + ".csv";
    //            qDebug() << "folder" << export_folder;
    csvFile.open(QIODevice::WriteOnly);

    csvFile.write(fileContents.toUtf8());
    csvFile.close();
    fileContents.clear();
    // }
  }

  //        QFileInfo fileinfo(absdir);
  //        QDesktopServices::openUrl("file:" + export_folder.replace ("/",
  //        "\\"));

  //    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    //    qDebug() << timestamp2 - timestamp1;
  file.close();
  return 1;
}

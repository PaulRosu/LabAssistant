#include "view.h"
class View;


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



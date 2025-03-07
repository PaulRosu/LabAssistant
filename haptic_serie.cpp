#include "haptic_serie.h"

// hapticSerie::hapticSerie(QChart *parent) : QScatterSeries(parent)
hapticSerie::hapticSerie(QObject *parent) : QScatterSeries(parent) {}

int hapticSerie::load_robottxt_opt(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  QDir dir = QFileInfo(path).absoluteDir();
  QString absdir = dir.absolutePath();

  QFile bus_config_file(absdir + "\\BUS_CONFIG.txt");

  struct bus_data {
    QString name;
    QString ID;
    uint ID_size;
    uint place;
    char val_pressed;
    char val_released;
    bool last_state;
  };

  struct data_index_struct {
    QVector<int> x_index;
    QVector<int> y_index;
  };

  QVector<bus_data> bus_config_vector;
  data_index_struct data_indexes;

  if (bus_config_file.exists()) {
    if (!bus_config_file.open(QIODevice::ReadOnly)) {
      qDebug() << "BUS_CONFIG.txt File Open Failed!";
    } else {

      QTextStream txtin(&bus_config_file);

      QVector<QString> line_vector;

      txtin.readLine(); // read out the header
      while (!txtin.atEnd()) {
        QString line = txtin.readLine();
        if (line.isEmpty())
          continue;

        line.replace(' ', "");
        line.replace('\t', "");

        //                qDebug() << line;
        auto line_list = line.split(";");
        line_vector = line_list.toVector();
        //                qDebug() << line_vector;

        if (line_vector.size() >= 5) {
          line_vector.replace(1, "#[" + line_vector.at(1) + ":");
          bus_data current_line;
          current_line.name = line_vector.at(0);
          current_line.ID = line_vector.at(1);
          current_line.ID_size = line_vector.at(1).size();
          current_line.place = line_vector.at(2).toUInt();
          current_line.val_pressed = line_vector.at(3).at(0).toLatin1();
          current_line.val_released = line_vector.at(4).at(0).toLatin1();
          current_line.last_state = false;
          bus_config_vector.append(current_line);
        } else if (line_vector.size() == 3) {
          if (line_vector.at(0) == 'X')
            data_indexes.x_index.append(line_vector.at(1).toInt());
          if (line_vector.at(0) == 'Y')
            data_indexes.y_index.append(line_vector.at(1).toInt());
        }

        line_vector.clear();
      }
      bus_config_file.close();
    }
    qDebug() << "BUS_CONFIG: " << bus_config_vector.size();
    qDebug() << "data_indexes: " << data_indexes.x_index
             << data_indexes.y_index;
  }

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
  auto has_data_config = (!data_indexes.x_index.isEmpty());
  int bus_index = -1;

  for (int i = 0; i < headerlist.size(); i++) {
    auto sl_parts = headerlist.at(i).split("&");

    if (sl_parts.size() < 3)
      continue;

    headerlist.replace(i, sl_parts.at(0) + " [" + sl_parts.at(1) + "]]");

    if (sl_parts.at(0).contains("VLIN")) {
      has_LIN_data = true;
      bus_index = i;
    }

    if (sl_parts.at(0).contains("VCAN")) {
      bus_index = i;
    }

    if (!has_data_config) {
      if (sl_parts.at(1).contains("mm")) {
        data_indexes.x_index.append(i);
      }
      if (sl_parts.at(0).contains("Fz")) {
        data_indexes.x_index.append(i);
      }
    }
  }

  qDebug() << "final header:" << headerlist << "has_LIN_data" << has_LIN_data;

  QVector<int> analog_index_vector;

  for (int i = 0; i < headerlist.size(); i++) {

    if (headerlist.at(i).contains("[V]"))
      analog_index_vector.append(i);
  }

  qDebug() << "analog_index_vector:" << analog_index_vector;

  char word[256];
  int word_index = 0;
  int word_count = -1;

  bool skip_line = false;

  int res = 0;
  int curr_line = 0;

  double this_travel = 0.0;

  double this_runningtime = 0.0;

  bool ok;
  qreal this_force{0.0};

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

      if ((word_count >= 0) and (word_count < headerlist.size())) {
        word[word_index] = '\0';
        //                word_index = 0;
        //                qDebug() << word_count << word;

        if (word_count ==
            data_indexes.x_index[0]) { // expand to handle multiple x data
          this_travel = fast_atof(word, &ok);
          if (!ok)
            skip_line = true;

        } else if (word_count == 0) {
          this_runningtime = fast_atof(word, &ok);
          if (!ok)
            skip_line = true;

          //                } else if (word_count == 11){

          //                    if (this->robotData[headerlist[word_count]] ==
          //                    nullptr)
          //                        this->robotData.insert
          //                        (headerlist[word_count], new
          //                        QVector<QPointF>);

          //                    this_force = fast_atof(word, &ok);
          //                    if (ok)
          //                    robotData[headerlist[word_count]]->append(QPointF(this_travel,
          //                    this_force * -1));

        } else if (word_count == bus_index) {
          QString BUS_data = QString::fromLatin1(word, word_index);
          QString debug_message;
          for (auto &bus_config : bus_config_vector) {

            int index1 = BUS_data.indexOf(bus_config.ID) + bus_config.ID_size;

            auto signal_state = BUS_data[index1 + bus_config.place];
            if (signal_state == bus_config.val_pressed) {

              // debug_message += bus_config.name + "Pressed!";
              if (!bus_config.last_state) {
                qDebug() << bus_config.name << "Pressed!";
                hapticPoints.insert("Bus: " + bus_config.name + " pressed",
                                    QPointF(this_travel, this_force * -1));
                bus_config.last_state = true;
              }

            } else if (signal_state == bus_config.val_released) {

              if (bus_config.last_state) {
                qDebug() << bus_config.name << "Released!";
                hapticPoints.insert("Bus: " + bus_config.name + " released",
                                    QPointF(this_travel, this_force * -1));
                bus_config.last_state = false;
              }
              // debug_message += bus_config.name + "Released!";
            } else {
              // debug_message += bus_config.name + "Undefined!";
            }
          }

          // qDebug () << debug_message;

        } else {
          for (auto index : analog_index_vector) {
            if (word_count == index) {
              if (this->robotData[headerlist[word_count]] == nullptr)
                this->robotData.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_voltage = fast_atof(word, &ok);
              if (ok)
                robotData[headerlist[word_count]]->append(
                    QPointF(this_travel, this_voltage));
            }
          }

          for (auto index : data_indexes.y_index) {
            if (word_count == index) {
              if (this->robotData[headerlist[word_count]] == nullptr)
                this->robotData.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_y = fast_atof(word, &ok);
              if (ok)
                robotData[headerlist[word_count]]->append(
                    QPointF(this_travel, this_y));
            }
          }
        }

        word_index = 0;
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

  qDebug() << "result container size:" << robotData.size();
  qDebug() << "first serie size:" << robotData.first()->size();

  qDebug() << "hapticPoints on file load:" << hapticPoints;

  //    qDebug() << durability_signals[headerlist[15]];

  //    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    //    qDebug() << timestamp2 - timestamp1;
  file.close();
  return 1;
}

void hapticSerie::findHapticPoints() {

  qDebug() << "hapticSerie::findHapticPoints ()" << this->name();
  //    QMap<QString, QPointF> hapticPoints;
  qreal differential_factor = 0.25; // 0.25
  qreal left_cont_rising_force = 0.5;
  qreal misuse_force_treshold = 50.0;
  qreal bendfactor_treshold = 0.85;
  qreal max_force_cutoff_factor = 1;
  qreal low_points_count_max_force_cutoff_factor = 0.8; // this may be hardcoded
  qreal overtravel_termen = 0.0;
  qreal radius_factor = 0.4; // 0.33

  //    qDebug() << Qt::endl  << Qt::endl << "Searching interest points in " <<
  //    specimen->name << specimen->samples << " measurements"; int timestamp1 =
  //    QDateTime::currentMSecsSinceEpoch();

  // *** find the approach distance, before the actuator touches the sample
  qreal approachDistance = 0;

  //    QMessageBox msgBox;

  //    msgBox.setWindowTitle("Mem test");
  //    msgBox.setText("Before:");
  //    msgBox.exec();

  auto pVector = this->points();

  //    msgBox.setWindowTitle("Mem test");
  //    msgBox.setText("After:");
  //    msgBox.exec();

  int start = 8;
  int end = 9;
  while ((pVector.at(end).y() - pVector.at(start).y()) <
         left_cont_rising_force) {
    // find a continuously rising slope of at least 0.5N

    if (end > (pVector.size() - 5)) {
      end = end - 5;
      break;
    }

    if (pVector.at(end + 3).y() > pVector.at(end).y()) {
      // if rising, move the end of the range forward
      // the checking could be made at an interval greater that 1 if the serie
      // is very noisy or have a huge number of points
      end++;
    } else {
      // reset the range ends at current position
      start = end;
      end += 3;
    }
  }

  qDebug() << "trec pe aici!";

  // Apply preload
  for (int i = 0; i < pVector.size(); i++) {
    if (pVector.at(i).y() >= 0.010) { // noise level: 0.01 ; more means preload
      start = i;
      break;
    }
  }

  approachDistance = pVector.at(start).x();
  this->approachrow = start;

  if (pVector.at(3).y() >= 0.15) {
    this->approachrow = 0;
    approachDistance = 0;
  }

  //    approachDistance = pVector.at (start).x ();
  //    this->approachrow = start;

  // apply the approach distance offset and find the max force value and
  // location

  qreal maxForce{0.0};
  int maxForceLocation{0};

  //        qDebug() << this->name () << "approachDistance" << approachDistance;

  for (int i = 0; i < pVector.size(); i++) {
    pVector.replace(
        i, QPointF(pVector.at(i).x() - approachDistance, pVector.at(i).y()));
    if (pVector.at(i).y() > maxForce) {
      maxForce = pVector.at(i).y();
      maxForceLocation = i;
    }
  }

  // also apply the approach offset to any contact/bus point found on load

  for (auto &point : this->hapticPoints) {
    point.setX(point.x() - approachDistance);
  }

  this->offset = approachDistance;

  // also apply the approach offset to any non force or torque type data
  //
  for (auto &data_vector : this->robotData.values()) {
    QString s_name = this->robotData.key(data_vector);
    if (!s_name.contains("[N")) {

      bool old_state = false;
      int i = 0;
      for (auto &point : (*this->robotData[s_name])) {

        point.setX((point.x() - approachDistance));

        if (s_name.contains("[V")) {
          // insert toggle points for analog signals
          if ((point.y() > 1.0) and (!old_state)) {
            hapticPoints.insert(s_name + " pressed", pVector.at(i));
            old_state = true;
          } else if ((point.y() < 1.0) and (old_state)) {
            hapticPoints.insert(s_name + " released", pVector.at(i));
            old_state = false;
          }
        }
        i++;
      }
    }
  }

  hapticPoints.insert("max force", pVector.at(maxForceLocation));

  // if the max force is huge, most probably this is a misuse test, so
  // concav/convex points are not relevant
  if (maxForce > misuse_force_treshold) {
    return;
  }

  // *** find the left bound for the upper curve
  start = end; // start from the approaching distance
  end = start + 1;
  while (pVector.at(start).y() <= pVector.at(end).y()) {
    // and move forward while the force is not falling
    end++;
    start++;
  }
  int leftUpperBound = end - 15; // come back a bit before the edge
  // ***

  // interest points must have at least 10% lower force.
  // 85% is better, but there are many curves for defective buttons with do not
  // respect this
  qreal maxInterestPointsForce = maxForce * max_force_cutoff_factor;

  // find max pressing (Higher) curve Interest Points Force Location

  // start from maximum force point
  int maxHInterestPointsForceLocation = maxForceLocation - 30;
  while ((pVector.at(maxHInterestPointsForceLocation).y() >
          maxInterestPointsForce)) {
    maxHInterestPointsForceLocation--; // go back while force is high
  }

  // *** find the right bound for the pressing (higher) curve
  start =
      maxHInterestPointsForceLocation - 2; // start from the max force location
  end = maxHInterestPointsForceLocation;
  while (pVector.at(start).y() < pVector.at(end).y()) {
    // go back while the force is falling
    start--;
    end--;
  }
  int rightUpperBound = start + 15;
  //        qDebug() << "maxHInterestPointsForceLocation" <<
  //        pVector.at(maxHInterestPointsForceLocation);
  // ***

  // *** find max Interest Points Force Location for the lower curve

  int maxLInterestPointsForceLocation = maxForceLocation;

  // start at max force location + 80 points (robot optimization)
  if (pVector.size() > 1000)
    maxLInterestPointsForceLocation += 80;

  while (pVector.at(maxLInterestPointsForceLocation).y() >
         maxInterestPointsForce) {
    // go forward while the force is high
    maxLInterestPointsForceLocation++;
  }
  // ***

  // *** find the right bound for the lower curve
  start = maxLInterestPointsForceLocation;
  end = maxLInterestPointsForceLocation + 1;
  while (pVector.at(start).y() > pVector.at(end).y()) {
    end++;
    start++;
  }
  int rightLowerBound = start - 15;
  // ***

  // *** find the lower curve left bound
  start = pVector.size() - 1; // start from the vector end
  end = start - 2;

  // optimizare claudius
  while (abs(pVector.at(start).x()) < 0.05) {
    start--;
    // qDebug() << "optimizare claudius X=" << pVector.at(start).x()
    //          << this->name();
  }
  end = start - 2;
  // qDebug() << "optimizare claudius end final:" << end << this->name();
  //

  while (pVector.at(end).y() - pVector.at(start).y() < 0.2) {
    // find a rising force slope of 0.3N
    if (end <= 1)
      break;

    if (pVector.at(end - 1).y() > pVector.at(end).y()) {
      end--;
    } else {
      start = end;
      end -= 1;
    }
  }

  start = end; // from the 0.3N slope end
  end = start - 1;
  int leftLowerBound = 0;
  if (end > 1) {
    while (pVector.at(end).y() >= pVector.at(start).y()) {
      // go up as long as the force is not falling
      end--;
      start--;
    }
    leftLowerBound = end + 15; //+ 30
  }
  // ***

  // *** find a radius in which to evaluate the peaks for higher and lower
  // curves interestPointsRadiusL = bounded range * factor (0.2 >= factor <= 1)
  qreal interestPointsRadiusL = 0;
  qreal interestPointsRadiusL_HigerCurve =
      (pVector.at(rightUpperBound).x() - pVector.at(leftUpperBound).x()) *
      radius_factor;

  qreal interestPointsRadiusL_LowerCurve =
      (pVector.at(rightLowerBound).x() - pVector.at(leftLowerBound).x()) *
      radius_factor;

  interestPointsRadiusL = interestPointsRadiusL_HigerCurve;

  // if (interestPointsRadiusL < 0.02) interestPointsRadiusL = 0.02; // !

  // ***

  //        qDebug() << this->name()
  //                 <<  "leftUpperBound" << pVector.at(leftUpperBound).x() <<
  //                 pVector.at(leftUpperBound).y() << Qt::endl
  //                 <<  "leftlowerBound" << pVector.at(leftLowerBound).x() <<
  //                 pVector.at(leftUpperBound).y() << Qt::endl
  //                 <<  "rightUpperBound" << pVector.at(rightUpperBound).x() <<
  //                 pVector.at(rightUpperBound).y() << Qt::endl
  //                 <<  "rightLowerBound" << pVector.at(rightLowerBound).x() <<
  //                 pVector.at(rightLowerBound).y()<< Qt::endl
  //                 <<  "interestPointsRadiusL" << interestPointsRadiusL <<
  //                 Qt::endl;

  qreal bendfactor = 0.0;

  //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << "interest point prep in ms" << timestamp2 - timestamp1;

  // prepare some lists to store the interest points location
  QList<int> upconcave;
  QList<int> upconvexe;
  QList<int> downconvexe;
  QList<int> downconcave;

  int backBound = 0;
  int forwardBound = 0;

  // ** search for haptic points in the valid range
  for (int l = leftUpperBound; l < leftLowerBound; l++) {
    // in mech stop area //orig: ±10; optimized for low res Tunis files
    if ((l > rightUpperBound + 3) and (l < rightLowerBound - 3)) {
      l = rightLowerBound - 3;
      continue;
    }
    // if (specimen->force.at(l) < 0.5) continue; //noise

    if (pVector.at(l).y() >= maxInterestPointsForce)
      continue;

    //         ultra low point count serie workaround (some Tunis files)
    if ((pVector.size() < 1000) and
        (pVector.at(l).y() >=
         (maxInterestPointsForce * low_points_count_max_force_cutoff_factor)))
      continue;

    // check if local convex peak
    if ((pVector.at(l).y() >= pVector.at(l + 5).y()) and
        (pVector.at(l).y() >= pVector.at(l - 5).y())) {

      // choose the upper\lower radius
      if (l < rightUpperBound + 5)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve; // 10
      if (l > rightLowerBound - 5)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve; // 10

      // *** find the bounds in which to evaluate the peak
      backBound = 0;
      forwardBound = 0;
      for (int j = l; j < pVector.size(); ++j) {
        // if reached the max force location, set the bound there
        if (j == maxHInterestPointsForceLocation) {
          forwardBound = j;
          break;
        }

        // if reached the force changed with differential_factor (default:
        // 0.25)N, set the bound there
        if (abs((pVector.at(l).y() - pVector.at(j).y())) >
            differential_factor) {
          forwardBound = j; //
          break;
        }

        // if reached the radius distance, set the bound there
        if (abs(pVector.at(j).x() - pVector.at(l).x()) >
            interestPointsRadiusL) {
          forwardBound = j;
          break;
        }
      }

      // error, continue
      if (forwardBound == 0)
        continue;

      for (int j = l; j > 0; --j) {
        if (j == maxLInterestPointsForceLocation) {
          backBound = j;
          break;
        }

        if (abs((pVector.at(l).y() - pVector.at(j).y())) >
            differential_factor) {
          backBound = j;
          break;
        }

        if (abs(pVector.at(l).x() - pVector.at(j).x()) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }
      // ***

      // check if the current point is maximum in range
      qreal current_point_force = pVector.at(l).y();

      bool is_current_point_max = true;
      for (int i = backBound; i <= forwardBound; i++) {
        if (pVector.at(i).y() > current_point_force) {
          is_current_point_max = false;
          break;
        }
      }

      //            qDebug ()<< " local convex" << pVector.at (l).y() <<
      //            pVector.at (l).x()
      //                     << "backBound" << pVector.at (backBound).x()
      //                     << "forwardBound" << pVector.at (forwardBound).x()
      //                     << "is_current_point_max" << is_current_point_max;

      if (is_current_point_max) {

        // the ration between the sum of change in force (Dy) from the point to
        // both extremes and the range length (x)

        bendfactor =
            abs((pVector.at(forwardBound).y() - abs(pVector.at(l).y())) +
                (pVector.at(backBound).y() - abs(pVector.at(l).y()))) /
            interestPointsRadiusL;

        //                bendfactor = abs(
        //                            abs((this->at(forwardBound).y() -
        //                            abs(this->at(l).y()))) +
        //                            abs((this->at(backBound).y() -
        //                            abs(this->at(l).y()))) ) /
        //                            interestPointsRadiusL;

        //                                qDebug () << Qt::endl << this->name()
        //                                << "convex point found: " <<
        //                                pVector.at (l).x() << Qt::endl
        //                                          << "mm" << pVector.at
        //                                          (l).y() << Qt::endl
        //                                          << "N" << " point on row nr:
        //                                          "
        //                                          << l
        //                                          << "bend factor:" <<
        //                                          bendfactor << Qt::endl
        //                                          << "f at forwardBound" <<
        //                                          pVector.at(forwardBound).y()
        //                                          << Qt::endl
        //                                          << "s at forwardBound" <<
        //                                          pVector.at(forwardBound).x()
        //                                          << Qt::endl
        //                                          << "f at backBound" <<
        //                                          pVector.at(backBound).y() <<
        //                                          Qt::endl
        //                                          << "s at backBound" <<
        //                                          pVector.at(backBound).x() <<
        //                                          Qt::endl
        //                                          << Qt::endl << Qt::endl
        //                                             ;

        qreal current_point_force = pVector.at(l).y();

        bool is_current_point_max = true;
        for (int i = backBound; i <= forwardBound; i++) {
          if (pVector.at(i).y() > current_point_force) {
            is_current_point_max = false;
            break;
          }
        }
        //                                qDebug () << Qt::endl <<
        //                                "is_current_point_max" <<
        //                                is_current_point_max;

        if (bendfactor < bendfactor_treshold)
          continue; // haptic imperceptible

        if (l < rightUpperBound + 3)
          upconvexe.append(l); // 10
        if (l > rightLowerBound - 3)
          downconvexe.append(l); // 10

        l = forwardBound - ((forwardBound - l) /
                            2); // move the cursor half the forward bound length

        //                 qDebug () << specimen->name << "la convex: " <<
        //                 upconvexe << upconcave << downconvexe << downconcave;

        continue;
      }
    }

    // check if local concav peak, and apply the same startegy to find the
    // interest points
    if (pVector.at(l).y() <= pVector.at(l + 5).y() and
        pVector.at(l).y() <= pVector.at(l - 5).y()) { // local concav peak

      if (l < rightUpperBound + 10)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve;
      if (l > rightLowerBound - 10)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve;

      backBound = 0;
      forwardBound = 0;

      for (int j = l; j < pVector.size(); ++j) {
        if (j == maxHInterestPointsForceLocation + 10) {
          forwardBound = j;
          break;
        }

        if (abs((pVector.at(l).y() - pVector.at(j).y())) >
            differential_factor) {
          forwardBound = j;
          break;
        }

        if (abs(pVector.at(j).x() - pVector.at(l).x()) >
            interestPointsRadiusL) {
          forwardBound = j;
          break;
        }
      }

      if (forwardBound == 0)
        break;

      for (int j = l; j > 0; --j) {
        if (j == maxLInterestPointsForceLocation - 10) {
          backBound = j;
          break;
        }

        if (abs((pVector.at(l).y() - pVector.at(j).y())) >
            differential_factor) {
          backBound = j;
          break;
        }

        if (abs(pVector.at(l).x() - pVector.at(j).x()) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }

      // check if the current point is minimum in range
      qreal current_point_force = pVector.at(l).y();
      bool is_current_point_min = true;
      for (int i = backBound; i <= forwardBound; i++) {
        if (pVector.at(i).y() < current_point_force) {
          is_current_point_min = false;
          break;
        }
      }

      //            qDebug ()<< "local concav" << pVector.at (l).y() <<
      //            pVector.at (l).x()
      //                     << "backBound" << pVector.at (backBound).x()
      //                     << "forwardBound" << pVector.at (forwardBound).x()
      //                     << "is_current_point_max" << is_current_point_min;

      if (is_current_point_min) {

        bendfactor =
            abs((pVector.at(forwardBound).y() - abs(pVector.at(l).y())) +
                (pVector.at(backBound).y() - abs(pVector.at(l).y()))) /
            interestPointsRadiusL;

        //                                qDebug () << this->name() <<  "concav
        //                                point found: " << pVector.at (l).x()
        //                                          << "mm" << pVector.at
        //                                          (l).y()
        //                                          << "N" << " point on row nr:
        //                                          "
        //                                          << l
        //                                          << "bend factor:" <<
        //                                          bendfactor;

        if (bendfactor < bendfactor_treshold)
          continue;

        if (l < rightUpperBound + 10)
          upconcave.append(l);
        if (l > rightLowerBound - 10)
          downconcave.append(l);

        // move the cursor half the forward bound length
        l = forwardBound - ((forwardBound - l) / 2);
      }
    }
  }

  qDebug() << this->name() << "initial: " << upconvexe << upconcave
           << downconcave << downconvexe;

  // ** sort the found points

  // sort the convex points on the upper curve
  if (upconvexe.size() > 1) {
    // double detent, set the !! first two !! convex points their values. points
    // >2 ignored!
    hapticPoints.insert("convex push det1", pVector.at(upconvexe.at(0)));
    hapticPoints.insert("convex push det2", pVector.at(upconvexe.at(1)));
  } else if (upconvexe.size() == 1) {
    hapticPoints.insert("convex push det1", pVector.at(upconvexe.at(0)));
  } else if (upconvexe.size() == 0) {
  }

  // same strategy applied for sorting the other points

  if (upconcave.size() > 1) {
    hapticPoints.insert("concav push det1", pVector.at(upconcave.at(0)));
    hapticPoints.insert("concav push det2", pVector.at(upconcave.at(1)));
  } else if (upconcave.size() == 1) {
    hapticPoints.insert("concav push det1", pVector.at(upconcave.at(0)));
  } else if (upconcave.size() == 0) {
  }

  if ((downconvexe.size() > 1) and (upconvexe.size() > 1)) {
    hapticPoints.insert("convex release det2", pVector.at(downconvexe.at(0)));
    hapticPoints.insert("convex release det1", pVector.at(downconvexe.at(1)));
  } else if (downconvexe.size() == 1) {
    hapticPoints.insert("convex release det1", pVector.at(downconvexe.at(0)));
  } else if (downconvexe.size() == 0) {
  }

  if ((downconcave.size() > 1) and (upconvexe.size() > 1)) {
    hapticPoints.insert("concav release det2", pVector.at(downconcave.at(0)));
    hapticPoints.insert("concav release det1", pVector.at(downconcave.at(1)));
  } else if (downconcave.size() == 1) {
    hapticPoints.insert("concav release det1", pVector.at(downconcave.at(0)));
  } else if (downconcave.size() == 0) {

  }

  else if (downconcave.size() > 1) {
    // There are more that one concave points on the release curve,
    // but only one convex point was found on the push curve.
    // This cannot be a double detent, so we consider only the last concav point
    // found (left most)
    hapticPoints.insert("concav release det1", pVector.at(downconcave.last()));
  }

  // ***

  // *** find the overtravel point
  //(projection to the right from the last convex point + overtravel_termen)
  if (!upconvexe.isEmpty())
    for (int j = upconvexe.last(); j < pVector.size(); ++j) {
      if (j == maxForceLocation)
        break;
      // pVector.at (j).y() - pVector.at (upconvexe.last ()).y()) >
      // overtravel_termen
      if ((pVector.at(j).y() - pVector.at(upconvexe.last()).y()) >
          overtravel_termen) { // pVector.at (j).y()  > 7.0
        // j -= 1;
        hapticPoints.insert("overtravel", pVector.at(j));
        break;
      }
    }

  //    qDebug() << "hapticPoints" << hapticPoints;

  //    // *** search for electrical contact changes for the first two switches.
  //    bool sw_1_state = false;
  //    bool sw_2_state = false;

  //    // the contact values are 8bit binary format
  //    qDebug() << "ElContactSize:" << specimen->contact1.size();

  //    if(specimen->contact1.size() > 10)
  //    {
  //        for (uint l = 8; l < specimen->samples; l++)
  //        {
  //            if ((specimen->contact1.at (l) & 1) and !sw_1_state)
  //            {
  //                sw_1_state = true;
  //                qDebug() << "sw_1 ON" << l -6;
  //                specimen->SW1_state.append(1);
  //                specimen->SW1_row.append (l -6);
  //                continue;
  //            }

  //            if ((specimen->contact1.at (l) & 2) and !sw_2_state)
  //            {
  //                sw_2_state = true;
  //                qDebug() << "sw_2 ON"<< l -6;
  //                specimen->SW2_state.append(1);
  //                specimen->SW2_row.append (l -6);
  //                continue;
  //            }

  //            if ((!(specimen->contact1.at (l) & 1)) and sw_1_state)
  //            {
  //                sw_1_state = false;
  //                qDebug() << "sw_1 OFF" << l-6;
  //                specimen->SW1_state.append(0);
  //                specimen->SW1_row.append (l -6);
  //                continue;
  //            }

  //            if ((!(specimen->contact1.at (l) & 2)) and sw_2_state)
  //            {
  //                sw_2_state = false;
  //                qDebug() << "sw_2 OFF" << l -6;
  //                specimen->SW2_state.append(0);
  //                specimen->SW2_row.append (l -6);
  //                continue;
  //            }
  //        }
  //        // ***
  //    }

  //    qDebug () << "final: " << specimen->name <<
  //    specimen->InterestPointsList;

  // append the specimen to the serie
  //    this->specimenList.append(*specimen);

  //    // clear the lists; maybe not needed
  //    specimen->InterestPointsList.clear ();
  //    specimen->IP_TravelList.clear ();
  //    specimen->IP_ForceList.clear ();
  //    specimen->sequence.clear ();

  //    specimen->SW1_state.clear ();
  //    specimen->SW1_row.clear ();
  //    specimen->SW2_state.clear ();
  //    specimen->SW2_row.clear ();

  upconcave.clear();
  upconvexe.clear();
  downconcave.clear();
  downconvexe.clear();

  //    timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << "interest point found in ms" << timestamp2 - timestamp1;

  this->replace(pVector);
  return;
}

//

/*
 *
 *
 *
void hapticSerie::findHapticPoints (){
//    QMap<QString, QPointF> hapticPoints;
    qreal differential_factor = 0.25;
    qreal left_cont_rising_force = 0.5;
    qreal misuse_force_treshold = 50.0;
    qreal bendfactor_treshold = 0.85;
    qreal max_force_cutoff_factor = 0.9;
    qreal low_points_count_max_force_cutoff_factor = 0.8; // this may be
hardcoded qreal overtravel_termen = 0.0; qreal radius_factor = 0.33;


    //    qDebug() << Qt::endl  << Qt::endl << "Searching interest points in "
<< specimen->name << specimen->samples << " measurements";
    //    int timestamp1 = QDateTime::currentMSecsSinceEpoch();

    // *** find the approach distance, before the actuator touches the sample
    qreal approachDistance = 0;

 auto test3 = this->pointsVector ();



qDebug () << "test: " << test << test2;
    int start = 8;
    int end = 9;
    while (this->at (end).y () - this->at (start).y () <
left_cont_rising_force){
        //find a continuously rising slope of at least 0.5N

        if (this->at (end + 1).y () > this->at (end).y ()){
            // if rising, move the end of the range forward
            // the checking could be made at an interval greater that 1 if the
serie is very noisy or have a huge number of points end++; }else{
            //reset the range ends at current position
            start = end;
            end += 1;
        }
    }

    approachDistance = this->at (start + 2).x ();


    // apply the approach distance offset and find the max force value and
location qreal maxForce {0.0}; int maxForceLocation{0};

//    qDebug() << this->name () << "approachDistance" << approachDistance;

    for (int i = 0; i < this->points ().size (); i++){
        this->replace (i, this->at (i).x () - approachDistance, this->at (i).y
()); if(this->at (i).y () > maxForce){ maxForce = this->at (i).y ();
                maxForceLocation = i;
            }
    }

    hapticPoints.insert ("max force", this->at (maxForceLocation));

    // if the max force is huge, most probably this is a misuse test, so
concav/convex points are not relevant if (maxForce > misuse_force_treshold) {
        return;
    }


    // *** find the left bound for the upper curve
    start = end; // start from the approaching distance
    end = start + 1;
    while (this->at(start).y () <= this->at (end).y ()){
        // and move forward while the force is not falling
        end++;
        start++;
    }
    int leftUpperBound = end - 15; // come back a bit before the edge
    // ***


    // interest points must have at least 10% lower force.
    // 85% is better, but there are many curves for defective buttons with do
not respect this qreal maxInterestPointsForce = maxForce *
max_force_cutoff_factor;

    //find max pressing (Higher) curve Interest Points Force Location

    // start from maximum force point
    int maxHInterestPointsForceLocation = maxForceLocation;
    while ((this->at (maxHInterestPointsForceLocation).y () >
maxInterestPointsForce)){ maxHInterestPointsForceLocation--; // go back while
force is high
    }


    // *** find the right bound for the pressing (higher) curve
    start = maxHInterestPointsForceLocation -2; // start from the max force
location end = maxHInterestPointsForceLocation; while (this->at (start).y () <
this->at (end).y ()){
        // go back while the force is falling
        start--;
        end--;
    }
    int rightUpperBound = start + 15 ;
//    qDebug() << "maxHInterestPointsForceLocation" <<
this->at(maxHInterestPointsForceLocation);
    // ***


    // *** find max Interest Points Force Location for the lower curve
    // start at max force location
    int  maxLInterestPointsForceLocation = maxForceLocation;
    while (this->at (maxLInterestPointsForceLocation).y() >
maxInterestPointsForce){
        // go forward while the force is high
        maxLInterestPointsForceLocation++;
    }
    // ***


    // *** find the right bound for the lower curve
    start = maxLInterestPointsForceLocation;
    end = maxLInterestPointsForceLocation +1;
    while (this->at (start).y() > this->at (end).y()){
        end++;
        start++;
    }
    int rightLowerBound = start - 15;
    // ***

    // *** find the lower curve left bound
    start = this->pointsVector().size() -1; // start from the vector end
    end = start - 2;

    while (this->at (end).y() - this->at (start).y() < 0.2){
        // find a rising force slope of 0.3N
        if (this->at (end - 1).y() > this->at (end).y()){
            end--;
        }else{
            start = end;
            end -= 1;
        }
    }

    start = end; // from the 0.3N slope end
    end = start - 1;
    while (this->at (end).y() >= this->at (start).y())
    {
        // go up as long as the force is not falling
        end--;
        start--;
    }
    int leftLowerBound = end + 30; // + 15
    // ***

    // *** find a radius in which to evaluate the peaks for higher and lower
curves
    // interestPointsRadiusL = bounded range * factor (0.2 >= factor <= 1)
    qreal interestPointsRadiusL = 0;
    qreal interestPointsRadiusL_HigerCurve =
            (this->at (rightUpperBound).x() - this->at (leftUpperBound).x()) *
radius_factor;

    qreal interestPointsRadiusL_LowerCurve =
            (this->at (rightLowerBound).x() - this->at (leftLowerBound).x()) *
radius_factor;


    interestPointsRadiusL = interestPointsRadiusL_HigerCurve;

    //if (interestPointsRadiusL < 0.02) interestPointsRadiusL = 0.02; // !

    // ***

//    qDebug() << this->name() << "leftLowerBound" <<
this->at(leftLowerBound).x()
//             <<  "leftUpperBound" << this->at(leftUpperBound).x() << Qt::endl
//              << "rightLowerBound" << this->at(rightLowerBound).x()
//              <<  "rightUpperBound" << this->at (rightUpperBound).x() <<
Qt::endl
//               <<  "interestPointsRadiusL" << interestPointsRadiusL;


    qreal bendfactor = 0.0;

    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
    //    qDebug() << "interest point prep in ms" << timestamp2 - timestamp1;

    // prepare some lists to store the interest points location
    QList<int> upconcave;
    QList<int> upconvexe;
    QList<int> downconvexe;
    QList<int> downconcave;

    int backBound = 0;
    int forwardBound = 0;


    // ** search for haptic points in the valid range
    for (int l = leftUpperBound; l < leftLowerBound; l++)
    {
        //in mech stop area //orig: ±10; optimized for low res Tunis files
        if(l > rightUpperBound + 3 and l < rightLowerBound - 3){
            l = rightLowerBound - 3;
            continue;
        }
        //if (specimen->force.at(l) < 0.5) continue; //noise

        if (this->at(l).y() >= maxInterestPointsForce) continue;

        // ultra low point count serie workaround (some Tunis files)
        if ((this->pointsVector().size () < 1000)
                and (this->at(l).y() >= (maxInterestPointsForce *
low_points_count_max_force_cutoff_factor))) continue;



        // check if local convex peak
        if(this->at (l).y() > this->at (l+3).y() and
                this->at (l).y() > this->at (l-3).y()){
            // choose the upper\lower radius
            if (l < rightUpperBound +3) interestPointsRadiusL =
interestPointsRadiusL_HigerCurve;//10 if (l > rightLowerBound -3)
interestPointsRadiusL = interestPointsRadiusL_LowerCurve;//10

            // *** find the bounds in which to evaluate the peak
            backBound = 0;
            forwardBound = 0;
            for (int j = l; j < this->pointsVector().size (); ++j){
                // if reached the max force location, set the bound there
                if (j == maxHInterestPointsForceLocation){
                    forwardBound = j;
                    break;
                }

                // if reached the force changed with differential_factor
(default: 0.25)N, set the bound there if (abs((this->at (l).y() - this->at
(j).y())) > differential_factor){ forwardBound = j;    // break;
                }

                // if reached the radius distance, set the bound there
                if (    abs(this->at (j).x() - this->at (l).x())
                        > interestPointsRadiusL){
                    forwardBound = j;
                    break;
                }
            }

            // error, continue
            if (forwardBound == 0) continue;

            for (int j = l; j > 0; --j){
                if (j == maxLInterestPointsForceLocation){
                    backBound = j;
                    break;
                }

                if (abs((this->at (l).y() - this->at (j).y()))
                        > differential_factor){
                    backBound = j;
                    break;
                }

                if (abs(this->at (l).x() - this->at (j).x())
                        > interestPointsRadiusL){
                    backBound = j;
                    break;
                }
            }
            // ***



//            qDebug ()<< " local convex" << this->at (l).y() << this->at
(l).x()
//                     << "backBound" << this->at (backBound).x()
//                     << "forwardBound" << this->at (forwardBound).x();

            // check if the current point is maximum in range
            qreal current_point_force = this->at (l).y();
            bool is_current_point_max = true;
            for (int i = backBound; i <= forwardBound; i++){
                if (this->at(backBound).y() > current_point_force){
                    is_current_point_max = false;
                    break;
                }
            }

            if (is_current_point_max){

                // the ration between the sum of change in force (Dy) from the
point to both extremes
                // and the range length (x)

                bendfactor = abs(
                            (this->at(forwardBound).y() - abs(this->at(l).y()))
+ (this->at(backBound).y() - abs(this->at(l).y())) ) / interestPointsRadiusL;

                //                bendfactor = abs(
                //                            abs((this->at(forwardBound).y() -
abs(this->at(l).y()))) +
                //                            abs((this->at(backBound).y() -
abs(this->at(l).y())))
                //                            ) / interestPointsRadiusL;


//                qDebug () << this->name() << "convex point found: " <<
this->at (l).x()
//                          << "mm" << this->at (l).y()
//                          << "N" << " point on row nr: "
//                          << l - 8 + 2
//                          << "bend factor:" << bendfactor
//                          << "f at leftUpperBound" <<
this->at(leftUpperBound).y()
//                          << "f at rightUpperBound" <<
this->at(rightUpperBound).y()
//                          << (l < rightUpperBound +3) <<  (l > rightLowerBound
-3)
//                             ;

                if (bendfactor < bendfactor_treshold) continue; //haptic
imperceptible

                if (l < rightUpperBound +3) upconvexe.append (l);//10
                if (l > rightLowerBound -3) downconvexe.append (l);//10

                l = forwardBound - ((forwardBound - l)/2); // move the cursor
half the forward bound length

                //                 qDebug () << specimen->name << "la convex: "
<< upconvexe << upconcave << downconvexe << downconcave;

                continue;
            }
        }

        // check if local concav peak, and apply the same startegy to find the
interest points if(this->at (l).y() < this->at (l+3).y() and this->at (l).y() <
this->at (l-3).y()){  //local concav peak

            if (l < rightUpperBound +10) interestPointsRadiusL =
interestPointsRadiusL_HigerCurve; if (l > rightLowerBound -10)
interestPointsRadiusL = interestPointsRadiusL_LowerCurve;

            backBound = 0;
            forwardBound = 0;

            for (int j = l; j < this->pointsVector().size (); ++j){
                if (j == maxHInterestPointsForceLocation + 10){
                    forwardBound = j;
                    break;
                }

                if (abs((this->at (l).y() - this->at (j).y()))
                        > differential_factor){
                    forwardBound = j;
                    break;
                }

                if (abs(this->at (j).x() - this->at (l).x())
                        > interestPointsRadiusL){
                    forwardBound = j;
                    break;
                }
            }


            if (forwardBound == 0) break;

            for (int j = l; j > 0; --j){
                if (j == maxLInterestPointsForceLocation -10){
                    backBound = j;
                    break;
                }

                if (abs((this->at (l).y() - this->at (j).y()))
                        > differential_factor){
                    backBound = j;
                    break;
                }

                if (abs(this->at (l).x() - this->at (j).x())
                        > interestPointsRadiusL){
                    backBound = j;
                    break;
                }
            }

            //            qDebug () << " local concav"
            //                      << "backBound" << specimen->travel.at
(backBound)
            //                      << "forwardBound" << specimen->travel.at
(forwardBound);


            // check if the current point is minimum in range
            qreal current_point_force = this->at (l).y();
            bool is_current_point_min = true;
            for (int i = backBound; i <= forwardBound; i++){
                if (this->at(backBound).y() < current_point_force){
                    is_current_point_min = false;
                    break;
                }
            }


            if (is_current_point_min){

                bendfactor = abs(
                            (this->at(forwardBound).y() - abs(this->at(l).y()))
+ (this->at(backBound).y() - abs(this->at(l).y())) ) / interestPointsRadiusL;


//                qDebug () << this->name() <<  "concav point found: " <<
this->at (l).x()
//                          << "mm" << this->at (l).y()
//                          << "N" << " point on row nr: "
//                          << l
//                          << "bend factor:" << bendfactor;

                if (bendfactor < bendfactor_treshold) continue;

                if (l < rightUpperBound +10) upconcave.append (l);
                if (l > rightLowerBound -10) downconcave.append (l);

                // move the cursor half the forward bound length
                l  =  forwardBound - ((forwardBound - l)/2);

            }
        }
    }

//    qDebug () << this->name() << "initial: " << upconvexe << upconcave <<
downconcave << downconvexe;




    // ** sort the found points

    // sort the convex points on the upper curve
    if (upconvexe.size() > 1){
        // double detent, set the !! first two !! convex points their values.
points >2 ignored! hapticPoints.insert ("convex push det1",
this->at(upconvexe.at(0))); hapticPoints.insert("convex push det2" ,this->at
(upconvexe.at(1))); }else if (upconvexe.size() == 1){ hapticPoints.insert
("convex push det1", this->at(upconvexe.at(0))); }else if (upconvexe.size() ==
0)
    {

    }


    // same strategy applied for sorting the other points

    if (upconcave.size() > 1){
        hapticPoints.insert ("concav push det1", this->at (upconcave.at(0)));
        hapticPoints.insert ("concav push det2", this->at (upconcave.at(1)));
    }
    else if (upconcave.size() == 1){
        hapticPoints.insert ("concav push det1", this->at (upconcave.at(0)));
    }
    else if (upconcave.size() == 0)
    {

    }



    if ((downconvexe.size() > 1)  and (upconvexe.size() > 1) ){
        hapticPoints.insert ("convex release det1", this->at
(downconvexe.at(0))); hapticPoints.insert ("convex release det2", this->at
(downconvexe.at(1)));
    }
    else if (downconvexe.size() == 1){
        hapticPoints.insert ("convex release det1", this->at
(downconvexe.at(0)));
    }
    else if (downconvexe.size() == 0)
    {

    }



    if ((downconcave.size() > 1) and (upconvexe.size() > 1)){
        hapticPoints.insert ("concav release det1", this->at
(downconcave.at(0))); hapticPoints.insert ("convav release det2", this->at
(downconcave.at(1)));
    }
    else if (downconcave.size() == 1){
        hapticPoints.insert ("concav release det1", this->at
(downconcave.at(0)));
    }
    else if (downconcave.size() == 0)
    {

    }

    else if (downconcave.size() > 1){
        // There are more that one convave points on the release curve,
        // but only one convex point was found on the push curve.
        // This cannot be a double detent, so we consider only the last concav
point found (left most) hapticPoints.insert ("concav release det1", this->at
(downconcave.last()));
    }

    // ***


    // *** find the overtravel point
    //(projection to the right from the last convex point + overtravel_termen)
    for (int j = upconcave.last (); j < this->points ().size (); ++j){
        if (j == maxForceLocation) break;

        if ((this->at (j).y() - this->at (upconvexe.last ()).y()) >
overtravel_termen ){ j -= 1; hapticPoints.insert ("overtravel", this->at (j));
            break;
        }
    }

//    qDebug() << "hapticPoints" << hapticPoints;

//    for (auto pointName : hapticPoints.keys ()){
//                    this->insertTooltip (hapticPoints.value (pointName),
pointName, Qt::red);
//    }

    return;

    /*
    // *** search for electrical contact changes for the first two switches.
    bool sw_1_state = false;
    bool sw_2_state = false;

    // the contact values are 8bit binary format
    qDebug() << "ElContactSize:" << specimen->contact1.size();

    if(specimen->contact1.size() > 10)
    {
        for (uint l = 8; l < specimen->samples; l++)
        {
            if ((specimen->contact1.at (l) & 1) and !sw_1_state)
            {
                sw_1_state = true;
                qDebug() << "sw_1 ON" << l -6;
                specimen->SW1_state.append(1);
                specimen->SW1_row.append (l -6);
                continue;
            }

            if ((specimen->contact1.at (l) & 2) and !sw_2_state)
            {
                sw_2_state = true;
                qDebug() << "sw_2 ON"<< l -6;
                specimen->SW2_state.append(1);
                specimen->SW2_row.append (l -6);
                continue;
            }

            if ((!(specimen->contact1.at (l) & 1)) and sw_1_state)
            {
                sw_1_state = false;
                qDebug() << "sw_1 OFF" << l-6;
                specimen->SW1_state.append(0);
                specimen->SW1_row.append (l -6);
                continue;
            }

            if ((!(specimen->contact1.at (l) & 2)) and sw_2_state)
            {
                sw_2_state = false;
                qDebug() << "sw_2 OFF" << l -6;
                specimen->SW2_state.append(0);
                specimen->SW2_row.append (l -6);
                continue;
            }
        }
        // ***
    }

    qDebug () << "final: " << specimen->name << specimen->InterestPointsList;

    // append the specimen to the serie
    this->specimenList.append(*specimen);

    // clear the lists; maybe not needed
    specimen->InterestPointsList.clear ();
    specimen->IP_TravelList.clear ();
    specimen->IP_ForceList.clear ();
    specimen->sequence.clear ();

    specimen->SW1_state.clear ();
    specimen->SW1_row.clear ();
    specimen->SW2_state.clear ();
    specimen->SW2_row.clear ();

    upconcave.clear ();
    upconvexe.clear ();
    downconcave.clear ();
    downconvexe.clear ();

    timestamp2 = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "interest point found in ms" << timestamp2 - timestamp1;



}
//

*/

double hapticSerie::fast_atof(const char *num, bool *ok) {
  if (!num || !*num) {
    *ok = false;
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
        *ok = false;
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
        *ok = true;
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

  *ok = true;
  return sign * (integerPart + fractionPart) * expPart;
}

double hapticSerie::pow10(int n) {
  double ret = 1.0;
  double r = 10.0;
  if (n < 0) {
    n = -n;
    r = 0.1;
  }

  while (n) {
    if (n & 1) {
      ret *= r;
    }
    r *= r;
    n >>= 1;
  }
  return ret;
}

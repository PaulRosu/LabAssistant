#include "dataloader.h"
#include "qglobal.h"

dataLoader::dataLoader(QString path) {
  qDebug() << "dataLoader" << path;
  this->path = path;

  QFileInfo fi(path);

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
           << Qt::endl;

  if (!fi.exists()) {
    qDebug() << "Inexistent location:" << fi;
    this->good = false;
  }

  if (fi.isDir()) {
    auto dir = QDir(fi.absoluteFilePath());
    dir.setSorting(QDir::Time); // QDir::LocaleAware

    QStringList files = dir.entryList(QStringList() << "*.txt\0"
                                                    << "*csv\0",
                                      QDir::Files);

    QCollator collator;
    collator.setNumericMode(true);
    std::sort(files.begin(), files.end(), collator);

    qDebug() << "files in dir:" << fi.dir() << files;

    QMap<QString, QString> filesMap;
    for (auto file : files)
      filesMap.insert(file, fi.filePath() + '\\' + file);

    this->append_Mode = true;
    for (auto file : filesMap.values()) {
      qDebug() << checkDataType(file);

      qDebug() << parseLabViewLog(file);
    }
    this->good = true;
  } else if (fi.isFile()) {

    auto type = checkDataType(fi.absoluteFilePath());
    if (type == dataType::Durability_robot) {
      if (parseRobotLog(fi.absoluteFilePath()) > 0) {
        findHapticPoints();
        this->good = true;
      } else
        this->good = false;
    } else if (type == dataType::BLF_Export) {
      if (parseBLFExport(fi.absoluteFilePath()) > 0) {
        //                findHapticPoints ();
        this->good = true;
      } else
        this->good = false;

    } else if (type == dataType::Zwick) {
      if (parseZwickFile(fi.absoluteFilePath()) > 0) {
        findHapticPoints();
        this->good = true;
      } else
        this->good = false;

    } else if (type == dataType::LabView) {
      if (parseLabViewLog(fi.absoluteFilePath()) > 0) {

        this->good = true;
      } else
        this->good = false;

    }

    else if (type == dataType::KWM) {
      if (parseKWM(fi.absoluteFilePath()) > 0) {
        findHapticPoints();
        this->good = true;
      } else
        this->good = false;

    }

    else if (type == dataType::SYSTEC) {
      if (parseSYSTEC(fi.absoluteFilePath()) > 0) {
        findHapticPoints();
        this->good = true;
      } else
        this->good = false;
    }
  }

  //    QFile file(path);
  //    if(!file.open(QIODevice::ReadOnly))
  //    {
  //        qDebug() << "Could not open " << path;
  //    }

  //    int fileSize =  file.size();
  //    qDebug() << "file size:" << fileSize;
  qDebug() << "return from dataloader";
}

dataLoader::dataType dataLoader::checkDataType(QString s_file) {
  auto file = QFile(s_file);

  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path << s_file;
    return Undefined;
  }

  QDir dir = QFileInfo(path).absoluteDir();
  QString absdir = dir.absolutePath();
  QFile config_file(absdir + "\\blfconfig.txt");
  if (config_file.exists()) {
    qDebug() << "dataType:"
             << "BLF_Export";
    file.close();
    this->type = BLF_Export;
    return BLF_Export;
  }

  auto firstLine = file.readLine(256);
  if (firstLine.contains("LabVIEW")) {
    qDebug() << "dataType:"
             << "LabVIEW";
    file.close();
    this->type = LabView;
    return LabView;
  } else if ((firstLine.contains("#RF2-MeasurementFile")) or
             (firstLine.contains("#Property"))) {
    qDebug() << "dataType:"
             << "Durability_robot";
    file.close();
    this->type = Durability_robot;
    return Durability_robot;
  } else if (firstLine.contains("Touch_Capacitance")) {
    qDebug() << "dataType:"
             << "BLF_Export";
    file.close();
    this->type = BLF_Export;
    return BLF_Export;
  } else if (firstLine.contains("\x1F\x8B\b\x00\x00\x00\x00\x00\x00\n")) {

    qDebug() << "dataType:"
             << "Zwick ZS2 file";
    file.close();
    this->type = Zwick;
    return Zwick;

  } else if (firstLine.contains("\xEF\xBB\xBF\\BEGIN:{Test-Sequence}\r\n")) {
    qDebug() << "dataType:"
             << "KWM haptic file file";
    file.close();
    this->type = KWM;
    return KWM;

  } else if (firstLine.contains("factor;")) {
    qDebug() << "dataType:"
             << "SYSTEC haptic file file";
    file.close();
    this->type = SYSTEC;
    return SYSTEC;

  } else {
    qDebug() << "unknown type:" << Qt::endl << firstLine;
    this->type = Undefined;
    file.close();
  }

  return Undefined;
}

int dataLoader::parseSYSTEC(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "SYSTEC file size:" << fileSize;

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

  QDataStream in(&file);

  QDir d = QFileInfo(path).absoluteDir();
  QString absolute = d.absolutePath();

  QString separator = ";";
  QString headerseparator;

  char ch;
  char carriage_return = '\r';
  char new_line = '\n';

  QString header, current_line;

  // seek trough the file until the header begin flag is found
  for (int i = 0; i < 60; i++) {
    while (!in.atEnd()) {
      in.readRawData(&ch, 1);
      current_line += ch;
      if (ch == carriage_return) {
        in.readRawData(&ch, 1);
        if (ch == new_line)
          break;
      }
    }
    if (current_line.contains("Smax;mm;"))
      break;
    qDebug() << "current_line" << current_line;
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

  //    for (int i = 0; i < headerlist.size (); i++){
  //        auto sl_parts = headerlist.at (i).split ("&");

  //        if (sl_parts.size () < 3) continue;

  //        QString temp_name = sl_parts.at (0);
  //        temp_name.replace ("[", "");
  //        headerlist.replace (i, temp_name + " [" + sl_parts.at (1) + "]");

  //        if (!has_data_config){
  //            if (sl_parts.at (1).contains ("Distance")){
  //                data_indexes.x_index.append (i);
  //            }
  //            if (sl_parts.at (0).contains ("Fzzzzzzz")){
  //                data_indexes.x_index.append (i);
  //                haptic_serie = headerlist.at (i);
  //            }
  //        }

  //    }

  //    if (has_data_config){
  //        if (haptic_serie.isEmpty ()) haptic_serie = headerlist.at
  //        (data_indexes.y_index.at (0));
  //    }

  if (!has_data_config) {

    //        data_indexes.x_index.append (1);

    for (int i = 0; i < headerlist.size(); i++) {
      if (headerlist.at(i).contains("Force")) {
        data_indexes.y_index.append(i);
        //                break;
      }

      if (headerlist.at(i).contains("Weg")) {
        data_indexes.x_index.append(i);
        //                break;
      }
    }

    if (haptic_serie.isEmpty())
      haptic_serie = headerlist.at(data_indexes.y_index.at(0));
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
          if (!ok) {
            skip_line = true;
          } else {

            if (this->x_dataMap[headerlist[word_count]] == nullptr)
              this->x_dataMap.insert(headerlist[word_count],
                                     new QVector<qreal>);

            if (ok)
              x_dataMap[headerlist[word_count]]->append(this_travel);
          }

        } else if (word_count == 0) {
          this_runningtime = fast_atof(word, &ok);
          if (!ok)
            skip_line = true;

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
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_voltage = fast_atof(word, &ok);
              if (ok)
                y_dataMap[headerlist[word_count]]->append(
                    QPointF(this_travel, this_voltage));
            }
          }

          for (auto index : data_indexes.y_index) {
            if (word_count == index) {
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_y = fast_atof(word, &ok);
              if (ok) {

                if (word_count == 12) {
                  this_y -= (this_travel * 0.0057666666);
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y * 0.328 * (1 / 0.085)));

                } else {
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y));
                }
              }
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

  qDebug() << "result container size:" << y_dataMap.size();
  qDebug() << "first serie size:" << y_dataMap.first()->size();

  qDebug() << "hapticPoints on file load:" << hapticPoints;

  file.close();
  return 1;
}

int dataLoader::parseKWM(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "KWM file size:" << fileSize;

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

  QDataStream in(&file);

  QDir d = QFileInfo(path).absoluteDir();
  QString absolute = d.absolutePath();

  QString separator = ";";
  QString headerseparator;

  char ch;
  char carriage_return = '\r';
  char new_line = '\n';

  QString header, current_line;

  // seek trough the file until the header begin flag is found
  for (int i = 0; i < 60; i++) {
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
    qDebug() << "current_line" << current_line;
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

    QString temp_name = sl_parts.at(0);
    temp_name.replace("[", "");
    headerlist.replace(i, temp_name + " [" + sl_parts.at(1) + "]");

    if (!has_data_config) {
      if (sl_parts.at(1).contains("Distance")) {
        data_indexes.x_index.append(i);
      }
      if (sl_parts.at(0).contains("Fzzzzzzz")) {
        data_indexes.x_index.append(i);
        haptic_serie = headerlist.at(i);
      }
    }
  }

  if (has_data_config) {
    if (haptic_serie.isEmpty())
      haptic_serie = headerlist.at(data_indexes.y_index.at(0));
  }

  if (!has_data_config) {

    data_indexes.x_index.append(1);

    for (int i = 0; i < headerlist.size(); i++) {
      if (headerlist.at(i).contains("Kraft")) {
        data_indexes.y_index.append(i);
        break;
      }
    }

    if (haptic_serie.isEmpty())
      haptic_serie = headerlist.at(data_indexes.y_index.at(0));
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
          if (!ok) {
            skip_line = true;
          } else {

            if (this->x_dataMap[headerlist[word_count]] == nullptr)
              this->x_dataMap.insert(headerlist[word_count],
                                     new QVector<qreal>);

            if (ok)
              x_dataMap[headerlist[word_count]]->append(this_travel);
          }

        } else if (word_count == 0) {
          this_runningtime = fast_atof(word, &ok);
          if (!ok)
            skip_line = true;

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
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_voltage = fast_atof(word, &ok);
              if (ok)
                y_dataMap[headerlist[word_count]]->append(
                    QPointF(this_travel, this_voltage));
            }
          }

          for (auto index : data_indexes.y_index) {
            if (word_count == index) {
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_y = fast_atof(word, &ok);
              if (ok) {

                if (word_count == 12) {
                  this_y -= (this_travel * 0.0057666666);
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y * 0.328 * (1 / 0.085)));

                } else {
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y));
                }
              }
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

  qDebug() << "result container size:" << y_dataMap.size();
  qDebug() << "first serie size:" << y_dataMap.first()->size();

  qDebug() << "hapticPoints on file load:" << hapticPoints;

  file.close();
  return 1;
}

int dataLoader::parseLabViewLog(QString path) {
  auto file = QFile(path);

  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return Undefined;
  }

  qDebug() << "parsing: " << path;

  QString separator = "\t";
  QString headerseparator;

  char ch;
  char carriage_return = '\r';
  char new_line = '\n';

  QString header, current_line;

  // seek trough the file until the header begin flag is found
  for (int i = 0; i < 30; i++) {
    while (!file.atEnd()) {
      file.getChar(&ch);
      current_line += ch;
      if (ch == carriage_return) {
        file.getChar(&ch);
        if (ch == new_line)
          break;
      }
    }
    if (current_line.contains("Time")) {
      qDebug() << "current_line" << current_line;
      header = current_line;
      break;
    }
    current_line.clear();
  }

  QList<QString> headerlist = header.split("\t");

  for (auto &header_element : headerlist) {
    header_element.replace(" ", "");
    header_element.replace("\r", "");
    if (header_element.isEmpty())
      headerlist.removeOne(header_element);
  }

  qDebug() << "Cured header:" << headerlist << Qt::endl;

  char word[256];
  int word_index = 0;
  int word_count = -1;

  bool skip_line = false;

  int res = 0;
  int curr_line = 0;

  double this_runningtime = 0.0;
  double this_voltage = 0;

  if (this->append_Mode)
    this_runningtime = this->initial_time;

  bool ok = false;

  QVector<int> samples_index_vector;
  for (int i = 3; i < headerlist.size(); i++)
    samples_index_vector << i;

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

        if (word_count == 0) {
          this_runningtime +=
              0.5 / 60.0 / 60.0 /
              24.0; // add half a second  //fast_atof(word, &ok);
          // if(!ok) skip_line = true;

        } else {
          for (auto index : samples_index_vector) {
            if (word_count == index) {

              if (index == 3) {

                if (y_dataMap[headerlist[word_count]] == nullptr)
                  y_dataMap.insert(headerlist[word_count],
                                   new QVector<QPointF>);

                if (y_dataMap["Min_Tolerance"] == nullptr)
                  y_dataMap.insert("Min_Tolerance", new QVector<QPointF>);

                if (y_dataMap["Max_Tolerance"] == nullptr)
                  y_dataMap.insert("Max_Tolerance", new QVector<QPointF>);

                this_voltage = fast_atof(word, &ok);

                if (ok) {

                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_runningtime, this_voltage));

                  y_dataMap["Max_Tolerance"]->append(
                      QPointF(this_runningtime, 0.58 * this_voltage + 1.23));
                  y_dataMap["Min_Tolerance"]->append(
                      QPointF(this_runningtime, 0.52 * this_voltage + 1.02));
                }
              } else {

                if (y_dataMap[headerlist[word_count] + "_" +
                              QString::number(index - 3)] == nullptr)
                  y_dataMap.insert(headerlist[word_count] + "_" +
                                       QString::number(index - 3),
                                   new QVector<QPointF>);

                this_voltage = fast_atof(word, &ok);

                if (ok)
                  y_dataMap[headerlist[word_count] + "_" +
                            QString::number(index - 3)]
                      ->append(QPointF(this_runningtime, this_voltage));
              }
            }
          }
        }

        word_index = 0;
      }

    } else {
      if (ch != ' ') { // skip white spaces
        word[word_index] = ch;
        word_index += 1;
      }
    }

    if (ch == new_line) {
      word_index = 0;
      word_count = -1;
      curr_line++;
      skip_line = false;
    }
  }
  this->initial_time = this_runningtime;

  qDebug() << "result container size:" << y_dataMap.size();
  qDebug() << "first serie size:" << y_dataMap.first()->size();

  //    qDebug() << dataMap[headerlist[3]]->at (1);

  file.close();

  return 1;
}

int dataLoader::parseRobotLog(QString path) {
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

      auto header_line = txtin.readLine(); // read out the header
      qDebug() << "header_line:" << header_line;

      while (!txtin.atEnd()) {
        QString line = txtin.readLine();
        if (line.isEmpty())
          continue;

        line.replace(' ', "");
        line.replace('\t', "");

        qDebug() << "config line:" << line;

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
        } else if (line_vector.size() >= 3) {
          if (line_vector.at(0) == 'X') {
            qDebug() << "adding x index..." << line_vector.at(1).toInt();
            data_indexes.x_index.append(line_vector.at(1).toInt());
          }

          if (line_vector.at(0) == 'Y') {
            qDebug() << "adding y index..." << line_vector.at(1).toInt();
            data_indexes.y_index.append(line_vector.at(1).toInt());
          }
        }

        line_vector.clear();
      }
      bus_config_file.close();
    }
    qDebug() << "BUS_CONFIG : " << bus_config_vector.size();
    qDebug() << "data_indexes: " << data_indexes.x_index
             << data_indexes.y_index;
  } else {
    qDebug() << "BUS_CONFIG file not found!";
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

    QString temp_name = sl_parts.at(0);
    temp_name.replace("[", "");
    headerlist.replace(i, temp_name + " [" + sl_parts.at(1) + "]");

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
        haptic_serie = headerlist.at(i);
      }
    }
  }

  if (has_data_config) {
    if (haptic_serie.isEmpty())
      haptic_serie = headerlist.at(data_indexes.y_index.at(0));
  }

  if (!has_data_config) {

    data_indexes.x_index.append(1);

    for (int i = 0; i < headerlist.size(); i++) {
      if (headerlist.at(i).contains("Fabs")) {
        data_indexes.y_index.append(i);
        break;
      }
    }

    if (haptic_serie.isEmpty())
      haptic_serie = headerlist.at(data_indexes.y_index.at(0));
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
          if (!ok) {
            skip_line = true;
          } else {

            if (this->x_dataMap[headerlist[word_count]] == nullptr)
              this->x_dataMap.insert(headerlist[word_count],
                                     new QVector<qreal>);

            if (ok)
              x_dataMap[headerlist[word_count]]->append(this_travel);
          }

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
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_voltage = fast_atof(word, &ok);
              if (ok)
                y_dataMap[headerlist[word_count]]->append(
                    QPointF(this_travel, this_voltage));
            }
          }

          for (auto index : data_indexes.y_index) {
            if (word_count == index) {
              if (this->y_dataMap[headerlist[word_count]] == nullptr)
                this->y_dataMap.insert(headerlist[word_count],
                                       new QVector<QPointF>);

              qreal this_y = fast_atof(word, &ok);
              if (ok) {

                if (word_count == 12) {
                  this_y -= (this_travel * 0.0057666666);
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y * 0.328 * (1 / 0.085)));

                } else {
                  y_dataMap[headerlist[word_count]]->append(
                      QPointF(this_travel, this_y));
                }
              }
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

  qDebug() << "result container size:" << y_dataMap.size();
  qDebug() << "first serie size:" << y_dataMap.first()->size();

  qDebug() << "hapticPoints on file load:" << hapticPoints;

  //    qDebug() << durability_signals[headerlist[15]];

  //    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    //    qDebug() << timestamp2 - timestamp1;
  file.close();
  return 1;
}

int dataLoader::parseZwickFile(QString path) {

  QFuture<MeasSerie> future =
      QtConcurrent::run([&](QString path) { return loadData(path); }, path);

  while (!future.isFinished())
    QApplication::processEvents();

  MeasSerie serie = future.result();

  qDebug() << "serie size:" << serie.specimenList.size();

  if (!serie.status) {
    qDebug() << "Error:"
             << "The file at\n" + path + "\ncould not be loaded!";
    return -1;
  }

  this->serieName = serie.name;
  this->folder = serie.folder;
  this->path = path;

  auto travelVector = new QVector<qreal>;

  for (uint l = 12; l < serie.specimenList.at(0).samples; l++) {
    travelVector->append((qreal)serie.specimenList.at(0).travel.at(l));
  }

  this->x_dataMap.insert("Travel", travelVector);

  for (MeasSpecimen specimen : serie.specimenList) {
    auto forceVector = new QVector<QPointF>;

    for (uint l = 12; l < specimen.samples; l++) {
      forceVector->append(
          QPointF((qreal)specimen.travel.at(l), (qreal)specimen.force.at(l)));

      // travel serie for normal tests
      //->append ((qreal)specimen.travel.at (l) - approachDistance, (qreal)
      // specimen.force.at (l));
      // time serie for misuse tests
      // series->append ((qreal)serie.time.at (l) - serie.time.at
      // (serie.approachRow), (qreal) serie.force.at (l));
    }

    this->y_dataMap.insert(specimen.name, forceVector);
  }

  return 1;
}

int dataLoader::parseBLFExport(QString path) {

  qDebug() << "entering parse blf export...";
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Could not open " << path;
    return -1;
  }

  int fileSize = file.size();
  qDebug() << "file size:" << fileSize;

  QDir dir = QFileInfo(path).absoluteDir();
  QString absdir = dir.absolutePath();
  QFile config_file(absdir + "\\blfconfig.txt");

  //    QList<QString> series_names;
  //    QList<QVector<QPointF>> series_data;
  //    QString X_name;
  //    QMap <QString, int> Y_axes_series_correlation;

  int x_index;
  int y_indexes[256];
  std::fill_n(y_indexes, 256, -1);
  QList<QString> Y_header_tokens;
  QString X_token;

  int header_index = 0;

  QRegularExpression rx_name("\"(.*)\"");
  QRegularExpression rx_token("%(.*)%");

  if (config_file.exists()) {
    if (!config_file.open(QIODevice::ReadOnly)) {
      qDebug() << "blfconfig.txt File Open Failed!";
    } else {
      QTextStream txtin(&config_file);
      QList<QString> line_list;

      QString first_line = txtin.readLine();
      qDebug() << "first_line" << first_line << absdir + "\\blfconfig.txt";

      while (!txtin.atEnd()) {
        QString line = txtin.readLine();
        if (line.isEmpty())
          continue;

        line_list = line.split(";");

        if (line_list.size() >= 5) {
          // data process params detected
          qDebug() << "line_list size over 5!:" << line_list;
        } else if (line_list.size() >= 3) {
          if (line_list.at(0) == 'X') {
            this->X_name = line_list.at(1);
            X_token = line_list.at(2);

            QRegularExpressionMatch match = rx_token.match(X_token);
            if (match.hasMatch()) {
              qDebug() << "rx_token" << match.captured(1);
              X_token = match.captured(1);
            }
            qDebug() << "X_token" << X_token;
          }

          if (line_list.at(0).contains('Y')) {
            auto s_name = line_list.at(1);

            QRegularExpressionMatch match = rx_name.match(s_name);
            if (match.hasMatch()) {
              this->series_names.append(match.captured(1));
            }

            auto token = line_list.at(2);

            match = rx_token.match(token);
            if (match.hasMatch()) {
              Y_header_tokens.append(match.captured(1));
            }
          }
        }
        line_list.clear();
      }
      config_file.close();
    }
  }

  qDebug() << "Y_axes_series_correlation" << Y_axes_series_correlation;
  qDebug() << "series_names" << series_names;
  qDebug() << "Y_header_tokens" << Y_header_tokens;

  // return -1;

  QDataStream in(&file);

  QDir d = QFileInfo(path).absoluteDir();
  QString absolute = d.absolutePath();

  QString separator = ",";
  QString headerseparator;

  char ch;
  char carriage_return = '\r';
  char new_line = '\n';

  QString header, current_line;

  auto taste = file.readLine(64);
  int lfcounter = 0;
  bool DMM_export = false;
  if (taste.contains("Style,Standard")) {
    DMM_export = true;
    while (!in.atEnd()) {
      in.readRawData(&ch, 1);
      if (ch == new_line)
        lfcounter += 1;
      if (lfcounter == 7)
        break;
    }
    auto fileposition = file.pos();
    auto taste = file.readLine(255);
    taste = file.readLine(255);
    series_names[0] = "Meas. " + taste.split(',').at(1);
    file.seek(fileposition);
  } else {
    file.seek(0);
  }

  if (taste.contains(';')) {
    separator = ';';
  } else if (taste.contains(',')) {
    separator = ',';
  } else if (taste.contains(char(9))) {
    separator = char(9);
  }

  qDebug() << "separator:" << separator;

  // get the header
  while (!in.atEnd()) {
    in.readRawData(&ch, 1);

    if ((ch == new_line) or (ch == new_line)) {
      //            in.readRawData(&ch, 1);
      //            if ((ch == new_line) or (ch == new_line))
      //            {
      // qDebug() << "header:" << header << header.size () << Qt::endl;
      break;
      //            }
    } else {
      header += ch;
    }
  }

  qDebug() << "raw header:" << header;
  QList<QString> headerlist = header.split(separator);
  qDebug() << headerlist;

  // fill the header indexes array with the series names indexes
  for (int i = 0; i < headerlist.size(); i++) {
    for (int j = 0; j < Y_header_tokens.size(); j++) {
      if (headerlist.at(i).contains(Y_header_tokens.at(j))) {
        y_indexes[i] = j;
        //                qDebug() <<
        //                    "headerlist.at(" << i << ")" << headerlist.at(i)
        //                    << "Y_header_tokens.at(" << j << "))" <<
        //                    Y_header_tokens.at(j);
      }
    }
    if (headerlist.at(i).contains(X_token))
      x_index = i;
  }

  //    qDebug() << "Final header:" << headerlist
  qDebug() << "y_indexes:" << y_indexes;
  qDebug() << "series_names" << series_names;

  for (int i = 0; i < 255; i++) {
    if (y_indexes[i] > -1)
      qDebug() << "y_index:" << i << series_names.at(y_indexes[i]);
  }

  char word[256];
  int word_index = 0;
  int word_count = -1;
  bool skip_line = false;
  int res = 0;
  int curr_line = 0;

  double this_time = -1.0;
  double time_offset = 0.0;
  bool ok = false;
  double this_y = 0.0;
  double old_supply_v = 0.0;
  double old_HV_supply_U = 0.0;
  double old_HV_supply_I = 0.0;

  while (!file.atEnd()) {
    res = file.getChar(&ch);

    if (res < 1) { // file read error
      qDebug() << "File read error:" << res;
      return -1;
    }

    if (ch == '\0') { // probably memory leak in file during windows crash
      word_index = 0;
      word_count = -1;
      skip_line = true;
      continue;
    }

    if (word_index > 21) { // buffer overrun
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

        if (word_count == x_index) {

          // Bosch CVM modification

          if (this_time == -1.0) { //
            if (DMM_export == false) {
              this_time = fast_atof(word, &ok);
              if (ok) {
                time_offset = this_time / 60 / 60 / 24;
              }
            }

            this_time = 0.0;
          } else {
            if (DMM_export == false) {

              this_time = fast_atof(word, &ok);
              // qDebug() << "this_time" << this_time;
              if (ok) {
                this_time = this_time / 60 / 60 / 24;
                this_time -= time_offset;
              }

              //                        this_time = this_time +
              //                        0.0000115740740740741;//one second in
              //                        days
            } else {
              this_time = this_time + 0.0000115740740740741 / 10000;
            }
          }
        }

        // ref
        //     QList<QString> series_names;
        //     QList<QVector<QPointF>> series_data;
        //     QString X_name;
        //     QMap <QString, int> Y_axes_series_correlation;

        if (y_indexes[word_count] > -1) {
          if (this->y_dataMap[series_names.at(y_indexes[word_count])] ==
              nullptr)
            this->y_dataMap.insert(series_names.at(y_indexes[word_count]),
                                   new QVector<QPointF>);

          this_y = fast_atof(word, &ok);
          if (ok) {

            //                        if (this_y < 100000)

            if (series_names.at(y_indexes[word_count]).contains("Cell")) {
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, this_y / 1000));
            } else if (series_names.at(y_indexes[word_count])
                           .contains("Supply Voltage")) {

              if (series_names.at(y_indexes[word_count])
                      .contains("PackVoltage") or
                  series_names.at(y_indexes[word_count])
                      .contains("LinkVoltage")) {
                qDebug() << series_names.at(y_indexes[word_count]) << this_y;
                if (this_y > 0) {
                  y_dataMap[series_names.at(y_indexes[word_count])]->append(
                      QPointF(this_time, this_y));
                }
              } else {

                y_dataMap[series_names.at(y_indexes[word_count])]->append(
                    QPointF(this_time, old_supply_v));
                y_dataMap[series_names.at(y_indexes[word_count])]->append(
                    QPointF(this_time, this_y));
                old_supply_v = this_y;
              }
            } else if (series_names.at(y_indexes[word_count])
                           .contains("Sup1")) {
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, old_HV_supply_U));
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, this_y));
              old_HV_supply_U = this_y;
            } else if (series_names.at(y_indexes[word_count])
                           .contains("Sup2")) {
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, old_HV_supply_I));
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, this_y));
              old_HV_supply_I = this_y;
            } else {
              y_dataMap[series_names.at(y_indexes[word_count])]->append(
                  QPointF(this_time, this_y));
            }

            //                        else if
            //                        (series_names.at(y_indexes[word_count]).contains
            //                        ("Supply Voltage")){
            //                            y_dataMap[series_names.at(y_indexes[word_count])]->append(QPointF(this_time,
            //                            old_supply_v));
            //                            y_dataMap[series_names.at(y_indexes[word_count])]->append(QPointF(this_time,
            //                            this_y)); old_supply_v = this_y;
            //                        }
          }
        }
      }

      word_index = 0;

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

  for (const auto &y : y_dataMap.keys()) {
    qDebug() << y << y_dataMap.value(y)->size();
  }

  qDebug() << "result container size:" << y_dataMap.size();
  qDebug() << "first serie size:" << y_dataMap.first()->size();
  qDebug() << "first serie point 0:" << y_dataMap.first()->at(0);
  qDebug() << "first serie point 100:" << y_dataMap.first()->at(100);
  qDebug() << "first serie point 0:" << y_dataMap.first()->last();

  //    qDebug() << durability_signals[headerlist[15]];

  //    //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    //    qDebug() << timestamp2 - timestamp1;
  file.close();
  return 1;
}

void dataLoader::findHapticPoints() {

  qDebug() << "Intrat333:"
           << "findHapticPoints"; // this->name();

  //    QMap<QString, QPointF> hapticPoints;
  qreal differential_factor = 0.3;    // 0.25
  qreal left_cont_rising_force = 0.4; // 0.3
  qreal misuse_force_treshold = 50.0;
  qreal bendfactor_treshold = 0.5;                      // 0.85
  qreal max_force_cutoff_factor = 6;                    // 1
  qreal low_points_count_max_force_cutoff_factor = 0.8; // this may be hardcoded
  qreal overtravel_termen = 0.0;
  qreal radius_factor = 0.33; // 0.33

  //    qDebug() << Qt::endl  << Qt::endl << "Searching interest points in " <<
  //    specimen->name << specimen->samples << " measurements"; int timestamp1 =
  //    QDateTime::currentMSecsSinceEpoch();

  // *** find the approach distance, before the actuator touches the sample
  qreal approachDistance = 0;

  //    QMessageBox msgBox;

  //    msgBox.setWindowTitle("Mem test");
  //    msgBox.setText("Before:");
  //    msgBox.exec();

  //    auto pVector = this->pointsVector ();
  haptic_serie = y_dataMap.keys().at(0);
  QVector<QPointF> *pVector = nullptr;
  if (!haptic_serie.isEmpty()) {

    //         qDebug() << "incerc sa scot vectorul" << haptic_serie;
    //         qDebug() << "data map keys:" << dataMap.keys ();

    pVector = y_dataMap[haptic_serie];

  } else {
    qDebug() << "WARNING! haptic_serie name is EMPTY!";
    return;
  }

  //        qDebug() << "dataMap[haptic_serie] size:" << pVector->size ();

  //    qDebug() << "pe aici";
  //    msgBox.setWindowTitle("Mem test");
  //    msgBox.setText("After:");
  //    msgBox.exec();

  //    if (pVector->size () < 2000) radius_factor = 0.3;

  int start = 0; // fix for claudius initial = 8
  int end = start + 1;
  while ((pVector->at(end).y() - pVector->at(start).y()) <
         left_cont_rising_force) {
    // find a continuously rising slope of at least 0.5N

    if (end >= (pVector->size() - 30))
      break;
    //        qDebug() << "test: " << start << end;

    if (pVector->at(end + 3).y() > pVector->at(end).y()) {
      // if ((pVector->at (end).y () - pVector->at (end + 5).y ()) > 0.005){//3
      // if rising, move the end of the range forward
      // the checking could be made at an interval greater that 1 if the serie
      // is very noisy or have a huge number of points
      end++;
    } else {
      // reset the range ends at current position
      start = end;
      end += 1; // 3
    }
  }

  if (end >= pVector->size() - 30) {
    approachDistance = 0.01;
    this->approachrow = 30;
    start = 30;
    end = 31;
    qDebug() << "approachDistance" << approachDistance << "approachrowwwwwwww"
             << approachrow;

  } else {
    approachDistance =
        pVector->at(start).x(); // fix for claudius, remove (+5) from start
    this->approachrow = start;
    qDebug() << "approachDistance" << approachDistance << "approachrow"
             << approachrow;
  }

  //    //Apply preload
  //    for (int i = 0; i < pVector->size (); i++){
  //        if (pVector->at(i).y () >= 0.5){
  //        start = i;
  //        break;
  //        }
  //    }

  //        approachDistance = pVector->at (start + 5).x ();
  //        this->approachrow = start + 5;
  //    //

  // apply the approach distance offset and find the max force value and
  // location

  qreal maxForce{0.0};
  int maxForceLocation{0};

  //        qDebug() << this->name () << "approachDistance" << approachDistance;

  for (int i = 0; i < pVector->size(); i++) {
    pVector->replace(
        i, QPointF(pVector->at(i).x() - approachDistance, pVector->at(i).y()));
    if (pVector->at(i).y() > maxForce) {
      maxForce = pVector->at(i).y();
      maxForceLocation = i;
    }
  }

  //    qDebug() << "maxForceLocation" << maxForceLocation << "maxForce" <<
  //    maxForce;

  // also apply the approach offset to any contact/bus point found on load

  for (auto &point : this->hapticPoints) {
    point.setX(point.x() - approachDistance);
  }

  this->offset = approachDistance;

  // also apply the approach offset to any non force or torque type data
  //
  for (auto &data_vector : this->y_dataMap.values()) {

    // todo: handle systec analog points
    if (this->type == SYSTEC)
      break;

    QString s_name = this->y_dataMap.key(data_vector);
    if (!s_name.contains("[N")) {
      qDebug() << "analog points evaluation area" << s_name;
      bool old_state = false;
      int i = 0;
      for (auto &point : (*this->y_dataMap[s_name])) {

        point.setX((point.x() - approachDistance));

        if (s_name.contains("[V")) {
          // insert toggle points for analog signals
          if ((point.y() > 1.0) and (!old_state)) {
            hapticPoints.insert(s_name + " pressed", pVector->at(i));
            old_state = true;
          } else if ((point.y() < 1.0) and (old_state)) {
            hapticPoints.insert(s_name + " released", pVector->at(i));
            old_state = false;
          }
        }
        i++;
      }
    }
  }

  hapticPoints.insert("max force", pVector->at(maxForceLocation));

  // if the max force is huge, most probably this is a misuse test, so
  // concav/convex points are not relevant
  if (maxForce > misuse_force_treshold) {
    return;
  }

  if (maxForce < 0.5) {
    return;
  }

  // *** find the left bound for the upper curve
  start = end; // start from the approaching distance
  end = start + 1;
  while (pVector->at(start).y() <= pVector->at(end + 5).y()) {
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
  int maxHInterestPointsForceLocation = maxForceLocation - 5; // -30
  while ((pVector->at(maxHInterestPointsForceLocation).y() >
          maxInterestPointsForce)) {
    maxHInterestPointsForceLocation--; // go back while force is high
  }

  //         qDebug() << "here:" << __LINE__;

  // *** find the right bound for the pressing (higher) curve
  start =
      maxHInterestPointsForceLocation - 2; // start from the max force location
  end = maxHInterestPointsForceLocation;
  while (pVector->at(start).y() < pVector->at(end).y()) {
    // go back while the force is falling
    start--;
    end--;
  }
  int rightUpperBound = start + 15;
  //        qDebug() << "maxHInterestPointsForceLocation" <<
  //        pVector->at(maxHInterestPointsForceLocation);
  // ***

  // *** find max Interest Points Force Location for the lower curve

  int maxLInterestPointsForceLocation = maxForceLocation;

  // start at max force location + 80 points (robot optimization)
  if (pVector->size() > 1500)
    maxLInterestPointsForceLocation += 80;

  while (pVector->at(maxLInterestPointsForceLocation).y() >
         maxInterestPointsForce) {
    // go forward while the force is high
    maxLInterestPointsForceLocation++;
  }
  // ***

  // *** find the right bound for the lower curve
  start = maxLInterestPointsForceLocation;
  end = maxLInterestPointsForceLocation + 1;
  while (pVector->at(start).y() > pVector->at(end).y()) {
    end++;
    start++;
  }
  int rightLowerBound = start - 15;
  // ***

  // *** find the lower curve left bound
  start = pVector->size() - 1; // start from the vector end
  end = start - 2;

  while (pVector->at(end).y() - pVector->at(start).y() < 0.2) {
    // find a rising force slope of 0.3N
    if (pVector->at(end - 1).y() > pVector->at(end).y()) {
      end--;
    } else {
      start = end;
      end -= 1;
    }
  }

  start = end;      // from the 0.3N slope end
  end = start - 10; // 1
  while (pVector->at(end).y() >= pVector->at(start).y()) {
    // go up as long as the force is not falling
    end--;
    start--;
  }
  int leftLowerBound = end + 30; // + 15
  // ***

  // *** find a radius in which to evaluate the peaks for higher and lower
  // curves interestPointsRadiusL = bounded range * factor (0.2 >= factor <= 1)
  qreal interestPointsRadiusL = 0;
  qreal interestPointsRadiusL_HigerCurve =
      (pVector->at(rightUpperBound).x() - pVector->at(leftUpperBound).x()) *
      radius_factor;

  qreal interestPointsRadiusL_LowerCurve =
      (pVector->at(rightLowerBound).x() - pVector->at(leftLowerBound).x()) *
      radius_factor;

  interestPointsRadiusL = interestPointsRadiusL_HigerCurve;

  // if (interestPointsRadiusL < 0.02) interestPointsRadiusL = 0.02; // !

  // ***

  //        qDebug() << this->name()
  //                 <<  "leftUpperBound" << pVector->at(leftUpperBound).x() <<
  //                 pVector->at(leftUpperBound).y() << Qt::endl
  //                 <<  "leftlowerBound" << pVector->at(leftLowerBound).x() <<
  //                 pVector->at(leftUpperBound).y() << Qt::endl
  //                 <<  "rightUpperBound" << pVector->at(rightUpperBound).x()
  //                 << pVector->at(rightUpperBound).y() << Qt::endl
  //                 <<  "rightLowerBound" << pVector->at(rightLowerBound).x()
  //                 << pVector->at(rightLowerBound).y()<< Qt::endl
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

  qDebug() << "inainte de cautat puncte:"
           << "leftUpperBound" << leftUpperBound << "leftLowerBound"
           << leftLowerBound << Qt::endl;

  for (int l = leftUpperBound; l < leftLowerBound; l++) {
    // in mech stop area //orig: ±10; optimized for low res Tunis files
    if ((l > rightUpperBound + 3) and (l < rightLowerBound - 3)) {
      l = rightLowerBound - 3;
      continue;
    }
    // if (specimen->force.at(l) < 0.5) continue; //noise

    if (pVector->at(l).y() >= maxInterestPointsForce)
      continue;

    //         ultra low point count serie workaround (some Tunis files)
    //        if ((pVector->size () < 1000)
    //                and (pVector->at(l).y() >= (maxInterestPointsForce *
    //                low_points_count_max_force_cutoff_factor))) continue;

    // check if local convex peak
    if ((pVector->at(l).y() >= pVector->at(l + 5).y()) and
        (pVector->at(l).y() >= pVector->at(l - 5).y())) {

      // choose the upper\lower radius
      if (l < rightUpperBound + 5)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve; // 10
      if (l > rightLowerBound - 5)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve; // 10

      // *** find the bounds in which to evaluate the peak
      backBound = 0;
      forwardBound = 0;
      for (int j = l; j < pVector->size(); ++j) {
        // if reached the max force location, set the bound there
        if (j == maxHInterestPointsForceLocation) {
          //                    qDebug() << "forwardBound by
          //                    maxHInterestPointsForceLocation";
          forwardBound = j;
          break;
        }

        // if reached the force changed with differential_factor (default:
        // 0.25)N, set the bound there
        if (abs((pVector->at(l).y() - pVector->at(j).y())) >
            differential_factor) {
          //                    qDebug() << "forwardBound by
          //                    differential_factor";
          forwardBound = j; //
          break;
        }

        // if reached the radius distance, set the bound there
        if (abs(pVector->at(j).x() - pVector->at(l).x()) >
            interestPointsRadiusL) {
          //                    qDebug() << "forwardBound by
          //                    interestPointsRadiusL";
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

        if (abs((pVector->at(l).y() - pVector->at(j).y())) >
            differential_factor) {
          backBound = j;
          break;
        }

        if (abs(pVector->at(l).x() - pVector->at(j).x()) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }
      // ***

      // check if the current point is maximum in range
      qreal current_point_force = pVector->at(l).y();

      bool is_current_point_max = true;

      //             fix for Claudius low res files
      //            if((maxHInterestPointsForceLocation - l) < 100)
      //            {
      ////                forwardBound = l + 6;
      //                qDebug() << "fix for Claudius low res files";
      //                for (int i = backBound; i <= l + 6; i++){
      //                    if (pVector->at(i).y() > current_point_force){
      //                        is_current_point_max = false;
      //                        break;
      //                    }
      //                }

      //            } else {

      for (int i = backBound; i <= forwardBound; i++) {
        if (pVector->at(i).y() > current_point_force) {
          is_current_point_max = false;
          break;
        }
      }

      //            }

      // qDebug() << " local convex" << pVector->at(l).y() << pVector->at(l).x()
      //          << "backBound" << pVector->at(backBound).x() << "forwardBound"
      //          << pVector->at(forwardBound).x() << "is_current_point_max"
      //          << is_current_point_max;

      if (is_current_point_max) {

        // the ration between the sum of change in force (Dy) from the point to
        // both extremes and the range length (x)

        bendfactor =
            abs((pVector->at(forwardBound).y() - abs(pVector->at(l).y())) +
                (pVector->at(backBound).y() - abs(pVector->at(l).y()))) /
            interestPointsRadiusL;

        //                bendfactor = abs(
        //                            abs((this->at(forwardBound).y() -
        //                            abs(this->at(l).y()))) +
        //                 l           abs((this->at(backBound).y() -
        //                 abs(this->at(l).y())))
        //                            ) / interestPointsRadiusL;

        qDebug() << Qt::endl
                 << "this->name()"
                 << "convex point found: " << pVector->at(l).x() << Qt::endl
                 << "mm" << pVector->at(l).y() << Qt::endl
                 << "N"
                 << " point on row nr: " << l << "bend factor:" << bendfactor
                 << Qt::endl
                 << "f at forwardBound" << pVector->at(forwardBound).y()
                 << Qt::endl
                 << "s at forwardBound" << pVector->at(forwardBound).x()
                 << Qt::endl
                 << "f at backBound" << pVector->at(backBound).y() << Qt::endl
                 << "s at backBound" << pVector->at(backBound).x() << Qt::endl
                 << Qt::endl
                 << Qt::endl;

        qreal current_point_force = pVector->at(l).y();

        //                bool is_current_point_max = true;
        //                for (int i = backBound; i <= forwardBound; i++){
        //                    if (pVector->at(i).y() > current_point_force){
        //                        is_current_point_max = false;
        //                        break;
        //                    }
        //                }
        //                //                                qDebug () <<
        //                Qt::endl << "is_current_point_max" <<
        //                is_current_point_max;

        if (bendfactor < bendfactor_treshold)
          continue; // haptic imperceptible

        if (l < rightUpperBound + 3)
          upconvexe.append(l); // 10
        if (l > rightLowerBound - 3)
          downconvexe.append(l); // 10

        l = forwardBound - ((forwardBound - l) /
                            2); // move the cursor half the forward bound length

        qDebug() << "la convex: " << upconvexe << upconcave << downconvexe
                 << downconcave;

        continue;
      }
    }

    // check if local concav peak, and apply the same startegy to find the
    // interest points
    if (pVector->at(l).y() <= pVector->at(l + 5).y() and
        pVector->at(l).y() <= pVector->at(l - 5).y()) { // local concav peak

      if (l < rightUpperBound + 10)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve;
      if (l > rightLowerBound - 10)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve;

      backBound = 0;
      forwardBound = 0;

      for (int j = l; j < pVector->size(); ++j) {
        if (j == maxHInterestPointsForceLocation + 10) {
          forwardBound = j;
          break;
        }

        if (abs((pVector->at(l).y() - pVector->at(j).y())) >
            differential_factor) {
          forwardBound = j;
          break;
        }

        if (abs(pVector->at(j).x() - pVector->at(l).x()) >
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

        if (abs((pVector->at(l).y() - pVector->at(j).y())) >
            differential_factor) {
          backBound = j;
          break;
        }

        if (abs(pVector->at(l).x() - pVector->at(j).x()) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }

      // check if the current point is minimum in range
      qreal current_point_force = pVector->at(l).y();
      bool is_current_point_min = true;
      for (int i = backBound; i <= forwardBound; i++) {
        if (pVector->at(i).y() < current_point_force) {
          is_current_point_min = false;
          break;
        }
      }

      // qDebug() << "local concav" << pVector->at(l).y() << pVector->at(l).x()
      //          << "backBound" << pVector->at(backBound).x() << "forwardBound"
      //          << pVector->at(forwardBound).x() << "is_current_point_max"
      //          << is_current_point_min;

      if (is_current_point_min) {

        bendfactor =
            abs((pVector->at(forwardBound).y() - abs(pVector->at(l).y())) +
                (pVector->at(backBound).y() - abs(pVector->at(l).y()))) /
            interestPointsRadiusL;

        qDebug() << "this->name()"
                 << "concav point found: " << pVector->at(l).x() << "mm"
                 << pVector->at(l).y() << "N"
                 << " point on row nr: " << l << "bend factor:" << bendfactor;

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

  qDebug() << "this->name()"
           << "initial: " << upconvexe << upconcave << downconcave
           << downconvexe;

  // ** sort the found points

  // sort the convex points on the upper curve
  if (upconvexe.size() > 1) {
    // double detent, set the !! first two !! convex points their values. points
    // >2 ignored!
    hapticPoints.insert("convex push det1", pVector->at(upconvexe.at(0)));
    hapticPoints.insert("convex push det2", pVector->at(upconvexe.at(1)));
  } else if (upconvexe.size() == 1) {
    hapticPoints.insert("convex push det1", pVector->at(upconvexe.at(0)));
  } else if (upconvexe.size() == 0) {
  }

  // same strategy applied for sorting the other points

  if (upconcave.size() > 1) {
    hapticPoints.insert("concav push det1", pVector->at(upconcave.at(0)));
    hapticPoints.insert("concav push det2", pVector->at(upconcave.at(1)));
  } else if (upconcave.size() == 1) {
    hapticPoints.insert("concav push det1", pVector->at(upconcave.at(0)));
  } else if (upconcave.size() == 0) {
  }

  if ((downconvexe.size() > 1) and (upconvexe.size() > 1)) {
    hapticPoints.insert("convex release det2", pVector->at(downconvexe.at(0)));
    hapticPoints.insert("convex release det1", pVector->at(downconvexe.at(1)));
  } else if (downconvexe.size() == 1) {
    hapticPoints.insert("convex release det1", pVector->at(downconvexe.at(0)));
  } else if (downconvexe.size() == 0) {
  }

  if ((downconcave.size() > 1) and (upconvexe.size() > 1)) {
    hapticPoints.insert("concav release det2", pVector->at(downconcave.at(0)));
    hapticPoints.insert("concav release det1", pVector->at(downconcave.at(1)));
  } else if (downconcave.size() == 1) {
    hapticPoints.insert("concav release det1", pVector->at(downconcave.at(0)));
  } else if (downconcave.size() == 0) {

  }

  else if (downconcave.size() > 1) {
    // There are more that one concave points on the release curve,
    // but only one convex point was found on the push curve.
    // This cannot be a double detent, so we consider only the last concav point
    // found (left most)
    hapticPoints.insert("concav release det1", pVector->at(downconcave.last()));
  }

  // ***

  // *** find the overtravel point
  //(projection to the right from the last convex point + overtravel_termen)
  if (!upconvexe.isEmpty())
    for (int j = upconvexe.last(); j < pVector->size(); ++j) {
      if (j == maxForceLocation)
        break;
      if ((pVector->at(j).y() - pVector->at(upconvexe.last()).y()) >
          overtravel_termen) {
        j -= 1;
        hapticPoints.insert("overtravel", pVector->at(j));
        break;
      }
    }

  qDebug() << "here I should add the electrical points";
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

  qDebug() << "final: " << hapticPoints;

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

  // this->replace (pVector);
  return;
}

double dataLoader::fast_atof(const char *num, bool *ok) {
  if (!num || !*num) {
    *ok = false;
    return 0;
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
        return 0;
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

double dataLoader::pow10(int n) {
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

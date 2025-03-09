#include "data/data.h"
#include "blf_parser/blfparser.h"
#include "ui_data_dialog.h"
#include "csv_parser/csvparser.h"

Data::Data() {
  qDebug() << "Data created!";
  DataManager = new data_Dialog(nullptr);
  DBC = new QHash<unsigned int, BusMessage>;
  SysVars = new QHash<QString, DataSerie>;
  BlfFileParser = new BlfParser();
  filesList = new QStringList();
  CsvFileParser = new CSVParser();

  QObject::connect(this->DataManager, &data_Dialog::filesDropped, this,
                   &Data::filesReceived);

  QObject::connect(this->DataManager, &data_Dialog::loadClick, this,
                   &Data::reload); // loadDBC
  QObject::connect(this->DataManager, &data_Dialog::saveClick, this,
                   &Data::saveDBC);
  QObject::connect(this->DataManager, &data_Dialog::signalCheckStateUpdated,
                   this, &Data::updateSignalCheckState);
  QObject::connect(this->DataManager, &data_Dialog::sysVarCheckStateUpdated,
                   this, &Data::updateSysVarCheckState);
  QObject::connect(this->DataManager, &data_Dialog::dataTypeChanged,
                   this, &Data::updateDataType);
  QObject::connect(BlfFileParser, &BlfParser::parsingCompleted, this,
                   &Data::BlfParsingComplete);
  QObject::connect(BlfFileParser, &BlfParser::parsingProgress, this,
                   &Data::fileParsingProgress, Qt::QueuedConnection);
  QObject::connect(CsvFileParser, &CSVParser::parsingCompleted, this,
                   &Data::CsvParsingComplete);
  QObject::connect(CsvFileParser, &CSVParser::parsingProgress, this,
                   &Data::fileParsingProgress, Qt::QueuedConnection);
}

const double DataSerie::secondsInADay = 86400.0;

void Data::fileParsingProgress(int percent) {
  //    qDebug() << "Data::fileParsingProgress(int percent)" << percent;
  this->DataManager->ui->progressBar->show();
  this->DataManager->ui->progressBar->setValue(percent);
  emit parsingProgress(percent, "BLF file");
}

void Data::saveDBC() {
  qDebug() << "Saving settings...";

  // If current file is CSV, save CSV settings
  if (fi.suffix().toLower() == "csv") {
    qDebug() << "Current file is CSV, saving CSV settings";
    saveCsvSettings(fi.filePath());
    return;
  }

  // For BLF files, save DBC settings
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "LabAssistant",
                     "LabAssistant");

  settings.beginGroup("DBC");
  saveMessages(settings, DBC);
  settings.endGroup();

  settings.beginGroup("SysVars");
  saveSysVars(settings, *SysVars);
  settings.endGroup();

  settings.sync();
}

void updateParentState(QTreeWidgetItem *item) {
  if (!item)
    return;

  int childCount = item->childCount();
  if (childCount == 0)
    return; // End child, nothing to do here

  int checkedCount = 0;
  int uncheckedCount = 0;

  //    qDebug() << "updateParentState" << item->text(0) << childCount;

  for (int i = 0; i < childCount; ++i) {
    QTreeWidgetItem *child = item->child(i);
    updateParentState(child); // Recurse first, so child states are correct

    Qt::CheckState childState = child->checkState(0);
    if (childState == Qt::Checked) {
      ++checkedCount;
    } else if (childState == Qt::Unchecked) {
      ++uncheckedCount;
    }
  }

  if (checkedCount == childCount) {
    item->setCheckState(0, Qt::Checked);
  } else if (uncheckedCount == childCount) {
    item->setCheckState(0, Qt::Unchecked);
  } else {
    item->setCheckState(0, Qt::PartiallyChecked);
  }
}

void Data::loadDBC() {
  // QSettings settings("../settings/settings.ini", QSettings::IniFormat);

  // qDebug() << "loadDBC" << this->DataManager->settings->fileName() <<
  // this->DataManager->settings->allKeys();

  this->DataManager->ui->treeWidget->blockSignals(true);

  auto msgRoot = this->DataManager->getRootItem("Bus");
  delete msgRoot;

  msgRoot = this->DataManager->getRootItem("Bus");

  loadMessages(*this->DataManager->settings);

  for (auto &message : *this->DBC) {
    this->DataManager->insertMessage(message);
  }

  updateParentState(msgRoot);

  loadSysVars(*this->DataManager->settings, SysVars);

  msgRoot = this->DataManager->getRootItem("SysVar");

  updateParentState(msgRoot);

  this->DataManager->loadAxes();

  this->DataManager->loadSeriesTable();

  this->DataManager->ui->treeWidget->blockSignals(false);
}

void Data::reload() {
  if (fi.suffix().toLower() == "csv") {
    // Clear existing series table
    QTableWidget* table = this->DataManager->ui->seriesTable;
    table->setRowCount(0);
    
    // Re-parse the CSV file - use full parsing for reload
    parseFullCSV(fi);
  } else if (fi.suffix().toLower() == "blf") {
    // For BLF files, only reload if we have a valid file
    if (!fi.exists()) {
      qDebug() << "Cannot reload - BLF file not found:" << fi.filePath();
      return;
    }
    
    // Clear existing series table
    QTableWidget* table = this->DataManager->ui->seriesTable;
    table->setRowCount(0);
    
    loadDBC();
    parseBLF(fi);
  }
}

void Data::filesReceived(QList<QFileInfo> dropFileList) {
  // qDebug() << "files list iterated in Data:";
  // qDebug() << "dropFileList" << dropFileList;
  // qDebug() << "this->filesList" << this->filesList;

  for (auto &fi : dropFileList) {
    if (fi.suffix().contains("dbc", Qt::CaseInsensitive))
      parseDBC(fi);
    else if (fi.suffix().contains("blf", Qt::CaseInsensitive))
      parseBLF(fi);
    else if (fi.suffix().contains("ldf", Qt::CaseInsensitive))
      parseLDF(fi);
    else if (fi.suffix().contains("csv", Qt::CaseInsensitive)) {
      initializeCsvSettings(fi.filePath());
      parseCSV(fi);
    }
  }
}

void Data::saveMessages(QSettings &settings,
                        const QHash<unsigned int, BusMessage> *messages) {
  while (!settings.group().isEmpty())
    settings.endGroup();

  // for (int i = 0; i < 10; i++)
  //     settings.endGroup();
  settings.beginGroup("BusMessages");
  for (auto it = messages->cbegin(); it != messages->cend(); ++it) {
    settings.beginGroup(QString::number(it.key()));

    BusMessage msg = it.value();
    settings.setValue("Name", msg.getName());
    settings.setValue("Id", msg.getId());
    settings.setValue("Dlc", msg.getDlc());
    settings.setValue("Checked", msg.checked);

    settings.beginWriteArray("Signals");
    const auto sigs = msg.getSignals();
    for (int i = 0; i < sigs.size(); ++i) {
      // continue;
      settings.setArrayIndex(i);
      const BusSignal &sig = sigs[i];
      settings.setValue("Name", sig.getName());
      settings.setValue("ByteOrder", static_cast<int>(sig.getByteOrder()));
      settings.setValue("StartBit", sig.getStartBit());
      settings.setValue("Length", sig.getLength());
      settings.setValue("Sign", static_cast<int>(sig.getSign()));
      settings.setValue("Minimum", sig.getMinimum());
      settings.setValue("Maximum", sig.getMaximum());
      settings.setValue("Factor", sig.getFactor());
      settings.setValue("Offset", sig.getOffset());
      settings.setValue("Unit", sig.getUnit());
      settings.setValue("Multiplexor", static_cast<int>(sig.getMultiplexor()));
      settings.setValue("MultiplexedNumber", sig.getMultiplexedNumber());
      //            qDebug() << "saving...:" << sig.getName() <<
      //            sig.getChecked();
      settings.setValue("Checked", sig.getChecked());
      settings.setValue("Occurence", sig.getOccurence());
    }
    settings.endArray();

    settings.endGroup();
  }
  settings.endGroup();
}

void Data::loadMessages(QSettings &settings) {
  // qDebug() << "loadMessages " << settings.group();

  while (!settings.group().isEmpty())
    settings.endGroup();

  // qDebug() << "loadMessages after set root " << settings.group();

  settings.beginGroup("BusMessages");

  // qDebug() << "loadMessages new root " << settings.group();

  QStringList keys = settings.childGroups();
  for (const QString &key : keys) {
    settings.beginGroup(key);

    BusMessage msg;
    msg.name = settings.value("Name").toString();
    msg.id = settings.value("Id").toUInt();
    msg.dlc = settings.value("Dlc").toULongLong();
    msg.checked = settings.value("Checked", QVariant(false)).toBool();

    // qDebug() << Qt::endl << msg.name << msg.id;

    if (DBC->contains(static_cast<unsigned int>(msg.id))) {
      //            qDebug() << "Message already in DBC!";
      settings.endGroup();
      continue;
    }

    int size = settings.beginReadArray("Signals");
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      BusSignal signal;
      signal.name = settings.value("Name").toString();
      signal.order =
          static_cast<ByteOrder>(settings.value("ByteOrder").toInt());
      signal.startBit = settings.value("StartBit").toUInt();
      signal.length = settings.value("Length").toUInt();
      signal.sign = static_cast<Sign>(settings.value("Sign").toInt());
      signal.minimum = settings.value("Minimum").toDouble();
      signal.maximum = settings.value("Maximum").toDouble();
      signal.factor = settings.value("Factor").toDouble();
      signal.offset = settings.value("Offset").toDouble();
      signal.unit = settings.value("Unit").toString();
      signal.multiplexor =
          static_cast<Multiplexor>(settings.value("Multiplexor").toInt());
      signal.multiplexNum = settings.value("MultiplexedNumber").toUInt();
      // qDebug() << signal.name << "settings.value(Checked)" <<
      // settings.value("Checked")
      //          << settings.value("Checked").toBool();
      signal.checked = settings.value("Checked").toBool();
      signal.setOccurence(settings.value("Occurence").toInt());
      msg.busSignals.append(signal);
    }
    settings.endArray();

    //        messages[static_cast<unsigned int>(key.toInt())] = msg;
    this->DBC->insert(static_cast<unsigned int>(msg.id), msg);
    //        qDebug() << "current msg count:" << this->DBC->count();
    settings.endGroup();
  }

  settings.endGroup();
}

void Data::saveSysVars(QSettings &settings,
                       const QHash<QString, DataSerie> &SysVars) {

  while (!settings.group().isEmpty())
    settings.endGroup();

  settings.beginWriteArray("DataSeries");
  int index = 0;
  for (auto it = SysVars.cbegin(); it != SysVars.cend(); ++it) {
    settings.setArrayIndex(index);
    settings.setValue("key", "SysVar" + it.key());
    settings.setValue("checked", it.value().checked);
    ++index;
  }
  settings.endArray();
}

void Data::loadSysVars(QSettings &settings,
                       QHash<QString, DataSerie> *SysVars) {
  if (!SysVars)
    return;

  while (!settings.group().isEmpty())
    settings.endGroup();

  int size = settings.beginReadArray("DataSeries");

  SysVars->clear();

  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    QString key = settings.value("key").toString().replace("SysVar", "");
    bool checked = settings.value("checked").toBool();

    //        qDebug() << "SysVar read from settings:" << key <<
    //        settings.value("checked") << checked;

    DataSerie dataSerie;
    dataSerie.checked = checked;

    SysVars->insert(key, dataSerie);
  }

  settings.endArray();

  if (SysVars->count() < 1) {
    //        qDebug() << Qt::endl << "No SysVars found!";
    return;
  }

  QList<QString> varkeys = SysVars->keys();

  quickSort(varkeys, 0, varkeys.size() - 1);

  auto msgRoot = this->DataManager->getRootItem("SysVar");
  delete msgRoot;

  for (auto &key : varkeys) {
    QString sysVarString = "SysVar" + key;
    bool checkedStatus = SysVars->value(key).checked;
    this->DataManager->insertSysVar(sysVarString, checkedStatus);
  }
  this->DataManager->sortTreeItems();
}

void Data::parseDBC(QFileInfo file) {
  DBCIterator dbc(file.filePath());

  for (auto &const_message : dbc) {
    BusMessage message = const_message;

    if (this->DBC->contains(message.getId()))
      continue;
    this->DBC->insert(message.getId(), message);
    this->DataManager->insertMessage(message);
  }
}

void parseFramesAndSignals(const QString &fileContent,
                           QHash<unsigned int, BusMessage> &DBC) {
  enum class State {
    SearchSignals,
    InSignals,
    SearchFrame,
    InFrame,
    SearchSignal
  };

  static const QRegularExpression lineSplitRegex("[\r\n]");
  QStringList lines = fileContent.split(lineSplitRegex, Qt::SkipEmptyParts);
  QHash<QString, BusSignal> linSignals;
  BusMessage currentMessage;
  State state = State::SearchSignals;
  bool exitLoop = false; // Control flag

  // qDebug() << "Starting to parse frames and signals.";

  for (const QString &line : lines) {
    if (exitLoop)
      break;
    const QString trimmedLine = line.trimmed();
    // qDebug() << "Processing line:" << trimmedLine;

    if (trimmedLine.isEmpty())
      continue;

    switch (state) {
    case State::SearchSignals:
      // qDebug() << "State: SearchSignals";
      if (trimmedLine.contains("Signals {")) {
        // qDebug() << "Entering Signals list:";
        state = State::InSignals;
      }
      break;

    case State::InSignals:
      // qDebug() << "State: InSignals";
      if (trimmedLine.contains(";")) {
        QStringList signalParts = trimmedLine.split(':');
        if (signalParts.length() < 2)
          continue; // Skip if not enough elements
        BusSignal currentSignal;
        currentSignal.name = signalParts[0].trimmed();
        currentSignal.length =
            signalParts[1].split(',').first().trimmed().toInt();

        currentSignal.factor = 1;
        currentSignal.offset = 0;
        currentSignal.sign = Sign::SIGNED;
        currentSignal.order = ByteOrder::MOTOROLA;
        currentSignal.multiplexor = Multiplexor::NONE;
        currentSignal.multiplexNum = -1;

        linSignals.insert(currentSignal.name, currentSignal);
        // qDebug() << "Added signal:" << currentSignal.name
        //          << "with length:" << currentSignal.length;
      } else if (trimmedLine.contains("}")) {
        // qDebug() << "Exiting InSignals state, entering SearchFrame." <<
        // Qt::endl
        //          << Qt::endl
        //          << Qt::endl;
        state = State::SearchFrame;
      }
      break;

    case State::SearchFrame:
      // qDebug() << "State: SearchFrame";
      if (trimmedLine.contains("Frames {")) {
        // qDebug() << "Entering Frames list." << Qt::endl;
        state = State::InFrame;
      }
      break;

    case State::InFrame:
      // qDebug() << "State: InFrame";
      if (trimmedLine.contains("{")) {
        QStringList frameParts = trimmedLine.split(":");
        if (frameParts.length() < 2)
          continue; // Skip if not enough elements

        currentMessage = BusMessage(); // Reset/start a new BusMessage
        currentMessage.name = frameParts[0].trimmed();
        currentMessage.id = frameParts[1].split(',').first().trimmed().toInt();
        state = State::SearchSignal;
        // qDebug() << "Processing frame:" << currentMessage.name
        //          << "with ID:" << currentMessage.id;
      } else if (trimmedLine.contains("}")) {
        // qDebug() << Qt::endl;
        // break from both the case and the while loop
        exitLoop = true;
      }
      break;

    case State::SearchSignal:
      // qDebug() << "State: SearchSignal";
      if (trimmedLine.contains(";")) {
        QStringList signalParts = trimmedLine.split(",");
        if (signalParts.length() < 2)
          continue; // Skip if not enough elements
        const QString signalName = signalParts.first().trimmed();
        if (!linSignals.contains(signalName))
          continue; // Skip if signalName not in linSignals

        linSignals[signalName].startBit =
            signalParts[1].replace(';', "").trimmed().toInt();

        currentMessage.busSignals.append(linSignals[signalName]);

        // qDebug() << "Added signal to frame:" << signalName << "start bit:" <<
        // linSignals[signalName].startBit;
      } else if (trimmedLine.contains("}")) {
        DBC.insert(currentMessage.id, currentMessage);
        // qDebug() << "Added frame to DBC:" << currentMessage.name << "with
        // ID:" << currentMessage.id << Qt::endl; qDebug () << "Signals in
        // frame:" << currentMessage.busSignals.length() << Qt::endl;
        state = State::InFrame;
      }
      break;
    }
  }

  // qDebug() << "Finished parsing frames and signals.";
}

void Data::parseLDF(QFileInfo file) {

  QFile inputFile(file.filePath());
  if (inputFile.open(QIODevice::ReadOnly)) {
    QTextStream reader(&inputFile);
    auto contents = QString(inputFile.readAll());
    parseFramesAndSignals(contents, *DBC);
    inputFile.close();
  }

  for (auto &message : *DBC) {
    this->DataManager->insertMessage(message);
  }
}

int extractNumber(const QString &str) {
  int startPos = str.lastIndexOf('[') + 1;
  int endPos = str.lastIndexOf(']');
  if (startPos < 0 || endPos < 0 || startPos >= endPos) {
    return -1; // Return -1 or another value to signify failure
  }
  return str.mid(startPos, endPos - startPos).toInt();
}

void Data::quickSort(QList<QString> &arr, int left, int right) {
  int i = left, j = right;
  QString pivot = arr[(left + right) / 2];
  QString temp;

  while (i <= j) {
    while (extractNumber(arr[i]) < extractNumber(pivot)) {
      i++;
    }
    while (extractNumber(arr[j]) > extractNumber(pivot)) {
      j--;
    }
    if (i <= j) {
      temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
      i++;
      j--;
    }
  }

  if (left < j) {
    quickSort(arr, left, j);
  }
  if (i < right) {
    quickSort(arr, i, right);
  }
}

void Data::updateSignalCheckState(qint32 msgId, QString signalName,
                                  bool boolCheckState) {
  if (DBC->contains(msgId)) {
    auto &dbcSignals = (*DBC)[msgId].busSignals;
    int msgSigCheckCount = 0;

    for (auto &sig : dbcSignals) {
      if (sig.name == signalName) {
        sig.setChecked(boolCheckState);
        //                qDebug() << "Signal" << signalName << msgId <<
        //                "checkState = " << boolCheckState
        //                         << "check: " << sig.getChecked();
        //                break;
      }

      if (sig.checked)
        ++msgSigCheckCount;
    }

    if (msgSigCheckCount > 0)
      (*DBC)[msgId].checked = true;
    else
      (*DBC)[msgId].checked = false;
  }
}

void Data::updateSysVarCheckState(QString sysVarName, bool boolCheckState) {
    QString varName = sysVarName.replace("SysVar", "");
    qDebug() << "Updating check state for" << varName << "to" << boolCheckState;

    if (SysVars->contains(varName)) {
        auto &serie = (*SysVars)[varName];
        serie.checked = boolCheckState;
        
        // If this is a CSV variable, also update the CSVParser data
        if (varName.startsWith("CSV::")) {
            QString csvVarName = varName.replace("CSV::", "");
            auto csvData = CsvFileParser->getData();
            if (csvData->contains(csvVarName)) {
                (*csvData)[csvVarName].checked = boolCheckState;
            }
        }
        
        qDebug() << "Updated check state in SysVars for" << varName << "to" << serie.checked;
    }
}

void Data::parseBLF(QFileInfo file) {
    qDebug() << "Parsing BLF file:" << file.filePath();
    (*filesList).append(file.absoluteFilePath());

    this->fi = file;
    timer.start();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
             << "BLF parsing started...";

    // Only check UUID on initial file load, not on reload
    static QSet<QString> processedFiles;
    if (!processedFiles.contains(file.absoluteFilePath())) {
        if (!readBlfUUID(file)) {
            return;
        }
        processedFiles.insert(file.absoluteFilePath());
    }

    // Open BLF file
    void *hBlfFile = BLCreateFile(file.filePath().toLocal8Bit(), GENERIC_READ);
    if (hBlfFile == nullptr) {
        qDebug() << "Unable to open file:" << file.filePath();
        return;
    }

    // Clear existing data
    for (auto it = DBC->begin(); it != DBC->end(); ++it) {
        BusMessage &busmsg = it.value();
        for (auto &bussig : busmsg.busSignals) {
            bussig.series->clear();
            bussig.occurence = 0;
        }
    }

    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        it.value().clear();
    }

    // Clear series table
    QTableWidget *table = this->DataManager->ui->seriesTable;
    int rowCount = table->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QTableWidgetItem *sampleCount = table->item(i, 2);
        QTableWidgetItem *serieSource = table->item(i, 10);

        if (sampleCount) {
            sampleCount->setText("");
        }
        if (serieSource) {
            serieSource->setText("");
        }
    }

    BlfFileParser->source = file.path();
    this->BlfFileParser->DumpBlfFile(hBlfFile, *DBC, *SysVars);
}

bool Data::readBlfUUID(QFileInfo file) {
    // Skip UUID check on reload
    static QSet<QString> checkedFiles;
    if (checkedFiles.contains(file.absoluteFilePath())) {
        return true;
    }

    void *hBlfFile = BLCreateFile(file.filePath().toLocal8Bit(), GENERIC_READ);
    if (hBlfFile == nullptr) {
        qDebug() << "Unable to open file: " << file.filePath().toLocal8Bit();
        return false;
    }

    // Rest of the UUID checking code...
    // After successful check:
    checkedFiles.insert(file.absoluteFilePath());
    return true;
}

void Data::processParsedSysVars() {
  if (SysVars->count() < 1) {
    qDebug() << "No SysVars found!";
    return;
  }

  auto processBranch = [&](const QString &branch) {
    QString varRealName = branch;
    varRealName.replace("SysVar", "");
    qDebug() << "varRealName" << varRealName;

    if ((SysVars->value(varRealName).checked) &&
        (SysVars->value(varRealName).points.count() > 0)) {
      auto foundRow = this->DataManager->tableContains(
          this->DataManager->ui->seriesTable, 9, branch);
      // qDebug() << "foundRow" << foundRow;
      if (foundRow > -1) {
        QTableWidgetItem* item = this->DataManager->getOrCreateItem(foundRow, 10);
        auto itemText = item ? item->text() : "";
        if (itemText.isEmpty()) {
          this->DataManager->insertSerie(
              fi.filePath(), branch,
              SysVars->value(varRealName).points.count());
        }
      } else {
        this->DataManager->insertSerie(
            fi.filePath(), branch, SysVars->value(varRealName).points.count());
      }
    }
  };

  QList<QString> keys = SysVars->keys();
  quickSort(keys, 0, keys.size() - 1);

  for (const auto &key : keys) {
    QString sysVarString = "SysVar" + key;
    bool checkedStatus = false;
    this->DataManager->insertSysVar(sysVarString, checkedStatus);
  }

  QApplication::processEvents();
  auto list =
      this->DataManager->getTreeItems(this->DataManager->getRootItem("SysVar"));

  for (const auto &branch : list) {
    processBranch(branch);
  }
}

void Data::processParsedMessages() {
  // Lambda for processing channels
  auto processChannel = [&](const BusSignal &signal, int channel) {
    QString channelName = QString("%1(ch_%2)").arg(signal.name).arg(channel);
    auto foundRow = this->DataManager->tableContains(
        this->DataManager->ui->seriesTable, 9, channelName);

    // qDebug() << "auto foundRow = this->DataManager->tableContains" <<
    // foundRow;

    if (foundRow >= 0) {
      QTableWidgetItem* item = this->DataManager->getOrCreateItem(foundRow, 10);
      auto itemText = item ? item->text() : "";
      // qDebug() << "itemText" << itemText << itemText.isEmpty();

      if (itemText.isEmpty()) {
        this->DataManager->insertSerie(fi.filePath(), channelName,
                                       signal.series->value(channel).count(),
                                       signal.getFactor(), signal.getOffset());
      }
    } else {
      this->DataManager->insertSerie(fi.filePath(), channelName,
                                     signal.series->value(channel).count(),
                                     signal.getFactor(), signal.getOffset());
    }
  };

  // Lambda for processing signals
  auto processSignal = [&](const BusSignal &signal) {
    auto channels = signal.series->keys();
    // qDebug() << "processSignal = [&](const auto &signal)";
    // qDebug() << channels;
    for (auto channel : channels) {
      if (signal.series->value(channel).count() > 0) {
        processChannel(signal, channel);
      }
    }
  };

  // Iterate through messages and signals
  for (auto &message : *DBC) {
    for (auto &signal : message) {
      // if (signal.series->count())
      //   qDebug() << "signal" << signal.name << "factor" << signal.factor
      //            << "first val" << signal.series->begin().value().first();

      processSignal(signal);
    }
  }
}

void Data::BlfParsingComplete() {
  qDebug() << "void Data::BlfParsingComplete()";
  qDebug() << "Time taken:" << timer.elapsed() << "ms";

  processParsedSysVars();

  processParsedMessages();

  this->fileParsingProgress(0);
  this->DataManager->ui->progressBar->hide();
  this->DataManager->hideEmptySeries();
  emit dataReady();
}

void Data::parseCSV(QFileInfo file) {
    qDebug() << "\nCSV file dropped: using new CSV parser";
    
    qDebug() << "\n=== CSV Parsing Status ===";
    qDebug() << "Settings before parse:" << (this->DataManager->settings ? "exists" : "null");
    
    qDebug() << "\n=== Starting CSV Parse ===";
    qDebug() << "Parsing CSV file:" << file.filePath();
    (*filesList).append(file.absoluteFilePath());
    
    this->fi = file;
    timer.start();
    
    // Check if settings exist for this file
    QString localSettingsPath = getCsvSettingsPath(file.filePath());
    bool settingsExist = QFile::exists(localSettingsPath);
    
    qDebug() << "Settings path:" << localSettingsPath;
    qDebug() << "Settings exist:" << settingsExist;
    
    // First parse the CSV file to get structure and fingerprint
    // For initial parsing, limit to 100 rows for data type detection
    if (!CsvFileParser->parseFile(file.filePath(), 100)) {
        qDebug() << "Failed to parse CSV file";
        return;
    }
    qDebug() << "CSV file parsed successfully";
    
    // Debug: Print data in CSVParser after initial parsing
    auto csvData = CsvFileParser->getData();
    qDebug() << "\n=== Data in CSVParser after initial parsing ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType)
                 << "Points:" << it.value().points.size();
    }
    
    // Initialize settings objects if needed, but don't overwrite existing settings
    if (!this->DataManager->settings || !settingsExist) {
        initializeCsvSettings(file.filePath());
    }
    
    // Load settings after parsing but before creating UI elements
    loadCsvSettings(file.filePath());
    
    // If we have checked columns, parse the full file to get all data for those columns only
    QStringList checkedColumns;
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        if (it.value().checked) {
            checkedColumns.append(it.key());
        }
    }
    
    if (!checkedColumns.isEmpty()) {
        qDebug() << "Found" << checkedColumns.size() << "checked columns, parsing full file to get data for:" << checkedColumns.join(", ");
        
        // Clear existing points before parsing
        for (auto it = csvData->begin(); it != csvData->end(); ++it) {
            it.value().points.clear();
        }
        
        if (!CsvFileParser->parseFile(file.filePath(), 0, checkedColumns)) {  // 0 means no limit, only process checked columns
            qDebug() << "Failed to parse full CSV file";
            return;
        }
        qDebug() << "Full CSV file parsed successfully";
        
        // Debug: Print data in CSVParser after full parsing
        qDebug() << "\n=== Data in CSVParser after full parsing ===";
        for (auto it = csvData->begin(); it != csvData->end(); ++it) {
            qDebug() << "Column:" << it.key() 
                     << "Data Type:" << static_cast<int>(it.value().dataType)
                     << "Checked:" << it.value().checked
                     << "Points:" << it.value().points.size();
        }
    } else {
        qDebug() << "No checked columns found, skipping full file parsing";
    }
    
    // Now proceed with creating UI elements
    CsvParsingComplete();
    
    qDebug() << "Settings after parse:" << (this->DataManager->settings ? "exists" : "null");
}

void Data::CsvParsingComplete() {
    qDebug() << "CSV parsing completed in" << timer.elapsed() << "ms";
    
    // Get the parsed data with potentially loaded check states
    auto csvData = CsvFileParser->getData();
    
    qDebug() << "\n=== Data Types in CSVParser ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType)
                 << "Checked:" << it.value().checked
                 << "Points:" << it.value().points.size();
    }
    
    // Block tree signals during updates
    this->DataManager->ui->treeWidget->blockSignals(true);
    
    // First clear existing data
    // Clear existing CSV variables from SysVars
    QStringList keysToRemove;
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            keysToRemove.append(it.key());
        }
    }
    for (const QString& key : keysToRemove) {
        SysVars->remove(key);
    }
    
    // Clear existing CSV items from tree
    auto root = this->DataManager->getRootItem("CSV");
    if (root) {
        for (int i = root->childCount() - 1; i >= 0; --i) {
            QTreeWidgetItem* child = root->child(i);
            delete root->takeChild(i);
        }
    }
    
    // First pass: Add all columns to SysVars and tree
    QList<QString> checkedColumns;  // Keep track of checked columns
    for(auto it = csvData->begin(); it != csvData->end(); ++it) {
        QString varName = "CSV::" + it.key();
        DataSerie serie = it.value();
        
        // Get the check state from the loaded settings
        bool isChecked = serie.checked;
        
        // Skip columns with NotUsed data type when adding points
        if (serie.dataType == DataType::NotUsed) {
            qDebug() << "Skipping NotUsed column:" << varName;
            serie.points.clear();  // Clear any points that might have been added
        }
        
        // Skip DateTime columns when adding points
        if (serie.dataType == DataType::DateTime) {
            qDebug() << "Skipping DateTime column:" << varName;
            serie.points.clear();  // Clear any points that might have been added
        }
        
        qDebug() << "Adding to SysVars:" << varName 
                 << "Data Type:" << static_cast<int>(serie.dataType)
                 << "Checked:" << isChecked;
        
        // Add to SysVars with the correct check state and data type
        SysVars->insert(varName, serie);
        
        // Add to UI tree with the correct check state and data type
        QTreeWidgetItem* item = this->DataManager->insertCSVVar(it.key(), isChecked, serie.dataType);
        if (item) {
            item->setCheckState(0, isChecked ? Qt::Checked : Qt::Unchecked);
            qDebug() << "Inserting" << varName << "with check state:" << isChecked 
                     << "and data type:" << static_cast<int>(serie.dataType);
            
            if (isChecked) {
                checkedColumns.append(varName);
            }
        }
    }
    
    // Update parent states in tree
    this->DataManager->ui->treeWidget->blockSignals(false);
    
    // Make sure the tree columns are properly sized
    this->DataManager->resizeTreeWidgetColumns();
    
    // Debug: Print data types in SysVars after CsvParsingComplete
    qDebug() << "\n=== Data Types in SysVars after CsvParsingComplete ===";
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            qDebug() << "Column:" << it.key() 
                     << "Data Type:" << static_cast<int>(it.value().dataType)
                     << "Checked:" << it.value().checked;
        }
    }
    
    // Second pass: Add checked items to series table
    // Block signals during series table updates
    this->DataManager->ui->seriesTable->blockSignals(true);
    
    // Clear existing series table
    this->DataManager->ui->seriesTable->setRowCount(0);
    
    qDebug() << "\n=== Adding checked columns to series table ===";
    qDebug() << "Number of checked columns:" << checkedColumns.size();
    
    for(const QString& varName : checkedColumns) {
        qDebug() << "Adding checked item to series table:" << varName;
        try {
            if (SysVars->contains(varName)) {
                const DataSerie& serie = SysVars->value(varName);
                
                // Skip NotUsed and DateTime columns
                if (serie.dataType == DataType::NotUsed || serie.dataType == DataType::DateTime) {
                    qDebug() << "Skipping NotUsed/DateTime column:" << varName;
                    continue;
                }
                
                qDebug() << "Serie" << varName 
                         << "has" << serie.points.size() << "points"
                         << "data type:" << static_cast<int>(serie.dataType);
                
                // Only add series with points
                if (serie.points.size() > 0) {
                    this->DataManager->insertSerie(
                        fi.fileName(),           // source
                        varName,                 // name
                        serie.points.size(),     // count
                        1.0,                     // default factor
                        0.0                      // default offset
                    );
                    qDebug() << "Successfully added" << varName << "to series table";
                } else {
                    qDebug() << "Skipping empty series:" << varName << " - no points!";
                    
                    // Debug: Print more information about the empty series
                    qDebug() << "  Data type:" << static_cast<int>(serie.dataType);
                    qDebug() << "  Is operating mode series:" << serie.isOperatingModeSeries;
                    qDebug() << "  Operating modes count:" << serie.operatingModes.size();
                }
            } else {
                qDebug() << "Error: Variable" << varName << "not found in SysVars";
            }
        } catch (const std::exception& e) {
            qDebug() << "Error adding" << varName << "to series table:" << e.what();
        }
        QApplication::processEvents();  // Allow UI updates
    }
    
    // Unblock signals
    this->DataManager->ui->seriesTable->blockSignals(false);
    
    // Update the UI
    this->fileParsingProgress(0);
    this->DataManager->ui->progressBar->hide();
    
    // Emit signal that data is ready
    emit dataReady();
}

QString Data::getTemplatesFolderPath() const {
    // Get the application executable path
    QString appPath = QCoreApplication::applicationDirPath();
    return appPath + "/templates/";
}

QString Data::findMatchingTemplateSettings(const QString& fingerprint) const {
    QDir templatesDir(getTemplatesFolderPath());
    if (!templatesDir.exists()) {
        qDebug() << "Templates directory does not exist:" << getTemplatesFolderPath();
        return QString();
    }

    // Get all .ini files in templates directory
    QStringList filters;
    filters << "*.ini";
    QFileInfoList files = templatesDir.entryInfoList(filters, QDir::Files);
    qDebug() << "Searching" << files.size() << "template files for matching fingerprint";

    // Check each template file for matching fingerprint
    for (const QFileInfo& fileInfo : files) {
        QSettings templateSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
        templateSettings.beginGroup("CSVFormat");
        QString storedFingerprint = templateSettings.value("fingerprint").toString();
        templateSettings.endGroup();

        qDebug() << "Checking template:" << fileInfo.fileName() 
                 << "Stored fingerprint:" << storedFingerprint 
                 << "Looking for:" << fingerprint;

        if (storedFingerprint == fingerprint) {
            qDebug() << "Found matching template:" << fileInfo.absoluteFilePath();
            return fileInfo.absoluteFilePath();
        }
    }

    qDebug() << "No matching template found for fingerprint:" << fingerprint;
    return QString();
}

void Data::copySettingsFile(const QString& sourcePath, const QString& destPath) {
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    QFile::copy(sourcePath, destPath);
}

QString Data::getCsvSettingsPath(const QString& csvPath) {
    QFileInfo csvFile(csvPath);
    // Create settings file next to CSV with .ini extension
    return csvFile.absolutePath() + "/" + csvFile.baseName() + ".ini";
}

void Data::loadCsvSettings(const QString& csvPath) {
    QString localSettingsPath = getCsvSettingsPath(csvPath);
    
    qDebug() << "\n=== Loading CSV Settings ===";
    qDebug() << "Loading CSV settings for file:" << csvPath;
    qDebug() << "Settings path:" << localSettingsPath;
    
    // Check if settings exist
    if (!QFile::exists(localSettingsPath)) {
        qDebug() << "No local settings file found";
        return;
    }
    
    // Create settings object
    QSettings localSettings(localSettingsPath, QSettings::IniFormat);
    
    // Verify fingerprint matches
    localSettings.beginGroup("CSVFormat");
    QString storedFingerprint = localSettings.value("fingerprint").toString();
    localSettings.endGroup();
    
    // Generate current fingerprint
    QString fingerprint = CsvFileParser->generateFingerprint();
    
    qDebug() << "Found local settings with fingerprint:" << storedFingerprint;
    qDebug() << "Current fingerprint:" << fingerprint;
    
    // Debug: Print data types in CSVParser before loading
    auto csvData = CsvFileParser->getData();
    qDebug() << "\n=== Data in CSVParser before loading settings ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType);
    }
    
    if (fingerprint == storedFingerprint) {
        qDebug() << "Loading local settings from:" << localSettingsPath;
        
        // Load settings into CsvFileParser
        CsvFileParser->loadSettings(&localSettings);
        
        // Load additional data types from CSVData group if available
        localSettings.beginGroup("CSVData");
        for (auto it = csvData->begin(); it != csvData->end(); ++it) {
            QString columnName = it.key();
            // Check if we have data type information
            if (localSettings.contains(QString("dataType/%1").arg(columnName))) {
                int dataTypeValue = localSettings.value(QString("dataType/%1").arg(columnName), 
                                                      static_cast<int>(DataType::Unknown)).toInt();
                (*csvData)[columnName].dataType = static_cast<DataType>(dataTypeValue);
                
                // Set operating mode flag if needed
                if ((*csvData)[columnName].dataType == DataType::OperatingMode) {
                    (*csvData)[columnName].isOperatingModeSeries = true;
                }
                
                qDebug() << "Loaded data type from CSVData for column:" << columnName 
                         << "Type:" << dataTypeValue;
            }
            
            // Check if we have checked state information
            if (localSettings.contains(QString("checked/%1").arg(columnName))) {
                bool checked = localSettings.value(QString("checked/%1").arg(columnName), false).toBool();
                (*csvData)[columnName].checked = checked;
                
                qDebug() << "Loaded checked state from CSVData for column:" << columnName 
                         << "Checked:" << checked;
            }
        }
        localSettings.endGroup();
        
        // Load settings into DataManager
        if (!this->DataManager->settings) {
            this->DataManager->settings = new QSettings(localSettingsPath, QSettings::IniFormat);
        }
        copySettingsToDataManager(&localSettings);
        
        // Load UI components
        this->DataManager->loadAxes(&localSettings);
        this->DataManager->loadSeriesTable(&localSettings);
        
        // Debug: Print loaded states
        qDebug() << "\n=== Data in CSVParser after loading settings ===";
        for (auto it = csvData->begin(); it != csvData->end(); ++it) {
            qDebug() << "Column:" << it.key() 
                     << "Check state:" << it.value().checked
                     << "Data type:" << static_cast<int>(it.value().dataType);
        }
        
        // Debug: Print data types in SysVars after loading
        qDebug() << "\n=== Data Types in SysVars after loading ===";
        for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
            if (it.key().startsWith("CSV::")) {
                qDebug() << "Column:" << it.key() 
                         << "Data Type:" << static_cast<int>(it.value().dataType);
            }
        }
        
        qDebug() << "Successfully loaded settings from:" << localSettingsPath;
        return;  // Successfully loaded local settings, we're done
    } else {
        qDebug() << "Fingerprint mismatch - local:" << storedFingerprint << "current:" << fingerprint;
    }
    
    // If we get here, try to find a matching template
    QString templatePath = findMatchingTemplateSettings(fingerprint);
    if (!templatePath.isEmpty()) {
        qDebug() << "Found matching template:" << templatePath;
        QSettings templateSettings(templatePath, QSettings::IniFormat);
        
        // Load template settings into CsvFileParser
        CsvFileParser->loadSettings(&templateSettings);
        
        // Load additional data types from CSVData group if available
        templateSettings.beginGroup("CSVData");
        for (auto it = csvData->begin(); it != csvData->end(); ++it) {
            QString columnName = it.key();
            // Check if we have data type information
            if (templateSettings.contains(QString("dataType/%1").arg(columnName))) {
                int dataTypeValue = templateSettings.value(QString("dataType/%1").arg(columnName), 
                                                      static_cast<int>(DataType::Unknown)).toInt();
                (*csvData)[columnName].dataType = static_cast<DataType>(dataTypeValue);
                
                // Set operating mode flag if needed
                if ((*csvData)[columnName].dataType == DataType::OperatingMode) {
                    (*csvData)[columnName].isOperatingModeSeries = true;
                }
                
                qDebug() << "Loaded data type from template for column:" << columnName 
                         << "Type:" << dataTypeValue;
            }
            
            // Check if we have checked state information
            if (templateSettings.contains(QString("checked/%1").arg(columnName))) {
                bool checked = templateSettings.value(QString("checked/%1").arg(columnName), false).toBool();
                (*csvData)[columnName].checked = checked;
                
                qDebug() << "Loaded checked state from template for column:" << columnName 
                         << "Checked:" << checked;
            }
        }
        templateSettings.endGroup();
        
        // Load template settings into DataManager
        if (!this->DataManager->settings) {
            this->DataManager->settings = new QSettings(localSettingsPath, QSettings::IniFormat);
        }
        copySettingsToDataManager(&templateSettings);
        
        // Load UI components
        this->DataManager->loadAxes(&templateSettings);
        this->DataManager->loadSeriesTable(&templateSettings);
        
        qDebug() << "Successfully loaded settings from template:" << templatePath;
    } else {
        qDebug() << "No matching template found";
    }
}

void Data::copySettingsToDataManager(QSettings* source) {
    if (!source || !this->DataManager || !this->DataManager->settings)
        return;
        
    // Copy all groups and keys from source to DataManager's settings
    QStringList groups = source->childGroups();
    for (const QString& group : groups) {
        source->beginGroup(group);
        this->DataManager->settings->beginGroup(group);
        
        QStringList keys = source->childKeys();
        for (const QString& key : keys) {
            QVariant value = source->value(key);
            this->DataManager->settings->setValue(key, value);
        }
        
        this->DataManager->settings->endGroup();
        source->endGroup();
    }
}

void Data::synchronizeTreeStates() {
    // Synchronize SysVar tree
    auto sysVarRoot = this->DataManager->getRootItem("SysVar");
    if (sysVarRoot) {
        qDebug() << "Synchronizing SysVar tree states...";
        
        // Function to recursively update tree items
        std::function<void(QTreeWidgetItem*)> updateSysVarTreeItem;
        updateSysVarTreeItem = [this, &updateSysVarTreeItem](QTreeWidgetItem* item) {
            if (!item) return;
            
            // Get the full path for this item
            QString itemPath;
            QTreeWidgetItem* current = item;
            while (current) {
                itemPath = current->text(0) + (itemPath.isEmpty() ? "" : "::" + itemPath);
                current = current->parent();
            }
            
            // If this is a leaf node (SysVar variable)
            if (item->childCount() == 0) {
                QString varName = itemPath;
                varName.replace("SysVar", "");
                if (SysVars->contains(varName) && !varName.startsWith("CSV::")) {
                    bool checked = SysVars->value(varName).checked;
                    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
                    qDebug() << "Setting SysVar tree item state for" << varName << "to" << checked;
                }
            }
            
            // Process children
            for (int i = 0; i < item->childCount(); ++i) {
                updateSysVarTreeItem(item->child(i));
            }
        };
        
        this->DataManager->ui->treeWidget->blockSignals(true);
        updateSysVarTreeItem(sysVarRoot);
        updateParentState(sysVarRoot);
    }
    
    // Synchronize CSV tree
    auto csvRoot = this->DataManager->getRootItem("CSV");
    if (csvRoot) {
        qDebug() << "Synchronizing CSV tree states...";
        
        // Function to recursively update tree items
        std::function<void(QTreeWidgetItem*)> updateCSVTreeItem;
        updateCSVTreeItem = [this, &updateCSVTreeItem](QTreeWidgetItem* item) {
            if (!item) return;
            
            // If this is a leaf node (CSV variable)
            if (item->childCount() == 0) {
                QString varName = "CSV::" + item->text(0);
                if (SysVars->contains(varName)) {
                    bool checked = SysVars->value(varName).checked;
                    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
                    qDebug() << "Setting CSV tree item state for" << varName << "to" << checked;
                }
            }
            
            // Process children
            for (int i = 0; i < item->childCount(); ++i) {
                updateCSVTreeItem(item->child(i));
            }
        };
        
        updateCSVTreeItem(csvRoot);
        updateParentState(csvRoot);
    }
    
    this->DataManager->ui->treeWidget->blockSignals(false);
}

void Data::initializeCsvSettings(const QString& csvPath) {
    QString localSettingsPath = getCsvSettingsPath(csvPath);
    
    qDebug() << "\n=== Initializing CSV Settings ===";
    qDebug() << "CSV file:" << csvPath;
    qDebug() << "Local settings path:" << localSettingsPath;
    
    // If local settings already exist, don't reinitialize
    if (QFile::exists(localSettingsPath)) {
        QSettings existingSettings(localSettingsPath, QSettings::IniFormat);
        existingSettings.beginGroup("CSVFormat");
        QString storedFingerprint = existingSettings.value("fingerprint").toString();
        existingSettings.endGroup();
        
        if (!storedFingerprint.isEmpty() && storedFingerprint != "0|0|") {
            qDebug() << "Valid local settings already exist with fingerprint:" << storedFingerprint;
            
            // Just set up the settings objects without overwriting
            if (this->DataManager->settings) {
                delete this->DataManager->settings;
            }
            if (this->DataManager->settingsTemplate) {
                delete this->DataManager->settingsTemplate;
            }
            
            this->DataManager->settings = new QSettings(localSettingsPath, QSettings::IniFormat);
            this->DataManager->settingsTemplate = new QSettings(
                getTemplatesFolderPath() + CsvFileParser->generateFingerprintCode() + ".ini", 
                QSettings::IniFormat
            );
            
            return;
        }
    }
    
    // Only proceed with initialization if no valid settings exist
    QString fingerprintCode = CsvFileParser->generateFingerprintCode();
    QString templatePath = getTemplatesFolderPath() + fingerprintCode + ".ini";
    
    qDebug() << "Template path:" << templatePath;
    
    qDebug() << "\n=== CSV Settings Status ===";
    qDebug() << "Before initialization:";
    qDebug() << "Settings:" << (this->DataManager->settings ? "exists" : "null");
    qDebug() << "Settings Template:" << (this->DataManager->settingsTemplate ? "exists" : "null");
    
    // Clean up existing settings if they exist
    if (this->DataManager->settings) {
        this->DataManager->settings->sync();
        delete this->DataManager->settings;
        this->DataManager->settings = nullptr;
    }
    
    if (this->DataManager->settingsTemplate) {
        this->DataManager->settingsTemplate->sync();
        delete this->DataManager->settingsTemplate;
        this->DataManager->settingsTemplate = nullptr;
    }
    
    // Create templates directory if it doesn't exist
    QDir templatesDir(getTemplatesFolderPath());
    if (!templatesDir.exists()) {
        qDebug() << "Creating templates directory:" << getTemplatesFolderPath();
        templatesDir.mkpath(".");
    }
    
    // Initialize new settings objects without writing any values
    this->DataManager->settings = new QSettings(localSettingsPath, QSettings::IniFormat);
    this->DataManager->settingsTemplate = new QSettings(templatePath, QSettings::IniFormat);
    
    // Save initial settings
    saveCsvSettings(csvPath);
    
    qDebug() << "\nAfter initialization:";
    qDebug() << "Settings path:" << this->DataManager->settings->fileName();
    qDebug() << "Template path:" << this->DataManager->settingsTemplate->fileName();
    qDebug() << "Settings status:" << this->DataManager->settings->status();
}

void Data::updateDataType(QString varName, DataType newType) {
    qDebug() << "\n=== Updating Data Type ===";
    qDebug() << "Updating data type for" << varName << "to" << static_cast<int>(newType);
    
    if (SysVars->contains(varName)) {
        // Update the data type in the SysVars hash
        (*SysVars)[varName].dataType = newType;
        
        // Update isOperatingModeSeries flag if needed
        if (newType == DataType::OperatingMode) {
            (*SysVars)[varName].isOperatingModeSeries = true;
        } else {
            (*SysVars)[varName].isOperatingModeSeries = false;
        }
        
        // Clear points if the data type is NotUsed or DateTime
        if (newType == DataType::NotUsed || newType == DataType::DateTime) {
            qDebug() << "Clearing points for" << varName << "as it is now" 
                     << (newType == DataType::NotUsed ? "NotUsed" : "DateTime");
            (*SysVars)[varName].points.clear();
        }
        
        // Update the CSVParser's data as well
        if (varName.startsWith("CSV::")) {
            QString columnName = varName.mid(5); // Remove "CSV::" prefix
            auto csvData = CsvFileParser->getData();
            if (csvData->contains(columnName)) {
                (*csvData)[columnName].dataType = newType;
                (*csvData)[columnName].isOperatingModeSeries = (newType == DataType::OperatingMode);
                qDebug() << "Updated CSVParser data for column:" << columnName 
                         << "dataType:" << static_cast<int>(newType);
            }
            
            // Save the settings to persist the change
            saveCsvSettings(fi.filePath());
        }
        
        qDebug() << "Updated data type for" << varName << "to" << static_cast<int>(newType);
    } else {
        qDebug() << "Error: Variable" << varName << "not found in SysVars";
    }
}

void Data::saveCsvSettings(const QString& csvPath) {
    QString localSettingsPath = getCsvSettingsPath(csvPath);
    
    qDebug() << "\n=== Saving CSV Settings ===";
    qDebug() << "Saving CSV settings for file:" << csvPath;
    qDebug() << "Settings path:" << localSettingsPath;
    
    // Debug: Print data types in SysVars before saving
    qDebug() << "\n=== Data Types in SysVars before saving ===";
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            qDebug() << "Column:" << it.key() 
                     << "Data Type:" << static_cast<int>(it.value().dataType)
                     << "Checked:" << it.value().checked;
        }
    }
    
    // Create settings object
    QSettings localSettings(localSettingsPath, QSettings::IniFormat);
    
    // Clear existing settings
    localSettings.clear();
    
    // First, update the CSVParser's data with the current data types from SysVars
    auto csvData = CsvFileParser->getData();
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            QString columnName = it.key().mid(5); // Remove "CSV::" prefix
            if (csvData->contains(columnName)) {
                // Update the data type in the CSVParser's data
                (*csvData)[columnName].dataType = it.value().dataType;
                (*csvData)[columnName].isOperatingModeSeries = it.value().isOperatingModeSeries;
                (*csvData)[columnName].checked = it.value().checked;
                
                qDebug() << "Updating CSVParser data for column:" << columnName 
                         << "dataType:" << static_cast<int>(it.value().dataType)
                         << "checked:" << it.value().checked;
            }
        }
    }
    
    // Now save the CSV format and data
    CsvFileParser->saveSettings(&localSettings);
    
    // Save additional data from SysVars
    localSettings.beginGroup("CSVData");
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            QString columnName = it.key().mid(5); // Remove "CSV::" prefix
            localSettings.setValue(QString("checked/%1").arg(columnName), it.value().checked);
            localSettings.setValue(QString("dataType/%1").arg(columnName), static_cast<int>(it.value().dataType));
            
            qDebug() << "Saving SysVars data for column:" << columnName 
                     << "checked:" << it.value().checked 
                     << "dataType:" << static_cast<int>(it.value().dataType);
        }
    }
    localSettings.endGroup();
    
    // Sync to ensure settings are written to disk
    localSettings.sync();
    
    // Debug: Print data types in CSVParser after saving
    qDebug() << "\n=== Data Types in CSVParser after saving ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType)
                 << "Checked:" << it.value().checked;
    }
    
    qDebug() << "CSV settings saved successfully";
}

void Data::parseFullCSV(QFileInfo file) {
    qDebug() << "\nParsing full CSV file for data loading";
    
    // Save the current checked status and data types before parsing
    QHash<QString, bool> checkedStatus;
    QHash<QString, DataType> dataTypes;
    QStringList checkedColumns;
    
    // Store the current checked status and data types
    for (auto it = SysVars->begin(); it != SysVars->end(); ++it) {
        if (it.key().startsWith("CSV::")) {
            QString columnName = it.key().mid(5); // Remove "CSV::" prefix
            checkedStatus[it.key()] = it.value().checked;
            dataTypes[it.key()] = it.value().dataType;
            
            if (it.value().checked) {
                checkedColumns.append(columnName);
            }
            
            qDebug() << "Saving state for" << it.key() 
                     << "checked:" << it.value().checked
                     << "dataType:" << static_cast<int>(it.value().dataType);
        }
    }
    
    timer.start();
    
    // Parse the entire CSV file to load all data for checked columns only
    if (checkedColumns.isEmpty()) {
        qDebug() << "No checked columns found, skipping full file parsing";
        return;
    }
    
    qDebug() << "Found" << checkedColumns.size() << "checked columns, parsing full file to get data for:" << checkedColumns.join(", ");
    
    // Clear existing points before parsing
    auto csvData = CsvFileParser->getData();
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        it.value().points.clear();
    }
    
    if (!CsvFileParser->parseFile(file.filePath(), 0, checkedColumns)) {  // 0 means no limit, only process checked columns
        qDebug() << "Failed to parse full CSV file";
        return;
    }
    
    qDebug() << "Full CSV file parsed successfully in" << timer.elapsed() << "ms";
    
    // Debug: Print data in CSVParser after parsing
    qDebug() << "\n=== Data in CSVParser after parsing ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType)
                 << "Points:" << it.value().points.size();
    }
    
    // Update the CSVParser's data with the saved checked status and data types
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        QString varName = "CSV::" + it.key();
        if (checkedStatus.contains(varName)) {
            (*csvData)[it.key()].checked = checkedStatus[varName];
            qDebug() << "Restoring checked status for" << varName 
                     << "to" << checkedStatus[varName];
        }
        if (dataTypes.contains(varName)) {
            (*csvData)[it.key()].dataType = dataTypes[varName];
            qDebug() << "Restoring data type for" << varName 
                     << "to" << static_cast<int>(dataTypes[varName]);
        }
    }
    
    // Debug: Print data in CSVParser after restoring state
    qDebug() << "\n=== Data in CSVParser after restoring state ===";
    for (auto it = csvData->begin(); it != csvData->end(); ++it) {
        qDebug() << "Column:" << it.key() 
                 << "Data Type:" << static_cast<int>(it.value().dataType)
                 << "Checked:" << it.value().checked
                 << "Points:" << it.value().points.size();
    }
    
    // Now proceed with creating UI elements
    CsvParsingComplete();
}

Data::~Data() {
  DBC->clear();
  delete DBC;
  delete DataManager;
  delete filesList;
}


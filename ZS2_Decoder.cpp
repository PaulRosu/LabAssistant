#include "ZS2_Decoder.h"
// #include "argvars.h"
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
// #include <QProgressDialog>
#include <QtWidgets/QApplication>

float noiseLevel = 0.05;
int interestPointsNr = 4;
float maxInterestPointsForce = 0.0;
// extern bool calloutDragging;

QString fileNameArg = "";
bool argFile = false;

MeasSerie loadData(QString fileName) {
  //    fileName = fileNameArg;
  //    MeasSpecimen* specimen = new MeasSpecimen;
  std::shared_ptr<MeasSpecimen> specimen(new MeasSpecimen);

  MeasSerie serie;
  serie.status = true;

  QMessageBox msgBox;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "File Open Failed!";

    serie.status = false;
    //        msgBox.setWindowTitle("File Open Error!");
    //        msgBox.setText("The file could not open! aborting...");
    //        msgBox.exec();

    return serie;
  }

  //    auto pd = new QProgressDialog("Decompressing " + fileName + "\nPlease
  //    wait...", "Cancel", 0, 0); pd->setValue (0); pd->resize(pd->size() +
  //    QSize(80, 20)); pd->show (); QApplication::processEvents();

  qDebug() << "File open success!";

  QByteArray decompressed;

  if (QCompressor::gzipDecompress(file.readAll(), decompressed)) {
    qDebug() << "Decompression successful! binary size:" << decompressed.size();
    file.close();
  } else {
    qDebug() << "Can't decompress!";
    serie.status = false;
    //        msgBox.setWindowTitle("Decompression Error!");
    //        msgBox.setText("The file could not be decompressed! aborting...");
    //        msgBox.exec();
    return serie;
  }

  QString folder = QString(fileName).mid(0, QString(fileName).length() - 4);
  serie.folder = folder;
  serie.fileName = fileName;

  QFile txtfile(folder + ".txt");
  QList<QString> txtfilenames;
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
  }

  //    QCoreApplication::setApplicationName( fileName );

  QDataStream in(decompressed);
  in.setByteOrder((QDataStream::LittleEndian));

  QByteArray buffer;

  uint32_t fourBytes;
  uint16_t twoBytes;
  uint8_t oneByte;

  uint8_t sectionType;
  QString sectionValue;
  uint8_t sectionNameSize;
  QString sectionName;
  uint8_t sectionSize_OneByte;
  uint32_t sectionSize_FourBytes;
  char rawCharBuffer[4096];
  QVector<double> doubleDataArray;
  QVector<float> singleDataArray;
  QVector<uint> boolDataArray;

  QString license;
  bool is_tunis_file = false;
  QString tunis_license = "14D9218F0037CCFE02597C6AAF4D16E89BD0B7633A0A5804AF12"
                          "EE01F9D03269762ECC3926DDAFFE7D19";

  bool sampleNameHere = false;

  QStringList cycles;

  int sampleCnt = 0;
  bool inSampleNameZone = false;
  bool inSamplesZone = false;
  bool inSampleDataZone = false;
  bool inCycleNameZone = false;

  QString sampleName;

  QByteArray sectionDataArray;
  int32_t sectionData;

  QList<QFuture<void>> futurelist;

  in >> fourBytes;
  qDebug() << "File signature:" << QString::number(fourBytes, 16).right(8)
           << QDateTime::currentMSecsSinceEpoch();
  if (fourBytes == 0xDEADBEAF) {
    qDebug() << "File signature is correct!";
  } else {
    qDebug() << "File signature is incorrect!";

    serie.status = false;
    msgBox.setWindowTitle("File Signature Error!");
    msgBox.setText("The file signature is not correct! aborting...");
    msgBox.exec();

    return serie;
  }

  auto timestamp1 = QDateTime::currentMSecsSinceEpoch();

  //    pd->setLabelText ("Parsing " + fileName + "\nPlease wait...");
  while (!in.atEnd()) {

    //        QApplication::processEvents();

    in >> sectionNameSize;       // first byte containg section name size.
    if (sectionNameSize == 0xFF) // section end
    {
      continue;
    }

    in.readRawData(rawCharBuffer,
                   sectionNameSize); // the section name utf8-encoded
    sectionName = QString::fromUtf8(rawCharBuffer, sectionNameSize);

    //        if (sectionName == "Modifikator") qDebug() << "Username Gasit!";

    if ("SeriesElements" == sectionName) {
      if (not inSamplesZone)
        qDebug() << "Entering samples zone..." << Qt::endl;
      inSamplesZone = true;
    }

    if ("SingleGroupDataBlock" == sectionName) {
      inSampleDataZone = true;
      qDebug() << "Data section nr" << sampleCnt << " found!";
    }

    in >> sectionType; // one byte section type identifier
    //        qDebug() << "Section: " << sectionName << " type: "
    //                 << QString::number(sectionType,16).toUpper();

    switch (sectionType) {

    case 0xDD: // structural section names. depending on context it may have or
               // not a utf8-encoded value
      in >> sectionSize_OneByte;
      if (sectionSize_OneByte > 0) {
        in.readRawData(rawCharBuffer, sectionSize_OneByte);
        sectionValue = QString::fromUtf8(rawCharBuffer, sectionSize_OneByte);
        // if (inSampleDataZone) qDebug() << "Section: " << sectionValue;
      }
      break;

    case 0xAA:                     // arrays, mostly unicode utf16 strings.
      in >> sectionSize_FourBytes; // 31st bit is flagging whether the array
                                   // containg strings.
      // knock off 31st bit flag and double it for utf16 string
      sectionSize_FourBytes = (sectionSize_FourBytes & 0x7FFFFFFF) * 2;

      if (sectionName == "Modifikator") {
        in.readRawData(rawCharBuffer,
                       sectionSize_FourBytes); // the section name utf8-encoded
        QString username =
            QString::fromUtf16(reinterpret_cast<char16_t *>(rawCharBuffer),
                               sectionSize_FourBytes / 2);
        qDebug() << "Modifikator:" << username;
        specimen->username = username;
        username = "";
      }
      //                    else if (sectionName == "LizenzInfo"){
      //                            in.readRawData(rawCharBuffer,
      //                            sectionSize_FourBytes);// the section name
      //                            utf8-encoded license =
      //                            QString::fromUtf16(reinterpret_cast<char16_t*>(rawCharBuffer),
      //                            sectionSize_FourBytes / 2); qDebug () <<
      //                            "LizenzInfo:" << license; if (license ==
      //                            tunis_license) is_tunis_file = true;
      ////                            specimen->username = username;
      ////                            license = "";
      //                        }
      else {
        in.skipRawData(sectionSize_FourBytes);
      }
      break;

    case 0x66: // 2 bytes int used for flags and id's. sample name id is 48154
      in >> twoBytes;
      sectionData = twoBytes;

      //              qDebug () << sectionName << sectionData;

      if ((sectionData == 48154) and (sectionName == "ID") and
          (inSamplesZone == true)) {
        qDebug() << "Sample Name location found! " << twoBytes;
        inSampleNameZone = true;
        if (!is_tunis_file)
          sampleNameHere = true;

        qDebug() << "inSampleNameZone"
                 << "is tunis:" << is_tunis_file;
      }

      if ((sectionData == 1105) and (sectionName == "ID") and
          (inSampleNameZone == true) and is_tunis_file) {
        qDebug() << "Is Tunis file, name should be next" << twoBytes;
        sampleNameHere = true;
      }

      if ((sectionData == 18655) and
          (sectionName == "ID")) // and (inSamplesZone == true)
      {
        qDebug() << "Entering cycle names location..." << twoBytes;
        inCycleNameZone = true;
      }

      if ((sectionData == 18656) and
          (sectionName == "ID")) // and (inSamplesZone == true)
      {
        qDebug() << "Leaving section names location..." << twoBytes;
        inCycleNameZone = false;
      }

      break;

    case 0x11: // 4 bytes parameter. number of lines is this type.
      in >> fourBytes;
      if (inSampleDataZone and (sectionName == "NumberOfLines")) {
        qDebug() << "Number of lines: " << fourBytes;

        specimen->samples = fourBytes;

        if (fourBytes == 0) {
          qDebug() << "This sample has no measurements!";
          inSampleDataZone = false;
        }
      }

      if ((inSamplesZone) and (sectionName == "TrsChannelId")) {

        //                 qDebug()<<  "TrsChannelId" << fourBytes;

        switch (fourBytes) {
          // id 40402 - "Standard load cell"
          // id 40403 - "Standard extensometer"
          // id 27700 - "Measurement channel Inputs/Outputs 1"
          // id 27677 - "Cycles counter, dynamic"
          // id 27710 - "Cycle section counter, dynamic"
          // id 27675 - meas validation - using it to flag data section end

        case 27700: // "Measurement channel Inputs/Outputs 1" - 4 bytes
          qDebug() << "Electrical Contact [bool]";

          specimen->contact1 =
              QVector<uint>(singleDataArray.begin(), singleDataArray.end());
          qDebug() << "Electrical Contact [bool]" << specimen->contact1.size();

          //                    for (int n = 0; n < specimen->contact1.size ();
          //                    n++){
          //                        qDebug() << "Electrical Contact n=" << n <<
          //                        " val:" << specimen->contact1.at (n);
          //                    }
          break;

        case 40402: // "Standard load cell" - 4 bytes
          qDebug() << "Force [N]";
          specimen->force = singleDataArray;
          break;

        case 40403: // "Standard extensometer" - 4 bytes
          qDebug() << "Travel [mm]";
          specimen->travel = singleDataArray;
          break;

        case 40400: // "System time sensor" - 8 bytes
          qDebug() << "Time [s]";
          specimen->time = doubleDataArray;

          //$$$$$$

          if (inSampleDataZone) {
            inSampleDataZone = false;
            qDebug() << "Leaving data section..." << Qt::endl;

            //******** interest points searching section **********

            //                        pd->setLabelText ("Searching interest
            //                        points on " + specimen->name + "\nPlease
            //                        wait..." ); QApplication::processEvents();

            QFuture<void> result =
                QtConcurrent::run(&findInterestPoints, specimen, &serie);
            futurelist.append(result);

            // findInterestPoints (specimen, &serie);

            //                                            specimen = new
            //                                            MeasSpecimen;

            std::shared_ptr<MeasSpecimen> tempspecimen(new MeasSpecimen);
            specimen = tempspecimen;

          } // ## int points search end

          //$$$$$

          break;

        case 27675: // "Sequence cycle number" - 4 bytes

          qDebug() << "Sequence cycle number [nr]";
          specimen->cycle =
              QVector<uint>(singleDataArray.begin(), singleDataArray.end());

          if (inSampleDataZone) {
            inSampleDataZone = false;
            qDebug() << "Leaving data section..." << Qt::endl;

            //******** interest points searching section **********

            //                        pd->setLabelText ("Searching interest
            //                        points on " + specimen->name + "\nPlease
            //                        wait..." ); QApplication::processEvents();

            QFuture<void> result =
                QtConcurrent::run(&findInterestPoints, specimen, &serie);
            futurelist.append(result);

            //                                            findInterestPoints
            //                                            (specimen, &serie);

            //                                            specimen = new
            //                                            MeasSpecimen;
            std::shared_ptr<MeasSpecimen> tempspecimen(new MeasSpecimen);
            specimen = tempspecimen;

          } // ## int points search end
          break;
        }
      }

      break;

    case 0x22: // various 4 bytes parameters .skipped
    case 0x33:
    case 0x44:
    case 0xBB:
      if (sectionName == "ProgVersion") {
        in.readRawData(rawCharBuffer, 4);
        union Convert {
          unsigned char byte[4];
          float real;
        };

        Convert convert;
        convert.byte[0] = rawCharBuffer[0];
        convert.byte[1] = rawCharBuffer[1];
        convert.byte[2] = rawCharBuffer[2];
        convert.byte[3] = rawCharBuffer[3];

        QString prog_version = QString::number(convert.real);
        qDebug() << "Program Version:" << prog_version;
        //                if (prog_version == QString("4.2")) is_tunis_file =
        //                true; qDebug () << "Is tunis file:" << is_tunis_file;

        if (prog_version.contains("4."))
          is_tunis_file = true;
        qDebug() << "Is version 4.x:" << is_tunis_file;

      } else {
        in.skipRawData(4);
      }
      break;

    case 0x88: // various 1 byte parameters. skipped.
    case 0x99:
      in.skipRawData(1);
      break;

    case 0xCC: // various 8 bytes float parameters. skipped.
      in.skipRawData(8);
      break;

    case 0x55: // two bytes int parameter. skipped.
      in.skipRawData(2);
      break;

    case 0xEE: // various types collection. 2 bytes subtype. second one unused.
      in >> sectionType;
      in >> oneByte;
      //            qDebug() << "0xEE subType: "  <<
      //            QString::number(sectionType,16);

      switch (sectionType) {
      case 0x00: // empty list, still have 4 bytes size
        in >> sectionSize_FourBytes;
        // if size non-zero, try skipping the size in bytes. unsafe.
        if (sectionSize_FourBytes > 0)
          in.skipRawData(sectionSize_FourBytes);
        break;

      case 0x04: // 4 bytes single-precision floating point. measurements are
                 // this type
        // id 40402 - "Standard load cell"
        // id 40403 - "Standard extensometer"
        // id 27700 - "Measurement channel Inputs/Outputs 1"
        // id 27677 - "Cycles counter, dynamic"
        // id 27710 - "Cycle section counter, dynamic"
        // id 27675 - meas validation - using it to flag data section end

        in >>
            sectionSize_FourBytes; // the size indicates the number of elements!
        if (!inSampleDataZone) {
          in.skipRawData(sectionSize_FourBytes * 4);
          break;
        }

        //                qDebug() << "Array size: " << sectionSize_FourBytes;

        singleDataArray.resize(sectionSize_FourBytes);

        in.readRawData((reinterpret_cast<char *>(singleDataArray.data())),
                       sectionSize_FourBytes * 4);

        break;

      case 0x05: // 8 bytes double-precision floating point. meas time is this
                 // type.
        // id 40400 - "System time sensor"
        in >> sectionSize_FourBytes;
        if (!inSampleDataZone) {
          in.skipRawData(sectionSize_FourBytes * 8);
          break;
        }

        doubleDataArray.resize(sectionSize_FourBytes);

        in.readRawData((reinterpret_cast<char *>(doubleDataArray.data())),
                       sectionSize_FourBytes * 8);
        break;

      case 0x11: // 1 byte data context dependent structures. sample names are
                 // here.
        in >> sectionSize_FourBytes;
        //                qDebug () << "EE11 section size:" <<
        //                sectionSize_FourBytes;

        if ((sectionName == "QS_TextPar") and
            inSampleNameZone == true) // and sampleNameHere //48154
        {
          // proceed to extract the sample name.

          in >> fourBytes; // structure type area
          in >> oneByte;   // first struct element

          for (int x = 0; x < (int)sectionSize_FourBytes - 5;
               x++) // max custom name is less than 30 chars //30
          {
            in >> oneByte; // get one byte from the utf16 string param
            if ((QString(QChar(static_cast<char>(oneByte))) == "\u0007") or
                (QString(QChar(static_cast<char>(oneByte))) == "\u0006") or
                is_tunis_file) // next param boundary //r (QString(oneByte) ==
                               // "\u0006")
            {
              in.skipRawData(sectionSize_FourBytes - 5 - x * 2 -
                             1); // skip remaining bytes
              break;
            }
            sampleName.append(QString(QChar(static_cast<char>(oneByte))));
            in >> oneByte; // next (useless) byte from the utf16 string param.
          }
          sampleCnt++; // new sample name retrieved.

          // if no user defined name, check in the txt names file at the sample
          // nr index;
          // else, default "Specimenx" name is used.
          if (sampleName == "") {
            if (txtfilenames.size() >= sampleCnt) {
              sampleName = txtfilenames.at(sampleCnt - 1);
            } else {
              sampleName = "Specimen_" + QString("%1").arg(sampleCnt, 2, 10,
                                                           QLatin1Char('0'));
            }
          }

          qDebug() << "Specimen" << sampleCnt << "name: " << sampleName;

          specimen->name = sampleName;

          sampleName.clear();
          inSampleNameZone = false;
          break;
        }

        if ((sectionName == "QS_SelProp") and inCycleNameZone) {
          // proceed to extract the sample name.

          in >> fourBytes; // structure type area
          // in >> oneByte;   // first struct element

          QString text;

          in.readRawData(rawCharBuffer, sectionSize_FourBytes - 4);

          text = QString::fromUtf16(reinterpret_cast<char16_t *>(rawCharBuffer),
                                    (sectionSize_FourBytes - 4) / 2);

          QString cured;

          for (int i = 0; i < text.size(); ++i) {
            char latin1Char = text.at(i).toLatin1();
            if (static_cast<int>(latin1Char) > 31) {
              cured.append(QChar(latin1Char));
            }
          }

          cured = cured.replace("  ", ";");
          cured = cured.replace(" ", "");
          cured = cured.replace(";", " ");
          cured = cured.replace("gb ", "gb");
          cured = cured.replace(" 1252:gb", "1252:gb");
          cured = cured.trimmed();
          cured = cured.remove("1252:gb1252:gb1252:gb");
          cured = cured.remove("1252:gb1252:gb");

          // qDebug() << cured << endl;
          cycles = cured.split("1252:gb");

          if (cycles.size() < 2) {
            cycles.append("NO DATA");
            cycles.append("NO DATA");
          }
          //                    qDebug() << cycles << Qt::endl;
          serie.cycles = cycles;

          inCycleNameZone = false;
          break;
        }

        in.skipRawData(sectionSize_FourBytes);
        break; // end case 0xEE:0x11

      case 0x16: // 4 bytes int as bool. skipping.
        in >> sectionSize_FourBytes;
        in.skipRawData(sectionSize_FourBytes * 4);
        break;

      default:
        qDebug() << "Error: Unknown suntype! aborting...";
        serie.status = false;
        msgBox.setWindowTitle("Parsing Error!");
        msgBox.setText("Unknown suntype! aborting..." +
                       QString::number(sectionType, 16));
        msgBox.exec();
        return serie;
        break;
      }      // end switch 0xEE subtype
      break; // end case 0xEE

    default: // unidentified section
      //                    msgBox.setWindowTitle("Error: unidentified section
      //                    type!"); msgBox.setText("Unidentified section type:
      //                    " + QString::number(sectionType,16)); msgBox.exec();
      qDebug() << "Error: unidentified section type!";
      qDebug() << "Unidentified section type: " +
                      QString::number(sectionType, 16);

      serie.status = false;
      return serie;
      break;

    } // end select sectionType
  }   // end while file is not at end
  qDebug() << "Reached file END!" << Qt::endl;

  //    pd->setLabelText ("Saving CSV files. Please wait...");
  //    QApplication::processEvents();

  for (QFuture<void> result : futurelist) {
    result.waitForFinished();
  }

  //    int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << timestamp2 - timestamp1;
  //    timestamp1 = QDateTime::currentMSecsSinceEpoch();

  std::string fname = serie.fileName.toStdString();
  int index = fname.find_last_of("/\\");
  std::string input_trace_filename = fname.substr(index + 1);
  QString seriename = QString::fromStdString(input_trace_filename);
  seriename = seriename.mid(0, seriename.size() - 4);
  serie.name = serie.folder + "\\" + seriename;

  //    QDir().mkdir(serie.folder);
  //    QDir().mkdir(serie.folder + "\\pics");

  //    QFile::copy(QCoreApplication::applicationDirPath() + "\\Graphs.xlsm",
  //    serie.folder + "\\" + seriename + "_Graphs.xlsm");

  //      QFile::copy(QCoreApplication::applicationDirPath() +
  //      "\\Graphs_Misuse.xlsm", serie.folder + "\\" + "Graphs_Misuse.xlsm");
  //   QFile::copy(QCoreApplication::applicationDirPath() + "\\Graphs_BMW.xlsm",
  //   serie.folder + "\\" + "Graphs_BMW.xlsm"); qDebug () << serie.folder +
  //   "\\" + "Graphs_BMW.xlsm" ; qDebug () << serie.folder + "\\" +
  //   "Graphs_Misuse.xlsm" ;
  qDebug() << serie.folder + "\\" + "Graphs.xlsm";

  //    futurelist.clear ();
  //    for (MeasSpecimen specimen : serie.specimenList){
  //    QFuture<void> result = QtConcurrent::run(&saveCSV, &specimen, &serie);
  //    futurelist.append (result);
  //    }

  //    for (QFuture<void> result : futurelist){
  //        result.waitForFinished ();
  //    }

  QString fileContents;
  QFile csvFile;

  //    for (MeasSpecimen serie : serie.specimenList)
  //    {
  //        //        QApplication::processEvents();
  //        //        qDebug()<< "Aici 1!" ;

  //        fileContents.append ("Travel [mm];"
  //                             "Force [N];"
  //                             "Electrical Contacts [bin];"
  //                             "Sequence [nr];"
  //                             "Int. Points[row];"
  //                             "El. Cont. SW1[row];"
  //                             "El. Cont. SW1[state];"
  //                             "El. Cont. SW2[row];"
  //                             "El. Cont. SW2[state];"
  //                             "username:;" +
  //                             specimen->username +
  //                             ";cycle1:;" +
  //                             ";"
  //                             ";cycle2:;" +
  //                             "" +

  //                             QLocale().toString(serie.maxForce_row) +
  //                             ";BMW_F3;" +
  //                             QLocale().toString(serie.BMW_F3_row - 8 + 2) +
  //                             "\n");

  //        //cycle1:;" +
  //        //cycles.at(0) +
  //        //";cycle2:;" +
  //        //cycles.at(1) + ";" +
  //        //cycles.at(1) + ";" +

  //        QList<uint> interestPointsLocationsList = serie.InterestPointsList;
  //        QList<uint> SW1_state = serie.SW1_state;
  //        QList<uint> SW2_state = serie.SW2_state;
  //        QList<uint> SW1_row = serie.SW1_row;
  //        QList<uint> SW2_row = serie.SW2_row;
  //        int intpoint = 0;
  //        for (uint l = 8; l < serie.samples; l++)
  //        {
  //            //        qDebug()<< "Aici 2!" ;
  //            QString currentCSVLine =
  //                    QLocale().toString(serie.travel.at (l) -
  //                    serie.approachDistance) + ";"
  //                    //QLocale().toString(serie.time.at (l) - serie.time.at
  //                    (serie.approachRow)) + ";"
  //                    + QLocale().toString(serie.force.at (l))    + ";"
  //                    + ";" //QLocale().toString(serie.contact1.at (l)) + ";"
  //                    ";" ;//+ QLocale().toString(serie.cycle.at (l))    +

  //            if (!interestPointsLocationsList.isEmpty ())
  //            {
  //                intpoint = interestPointsLocationsList.takeFirst ();
  //                if (intpoint == 0)
  //                {
  //                    currentCSVLine += ";";
  //                    //                    currentCSVLine = currentCSVLine +
  //                    ";";
  //                }
  //                else if (intpoint > 0)
  //                {
  //                    currentCSVLine = currentCSVLine
  //                            + QString::number (intpoint)
  //                            + ";";
  //                }
  //            }

  //            if (!SW1_row.isEmpty ())
  //                currentCSVLine = currentCSVLine
  //                        + QString::number (SW1_row.takeFirst ())
  //                        + ";";

  //            if (!SW1_state.isEmpty ())
  //                currentCSVLine = currentCSVLine
  //                        + QString::number (SW1_state.takeFirst ())
  //                        + ";";

  //            if (!SW2_row.isEmpty ())
  //                currentCSVLine = currentCSVLine
  //                        + QString::number (SW2_row.takeFirst ())
  //                        + ";";

  //            if (!SW2_state.isEmpty ())
  //                currentCSVLine = currentCSVLine
  //                        + QString::number (SW2_state.takeFirst ())
  //                        + ";";

  //            currentCSVLine += "\n";

  //            fileContents.append (currentCSVLine);
  //        }
  //        interestPointsLocationsList.clear ();

  //        SW1_state.clear ();
  //        SW1_row.clear ();
  //        SW2_state.clear ();
  //        SW2_row.clear ();

  //        csvFile.setFileName (folder + "\\" + serie.name + ".csv");
  //        qDebug() << "CSV File: " << folder + "\\" + serie.name + ".csv";
  //        csvFile.open (QIODevice::WriteOnly);

  //        csvFile.write(fileContents.toUtf8 ());
  //        csvFile.close ();
  //        fileContents.clear ();
  //        //        QApplication::processEvents();
  //    }

  //    pd->setValue (100);
  //    pd->setLabelText ("Complete!");
  //    QApplication::processEvents();
  //    pd->close ();
  //    QApplication::processEvents();

  auto timestamp2 = QDateTime::currentMSecsSinceEpoch();
  qDebug() << timestamp2 - timestamp1;

  qDebug() << "serie empty:" << serie.specimenList.isEmpty();

  if (!serie.specimenList.isEmpty()) {
    qDebug() << serie.specimenList.at(0).samples;
  } else {
    serie.status = false;
    return serie;
  }

  return serie;
}

// void findInterestPoints (MeasSpecimen *specimen, MeasSerie *serie)
void findInterestPoints(std::shared_ptr<MeasSpecimen> specimen,
                        MeasSerie *serie) {
  qDebug() << Qt::endl
           << Qt::endl
           << "Seasgdfrching interest points in " << specimen->name
           << specimen->samples << " measurements";
  int timestamp1 = QDateTime::currentMSecsSinceEpoch();

  // *** find the approach distance, before the actuator touches the sample
  float approachDistance = 0;

  int st = 9;
  int en = 9;
  while (specimen->force.at(en) - specimen->force.at(st) < 0.15) // ** 0.5
  {
    // find a rising slope of at least 0.5N
    if (en == (specimen->force.size() - 1)) {
      en = en - 3;
      break;
    }

    if (specimen->force.at(en + 1) > specimen->force.at(en)) {
      // if rising, move the end of the range forward
      en++;
    } else // reset the range ends at current position
    {
      st = en;
      en += 1;
    }
  }

  approachDistance = specimen->travel.at(st - 9);

  if (specimen->force.at(3) >= 0.15)
    approachDistance = 0;

  specimen->approachDistance = approachDistance;
  specimen->approachRow = st; // + 2

  // ***
  qDebug() << "specimen->approachDistance in zs2 module"
           << specimen->approachDistance;

  // *** find the left bound for the upper curve
  st = en; // start from the approaching distance
  en = st + 1;
  while (specimen->force.at(st) <= specimen->force.at(en)) {
    // and move forward while the force is not falling
    en++;
    st++;
  }
  int leftUpperBound = en - 15;
  // ***

  // *** find the maximum force and it's location
  auto maxElemIter =
      std::max_element(specimen->force.begin(), specimen->force.end());
  float maxForce = *maxElemIter;

  int maxForceLocation = std::distance(specimen->force.begin(), maxElemIter);
  specimen->maxForce_row = maxForceLocation;

  if (maxForce > 50) {

    // append the specimen to the serie
    serie->specimenList.append(*specimen);

    // clear the lists; maybe not needed
    specimen->InterestPointsList.clear();
    specimen->IP_TravelList.clear();
    specimen->IP_ForceList.clear();
    specimen->sequence.clear();

    specimen->SW1_state.clear();
    specimen->SW1_row.clear();
    specimen->SW2_state.clear();
    specimen->SW2_row.clear();

    //        timestamp2 = QDateTime::currentMSecsSinceEpoch();
    //        qDebug() << "interest point found in ms" << timestamp2 -
    //        timestamp1;

    return; // most probably misuse test, so concav/convex not relevant
  }

  // interest points must have at least 10% lower force.
  // 85% is better, but there are many curves for defective buttons with do not
  // respect this
  maxInterestPointsForce = maxForce * 0.9;

  // find max Higher curve Interest Points Force Location
  uint maxHInterestPointsForceLocation = maxForceLocation;
  while ((specimen->force.at(maxHInterestPointsForceLocation) -
          maxInterestPointsForce) > 0.1) {
    maxHInterestPointsForceLocation--;
  }
  // ***

  // *** find the right bound for the higher curve
  st = maxHInterestPointsForceLocation - 1; // start from the max force location
  en = maxHInterestPointsForceLocation;
  while (specimen->force.at(st) < specimen->force.at(en)) {
    // go back while the force is falling
    st--;
    en--;
  }
  int rightUpperBound = st + 3; // 15
  // ***

  // *** find max Interest Points Force Location for the lower curve
  uint maxLInterestPointsForceLocation = maxForceLocation;
  while (specimen->force.at(maxLInterestPointsForceLocation) -
             maxInterestPointsForce >
         0.1) {
    maxLInterestPointsForceLocation++;
  }
  // ***

  // *** finf the right bound for the lower curve
  st = maxLInterestPointsForceLocation;
  en = maxLInterestPointsForceLocation + 1;
  while (specimen->force.at(st) > specimen->force.at(en)) {
    en++;
    st++;
  }
  int rightLowerBound = st - 3; // 15
  // ***

  // *** find the lower curve left bound
  st = specimen->travel.size() - 1; // start from the vector end
  en = st - 2;

  // optimizare claudius
  while ((abs(specimen->travel.at(st)) - specimen->travel.at(en)) < 0.05) {
    st--;
  }
  en = st - 2;

  while (specimen->force.at(en) - specimen->force.at(st) < 0.2) {
    if (en <= 1) {
      break;
    }

    // find a rising force slope of 0.3N
    if (specimen->force.at(en - 1) > specimen->force.at(en)) {
      en--;
    } else {
      st = en;
      en -= 1;
    }
  }

  st = en; // from the 0.3N slope end
  en = st - 1;
  int leftLowerBound = 0;
  if (en > 1) {
    while (specimen->force.at(en) >= specimen->force.at(st)) {
      // go up as long as the force is not falling
      en--;
      st--;
    }
    leftLowerBound = en + 30; // + 15
  }
  // ***

  // *** find a radius in which to evaluate the peaks for higher and lower
  // curves
  float interestPointsRadiusL = 0;
  float interestPointsRadiusL_HigerCurve =
      (specimen->travel.at(rightUpperBound) -
       specimen->travel.at(leftUpperBound)) *
      0.33;

  float interestPointsRadiusL_LowerCurve =
      (specimen->travel.at(rightLowerBound) -
       specimen->travel.at(leftLowerBound)) *
      0.33;

  interestPointsRadiusL = interestPointsRadiusL_HigerCurve;

  // if (interestPointsRadiusL < 0.02) interestPointsRadiusL = 0.02; // !
  //  ***

  //    qDebug() << specimen->name << "leftLowerBound" <<
  //    specimen->travel.at(leftLowerBound) -approachDistance
  //             <<  "leftUpperBound" << specimen->travel.at(leftUpperBound)
  //             -approachDistance << Qt::endl
  //              << "rightLowerBound" << specimen->travel.at (rightLowerBound)
  //              -approachDistance
  //              <<  "rightUpperBound" << specimen->travel.at (rightUpperBound)
  //              -approachDistance << Qt::endl
  //               <<  "interestPointsRadiusL" << interestPointsRadiusL;

  float bendfactor = 0.0;

  int timestamp2 = QDateTime::currentMSecsSinceEpoch();
  //    qDebug() << "interest point prep in ms" << timestamp2 - timestamp1;

  // prepare some lists to store the interest points location
  QList<int> upconcave;
  QList<int> upconvexe;
  QList<int> downconvexe;
  QList<int> downconcave;

  uint backBound = 0;
  uint forwardBound = 0;

  // ** search for interest points in the valid range
  for (int l = leftUpperBound; l < leftLowerBound; l++) {
    //        QApplication::processEvents();

    // in mech stop area
    if (l > rightUpperBound + 3 and l < rightLowerBound - 3) ////10
    {
      l = rightLowerBound - 3;
      continue;
    }
    // if (specimen->force.at(l) < 0.5) continue; //noise

    if (specimen->force.at(l) >= maxInterestPointsForce)
      continue;

    if ((specimen->force.size() < 1000) and
        (specimen->force.at(l) >= (maxInterestPointsForce * 0.8)))
      continue;

    // check if local convex peak
    if (specimen->force.at(l) > specimen->force.at(l + 3) and
        specimen->force.at(l) > specimen->force.at(l - 3)) {

      // choose the upper\lower radius
      if (l < rightUpperBound + 3)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve; // 10
      if (l > rightLowerBound - 3)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve; // 10

      // *** find the bounds in which to evaluate the peak
      backBound = 0;
      forwardBound = 0;
      for (uint j = l; j < specimen->samples - 8; ++j) {
        // if reached the max force location, set the bound there
        if (j == maxHInterestPointsForceLocation) {
          forwardBound = j;
          break;
        }

        // if reached the force changed with 0.4N, set the bound there
        if (abs((specimen->force.at(l) - specimen->force.at(j))) >
            specimen->differential_factor) {
          forwardBound = j; //
          break;
        }
        // if reached the radius distance, set the bound there
        if (abs(specimen->travel.at(j) - specimen->travel.at(l)) >
            interestPointsRadiusL) {
          forwardBound = j;
          break;
        }
      }

      if (forwardBound == 0)
        continue;

      for (uint j = l; j > 8; --j) {
        if (j == maxLInterestPointsForceLocation) {
          backBound = j;
          break;
        }

        if (abs((specimen->force.at(l) - specimen->force.at(j))) >
            specimen->differential_factor) {
          backBound = j;
          break;
        }
        if (abs(specimen->travel.at(l) - specimen->travel.at(j)) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }
      // ***

      //                        qDebug ()<< " local convex" <<
      //                        specimen->force.at (l) << specimen->travel.at
      //                        (l)
      //                                 << "backBound" << specimen->travel.at
      //                                 (backBound)  - approachDistance
      //                                 << "forwardBound" <<
      //                                 specimen->travel.at (forwardBound)  -
      //                                 approachDistance;

      if (*std::max_element(&specimen->force.at(backBound),
                            &specimen->force.at(forwardBound)) ==
          specimen->force.at(l)) {

        // the ration between the change in force (abs Dy) from the point to the
        // radius extremes and the radius length (x)
        bendfactor = abs(abs((specimen->force.at(forwardBound) -
                              abs(specimen->force.at(l)))) +
                         abs((specimen->force.at(backBound) -
                              abs(specimen->force.at(l))))) /
                     interestPointsRadiusL;

        //                qDebug () << specimen->name << "convex point found: "
        //                << specimen->travel.at (l) - approachDistance
        //                          << "mm" << specimen->force.at (l)
        //                          << "N" << " point on row nr: "
        //                          << l - 8 + 2
        //                          << "bend factor:" << bendfactor
        //                          << "f at leftUpperBound" <<
        //                          specimen->force.at (leftUpperBound)
        //                          << "f at rightUpperBound" <<
        //                          specimen->force.at (rightUpperBound)
        //                           << (l < rightUpperBound +3) <<  (l >
        //                           rightLowerBound -3)

        //                             ;

        if (bendfactor < 0.85)
          continue; // imperceptible

        if (l < rightUpperBound + 3)
          upconvexe.append(l); // 10
        if (l > rightLowerBound - 3)
          downconvexe.append(l); // 10

        l = forwardBound -
            ((forwardBound - l) / 2); // move the cursor half the forward bound

        //                 qDebug () << specimen->name << "la convex: " <<
        //                 upconvexe << upconcave << downconvexe << downconcave;

        continue;
      }
    }

    // check if local concav peak, and apply the same startegy to find the
    // interest points
    if (specimen->force.at(l) < specimen->force.at(l + 3) and
        specimen->force.at(l) < specimen->force.at(l - 3)) // local concav peak
    {

      if (l < rightUpperBound + 10)
        interestPointsRadiusL = interestPointsRadiusL_HigerCurve;
      if (l > rightLowerBound - 10)
        interestPointsRadiusL = interestPointsRadiusL_LowerCurve;

      backBound = 0;
      forwardBound = 0;

      for (uint j = l; j < specimen->samples - 8; ++j) {
        if (j == maxHInterestPointsForceLocation + 10) {
          forwardBound = j;
          break;
        }

        if (abs((specimen->force.at(l) - specimen->force.at(j))) >
            specimen->differential_factor) {
          forwardBound = j;
          break;
        }

        if (abs(specimen->travel.at(j) - specimen->travel.at(l)) >
            interestPointsRadiusL) {
          forwardBound = j;
          break;
        }
      }

      if (forwardBound == 0)
        break;

      for (uint j = l; j > 8; --j) {
        if (j == maxLInterestPointsForceLocation - 10) {
          backBound = j;
          break;
        }

        if (abs((specimen->force.at(l) - specimen->force.at(j))) >
            specimen->differential_factor) {
          backBound = j;
          break;
        }

        if (abs(specimen->travel.at(l) - specimen->travel.at(j)) >
            interestPointsRadiusL) {
          backBound = j;
          break;
        }
      }

      //            qDebug () << " local concav"
      //                      << "backBound" << specimen->travel.at (backBound)
      //                      - approachDistance
      //                      << "forwardBound" << specimen->travel.at
      //                      (forwardBound) - approachDistance;

      if (*std::min_element(&specimen->force.at(backBound),
                            &specimen->force.at(forwardBound)) ==
          specimen->force.at(l)) {

        bendfactor = abs(abs((specimen->force.at(forwardBound) -
                              abs(specimen->force.at(l)))) +
                         abs((specimen->force.at(backBound) -
                              abs(specimen->force.at(l))))) /
                     interestPointsRadiusL;

        //                qDebug () << specimen->name <<  "concav point found: "
        //                << specimen->travel.at (l) - approachDistance
        //                          << "mm" << specimen->force.at (l)
        //                          << "N" << " point on row nr: "
        //                          << l - 8 + 2
        //                          << "bend factor:" << bendfactor;

        if (bendfactor < 0.85)
          continue;

        if (l < rightUpperBound + 10)
          upconcave.append(l);
        if (l > rightLowerBound - 10)
          downconcave.append(l);

        l = forwardBound - ((forwardBound - l) / 2);
      }
    }
  }

  //        qDebug () << specimen->name << "initial: " << upconvexe << upconcave
  //        << downconvexe << downconcave;

  //    qDebug () <<  upconcave;
  //    qDebug () <<  upconvexe;
  //    qDebug () <<  downconvexe;
  //    qDebug () <<  downconcave;

  // ** sort the found points

  // sort the convex points on the upper curve
  if (upconvexe.size() > 1) {
    // double detent, set the !! first two !! convex points their values. points
    // >2 ignored!
    specimen->InterestPointsList.append(upconvexe.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconvexe.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconvexe.at(0)));

    specimen->InterestPointsList.append(upconvexe.at(1) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconvexe.at(1)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconvexe.at(1)));

  } else if (upconvexe.size() == 1) {
    // one convex point found; set the point for the second detent to zero
    specimen->InterestPointsList.append(upconvexe.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconvexe.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconvexe.at(0)));

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  } else if (upconvexe.size() == 0) {
    // no convex points found; set points for both detents to zero
    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  }

  // same strategy applied for sorting the other points

  if (upconcave.size() > 1) {
    specimen->InterestPointsList.append(upconcave.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconcave.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconcave.at(0)));

    specimen->InterestPointsList.append(upconcave.at(1) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconcave.at(1)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconcave.at(1)));

  } else if (upconcave.size() == 1) {
    specimen->InterestPointsList.append(upconcave.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(upconcave.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(upconcave.at(0)));

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  } else if (upconcave.size() == 0) {
    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  }

  if ((downconvexe.size() > 1) and (upconvexe.size() > 1)) {

    specimen->InterestPointsList.append(downconvexe.at(1) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconvexe.at(1)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconvexe.at(1)));

    specimen->InterestPointsList.append(downconvexe.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconvexe.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconvexe.at(0)));

  } else if (downconvexe.size() == 1) {
    specimen->InterestPointsList.append(downconvexe.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconvexe.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconvexe.at(0)));

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  } else if (downconvexe.size() == 0) {
    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  }

  if ((downconcave.size() > 1) and (upconvexe.size() > 1)) {
    specimen->InterestPointsList.append(downconcave.at(1) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconcave.at(1)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconcave.at(1)));

    specimen->InterestPointsList.append(downconcave.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconcave.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconcave.at(0)));

  } else if (downconcave.size() == 1) {

    specimen->InterestPointsList.append(downconcave.at(0) - 8 + 2);
    specimen->IP_TravelList.append(specimen->travel.at(downconcave.at(0)) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconcave.at(0)));

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  } else if (downconcave.size() == 0) {
    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  }

  else if (downconcave.size() > 1) {
    qDebug() << "downconcave.isEmpty(): " << downconcave.isEmpty();
    qDebug() << downconcave.last();

    specimen->InterestPointsList.append(downconcave.last() - 8 + 2);

    specimen->IP_TravelList.append(specimen->travel.at(downconcave.last()) -
                                   approachDistance);
    specimen->IP_ForceList.append(specimen->force.at(downconcave.last()));

    specimen->InterestPointsList.append(0);
    specimen->IP_TravelList.append(0);
    specimen->IP_ForceList.append(0);
  }

  // ***

  specimen->BMW_F3_row = 0;
  // *** find the travel at last up convex point + 1N, for BMW projects
  if (!upconcave.isEmpty() &&  !upconvexe.isEmpty()) {
    for (uint j = upconcave.last(); j < specimen->samples - 8; ++j) {
      if (j == (uint)maxForceLocation)
        break;

      if ((specimen->force.at(j) - specimen->force.at(upconvexe.last())) //- 1.0
          > 0.0) {
        j -= 1;
        specimen->BMW_F3_row = j;
        break;
      }
    }
  }

  // *** search for electrical contact changes for the first two switches.
  bool sw_1_state = false;
  bool sw_2_state = false;



  // the contact values are 8bit binary format
  // qDebug() << specimen->name << " ElContactSize:" << specimen->contact1.size();
  // qDebug() << "all points:" << specimen->contact1;

  if (specimen->contact1.size() > 10) {

      sw_1_state = (specimen->contact1.first() & 1) || (specimen->contact1.first() & 4);
      sw_2_state = (specimen->contact1.first() & 2) || (specimen->contact1.first() & 8);

    for (uint l = 8; l < specimen->samples; l++) {

      if (((specimen->contact1.at(l) & 1) || (specimen->contact1.at(l) & 4)) and
          !sw_1_state) {
        sw_1_state = true;
        qDebug() << "sw_1 ON" << l - 6;
        specimen->SW1_state.append(1);
        specimen->SW1_row.append(l - 6);
        continue;
      }

      if (((specimen->contact1.at(l) & 2) || (specimen->contact1.at(l) & 8)) and
          !sw_2_state) {
        sw_2_state = true;
        qDebug() << "sw_2 ON" << l - 6;
        specimen->SW2_state.append(1);
        specimen->SW2_row.append(l - 6);
        continue;
      }

      if (((!(specimen->contact1.at(l) & 1)) &&
           (!(specimen->contact1.at(l) & 4))) and
          sw_1_state) {
        sw_1_state = false;
        qDebug() << "sw_1 OFF" << l - 6;
        specimen->SW1_state.append(0);
        specimen->SW1_row.append(l - 6);
        continue;
      }

      if (((!(specimen->contact1.at(l) & 2)) &&
           (!(specimen->contact1.at(l) & 8))) and
          sw_2_state) {
        sw_2_state = false;
        qDebug() << "sw_2 OFF" << l - 6;
        specimen->SW2_state.append(0);
        specimen->SW2_row.append(l - 6);
        continue;
      }
    }
    // ***
  }

  //    qDebug () << "final: " << specimen->name <<
  //    specimen->InterestPointsList;

  // append the specimen to the serie
  serie->specimenList.append(*specimen);

  // clear the lists; maybe not needed
  specimen->InterestPointsList.clear();
  specimen->IP_TravelList.clear();
  specimen->IP_ForceList.clear();
  specimen->sequence.clear();

  specimen->SW1_state.clear();
  specimen->SW1_row.clear();
  specimen->SW2_state.clear();
  specimen->SW2_row.clear();

  upconcave.clear();
  upconvexe.clear();
  downconcave.clear();
  downconvexe.clear();

  // timestamp2 = QDateTime::currentMSecsSinceEpoch();
  // qDebug() << "interest point found in ms" << timestamp2 - timestamp1;
}

#include "blfparser.h"

BlfParser::BlfParser(QObject *parent) : QObject{parent} {}

const double BlfParser::secondsInADay = 86400.0;

void BlfParser::DumpBlfFile(void *hBlfFile,
                            QHash<unsigned int, BusMessage> &DBC,
                            QHash<QString, DataSerie> &SysVars) {
  uint32_t objCount = DumpLogHeader(hBlfFile);
  auto result = QtConcurrent::run([hBlfFile, &DBC, &SysVars, objCount, this]() {
    if (DumpBlfObjects(hBlfFile, DBC, SysVars, objCount)) {

      emit parsingCompleted();
    }
  });
}

bool BlfParser::getBlfUUID(void *hBlfFile) {
  bool bSuccess = true;
  VBLObjectHeaderBase objectHeaderBase;

  qDebug() << this->loggingBlockUUID.isEmpty();
  while (this->loggingBlockUUID.isEmpty() && bSuccess &&
         BLPeekObject(hBlfFile, &objectHeaderBase)) {

    switch (objectHeaderBase.mObjectType) {
    case BL_OBJ_TYPE_APP_TEXT:
      bSuccess = DumpBLFAPPText(hBlfFile, objectHeaderBase);
      break;
    default:
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;
    }
  }
  BLCloseHandle(hBlfFile);
  qDebug() << "i think i found the configuration file:"
           << this->loggingBlockUUID;
  return bSuccess;
}

bool BlfParser::DumpBlfObjects(void *hBlfFile,
                               QHash<unsigned int, BusMessage> &DBC,
                               QHash<QString, DataSerie> &SysVars,
                               int objCount) {
  bool bSuccess = true;
  VBLObjectHeaderBase objectHeaderBase;

  double reciprocal = 100.0 / objCount;
  int lastPercent = -1; // Initialize to an impossible value
  int objCounter = 0;
  int currentPercent = 0;

  qDebug() << "Start dumping objects...";

  while (bSuccess && BLPeekObject(hBlfFile, &objectHeaderBase)) {
    switch (objectHeaderBase.mObjectType) {
    case BL_OBJ_TYPE_CAN_MESSAGE:
    case BL_OBJ_TYPE_CAN_MESSAGE2:
      bSuccess = DumpCANMessage(hBlfFile, objectHeaderBase, DBC);
      break;

    case BL_OBJ_TYPE_SYS_VARIABLE:
      bSuccess = DumpSysVar(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_CAN_ERROR:
      bSuccess = DumpCanError(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_CAN_ERROR_EXT:
      bSuccess = DumpCANErrorExt(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_CAN_DRIVER_ERROR:
      bSuccess = DumpCANDriverError(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_CAN_SETTING_CHANGED:
      bSuccess = DumpCANSettingsChanged(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_CAN_FD_MESSAGE_64:
      bSuccess = DumpCANFDMessage64(hBlfFile, objectHeaderBase, DBC);
      break;

    case BL_OBJ_TYPE_CAN_FD_ERROR_64:
      bSuccess = DumpCANFDError64(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_OVERRUN_ERROR:
      bSuccess = DumpOverrunError(hBlfFile, objectHeaderBase, SysVars);
      break;

    case BL_OBJ_TYPE_APP_TEXT:
      bSuccess = DumpBLFAPPText(hBlfFile, objectHeaderBase);
      break;

    case BL_OBJ_TYPE_TRIGGER_CONDITION:
      qDebug() << "BL_OBJ_TYPE_TRIGGER_CONDITION";
      bSuccess = DumpBLFAPPText(hBlfFile, objectHeaderBase);
      break;

      // Cases of Objects that should not be dumped and skipped
    case BL_OBJ_TYPE_CAN_STATISTIC:

      // qDebug() << "BL_OBJ_TYPE_CAN_STATISTIC";
      // typedef struct VBLCANDriverStatistic_t
      // {
      //   VBLObjectHeader mHeader;                     /* object header */
      //   uint16_t        mChannel;                    /* application channel
      //   */ uint16_t        mBusLoad;                    /* CAN bus load */
      //   uint32_t        mStandardDataFrames;         /* standard CAN id data
      //   frames */ uint32_t        mExtendedDataFrames;         /* extended
      //   CAN id data frames */ uint32_t        mStandardRemoteFrames;       /*
      //   standard CAN id remote frames */ uint32_t mExtendedRemoteFrames; /*
      //   extented CAN id remote frames */ uint32_t        mErrorFrames; /* CAN
      //   error frames */ uint32_t        mOverloadFrames;             /* CAN
      //   overload frames */
      // } VBLCANDriverStatistic;
      // #
      // static int canStatisticCount = 0;
      // VBLCANDriverStatistic canStatistic;
      // canStatistic.mHeader.mBase = objectHeaderBase;

      // if (BLReadObjectSecure(hBlfFile, &canStatistic.mHeader.mBase,
      //                        sizeof(canStatistic))) {
      //   canStatisticCount++;
      //   qDebug() << Qt::endl << "canStatisticCount: " << canStatisticCount;
      //   qDebug() << "mChannel: " << canStatistic.mChannel;
      //   qDebug() << "mBusLoad: " << canStatistic.mBusLoad;
      //   qDebug() << "mStandardDataFrames: " <<
      //   canStatistic.mStandardDataFrames; qDebug() << "mExtendedDataFrames: "
      //   << canStatistic.mExtendedDataFrames; qDebug() <<
      //   "mStandardRemoteFrames: "
      //            << canStatistic.mStandardRemoteFrames;
      //   qDebug() << "mExtendedRemoteFrames: "
      //            << canStatistic.mExtendedRemoteFrames;
      //   qDebug() << "mErrorFrames: " << canStatistic.mErrorFrames;
      //   qDebug() << "mOverloadFrames: " << canStatistic.mOverloadFrames;
      // } else {
      //   qDebug() << "Error reading CAN statistic object";
      //   bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      // }
      // BLFreeObject(hBlfFile, &canStatistic.mHeader.mBase);
      // break;

    case BL_OBJ_TYPE_TEST_STRUCTURE:
    case BL_OBJ_TYPE_reserved_5:
    case BL_OBJ_TYPE_DIAG_REQUEST_INTERPRETATION:
      //            qDebug() << "BL_OBJ_TYPE_DIAG_REQUEST_INTERPRETATION";
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;

    case BL_OBJ_TYPE_ENV_DOUBLE:
      // qDebug() << "BL_OBJ_TYPE_ENV_DOUBLE";
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;

    case BL_OBJ_TYPE_ENV_INTEGER:
      // qDebug() << "BL_OBJ_TYPE_ENV_INTEGER";
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;

    case BL_OBJ_TYPE_ENV_STRING:
      // qDebug() << "BL_OBJ_TYPE_ENV_STRING";
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;

    case BL_OBJ_TYPE_ENV_DATA:
      // qDebug() << "BL_OBJ_TYPE_ENV_DATA";
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;

      //        case BL_OBJ_TYPE_LIN_MESSAGE:
    case BL_OBJ_TYPE_LIN_MESSAGE2:
      // qDebug() << "BL_OBJ_TYPE_LIN_MESSAGE";
      bSuccess = DumpLINMessage(hBlfFile, objectHeaderBase, DBC);
      break;

      // Default case if an unrecognized object is encountered
    default:
      qDebug() << "BlfDump: Unhandled type: " << objectHeaderBase.mObjectType;
      bSuccess = BLSkipObject(hBlfFile, &objectHeaderBase);
      break;
    }

    objCounter++;
    currentPercent = static_cast<int>(objCounter * reciprocal);

    if (currentPercent > lastPercent) {
      emit parsingProgress(currentPercent);
      lastPercent = currentPercent;
    }
  }
  qDebug() << "***  " << objCounter << "  ***";
  BLCloseHandle(hBlfFile);
  return true;
}

bool BlfParser::DumpLINMessage(void *hBlfFile,
                               const VBLObjectHeaderBase &linObjectHeaderBase,
                               QHash<unsigned int, BusMessage> &DBC) {
  bool bSuccess = true;
  VBLLINMessage2 linMessage;
  linMessage.mHeader.mBase = linObjectHeaderBase;

  if (BLReadObjectSecure(hBlfFile, &linMessage.mHeader.mBase,
                         sizeof(linMessage))) {

    auto linMsgID = linMessage.mLinTimestampEvent.mLinMsgDescrEvent.mID;
    auto linMsgChannel = linMessage.mLinTimestampEvent.mLinMsgDescrEvent
                             .mLinSynchFieldEvent.mLinBusEvent.mChannel;

    if (DBC.contains(linMsgID) && DBC[linMsgID].checked) {
      double timestamp = ConvertTimestamp(linMessage.mHeader.mObjectTimeStamp);
      if (this->timeOffset == 0) {
        this->timeOffset = timestamp;
      }

      for (BusSignal &signal : DBC[linMsgID].getSignals()) {
        if (signal.checked)
          ProcessSignal(DBC[linMsgID], signal, linMessage.mData, timestamp,
                        linMsgChannel);
      }
    }

    BLFreeObject(hBlfFile, &linMessage.mHeader.mBase);
  } else {
    qDebug() << "BlfDump: Error reading LIN Message";
    bSuccess = false;
  }

  return bSuccess;
}

// ###
int64_t BlfParser::extractedBits(uint8_t *source, int startBit,
                                 int numberOfBits, ByteOrder byteOrder,
                                 bool isSigned) {
  if (startBit < 0 || numberOfBits <= 0 || numberOfBits > 64 || !source) {
    return 0;
  }

  int byteIndex, bitIndex;
  int bitsProcessed = 0;
  int byteIndexStep = 1;

  if (byteOrder == ByteOrder::MOTOROLA) {
    startBit += 1; // Adjust startBit for MOTOROLA order
    byteIndexStep = -1;
  }

  byteIndex = startBit / 8;
  bitIndex = startBit % 8;

  uint64_t result = 0;
  while (bitsProcessed < numberOfBits) {
    int bitsToEndOfByte = 8 - bitIndex;
    int remainingBits = numberOfBits - bitsProcessed;
    int bitsInThisByte =
        (bitsToEndOfByte < remainingBits) ? bitsToEndOfByte : remainingBits;

    uint64_t mask = (1ULL << bitsInThisByte) - 1;
    uint64_t bits = (source[byteIndex] >> bitIndex) & mask;
    result |= bits << bitsProcessed;

    bitsProcessed += bitsInThisByte;
    bitIndex = 0; // Reset bit index for the next byte
    byteIndex += byteIndexStep;
  }

  // Check if the value is negative (if the sign bit is set).
  // This is done by checking the bit at position (numberOfBits - 1).
  // Skip sign extension for single-bit values to avoid interpreting them as
  // negative.
  if (isSigned) {
    if (numberOfBits > 8 && numberOfBits < 128 &&
        (result & (1ULL << (numberOfBits - 1)))) {
      result |= (~0ULL) << numberOfBits; // Fill all bits above numberOfBits
                                         // with 1s to extend the sign.
    }
  }

  return static_cast<int64_t>(result);
}

// #####

// uint32_t BlfParser::extractedBits(uint8_t *source, int startBit,
//                                   int numberOfBits, ByteOrder byteOrder) {
//   if (startBit < 0 || numberOfBits <= 0 || numberOfBits > 32 || !source) {
//     return 0;
//   }

//   uint32_t result = 0;
//   int byteIndex = startBit / 8;
//   int bitIndex = startBit % 8;
//   int bitsRemaining = numberOfBits;

//   if (byteOrder == ByteOrder::INTEL) {
//     // Handle misaligned start
//     if (bitIndex != 0) {
//       int bitsInFirstByte = 8 - bitIndex;
//       if (bitsInFirstByte > bitsRemaining)
//         bitsInFirstByte = bitsRemaining;

//       result = (source[byteIndex] >> bitIndex) & ((1 << bitsInFirstByte) -
//       1);

//       bitsRemaining -= bitsInFirstByte;
//       byteIndex++;
//     }

//     // Handle full bytes
//     while (bitsRemaining >= 8) {
//       result |= (uint32_t)source[byteIndex] << (numberOfBits -
//       bitsRemaining); bitsRemaining -= 8; byteIndex++;
//     }

//     // Handle remaining bits
//     if (bitsRemaining > 0) {
//       result |= (source[byteIndex] & ((1 << bitsRemaining) - 1))
//                 << (numberOfBits - bitsRemaining);
//     }
//   } else { // ByteOrder::MOTOROLA
//     // Handle misaligned start
//     if (bitIndex != 0) {
//       int bitsInFirstByte = 8 - bitIndex;
//       if (bitsInFirstByte > bitsRemaining)
//         bitsInFirstByte = bitsRemaining;

//       result = (source[byteIndex] >> bitIndex) & ((1 << bitsInFirstByte) -
//       1); result <<= (numberOfBits - bitsInFirstByte);

//       bitsRemaining -= bitsInFirstByte;
//       byteIndex++;
//     }

//     // Handle full bytes
//     while (bitsRemaining >= 8) {
//       result |= (uint32_t)source[byteIndex] << (bitsRemaining - 8);
//       bitsRemaining -= 8;
//       byteIndex++;
//     }

//     // Handle remaining bits
//     if (bitsRemaining > 0) {
//       result |= (source[byteIndex] >> (8 - bitsRemaining)) &
//                 ((1 << bitsRemaining) - 1);
//     }
//   }

//   return result;
// }

// Process each signal
void BlfParser::ProcessSignal(BusMessage &message, BusSignal &signal,
                              quint8 *data, double timestamp, int channel) {
  // qDebug() << "ProcessSignal" << signal.name;

  // this->stopwatch_active = true;
  // this->TX_msg_timestamp = 0;
  // NM_LSR_1_CBV_AWB_XIX_NM_LSR_1_XIX_HCP3_CANFD01

  if ((!signal.checked) and !(signal.multiplexor == Multiplexor::MULTIPLEXOR)) {
  }

  static int occurrence = 0;

  int64_t rawValue = extractedBits(data, signal.getStartBit(),
                                   signal.getLength(), signal.getByteOrder(),
                                   signal.sign == Sign::SIGNED ? true : false);
  double value = signal.getOffset() + signal.getFactor() * rawValue;

  if (signal.multiplexor == Multiplexor::MULTIPLEXOR) {
    message.simple_multiplexor_value = (int)value;
  }

  if (!signal.checked)
    return;

  if ((signal.multiplexor == Multiplexor::MULTIPLEXED) and
      (message.simple_multiplexor_value != signal.multiplexNum)) {
    return;
  }

  if (occurrence < 1000) {
    qDebug() << signal.name << signal.getStartBit() << signal.getLength()
             << rawValue << signal.getOffset() << signal.getFactor() << value
             << signal.getMinimum() << signal.getMaximum()
             << (int)signal.getByteOrder() << (int)signal.getSign() << Qt::hex
             << data[0] << data[1] << data[2] << data[3] << data[4] << data[5]
             << data[6] << data[7];
  }

  // qDebug() << "ProcessSignal" << signal.name << value;

  if (signal.name == "NM_LSR_1_CBV_AWB_XIX_NM_LSR_1_XIX_HCP3_CANFD01"){
    if (this->Tx_msg_is_new[channel] == true){
      value = timestamp - this->TX_msg_timestamp[channel];
      this->Tx_msg_is_new[channel] = false;
      // qDebug() << "ProcessSignal" << signal.name << value;
    } else {
      return;
    }
  }

  signal.setOccurence(occurrence++);
  auto &channelSeries = signal.series->operator[](channel);
  channelSeries.append(
      QPointF((timestamp - timeOffset) / secondsInADay, value));
}

bool BlfParser::DumpCANFDMessage64(
    void *hBlfFile, const VBLObjectHeaderBase &canfdObjectHeaderBase,
    QHash<unsigned int, BusMessage> &DBC) {
  VBLCANFDMessage64 canfdMessage64;
  return ProcessMessage(hBlfFile, canfdObjectHeaderBase, DBC, canfdMessage64,
                        sizeof(canfdMessage64));
}

bool BlfParser::DumpCANMessage(void *hBlfFile,
                               const VBLObjectHeaderBase &canObjectHeaderBase,
                               QHash<unsigned int, BusMessage> &DBC) {
  VBLCANMessage2 canMessage2;
  return ProcessMessage(hBlfFile, canObjectHeaderBase, DBC, canMessage2,
                        sizeof(canMessage2));
}

bool BlfParser::DumpCanError(void *hBlfFile,
                             const VBLObjectHeaderBase &canObjectHeaderBase,
                             QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLCANErrorFrame canError;
  canError.mHeader.mBase = canObjectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &canError.mHeader.mBase, sizeof(canError))) {
    auto &dataSerieRef =
        SysVars["::CAN" + QString::number(canError.mChannel) + "_ErrorFrames"];
    const quint64 time = canError.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &canError.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpCANErrorExt(void *hBlfFile,
                                const VBLObjectHeaderBase &canObjectHeaderBase,
                                QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLCANErrorFrameExt canErrorExt;
  canErrorExt.mHeader.mBase = canObjectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &canErrorExt.mHeader.mBase,
                         sizeof(canErrorExt))) {
    auto &dataSerieRef =
        SysVars["::CAN" + QString::number(canErrorExt.mChannel) +
                "_ErrorFramesExt"];
    const quint64 time = canErrorExt.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &canErrorExt.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpCANFDError64(
    void *hBlfFile, const VBLObjectHeaderBase &canfdObjectHeaderBase,
    QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLCANFDErrorFrame64 canfdError64;
  canfdError64.mHeader.mBase = canfdObjectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &canfdError64.mHeader.mBase,
                         sizeof(canfdError64))) {
    auto &dataSerieRef =
        SysVars["::CANFD" + QString::number(canfdError64.mChannel) +
                "_ErrorFrames"];
    const quint64 time = canfdError64.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &canfdError64.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpCANSettingsChanged(
    void *hBlfFile, const VBLObjectHeaderBase &canObjectHeaderBase,
    QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLCANSettingsChanged canSettingsChanged;
  canSettingsChanged.mHeader.mBase = canObjectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &canSettingsChanged.mHeader.mBase,
                         sizeof(canSettingsChanged))) {
    auto &dataSerieRef =
        SysVars["::CAN" + QString::number(canSettingsChanged.mChannel) +
                "_SettingsChanged"];
    const quint64 time = canSettingsChanged.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &canSettingsChanged.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpCANDriverError(
    void *hBlfFile, const VBLObjectHeaderBase &canObjectHeaderBase,
    QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLCANDriverError canDriverError;
  canDriverError.mHeader.mBase = canObjectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &canDriverError.mHeader.mBase,
                         sizeof(canDriverError))) {
    auto &dataSerieRef =
        SysVars["::CAN" + QString::number(canDriverError.mChannel) +
                "_DriverErrors"];
    const quint64 time = canDriverError.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &canDriverError.mHeader.mBase);
    return true;
  }
  return false;
}

void BlfParser::FormatSysVarDataDoubleArray(
    QHash<QString, DataSerie> &SysVars, const VBLSystemVariable &sysVariable,
    double timestamp, QString &varBaseName) {
  const uint32_t step = sizeof(double); // Size of data type
  varBaseName += '[';                   // Initialize with the opening bracket

  for (uint32_t i = 0; i < sysVariable.mDataLength; i += step) {
    auto &dataSerieRef = SysVars[varBaseName + QString::number(i / step) +
                                 ']']; // Create if not exists

    if (dataSerieRef.checked) {
      double value = *reinterpret_cast<const double *>(&sysVariable.mData[i]);

      // qDebug() << __FILE__ << __LINE__ << "Timestamp: " << timestamp << "
      // Value: " << value
      // << sysVariable.mData[i];
      dataSerieRef.addPoint(
          QPointF((timestamp - timeOffset) / secondsInADay, value));
    }
  }
}

void BlfParser::FormatSysVarDataLongArray(QHash<QString, DataSerie> &SysVars,
                                          const VBLSystemVariable &sysVariable,
                                          double timestamp,
                                          QString &varBaseName) {
  const uint32_t step = sizeof(long); // Size of data type
  varBaseName += '[';                 // Initialize with the opening bracket

  for (uint32_t i = 0; i < sysVariable.mDataLength; i += step) {
    auto &dataSerieRef = SysVars[varBaseName + QString::number(i / step) +
                                 ']']; // Create if not exists

    if (dataSerieRef.checked) {
      long value = *reinterpret_cast<const long *>(&sysVariable.mData[i]);
      // qDebug() << __FILE__ << __LINE__ << "Timestamp: " << timestamp << "
      // Value: " << value
      // << sysVariable.mData[i];
      dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                    static_cast<double>(value)));
    }
  }
}

bool BlfParser::DumpSysVar(void *hBlfFile,
                           const VBLObjectHeaderBase &sysVarObjectHeaderBase,
                           QHash<QString, DataSerie> &SysVars) {
  VBLSystemVariable sysVariable;
  sysVariable.mHeader.mBase = sysVarObjectHeaderBase;

  if (BLReadObjectSecure(hBlfFile, &sysVariable.mHeader.mBase,
                         sizeof(sysVariable))) {
    const quint64 time = sysVariable.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;
    if (this->timeOffset == 0) {
      this->timeOffset = timestamp;
    }
    QString varBaseName =
        QString::fromLocal8Bit(sysVariable.mName); //"SysVar" +

    auto &dataSerieRef = SysVars[varBaseName];

    double value = 0.0;

    switch (sysVariable.mType) {
    case BL_SYSVAR_TYPE_DOUBLE:
      if (dataSerieRef.checked) {
        memcpy(&value, sysVariable.mData, sizeof(double));
        dataSerieRef.addPoint(
            QPointF((timestamp - timeOffset) / secondsInADay, value));
      }
      break;

    case BL_SYSVAR_TYPE_LONG:
      if (dataSerieRef.checked) {
        value =
            static_cast<double>(*reinterpret_cast<long *>(sysVariable.mData));
        dataSerieRef.addPoint(
            QPointF((timestamp - timeOffset) / secondsInADay, value));
      }
      break;

    case BL_SYSVAR_TYPE_STRING:
      // Handle string data if needed
      // qDebug() << "String data type not supported";
      break;

    case BL_SYSVAR_TYPE_DOUBLEARRAY:
      FormatSysVarDataDoubleArray(SysVars, sysVariable, timestamp, varBaseName);
      break;
    case BL_SYSVAR_TYPE_LONGARRAY:
      FormatSysVarDataLongArray(SysVars, sysVariable, timestamp, varBaseName);
      break;

    default:
      // Handle unknown types if needed
      break;
    }

    BLFreeObject(hBlfFile, &sysVariable.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpBLFAPPText(void *hBlfFile,
                               const VBLObjectHeaderBase &AppTextHeaderBase) {
  VBLAppText_t AppText;
  AppText.mHeader.mBase = AppTextHeaderBase;

  if (BLReadObjectSecure(hBlfFile, &AppText.mHeader.mBase, sizeof(AppText))) {
    if (this->blfHeader.isEmpty()) // only insert the first AppText
    {
      this->blfHeader.append(QString::fromUtf8(AppText.mText));

      const quint64 time = AppText.mHeader.mObjectTimeStamp;
      double timestamp = static_cast<double>(time) / 1e9;
      this->timeOffset = timestamp;
      qDebug() << "Time offset:" << timestamp << "s"
               << "time:" << time << "ns";
    }

    qDebug().noquote() << "AppText:" << AppText.mTextLength << Qt::endl
                       << QString(AppText.mText).replace(';', '\n');

    // QRegularExpression re("block uuid=\"([^\"]+)\"");
    QRegularExpression re("configuration file=\"([^\"]+)\"");
    QRegularExpressionMatch match = re.match(QString(AppText.mText));

    if (match.hasMatch()) {
      QString blockUuid =
          match.captured(1); // The first captured group contains the UUID.
      qDebug() << "configuration file:" << blockUuid;
      this->loggingBlockUUID = blockUuid;

      const quint64 time = AppText.mHeader.mObjectTimeStamp;
      double timestamp = static_cast<double>(time) / 1e9;
      this->timeOffset = timestamp;
      qDebug() << "Time offset of the configuration file:" << timestamp << "s";
    }

    BLFreeObject(hBlfFile, &AppText.mHeader.mBase);
    return true;
  }
  return false;
}

bool BlfParser::DumpOverrunError(void *hBlfFile,
                                 const VBLObjectHeaderBase &objectHeaderBase,
                                 QHash<QString, DataSerie> &SysVars) {
  static int errorCounter = 0;
  VBLDriverOverrun driverOverrun;
  driverOverrun.mHeader.mBase = objectHeaderBase;
  if (BLReadObjectSecure(hBlfFile, &driverOverrun.mHeader.mBase,
                         sizeof(driverOverrun))) {
    auto &dataSerieRef =
        SysVars["::CAN" + QString::number(driverOverrun.mChannel) +
                "_DriverOverrun"];
    const quint64 time = driverOverrun.mHeader.mObjectTimeStamp;
    double timestamp = static_cast<double>(time) / 1e9;

    dataSerieRef.addPoint(QPointF((timestamp - timeOffset) / secondsInADay,
                                  static_cast<double>(++errorCounter)));
    BLFreeObject(hBlfFile, &driverOverrun.mHeader.mBase);
    return true;
  }
  return false;
}

// QString BlfParser::ConvertWeekdayToString(const quint16 dayOfWeekIndex) {
//   static const QString weekDays[7] = {"Sun", "Mon", "Tue", "Wed",
//                                       "Thu", "Fri", "Sat"};
//   if (dayOfWeekIndex < 7) {
//     return weekDays[dayOfWeekIndex];
//   }
//   return "";
// }

// QString BlfParser::ConvertMonthToString(const quint16 monthIndex) {
//   static const QString months[12] = {"Jan", "Feb", "Mar", "Apr", "May",
//   "Jun",
//                                      "Jul", "Aug", "Sep", "Oct", "Nov",
//                                      "Dec"};
//   if (monthIndex >= 1 && monthIndex < 13) {
//     return months[monthIndex - 1];
//   }
//   return "";
// }

int BlfParser::DumpLogHeader(void *hBlfFile) {
  logStatistics.mStatisticsSize = sizeof(VBLFileStatisticsEx);
  BLGetFileStatisticsEx(hBlfFile, &logStatistics);

  // SYSTEMTIME logStartTimestamp = logStatistics.mMeasurementStartTime;

  // QString weekday = ConvertWeekdayToString(logStartTimestamp.wDayOfWeek);
  // QString month = ConvertMonthToString(logStartTimestamp.wMonth);

  // qDebug().noquote() << "\n\n ***BLF header***";
  // qDebug() << "date" << weekday << month << logStartTimestamp.wDay
  //          << logStartTimestamp.wHour << logStartTimestamp.wMinute
  //          << logStartTimestamp.wSecond << logStartTimestamp.wMilliseconds;
  // qDebug().noquote() << "base hex timestamps absolute";
  // qDebug().noquote() << "internal events logged";
  // qDebug() << "Canoe version of measurement" <<
  // logStatistics.mApplicationMajor
  //          << logStatistics.mApplicationMinor <<
  //          logStatistics.mApplicationBuild
  //          << "Objects number" << logStatistics.mObjectCount
  //          << "Uncompressed size" << logStatistics.mUncompressedFileSize;

  return logStatistics.mObjectCount;
}

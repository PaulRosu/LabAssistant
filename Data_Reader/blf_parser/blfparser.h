#ifndef BLFPARSER_H
#define BLFPARSER_H

#pragma once

#include "binlog.h"
#include "data/data_types.h"
#include "dbc_parser/dbciterator.hpp"
#include <QApplication>
#include <QHash>
#include <QObject>
#include <QRegularExpression>
#include <QRunnable>
#include <QtConcurrent>
#include <QtCore>

class BlfParser : public QObject {
  Q_OBJECT
public:
  explicit BlfParser(QObject *parent = nullptr);

  // Functions that need to be called from the main thread
  int DumpLogHeader(void *hBlfFile);
  void DumpBlfFile(void *hBlfFile, QHash<unsigned int, BusMessage> &DBC,
                   QHash<QString, DataSerie> &SysVars);
  bool getBlfUUID(void *hBlfFile);
  double timeOffset = 0;
  QString source = "";
  static const double secondsInADay;
  QString blfHeader = "";
  QString loggingBlockUUID = "";
  VBLFileStatisticsEx logStatistics;
  double TX_msg_timestamp[15] = {0};
  double TX_previous_msg_timestamp[15] = {0};
  bool Tx_msg_is_new[15] = {false};

signals:
  // Signals to indicate progress or state
  void parsingCompleted();
  void parsingProgress(int percent);
  void parsingError(QString errorMessage);

public slots:
  // Slots for receiving signals, perhaps from a UI
  //    void initiateParsing(void *hBlfFile, QHash<unsigned int, BusMessage>
  //    &DBC);

private:
  // Functions that run on the worker thread
  bool DumpBlfObjects(void *hBlfFile, QHash<unsigned int, BusMessage> &DBC,
                      QHash<QString, DataSerie> &SysVars, int objCount);
  bool DumpCANMessage(void *hBlfFile,
                      const VBLObjectHeaderBase &canObjectHeaderBase,
                      QHash<unsigned int, BusMessage> &DBC);
  bool DumpCANStatistic(void *hBlfFile,
                        const VBLObjectHeaderBase &canObjectHeaderBase);
  bool DumpCANDriverError(void *hBlfFile,
                          const VBLObjectHeaderBase &canObjectHeaderBase,
                          QHash<QString, DataSerie> &SysVars);
  bool DumpCANSettingsChanged(void *hBlfFile,
                              const VBLObjectHeaderBase &canObjectHeaderBase,
                              QHash<QString, DataSerie> &SysVars);
  bool DumpCanError(void *hBlfFile,
                    const VBLObjectHeaderBase &canObjectHeaderBase,
                    QHash<QString, DataSerie> &SysVars);
  bool DumpCANErrorExt(void *hBlfFile,
                       const VBLObjectHeaderBase &canObjectHeaderBase,
                       QHash<QString, DataSerie> &SysVars);
  bool DumpCANFDMessage64(void *hBlfFile,
                          const VBLObjectHeaderBase &canfdObjectHeaderBase,
                          QHash<unsigned int, BusMessage> &DBC);
  bool DumpCANFDError64(void *hBlfFile,
                        const VBLObjectHeaderBase &canfdObjectHeaderBase,
                        QHash<QString, DataSerie> &SysVars);

  bool DumpLINMessage(void *hBlfFile,
                      const VBLObjectHeaderBase &linObjectHeaderBase,
                      QHash<unsigned int, BusMessage> &DBC);

  void ProcessSignal(BusMessage &message, BusSignal &signal, quint8 *data,
                     double timestamp, int channel);

  template <typename MessageType>
  bool ProcessMessage(void *hBlfFile,
                      const VBLObjectHeaderBase &objectHeaderBase,
                      QHash<unsigned int, BusMessage> &DBC,
                      MessageType &message, size_t objectSize);

  void FormatSysVarDataDoubleArray(QHash<QString, DataSerie> &SysVars,
                                   const VBLSystemVariable &sysVariable,
                                   double timestamp, QString &varBaseName);

  void FormatSysVarDataLongArray(QHash<QString, DataSerie> &SysVars,
                                 const VBLSystemVariable &sysVariable,
                                 double timestamp, QString &varBaseName);

  bool DumpSysVar(void *hBlfFile,
                  const VBLObjectHeaderBase &sysVarObjectHeaderBase,
                  QHash<QString, DataSerie> &SysVars);

  bool DumpBLFAPPText(void *hBlfFile,
                      const VBLObjectHeaderBase &AppTextHeaderBase);

  bool DumpOverrunError(void *hBlfFile,
                        const VBLObjectHeaderBase &objectHeaderBase,
                        QHash<QString, DataSerie> &SysVars);

  // Utility Functions
  QString ConvertWeekdayToString(const quint16 dayOfWeekIndex);
  QString ConvertMonthToString(const quint16 monthIndex);
  void DumpLoglineHeader(const quint64 timestamp, const qint32 channel,
                         const QString &protocol);
  double timestampToDays(const quint64 timestamp);
  int64_t extractedBits(uint8_t *source, int startBit, int numberOfBits,
                        ByteOrder byteOrder, bool isSigned);
  // Convert timestamp to double
  static inline double ConvertTimestamp(const quint64 &rawTime) {
    constexpr double divisor = 1000 * 1000000;
    return static_cast<double>(rawTime) / divisor;
  }
};

template <typename MessageType>
bool BlfParser::ProcessMessage(void *hBlfFile,
                               const VBLObjectHeaderBase &objectHeaderBase,
                               QHash<unsigned int, BusMessage> &DBC,
                               MessageType &message, size_t objectSize) {
  message.mHeader.mBase = objectHeaderBase;

  if (!BLReadObjectSecure(hBlfFile, &message.mHeader.mBase, objectSize)) {
    return false;
  }

  if (message.mID == 2600468735) {
    double timestamp = ConvertTimestamp(message.mHeader.mObjectTimeStamp);
    if ((timestamp - TX_previous_msg_timestamp[message.mChannel]) > 5) {
      

        // qDebug() << message.mChannel << message.mID
        //         << QString::number(timestamp, 'g', 17);

      this->TX_msg_timestamp[message.mChannel] = timestamp;
      this->Tx_msg_is_new[message.mChannel] = true;
      this->TX_previous_msg_timestamp[message.mChannel] = timestamp;
    } else {
      this->TX_previous_msg_timestamp[message.mChannel] = timestamp;
    }
  }

  // qDebug() << message.mID << message.mFlags;
  // bool isTX = (message.mFlags & 0x1) == 0x1;
  // qDebug() << "Message ID:" << message.mID << "Direction:" << (isTX ? "TX" :
  // "RX"); if (CAN_MSG_DIR(message.mFlags))
  if (!DBC.contains(message.mID) || !DBC[message.mID].checked) {
    BLFreeObject(hBlfFile, &message.mHeader.mBase);
    return true;
  }

  double timestamp = ConvertTimestamp(message.mHeader.mObjectTimeStamp);

  for (BusSignal &signal : DBC[message.mID].getSignals()) {
    ProcessSignal(DBC[message.mID], signal, message.mData, timestamp,
                  message.mChannel);
  }

  BLFreeObject(hBlfFile, &message.mHeader.mBase);
  return true;
}

#endif // BLFPARSER_H

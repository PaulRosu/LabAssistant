#ifndef DATA_H_
#define DATA_H_

#pragma once
#include <QHash>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QTime>
#include <QtCore>

#include "blf_parser/blfparser.h"
#include "data/data_dialog.h"
#include "data/data_types.h"
#include "dbc_parser/dbciterator.hpp"
#include "csv_parser/csvparser.h"

class Data : public QObject
{
    Q_OBJECT

  private:
    void parseDBC(QFileInfo file);
    void parseLDF(QFileInfo file);


    void saveMessages(QSettings &settings, const QHash<unsigned int, BusMessage>* messages);
    void saveSysVars(QSettings &settings, const QHash<QString, DataSerie> &SysVars);
    void loadSysVars(QSettings &settings, QHash<QString, DataSerie> *SysVars);
    void loadMessages(QSettings &settings);
    void processParsedSysVars();
    void processParsedMessages();

    void quickSort(QList<QString> &arr, int left, int right);

    QElapsedTimer timer;

    CSVParser* CsvFileParser;

    QString getCsvSettingsPath(const QString& csvPath);
    void saveCsvSettings(const QString& csvPath);
    void loadCsvSettings(const QString& csvPath);

    QString getTemplatesFolderPath() const;
    QString findMatchingTemplateSettings(const QString& fingerprint) const;
    void copySettingsFile(const QString& sourcePath, const QString& destPath);
    void copySettingsToDataManager(QSettings* source);

    void synchronizeTreeStates();



  public:
    explicit Data();
    ~Data();

    //    void insertSysVar(QString name, double value);
    //    void insertSignal();

    QHash<unsigned int, BusMessage> *DBC;
    QFileInfo fi;
    void parseBLF(QFileInfo file);
    void parseCSV(QFileInfo file);
    void initializeCsvSettings(const QString& csvPath);

    //    QHash<QString, QList<QPointF>> *SysVars;

    QHash<QString, DataSerie> *SysVars;
    data_Dialog *DataManager;
    BlfParser *BlfFileParser;
    QStringList *filesList;

    public slots:
    void filesReceived(QList<QFileInfo> fileList);
    void updateSignalCheckState(qint32 msgId, QString signalName, bool boolCheckState);
    void updateSysVarCheckState(QString sysVarName, bool boolCheckState);
    bool readBlfUUID(QFileInfo file);
    void saveDBC();
    void loadDBC();
    void reload();
    void BlfParsingComplete();
    void fileParsingProgress(int percent);
    void CsvParsingComplete();


    signals:
    void dataReady();
    void parsingProgress(int percent, QString name);
};

#endif

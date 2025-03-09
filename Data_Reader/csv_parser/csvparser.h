#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QObject>
#include <QFile>
#include <QStringList>
#include <QList>
#include <QHash>
#include "../data/data_types.h"
#include <QSettings>

class CSVParser : public QObject {
    Q_OBJECT

public:
    explicit CSVParser(QObject *parent = nullptr);
    ~CSVParser();

    bool parseFile(const QString& filePath, int maxRowsForTypeDetection = 0, const QStringList& columnsToProcess = QStringList());
    QStringList getHeaders() const { return headers; }
    QHash<QString, DataSerie>* getData() { return &data; }

    QString generateFingerprint() const;
    QString generateFingerprintCode() const;
    bool matchesFingerprint(const QString& fingerprint) const;
    void saveSettings(QSettings* settings) const;
    void loadSettings(QSettings* settings);

private:
    QStringList headers;
    QHash<QString, DataSerie> data;
    QString delimiter;
    int maxRowsForTypeDetection = 0;
    QStringList columnsToProcess;

    void detectDelimiter(const QString& line);
    QStringList parseLine(const QString& line);
    void processDataLine(const QString& line, int& errorCount, QHash<QString, bool>& typeDetected);
    
    // Fast string to double conversion functions
    double fast_atof(const char *num, bool *ok = nullptr);
    double pow10(int n);

    QString calculateHeaderHash() const;

signals:
    void parsingProgress(int percent);
    void parsingCompleted();
};

#endif // CSVPARSER_H 
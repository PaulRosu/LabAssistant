#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QObject>
#include <QFile>
#include <QStringList>
#include <QVector>
#include <QHash>
#include "../data/data_types.h"
#include <QSettings>

class CSVParser : public QObject {
    Q_OBJECT

public:
    explicit CSVParser(QObject *parent = nullptr);
    ~CSVParser();

    bool parseFile(const QString& filePath);
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

    void detectDelimiter(const QString& line);
    QStringList parseLine(const QString& line);

    QString calculateHeaderHash() const;

signals:
    void parsingProgress(int percent);
    void parsingCompleted();
};

#endif // CSVPARSER_H 
#include "csvparser.h"
#include <QTextStream>
#include <QDebug>

CSVParser::CSVParser(QObject *parent) : QObject(parent) {
}

CSVParser::~CSVParser() {
}

void CSVParser::detectDelimiter(const QString& line) {
    QVector<QString> possibleDelimiters = {";", ",", "\t"};
    QVector<int> counts;
    
    qDebug() << "\n=== Delimiter Detection ===";
    qDebug() << "Analyzing line:" << line;
    
    for(const QString& del : possibleDelimiters) {
        int count = line.count(del);
        counts.append(count);
        qDebug() << "Delimiter:" << del << "Count:" << count;
    }
    
    int maxCount = 0;
    int maxIndex = 0;
    for(int i = 0; i < counts.size(); i++) {
        if(counts[i] > maxCount) {
            maxCount = counts[i];
            maxIndex = i;
        }
    }
    
    delimiter = possibleDelimiters[maxIndex];
    qDebug() << "Selected delimiter:" << delimiter << "with count:" << maxCount;
}

QStringList CSVParser::parseLine(const QString& line) {
    QStringList fields;
    QString field;
    bool inQuotes = false;
    
    for(int i = 0; i < line.length(); i++) {
        QChar currentChar = line[i];
        
        if(currentChar == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        
        if(!inQuotes && currentChar == delimiter[0]) {
            fields.append(field.trimmed());
            field.clear();
            continue;
        }
        
        field += currentChar;
    }
    
    // Append the last field
    if(!field.isEmpty() || fields.size() > 0) {
        fields.append(field.trimmed());
    }
    
    return fields;
}

bool CSVParser::parseFile(const QString& filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open CSV file:" << filePath;
        return false;
    }

    QTextStream in(&file);

    in.setEncoding(QStringConverter::Utf8);

    QString firstLine = in.readLine();
    if(firstLine.isEmpty()) {
        qDebug() << "First line is empty";
        file.close();
        return false;
    }

    qDebug() << "\n=== CSV Parsing Started ===";
    qDebug() << "First line:" << firstLine;

    // Detect the delimiter from the first line
    detectDelimiter(firstLine);
    
    // Parse headers
    headers = parseLine(firstLine);
    qDebug() << "Found" << headers.size() << "headers:" << headers;
    
    // Initialize DataSerie for each column
    for(const QString& header : headers) {
        DataSerie series;
        series.checked = false;
        series.points.clear();
        data.insert(header, series);
    }

    // Get file size for progress calculation
    qint64 fileSize = file.size();
    qint64 currentPos = file.pos();
    int lineCount = 0;
    int errorCount = 0;
    
    // Parse data
    while(!in.atEnd()) {
        QString line = in.readLine();
        lineCount++;
        QStringList fields = parseLine(line);
        
        // Pad the fields list with empty strings if necessary
        while(fields.size() < headers.size()) {
            fields.append("");
        }

        // Convert the running time (first column) to double, replacing comma with period
        bool ok;
        double timestamp = fields[0].replace(",", ".").toDouble(&ok);
        if(!ok) {
            if(errorCount <= 5) {
                qDebug() << "Failed to parse timestamp at line" << lineCount << ":" << fields[0];
            }
            errorCount++;
            continue;
        }
        
        for(int i = 0; i < fields.size() && i < headers.size(); i++) {
            QString field = fields[i].trimmed();
            if(field.isEmpty()) continue;
            
            // Try to convert to number if possible
            QString normalizedField = field.replace(",", ".");
            bool isNumber = false;
            double value = normalizedField.toDouble(&isNumber);
            
            // Special handling for Temperature, Humidity, and Voltage
            if(isNumber || headers[i].contains("Temperature") || 
               headers[i].contains("Humidity") || 
               headers[i].contains("Voltage") ||
               headers[i].contains("current") ||
               headers[i].contains("deviation")) {
                if(isNumber) {
                    data[headers[i]].points.append(QPointF(timestamp, value));
                }
            }
        }
        
        // Update progress every 1000 lines
        if(lineCount % 1000 == 0) {
            currentPos = file.pos();
            int progress = (currentPos * 100) / fileSize;
            emit parsingProgress(progress);
        }
    }

    qDebug() << "\n=== CSV Parsing Complete ===";
    qDebug() << "Total lines processed:" << lineCount;
    qDebug() << "Error count:" << errorCount;
    
    // Sort columns by number of points for better debug readability
    QList<QPair<QString, int>> sortedColumns;
    for(auto it = data.begin(); it != data.end(); ++it) {
        sortedColumns.append(qMakePair(it.key(), it.value().points.size()));
    }
    std::sort(sortedColumns.begin(), sortedColumns.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });
              
    for(const auto& col : sortedColumns) {
        qDebug() << "Column:" << col.first << "Points:" << col.second;
    }

    file.close();
    emit parsingCompleted();
    return true;
}

QString CSVParser::generateFingerprint() const {
    // Generate full fingerprint for comparison
    return QString("%1|%2|%3")
        .arg(headers.size())
        .arg(calculateHeaderHash())
        .arg(delimiter);
}

QString CSVParser::generateFingerprintCode() const {
    // Generate clean fingerprint code for filenames
    return calculateHeaderHash();
}

QString CSVParser::calculateHeaderHash() const {
    // Create a concatenated string of all headers
    QString headerStr = headers.join("|");
    // Calculate hash of the headers
    return QString::number(qHash(headerStr), 16);
}

bool CSVParser::matchesFingerprint(const QString& fingerprint) const {
    qDebug() << "\n=== Fingerprint Check ===";
    qDebug() << "Checking fingerprint:" << fingerprint;
    
    QStringList parts = fingerprint.split("|");
    if (parts.size() != 3) {
        qDebug() << "Invalid fingerprint format - expected 3 parts, got" << parts.size();
        return false;
    }
    
    int storedColumnCount = parts[0].toInt();
    QString storedHeaderHash = parts[1];
    QString storedDelimiter = parts[2];
    
    QString currentHeaderHash = calculateHeaderHash();
    
    qDebug() << "Comparing:"
             << "\nColumns - Stored:" << storedColumnCount << "Current:" << headers.size()
             << "\nHash - Stored:" << storedHeaderHash << "Current:" << currentHeaderHash
             << "\nDelimiter - Stored:" << storedDelimiter << "Current:" << delimiter;
    
    bool matches = storedColumnCount == headers.size() &&
                  storedHeaderHash == currentHeaderHash &&
                  storedDelimiter == delimiter;
                  
    qDebug() << "Match result:" << matches;
    return matches;
}

void CSVParser::saveSettings(QSettings* settings) const {
    if (!settings) return;
    
    settings->beginGroup("CSVFormat");
    settings->setValue("fingerprint", generateFingerprint());
    settings->setValue("delimiter", delimiter);
    
    settings->beginWriteArray("Columns");
    for (int i = 0; i < headers.size(); ++i) {
        settings->setArrayIndex(i);
        QString fullPath = "CSV::" + headers[i];
        bool isChecked = data[headers[i]].checked;
        settings->setValue("name", fullPath);
        settings->setValue("checked", isChecked);
        qDebug() << "Saving check state for" << fullPath << ":" << isChecked;
    }
    settings->endArray();
    settings->endGroup();
}

void CSVParser::loadSettings(QSettings* settings) {
    if (!settings) return;
    
    settings->beginGroup("CSVFormat");
    
    // Load and verify fingerprint
    QString storedFingerprint = settings->value("fingerprint").toString();
    if (!matchesFingerprint(storedFingerprint)) {
        qDebug() << "CSV format mismatch - cannot load settings";
        settings->endGroup();
        return;
    }
    
    // Debug: Print all settings keys
    qDebug() << "All settings keys in CSVFormat:";
    QStringList allKeys = settings->allKeys();
    for (const QString& key : allKeys) {
        qDebug() << "  Key:" << key << "Value:" << settings->value(key);
    }
    
    // Load column settings
    int size = settings->beginReadArray("Columns");
    qDebug() << "Loading settings for" << size << "columns";
    
    // Debug: Print current data state
    qDebug() << "Current data columns:";
    for (auto it = data.begin(); it != data.end(); ++it) {
        qDebug() << "  Column:" << it.key();
    }
    
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString fullPath = settings->value("name").toString();
        QString name = fullPath.replace("CSV::", "");  // Remove prefix to match data keys
        bool checked = settings->value("checked").toBool();
        
        qDebug() << "Loading column" << i << ":"
                 << "\n  Full path:" << fullPath
                 << "\n  Name:" << name
                 << "\n  Checked:" << checked
                 << "\n  Exists in data:" << data.contains(name);
        
        if (data.contains(name)) {
            data[name].checked = checked;
            qDebug() << "  Successfully set check state for" << name << "to" << checked;
        } else {
            qDebug() << "  Warning: Column" << fullPath << "not found in current data";
        }
    }
    settings->endArray();
    settings->endGroup();
} 
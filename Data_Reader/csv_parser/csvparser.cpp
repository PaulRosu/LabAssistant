#include "csvparser.h"
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include <QRegularExpression>

// Maximum line size to prevent buffer overflows with corrupted data
#define MAX_LINE_SIZE 8192

CSVParser::CSVParser(QObject *parent) : QObject(parent) {
}

CSVParser::~CSVParser() {
}

// Fast power of 10 calculation - much faster than std::pow for integer exponents
double CSVParser::pow10(int n) {
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

// Fast string to double conversion - much faster than QString::toDouble
double CSVParser::fast_atof(const char *num, bool *ok) {
    if (!num || !*num) {
        if (ok) *ok = false;
        return 0;
    }

    // For debugging
    static int debugCount = 0;
    bool shouldDebug = (debugCount < 10);  // Only debug the first few calls
    QString originalInput = QString::fromUtf8(num);
    
    if (shouldDebug) {
        qDebug() << "fast_atof input:" << originalInput;
        debugCount++;
    }

    int sign = 1;
    double integerPart = 0.0;
    double fractionPart = 0.0;
    bool hasFraction = false;
    bool hasExpo = false;
    bool hasDigit = false;

    // Skip leading white space
    while (*num == ' ') {
        ++num;
    }

    // Handle sign
    if (*num == '-') {
        ++num;
        sign = -1;
    } else if (*num == '+') {
        ++num;
    }

    while (*num != '\0') {
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
        } else if (*num == 'E' || *num == 'e') {
            hasExpo = true;
            ++num;
            break;
        } else if (*num == ' ') {
            // Skip white spaces
        } else {
            if (!hasDigit) {
                if (ok) *ok = false;
                if (shouldDebug) {
                    qDebug() << "  Not a number (no digits):" << originalInput;
                }
                return 0;
            }
            if (ok) *ok = true;
            if (shouldDebug) {
                qDebug() << "  Parsed as number:" << (sign * integerPart);
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
            } else if (*num == 'E' || *num == 'e') {
                hasExpo = true;
                ++num;
                break;
            } else {
                if (ok) *ok = true;
                return sign * (integerPart + fractionPart);
            }
            ++num;
        }
    }

    // Parse exponent part
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

    if (ok) *ok = true;
    double result = sign * (integerPart + fractionPart) * expPart;
    
    if (shouldDebug) {
        qDebug() << "  Parsed as number:" << result;
    }
    
    return result;
}

void CSVParser::detectDelimiter(const QString& line) {
    QList<QString> possibleDelimiters = {";", ",", "\t"};
    QList<int> counts;
    
    qDebug() << "\n=== Delimiter Detection ===";
    qDebug() << "Analyzing line:" << line;
    
    for (const auto& del : possibleDelimiters) {
        counts.append(line.count(del));
        qDebug() << "Delimiter:" << del << "Count:" << counts.last();
    }
    
    int maxCount = 0;
    int maxIndex = 0;
    for (int i = 0; i < counts.size(); i++) {
        if (counts[i] > maxCount) {
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
    
    for (int i = 0; i < line.length(); i++) {
        QChar currentChar = line[i];
        
        if (currentChar == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        
        if (!inQuotes && currentChar == delimiter[0]) {
            fields.append(field.trimmed());
            field.clear();
            continue;
        }
        
        field += currentChar;
    }
    
    // Append the last field
    if (!field.isEmpty() || !fields.isEmpty()) {
        fields.append(field.trimmed());
    }
    
    return fields;
}

bool CSVParser::parseFile(const QString& filePath, int maxRowsForTypeDetection, const QStringList& columnsToProcess) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open CSV file:" << filePath;
        return false;
    }

    qDebug() << "\n=== CSV Parsing Started ===";
    qDebug() << "File:" << filePath;
    qDebug() << "Max rows for type detection:" << maxRowsForTypeDetection;
    qDebug() << "Columns to process:" << (columnsToProcess.isEmpty() ? "All" : columnsToProcess.join(", "));
    
    // Store the parameters as member variables
    this->maxRowsForTypeDetection = maxRowsForTypeDetection;
    this->columnsToProcess = columnsToProcess;
    
    // Get file size for progress calculation
    qint64 fileSize = file.size();
    qint64 currentPos = 0;
    int lineCount = 0;
    int errorCount = 0;
    
    // Buffer for reading lines
    char lineBuffer[MAX_LINE_SIZE];
    int bufferPos = 0;
    bool lineComplete = false;
    
    // Variables for header detection
    QList<QString> candidateLines;
    QList<int> delimiterCounts;
    int maxDelimiterCount = 0;
    int headerLineIndex = -1;
    bool headerFound = false;
    bool processingData = false;
    
    // Variables for data type detection
    int dataRowsProcessed = 0;
    bool allTypesDetected = false;
    QHash<QString, bool> typeDetected;
    
    // Read the file in chunks for better performance
    while (!file.atEnd()) {
        // Read a chunk of data
        QByteArray chunk = file.read(4096); // Read 4KB at a time
        
        // Process each character in the chunk
        for (int i = 0; i < chunk.size(); i++) {
            char ch = chunk[i];
            
            // Check for buffer overflow
            if (bufferPos >= MAX_LINE_SIZE - 1) {
                qDebug() << "Warning: Line exceeds maximum length at line" << lineCount + 1;
                // Skip to the end of this line
                while (i < chunk.size() && chunk[i] != '\n') i++;
                bufferPos = 0;
                lineComplete = false;
                continue;
            }
            
            // Check for line end
            if (ch == '\n' || ch == '\r') {
                if (ch == '\r' && i + 1 < chunk.size() && chunk[i + 1] == '\n') {
                    i++; // Skip the \n in \r\n sequence
                }
                
                // Null-terminate the string
                lineBuffer[bufferPos] = '\0';
                lineComplete = true;
            } else {
                // Add character to buffer
                lineBuffer[bufferPos++] = ch;
                continue;
            }
            
            // Process the completed line
            if (lineComplete) {
                QString line = QString::fromUtf8(lineBuffer);
                lineCount++;
                
                // If we're still looking for the header (within first 50 lines)
                if (!headerFound && lineCount <= 50) {
                    // Skip empty lines or lines with very few characters
                    if (line.trimmed().length() < 3) {
                        bufferPos = 0;
                        lineComplete = false;
                        continue;
                    }
                    
                    // Count delimiters in this line
                    int commaCount = line.count(',');
                    int semicolonCount = line.count(';');
                    int tabCount = line.count('\t');
                    int maxCount = std::max({commaCount, semicolonCount, tabCount});
                    
                    // Store the line and its delimiter count
                    candidateLines.append(line);
                    delimiterCounts.append(maxCount);
                    
                    // Update max delimiter count if needed
                    if (maxCount > maxDelimiterCount) {
                        maxDelimiterCount = maxCount;
                        headerLineIndex = candidateLines.size() - 1;
                    }
                    
                    // If we've analyzed 50 lines or reached EOF, determine the header
                    if (lineCount == 50 || file.atEnd()) {
                        // If we found a line with a significant number of delimiters
                        if (maxDelimiterCount >= 2) {
                            qDebug() << "Detected header at line" << (headerLineIndex + 1) 
                                     << "with" << maxDelimiterCount << "delimiters";
                            
                            // Use the line with the most delimiters as the header
                            QString headerLine = candidateLines[headerLineIndex];
                            
                            // Detect the delimiter from the header line
                            detectDelimiter(headerLine);
                            
                            // Parse headers
                            headers = parseLine(headerLine);
                            qDebug() << "Found" << headers.size() << "headers:" << headers;
                            
                            // Initialize DataSerie for each column
                            for (const auto& header : headers) {
                                DataSerie series;
                                series.checked = false;
                                series.points.clear();
                                series.dataType = DataType::Unknown;  // Explicitly set to Unknown
                                qDebug() << "Initializing column:" << header << "with data type:" << static_cast<int>(series.dataType);
                                data.insert(header, series);
                                typeDetected[header] = false;
                            }
                            
                            headerFound = true;
                            processingData = true;
                            
                            // Process any lines we've already read that come after the header
                            for (int j = headerLineIndex + 1; j < candidateLines.size(); j++) {
                                processDataLine(candidateLines[j], errorCount, typeDetected);
                            }
                        } else {
                            // If no clear header was found, use the first non-empty line
                            qDebug() << "No clear header detected, using first line as header";
                            
                            QString headerLine = candidateLines[0];
                            detectDelimiter(headerLine);
                            headers = parseLine(headerLine);
                            qDebug() << "Found" << headers.size() << "headers:" << headers;
                            
                            for (const auto& header : headers) {
                                DataSerie series;
                                series.checked = false;
                                series.points.clear();
                                series.dataType = DataType::Unknown;  // Explicitly set to Unknown
                                qDebug() << "Initializing column:" << header << "with data type:" << static_cast<int>(series.dataType);
                                data.insert(header, series);
                                typeDetected[header] = false;
                            }
                            
                            headerFound = true;
                            processingData = true;
                            
                            // Process any lines we've already read that come after the header
                            for (int j = 1; j < candidateLines.size(); j++) {
                                processDataLine(candidateLines[j], errorCount, typeDetected);
                            }
                        }
                    }
                }
                // If we've found the header and are now processing data
                else if (headerFound && processingData) {
                    processDataLine(line, errorCount, typeDetected);
                    
                    // Check if we've processed enough rows for type detection
                    if (maxRowsForTypeDetection > 0) {
                        dataRowsProcessed++;
                        
                        // Check if all types have been detected
                        bool allDetected = true;
                        for (auto it = typeDetected.begin(); it != typeDetected.end(); ++it) {
                            if (!it.value()) {
                                allDetected = false;
                                break;
                            }
                        }
                        
                        if (allDetected) {
                            qDebug() << "All data types detected after" << dataRowsProcessed << "rows";
                            allTypesDetected = true;
                        }
                        
                        // Stop processing if we've reached the maximum number of rows or detected all types
                        if (dataRowsProcessed >= maxRowsForTypeDetection || allTypesDetected) {
                            qDebug() << "Stopping data processing after" << dataRowsProcessed << "rows";
                            break;
                        }
                    }
                }
                
                // Reset buffer for next line
                bufferPos = 0;
                lineComplete = false;
                
                // Update progress every 1000 lines
                if (lineCount % 1000 == 0) {
                    currentPos = file.pos();
                    int progress = static_cast<int>((currentPos * 100) / fileSize);
                    emit parsingProgress(progress);
                }
            }
        }
    }
    
    // Handle any remaining data in the buffer
    if (bufferPos > 0 && headerFound) {
        lineBuffer[bufferPos] = '\0';
        QString line = QString::fromUtf8(lineBuffer);
        processDataLine(line, errorCount, typeDetected);
    }

    qDebug() << "\n=== CSV Parsing Complete ===";
    qDebug() << "Total lines processed:" << lineCount;
    qDebug() << "Error count:" << errorCount;
    
    // Sort columns by number of points for better debug readability
    QList<QPair<QString, int>> sortedColumns;
    for (auto it = data.begin(); it != data.end(); ++it) {
        sortedColumns.append(qMakePair(it.key(), it.value().points.size()));
    }
    
    std::sort(sortedColumns.begin(), sortedColumns.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
              
    qDebug() << "\n=== CSV Parsing Results ===";
    for (const auto& col : sortedColumns) {
        qDebug() << "Column:" << col.first 
                 << "Points:" << col.second
                 << "Data Type:" << static_cast<int>(data[col.first].dataType);
    }

    file.close();
    emit parsingCompleted();
    return true;
}

// Helper method to process a data line
void CSVParser::processDataLine(const QString& line, int& errorCount, QHash<QString, bool>& typeDetected) {
    QStringList fields = parseLine(line);
    
    // Pad the fields list with empty strings if necessary
    while (fields.size() < headers.size()) {
        fields.append("");
    }
    
    // Convert the running time (first column) to double
    bool ok = false;
    double timestamp = 0.0;
    
    if (!fields.isEmpty()) {
        // Use fast_atof directly without replacing commas with periods
        timestamp = fast_atof(fields[0].toUtf8().constData(), &ok);
        
        if (!ok) {
            if (errorCount <= 5) {
                qDebug() << "Failed to parse timestamp:" << fields[0];
            }
            errorCount++;
        } else {
            // Process each field
            for (int i = 0; i < fields.size() && i < headers.size(); i++) {
                // Skip if this column is not in the list of columns to process
                if (!columnsToProcess.isEmpty() && !columnsToProcess.contains(headers[i]) && i > 0) {
                    // Always process the first column (timestamp)
                    continue;
                }
                
                QString field = fields[i].trimmed();
                if (field.isEmpty()) continue;
                
                // Skip if this column is marked as NotUsed
                if (data[headers[i]].dataType == DataType::NotUsed) continue;
                
                // For type detection, skip if we've already detected the type
                if (maxRowsForTypeDetection > 0 && typeDetected[headers[i]]) continue;
                
                // Try to convert to number if possible - directly use fast_atof
                bool isNumber = false;
                double value = fast_atof(field.toUtf8().constData(), &isNumber);
                
                // Debug: Check if isNumber is working correctly
                if (i < 5) {  // Only log for the first few columns to avoid spam
                    qDebug() << "Field:" << field << "isNumber:" << isNumber;
                    if (isNumber) {
                        qDebug() << "  Converted to:" << value;
                    }
                }
                
                // Check if it's a date or time format
                bool isDateTime = false;
                
                // Check for date format (dd.MM.yyyy or yyyy-MM-dd)
                if (field.contains(QRegularExpression("^\\d{1,2}\\.\\d{1,2}\\.\\d{4}$")) || 
                    field.contains(QRegularExpression("^\\d{4}-\\d{1,2}-\\d{1,2}$"))) {
                    isDateTime = true;
                    qDebug() << "Detected DATE format for field:" << field;
                }
                
                // Check for time format (HH:MM:SS)
                else if (field.contains(QRegularExpression("^\\d{1,2}:\\d{1,2}(:\\d{1,2})?$"))) {
                    isDateTime = true;
                    qDebug() << "Detected TIME format for field:" << field;
                }
                
                // Add the point to the data series based on detected type
                if (isDateTime) {
                    // If this is a date/time field, set the data type to DateTime
                    if (data[headers[i]].dataType == DataType::Unknown) {
                        data[headers[i]].dataType = DataType::DateTime;
                        qDebug() << "Detected DATETIME type for column:" << headers[i] << "Value:" << field;
                        typeDetected[headers[i]] = true;
                    }
                    // We don't add points for DateTime fields by default
                } else if (isNumber) {
                    // If this is a numeric field, set the data type to Numeric if not already set
                    if (data[headers[i]].dataType == DataType::Unknown) {
                        data[headers[i]].dataType = DataType::Numeric;
                        qDebug() << "Detected NUMERIC type for column:" << headers[i] << "Value:" << value;
                        typeDetected[headers[i]] = true;
                    }
                    
                    // Always add points for numeric fields, regardless of whether we're just detecting types
                    data[headers[i]].points.append(QPointF(timestamp, value));
                } else {
                    // This is a string field - could be an operating mode or error
                    // Check if it looks like an operating mode (no special characters, just alphanumeric)
                    bool isOperatingMode = field.contains(QRegularExpression("^[a-zA-Z0-9_\\s]+$"));
                    qDebug() << "String field:" << field << "isOperatingMode:" << isOperatingMode;
                    
                    if (isOperatingMode) {
                        // Looks like an operating mode
                        if (data[headers[i]].dataType == DataType::Unknown || 
                            data[headers[i]].dataType == DataType::OperatingMode) {
                            data[headers[i]].dataType = DataType::OperatingMode;
                            data[headers[i]].isOperatingModeSeries = true;
                            qDebug() << "Detected OPERATING MODE type for column:" << headers[i] << "Value:" << field;
                            typeDetected[headers[i]] = true;
                            
                            // Always add points for operating mode fields
                            data[headers[i]].addOperatingModePoint(field, timestamp);
                        }
                    } else {
                        // Might be an error message
                        if (data[headers[i]].dataType == DataType::Unknown || 
                            data[headers[i]].dataType == DataType::Error) {
                            data[headers[i]].dataType = DataType::Error;
                            qDebug() << "Detected ERROR type for column:" << headers[i] << "Value:" << field;
                            typeDetected[headers[i]] = true;
                            
                            // Always add points for error fields
                            data[headers[i]].addOperatingModePoint(field, timestamp);
                        }
                    }
                }
            }
        }
    }
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
    QStringList parts = fingerprint.split("|");
    if (parts.size() != 3) {
        return false;
    }
    
    int storedColumnCount = parts[0].toInt();
    QString storedHeaderHash = parts[1];
    QString storedDelimiter = parts[2];
    
    QString currentHeaderHash = calculateHeaderHash();
    
    bool matches = storedColumnCount == headers.size() &&
                  storedHeaderHash == currentHeaderHash &&
                  storedDelimiter == delimiter;
                  
    return matches;
}

void CSVParser::saveSettings(QSettings* settings) const {
    if (!settings) return;
    
    qDebug() << "\n=== CSVParser::saveSettings ===";
    
    settings->beginGroup("CSVFormat");
    settings->setValue("fingerprint", generateFingerprint());
    settings->setValue("delimiter", delimiter);
    
    settings->beginWriteArray("Columns");
    for (int i = 0; i < headers.size(); ++i) {
        settings->setArrayIndex(i);
        QString fullPath = "CSV::" + headers[i];
        bool isChecked = data[headers[i]].checked;
        int dataTypeValue = static_cast<int>(data[headers[i]].dataType);
        
        settings->setValue("name", fullPath);
        settings->setValue("checked", isChecked);
        settings->setValue("dataType", dataTypeValue);
        
        qDebug() << "Saving column:" << fullPath 
                 << "checked:" << isChecked 
                 << "dataType:" << dataTypeValue;
    }
    settings->endArray();
    settings->endGroup();
}

void CSVParser::loadSettings(QSettings* settings) {
    if (!settings) return;
    
    qDebug() << "\n=== CSVParser::loadSettings ===";
    
    settings->beginGroup("CSVFormat");
    
    // Load and verify fingerprint
    QString storedFingerprint = settings->value("fingerprint").toString();
    if (!matchesFingerprint(storedFingerprint)) {
        qDebug() << "CSV format mismatch - cannot load settings";
        settings->endGroup();
        return;
    }
    
    // Load column settings
    int size = settings->beginReadArray("Columns");
    
    qDebug() << "Loading" << size << "columns from settings";
    
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        QString fullPath = settings->value("name").toString();
        QString name = fullPath.replace("CSV::", "");  // Remove prefix to match data keys
        bool checked = settings->value("checked").toBool();
        int dataTypeValue = settings->value("dataType", static_cast<int>(DataType::Unknown)).toInt();
        
        if (data.contains(name)) {
            data[name].checked = checked;
            data[name].dataType = static_cast<DataType>(dataTypeValue);
            
            // Set operating mode flag if needed
            if (data[name].dataType == DataType::OperatingMode) {
                data[name].isOperatingModeSeries = true;
            }
            
            qDebug() << "Loaded column:" << name 
                     << "checked:" << checked 
                     << "dataType:" << dataTypeValue;
        } else {
            qDebug() << "Warning: Column" << name << "not found in data";
        }
    }
    settings->endArray();
    settings->endGroup();
    
    // Also check CSVData group for additional settings
    settings->beginGroup("CSVData");
    for (auto it = data.begin(); it != data.end(); ++it) {
        QString columnName = it.key();
        
        // Check if we have checked state information
        if (settings->contains(QString("checked/%1").arg(columnName))) {
            bool checked = settings->value(QString("checked/%1").arg(columnName), false).toBool();
            data[columnName].checked = checked;
            
            qDebug() << "Loaded checked state from CSVData for column:" << columnName 
                     << "Checked:" << checked;
        }
        
        // Check if we have data type information
        if (settings->contains(QString("dataType/%1").arg(columnName))) {
            int dataTypeValue = settings->value(QString("dataType/%1").arg(columnName), 
                                              static_cast<int>(DataType::Unknown)).toInt();
            data[columnName].dataType = static_cast<DataType>(dataTypeValue);
            
            // Set operating mode flag if needed
            if (data[columnName].dataType == DataType::OperatingMode) {
                data[columnName].isOperatingModeSeries = true;
            }
            
            qDebug() << "Loaded data type from CSVData for column:" << columnName 
                     << "Type:" << dataTypeValue;
        }
    }
    settings->endGroup();
} 
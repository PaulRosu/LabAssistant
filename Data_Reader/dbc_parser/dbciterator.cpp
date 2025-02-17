#include "dbciterator.hpp"

#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDebug>

DBCIterator::DBCIterator(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        init(stream);
        file.close();
    } else {
        qCritical("The File could not be opened");
    }
}

DBCIterator::DBCIterator(QTextStream& stream) {
    init(stream);
}

void DBCIterator::init(QTextStream& stream) {
    stream.setEncoding(QStringConverter::Latin1);
    messageList.clear();
    QVector<BusMessage> messages;
    stream.resetStatus ();
    while (!stream.atEnd()) {
        BusMessage msg;
        stream >> msg;

        if (stream.status () != QTextStream::ReadCorruptData){
            messages.append(msg);
        }

    }
    messageList = messages;
}

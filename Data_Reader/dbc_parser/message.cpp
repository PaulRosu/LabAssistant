#include "message.hpp"
#include <QDebug>

// void listProperties(const BusSignal& busSignal) {
//     qDebug() << "BusSignal properties:";
//     qDebug() << "Name:" << busSignal.getName();
//     qDebug() << "ByteOrder:" << static_cast<int>(busSignal.getByteOrder());
//     qDebug() << "StartBit:" << busSignal.getStartBit();
//     qDebug() << "Length:" << busSignal.getLength();
//     qDebug() << "Sign:" << static_cast<int>(busSignal.getSign());
//     qDebug() << "Minimum:" << busSignal.getMinimum();
//     qDebug() << "Maximum:" << busSignal.getMaximum();
//     qDebug() << "Factor:" << busSignal.getFactor();
//     qDebug() << "Offset:" << busSignal.getOffset();
//     qDebug() << "Unit:" << busSignal.getUnit();
//     qDebug() << "Multiplexor:" << static_cast<int>(busSignal.getMultiplexor());
//     qDebug() << "MultiplexedNumber:" << busSignal.getMultiplexedNumber();
//     qDebug() << "Checked:" << busSignal.getChecked();
//     qDebug() << "Occurence:" << busSignal.getOccurence() << Qt::endl;
// }

QTextStream& operator>>(QTextStream& in, BusMessage& msg) {
    QString preamble;
    QChar ch;

    in >> preamble;

    if (preamble != "BO_") {
        while (!in.atEnd() && ch != '\n'){in >> ch; }
        in.setStatus (QTextStream::ReadCorruptData);
        return in;
    }

    in >> msg.id;

    QString name;
    in >> name;
    msg.name = name.left(name.length() - 1);

    in >> msg.dlc;

    while (!in.atEnd() && ch != '\n'){in >> ch; }
    in.resetStatus ();
    in.setStatus (QTextStream::Ok);

    while(!in.atEnd() && in.status () == QTextStream::Ok) {
        BusSignal sig;
        in >> sig;
        if (in.status () == QTextStream::Ok) {
            msg.busSignals.push_back(sig);
        }
    }

    while (!in.atEnd() && ch != '\n'){in >> ch; }

    in.resetStatus ();
    in.setStatus (QTextStream::Ok);
    return in;
}

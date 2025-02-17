
#include "signal.hpp"

#include <QChar>
#include <QDebug>
#include <QLatin1String>
#include <QStringList>
#include <QTextStream>

QTextStream &operator>>(QTextStream &in, BusSignal &sig) {
  QString line = in.readLine().trimmed();
  QChar ch;

  if (line.isEmpty()) {
    in.setStatus(QTextStream::ReadCorruptData);
    return in;
  }

  QTextStream sstream(&line, QIODeviceBase::ReadOnly);
  QString preamble;
  sstream >> preamble;

  if (preamble != "SG_") {
    while (!in.atEnd() && ch != '\n') {
      in >> ch;
    }
    in.setStatus(QTextStream::ReadCorruptData);
    return in;
  }

  sstream >> sig.name;

  QString multi;
  sstream >> multi;

  if (multi == ":") {
    sig.multiplexor = Multiplexor::NONE;
  } else {
    if (multi == "M") {
      sig.multiplexor = Multiplexor::MULTIPLEXOR;
    } else {
      multi.remove(0, 1);
      bool ok;
      quint16 multiNum = multi.toUShort(&ok);
      if (ok) {
        sig.multiplexor = Multiplexor::MULTIPLEXED;
        sig.multiplexNum = multiNum;
        // qDebug() << "Multiplexed signal: " << sig.name
        //          << " multiplexor: " << sig.multiplexNum;
      } else {
        in.setStatus(QTextStream::ReadCorruptData);
        return in;
      }
    }
    sstream >> multi;
  }

  sstream >> sig.startBit;
  sstream.seek(sstream.pos() + 1);
  sstream >> sig.length;
  sstream.seek(sstream.pos() + 1);

  int order;
  sstream >> order;
  sig.order = (order == 0) ? ByteOrder::MOTOROLA : ByteOrder::INTEL;

  QString signChar;
  sstream >> signChar;
  sig.sign = (signChar == "+") ? Sign::UNSIGNED : Sign::SIGNED;

  QString numbers;

  // Reading until '(' for factor and offset
  sstream.skipWhiteSpace();
  sstream >> numbers;

  int parenPos = numbers.indexOf('(');
  int paren2Pos = numbers.indexOf(')');
  if (parenPos != -1) {
    QStringList f_o = numbers.sliced(parenPos + 1, paren2Pos - 1).split(',');
    sig.factor = f_o[0].toDouble();
    sig.offset = f_o[1].toDouble();
  }

  // Reading until '[' for minimum and maximum
  sstream >> numbers;
  int bracketPos = numbers.indexOf('[');
  int bracket2Pos = numbers.indexOf(']');
  if (bracketPos != -1) {
    QStringList min_max =
        numbers.sliced(bracketPos + 1, bracket2Pos - 1).split('|');
    sig.minimum = min_max[0].toDouble();
    sig.maximum = min_max[1].toDouble();
  }

  QString unit;
  sstream >> unit;
  sig.unit = unit.trimmed().remove("\"");

  in.resetStatus();
  in.setStatus(QTextStream::Ok);
  return in;
}

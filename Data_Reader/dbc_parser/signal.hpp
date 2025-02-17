#ifndef SIGNAL_HPP_
#define SIGNAL_HPP_

#include <QDataStream>
#include <QSet>
#include <QString>
#include <QTextStream>
#include <QtCore>

enum class ByteOrder {
    MOTOROLA,
    INTEL
};

enum class Sign {
    UNSIGNED,
    SIGNED
};

enum class Multiplexor {
    NONE,
    MULTIPLEXED,
    MULTIPLEXOR
};

class BusSignal {
  public:
    using ToList = QSet<QString>;
    QString name;
    ByteOrder order;
    quint16 startBit;
    quint16 length;
    Sign sign;
    double minimum;
    double maximum;
    double factor;
    double offset;
    QString unit;
    Multiplexor multiplexor;
    quint16 multiplexNum;
    bool checked;
    int occurence;

    QHash<uint16_t, QList<QPointF>> *series;

    BusSignal() : checked(false), occurence(0)
    {
        series = new QHash<uint16_t, QList<QPointF>>;
    }

    ~BusSignal()
    {
        //        qDeleteAll(series->begin(), series->end());
        series->clear();
        //        delete series;
    }

    friend QTextStream& operator>>(QTextStream& in, BusSignal& sig);

    inline QString getName() const { return name; }
    inline ByteOrder getByteOrder() const { return order; }
    inline quint16 getStartBit() const { return startBit; }
    inline quint16 getLength() const { return length; }
    inline Sign getSign() const { return sign; }
    inline double getMinimum() const { return minimum; }
    inline double getMaximum() const { return maximum; }
    inline double getFactor() const { return factor; }
    inline double getOffset() const { return offset; }
    inline int getOccurence() const { return occurence;}

    // inline void incrementOccurence() { ++occurence; }
    inline void setOccurence(uint newoccurence) { occurence = newoccurence;}

    inline QString getUnit() const { return unit; }
    inline Multiplexor getMultiplexor() const { return multiplexor; }
    inline quint16 getMultiplexedNumber() const { return multiplexNum; }
    inline void setChecked(bool state) { checked = state; }
    inline bool getChecked() const { return checked; }

};

#endif /* SIGNAL_HPP_ */

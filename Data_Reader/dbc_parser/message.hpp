
#ifndef BUSMESSAGE_HPP_
#define BUSMESSAGE_HPP_

#include <QDataStream>
#include <QString>
#include <QTextStream>
#include <QVector>
#include <QtGlobal>

#include "signal.hpp"

class BusMessage {
public:
  QString name;
  QString source;
  quint32 id;
  quint64 dlc;
  bool checked;
  int simple_multiplexor_value = 0;

  typedef QVector<BusSignal> Signals_t;
  Signals_t busSignals;

  inline Signals_t &getSignals() { return busSignals; }

  typedef Signals_t::const_iterator const_iterator;

  friend QTextStream &operator>>(QTextStream &in, BusMessage &msg);

  inline QString getName() const { return name; }
  inline quint32 getId() const { return id; }
  inline quint64 getDlc() const { return dlc; }

  inline const_iterator begin() const { return busSignals.begin(); }
  inline const_iterator end() const { return busSignals.end(); }
  inline Signals_t::const_reference operator[](std::size_t elem) {
    return busSignals[elem];
  }
};

#endif

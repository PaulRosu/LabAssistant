#ifndef DBCTREE_HPP_
#define DBCTREE_HPP_

#include <QVector>
#include <QTextStream>
#include "message.hpp"

class DBCIterator {

    typedef QVector<BusMessage> messages_t;
    messages_t messageList;

public:
    typedef messages_t::const_iterator const_iterator;

    explicit DBCIterator(const QString& filePath);
    explicit DBCIterator(QTextStream& stream);

    const_iterator begin() const { return messageList.begin(); }
    const_iterator end() const { return messageList.end(); }
    const BusMessage& operator[](int index) const { return messageList[index]; }

private:
    void init(QTextStream& stream);

};
#endif /* DBCTREE_HPP_ */

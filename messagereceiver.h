#ifndef MESSAGERECEIVER_H
#define MESSAGERECEIVER_H

#include <QObject>
#include <QByteArray>
#include <QStringList>

class MessageReceiver : public QObject
{
    Q_OBJECT
public:
    explicit MessageReceiver(QObject *parent = nullptr);

    QStringList messages;

public slots:
    void receivedMessage(quint32 instanceId, QByteArray message);

};

#endif // MESSAGERECEIVER_H

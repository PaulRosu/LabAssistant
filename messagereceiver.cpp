#include <QDebug>
#include "messagereceiver.h"
#include <QMessageBox>

MessageReceiver::MessageReceiver(QObject *parent) : QObject(parent)
{
}

void MessageReceiver::receivedMessage(quint32 instanceId, QByteArray message)
{
//    qDebug() << "Received message from instance: " << instanceId;
//    qDebug() << "Message Text: " << message;

//    QMessageBox::information(nullptr,  "Info:",  QString::fromUtf8 ("Received message from instance: \n") +
//                             + instanceId
//                             + QString::fromUtf8 ("Message Text: \n") + message);

    auto args = message.split ('|');
    qDebug() << "message reached handler:" << instanceId <<args;
//    QMessageBox::information(0, "Message!:", args.last());

    messages.append (args.last ());
}

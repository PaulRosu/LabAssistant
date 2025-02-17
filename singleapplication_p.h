#ifndef SINGLEAPPLICATION_P_H
#define SINGLEAPPLICATION_P_H

#include <QtCore>
#include <QtNetwork>
#include "singleapplication.h"

struct InstancesInfo {
    bool primary;
    quint32 secondary;
    qint64 primaryPid;
    char primaryUser[128];
    quint16 checksum; // Must be the last field
};

struct ConnectionInfo {
    qint64 msgLen = 0;
    quint32 instanceId = 0;
    quint8 stage = 0;
};

class SingleApplicationPrivate : public QObject {
Q_OBJECT
public:
    enum ConnectionType : quint8 {
        InvalidConnection = 0,
        NewInstance = 1,
        SecondaryInstance = 2,
        Reconnect = 3
    };
    enum ConnectionStage : quint8 {
        StageHeader = 0,
        StageBody = 1,
        StageConnected = 2,
    };
    Q_DECLARE_PUBLIC(SingleApplication)

    SingleApplicationPrivate( SingleApplication *q_ptr );
    ~SingleApplicationPrivate() override;

    static QString getUsername();
    void genBlockServerName();
    void initializeMemoryBlock() const;
    void startPrimary();
    void startSecondary();
    bool connectToPrimary( int msecs, ConnectionType connectionType );
    quint16 blockChecksum() const;
    qint64 primaryPid() const;
    QString primaryUser() const;
    void readInitMessageHeader(QLocalSocket *socket);
    void readInitMessageBody(QLocalSocket *socket);
    static void randomSleep();

    SingleApplication *q_ptr;
    QSharedMemory *memory;
    QLocalSocket *socket;
    QLocalServer *server;
    quint32 instanceNumber;
    QString blockServerName;
    SingleApplication::Options options;
    QMap<QLocalSocket*, ConnectionInfo> connectionMap;

public Q_SLOTS:
    void slotConnectionEstablished();
    void slotDataAvailable( QLocalSocket*, quint32 );
    void slotClientConnectionClosed( QLocalSocket*, quint32 );
};

#endif // SINGLEAPPLICATION_P_H

#include "mainwindow.h"
#include <QApplication>
#include <QtNetwork>
#include <QMessageBox>


#include <singleapplication.h>
#include "messagereceiver.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <stdio.h>
#endif



//#include "kfr/dft.hpp"
//using namespace kfr;

 QString fileNameArg;
bool argFile = false;
 bool labViewEval;

void setupDebugOutput()
{
#if defined(Q_OS_WIN)
    // If running from console, attach to it
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
}

int main(int argc, char *argv[])
{

    setupDebugOutput();

    // qDebug() << "LabAssistant V" << APP_VERSION << Qt::endl;
    labViewEval = false;

    SingleApplication a( argc, argv, true);
    MessageReceiver msgReceiver;

    if( a.isSecondary()){
        auto resp =  a.sendMessage( a.arguments().join('|').toUtf8(), 30000);
        qDebug() << "App already running.";
        qDebug() << "Primary instance PID: " << a.primaryPid();
        qDebug() << "Primary instance user: " << a.primaryUser();
        if (!resp) qDebug() << "Send message error!:" << "The message was not sent!";
        return 0;
    }else{
        QObject::connect(&a, &SingleApplication::receivedMessage, &msgReceiver, &MessageReceiver::receivedMessage, Qt::QueuedConnection);
    }

    if (QCoreApplication::arguments().size () > 1){
        //            qDebug() << "File Arg:" <<  QCoreApplication::arguments().at (1);
        //                    for (int i = 0; i < QCoreApplication::arguments().size (); i++){
        //                    qDebug () << "args:" << i << QCoreApplication::arguments().at (i);}
        fileNameArg = QCoreApplication::arguments().at (1);
        argFile = true;
    }


    MainWindow w;
    w.setGeometry (QRect(10, 40, 1280, 800));


    if (QCoreApplication::arguments().size () > 2)
        for (int i = 2; i < QCoreApplication::arguments().size (); i++){
            QMetaObject::invokeMethod( &w, "addTab",Qt::QueuedConnection, Q_ARG( QString, QCoreApplication::arguments().at (i) ));
        }

    for (auto path : msgReceiver.messages){
        QMetaObject::invokeMethod( &w, "addTab",Qt::QueuedConnection, Q_ARG( QString, path ));
    }

    w.show();

    qDebug() << "aftershow";

    QObject::connect(&a, &SingleApplication::receivedMessage, &w, &MainWindow::messageHandler, Qt::QueuedConnection);

    // QFile updateLocationFile(QCoreApplication::applicationDirPath() + "\\update_location.txt");
    // updateLocationFile.open (QIODevice::ReadOnly| QIODevice::Text);

    //    if( updateLocationFile.isOpen ()){
    //          QTextStream in(&updateLocationFile);

    //          auto location = in.readLine ();
    //          if (!(location == ""))
    //          {

    //              qDebug() << location;

    //              QFile updateVersionFile(location + "\\versionfdghfg.dat");
    //              if(!updateVersionFile.exists ())
    //              {
    //                  qDebug() << "version file not found!";

    //              }
    //              else
    //              {

    //                     qDebug() << "citim...";

    //                  updateVersionFile.open (QIODevice::ReadOnly| QIODevice::Text);
    //                  QTextStream remote_in(&updateVersionFile);

    //                  auto version = remote_in.readLine ();
    //                  if (!(version == ""))
    //                  {

    //                      qDebug() << "update version:" << version;

    //                      if (!(APP_VERSION == version))
    //                      {
    //                          QString message = "Please use the new version available in the open folder just
    //                          opened!\n"; message += "Your version: "  APP_VERSION "\n"; message += "New version:" +
    //                          version;

    //                          QFileInfo fileinfo(updateVersionFile);
    //                          QDesktopServices::openUrl("file:" + fileinfo.path ().replace ("/", "\\"));

    //                          QMessageBox::information(0, "New version available!", message);
    //                      }
    //                  }
    //              }
    //          }
    //    } else {
    //        qDebug() << updateLocationFile.fileName () << "could not open!";
    //    }

    qDebug() << "gata";

    return a.exec();
}

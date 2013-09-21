/****************************************************************************
** Copyright (C) 2013 Frédéric Logier
** Contact: Frédéric Logier <frederic@logier.org>
**
** https://github.com/fredix/nodeseed
**
** This file is part of Nodeseed.
**
** Nodeseed is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Nodeseed is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Nodeseed.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef NODEFTP_H
#define NODEFTP_H

#include <QDialog>
#include <QMessageBox>
#include <QHash>
#include <QtFtp>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QNetworkSession>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QThread>
#include <QQueue>
#include <QTimer>

class Nodeftp: public QObject
{
    Q_OBJECT
public:
    Nodeftp(QString user, QString password, QString host, int port);
    ~Nodeftp();

public slots:
    void init();
    void queue_file(QString a_file);

private slots:
    void disconnect();
    void send_file();
    void unlock_file();
    void addToList(const QUrlInfo &urlInfo);
    void disconnected(int state);

//    void downloadFile();
//    void cancelDownload();
//    void connectToFtp();

    void ftpCommandFinished(int commandId, bool error);
    void updateDataTransferProgress(qint64 ,qint64);

//    void addToList(const QUrlInfo &urlInfo);
    //void processItem(QTreeWidgetItem *item, int column);
//    void updateDataTransferProgress(qint64 readBytes,
 //                                   qint64 totalBytes);
//    void enableDownloadButton();
//    void enableConnectButton();

signals:
    void ftp_connected();
    void ftp_disconnected();
    void ftp_error(QString error);
    void ftp_progress(qint64 readBytes,qint64 totalBytes);
    void ftp_update_status(QString filename, QString status);

private:
    qint64 readBytes_resume;
    QHash<QString, QString> remote_file;
    bool login;

    bool m_send_lock;
    QTimer *send_timer;
    QThread *thread_ftp;
    QString m_user, m_password, m_host;
    int m_port;
    QFile *m_file;
    QQueue <QString> m_queue;
    //QQueue <QFile*> m_queue_file;
    QFile *file_video;
    QFileInfo *fi;

    QHash<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp;
    QFile *file;

    QNetworkSession *networkSession;
    QNetworkConfigurationManager manager;
};



#endif // NODEFTP_H

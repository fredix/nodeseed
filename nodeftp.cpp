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

#include "nodeftp.h"

Nodeftp::~Nodeftp()
{
    send_timer->stop();
    if (ftp)
    {
        ftp->close();
        delete (ftp);
    }
}

Nodeftp::Nodeftp(QString user, QString password, QString host, int port) : m_user(user), m_password(password), m_host(host), m_port(port)
{

    thread_ftp = new QThread;

    this->moveToThread(thread_ftp);
    thread_ftp->start();
    thread_ftp->connect(thread_ftp, SIGNAL(started()), this, SLOT(init()));


    send_timer = new QTimer();
    connect(send_timer, SIGNAL(timeout()), this, SLOT(send_file ()), Qt::DirectConnection);
    send_timer->start (2000);
    /*
    connect(ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));
*/


/*
    connectButton->setEnabled(false);
    connectButton->setText(tr("Disconnect"));
    statusLabel->setText(tr("Connecting to FTP server %1...")
                         .arg(ftpServerLineEdit->text()));
                         */
}


void Nodeftp::init()
{
    login = false;

    ftp = new QFtp(this);
    connect(ftp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));

    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(ftp, SIGNAL(done(bool)), this, SLOT(unlock_file()));
    connect(ftp, SIGNAL(stateChanged(int)), this, SLOT(disconnected(int)));



    ftp->setTransferMode(QFtp::Passive);
    //ftp->setTransferMode(QFtp::Active);

    ftp->connectToHost(m_host, m_port);
    ftp->login(m_user, m_password);

    m_send_lock = false;
}


void Nodeftp::disconnected(int state)
{
    qDebug() << "FTP DISCONNECTED : " << state;

    if (state == QFtp::Unconnected || state == QFtp::Closing)
    {
        if (login)
        {
        qDebug() << "FTP SERVER CLOSED CONNECTION";
        disconnect();
        emit ftp_disconnected();
        }
    }

}

void Nodeftp::unlock_file()
{
    if (m_send_lock)
    {
        qDebug() << "Nodeftp::unlock_file : " << file_video->fileName();
        file_video->close();
        delete (file_video);
        remote_file.remove(fi->fileName());
        readBytes_resume = 0;

        emit ftp_update_status(fi->fileName(), "uploaded");

        delete (fi);
        m_send_lock = false;
    }
}



void Nodeftp::queue_file(QString a_file)
{

    qDebug() << "Nodeftp::queue_file : " << a_file;

    m_queue.enqueue(a_file);
}



void Nodeftp::send_file()
{
    qDebug() << "Nodeftp::send_file";

    if (!m_queue.isEmpty() && !m_send_lock)
    {
        qDebug() << "Nodeftp::send_file QQUEUE && NOT LOCKED";

        m_send_lock = true;

        ftp->list();

    }
    ftp->state();
}



void Nodeftp::ftpCommandFinished(int, bool error)
{

    qDebug() << "ERROR : " << error << " command : " << ftp->currentCommand() << " QFtp::ConnectToHost : " << QFtp::ConnectToHost;


    if (ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            qDebug() << "UNABLE TO CONNECT : " << ftp->errorString();
            /*QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server "
                                        "at %1. Please check that the host "
                                        "name is correct.")
                                     .arg(m_host));
            */

            emit ftp_error(tr("Unable to connect to the FTP server "
                              "at %1. Please check that the host "
                              "name is correct. error : %2")
                           .arg(m_host).arg(ftp->errorString()));

            disconnect();
            return;
        }
        //statusLabel->setText(tr("Logged onto %1.")
        //                     .arg(m_host));
        //fileList->setFocus();
        //downloadButton->setDefault(true);
        //connectButton->setEnabled(true);
        return;
    }




    if (ftp->currentCommand() == QFtp::Login)
    {
        if (error) {
            qDebug() << "UNABLE TO LOGIN : " << ftp->errorString();

            emit ftp_error(tr("Unable to login to the FTP server "
                              "at %1. Please check username or password. error : %2")
                           .arg(m_host).arg(ftp->errorString()));

            disconnect();
            emit ftp_disconnected();
            return;
        }
        login = true;
        emit ftp_connected();
        //ftp->list();
    }

    if (ftp->currentCommand() == QFtp::Put)
    {
        if (error) {
            qDebug() << "UNABLE TO SEND FILE : " << ftp->errorString();

            emit ftp_error(tr("Unable to send file to the FTP server "
                              "at %1. error : %2")
                           .arg(m_host).arg(ftp->errorString()));

            disconnect();
            emit ftp_disconnected();
            return;
        }

    }

    if (ftp->currentCommand() == QFtp::List)
    {
        if (error) {
            qDebug() << "UNABLE TO LIST FILE : " << ftp->errorString();

            emit ftp_error(tr("Unable to LIST file to the FTP server "
                              "at %1. error : %2")
                           .arg(m_host).arg(ftp->errorString()));

            disconnect();
            emit ftp_disconnected();
            return;
        }


        QString file = m_queue.dequeue();
        qDebug() << "Nodeftp::send_file : " << file;

        file_video = new QFile(file);
        //m_queue_file.enqueue(file_video);

        if (!file_video->open(QIODevice::ReadOnly))
                return;

        fi =  new QFileInfo(file);
        qDebug() << "Nodeftp::send_file : " << fi->fileName();

        readBytes_resume = 0;
        if (remote_file.contains(fi->fileName()))
        {
            qDebug() << "FIND FILE : " << fi->fileName();

          //  ftp->rawCommand("STOR " + fi->fileName());

            readBytes_resume = file_video->size() - (file_video->size() - remote_file[fi->fileName()].toLongLong());
//             read_file = file_video->size() - read_file;
            qDebug() << "readBytes_resume : " << readBytes_resume;

           // updateDataTransferProgress(read_file, file_video->size());
            file_video->seek(1);
//             emit setValue(300);

            if (file_video->seek(remote_file[fi->fileName()].toLongLong()))
            {
                qDebug() << "SEEK : " << remote_file[fi->fileName()] << " LONGLONG : " << remote_file[fi->fileName()].toLongLong();


                if (file_video->atEnd())
                {
                    unlock_file();
                    return;
                }
                ftp->rawCommand("REST " + remote_file[fi->fileName()]);
            }
            else
            {
                qDebug() << "ERROR ON SEEK FILE : " << file_video->fileName();
                file_video->close();
                readBytes_resume = 0;
                m_send_lock = false;
                return;
            }
        }
        qDebug() << "BEFORE PUT : EMIT UPLOADING";
        emit ftp_update_status(fi->fileName(), "uploading");
        ftp->put(file_video, fi->fileName(), QFtp::Binary);


    }


}



void Nodeftp::disconnect()
{
    send_timer->stop();

    if (ftp) {
        ftp->abort();
        ftp->deleteLater();
        ftp = 0;

        //fileList->setEnabled(false);
        //cdToParentButton->setEnabled(false);
        //downloadButton->setEnabled(false);
        //connectButton->setEnabled(true);
        //connectButton->setText(tr("Connect"));


       // statusLabel->setText(tr("Please enter the name of an FTP server."));
        return;
    }
}



void Nodeftp::updateDataTransferProgress(qint64 readBytes,
                                           qint64 totalBytes)
{
    emit ftp_progress(readBytes + readBytes_resume, totalBytes);
}



void Nodeftp::addToList(const QUrlInfo &urlInfo)
{
    remote_file[urlInfo.name()] =  QString::number(urlInfo.size());

    qDebug() << "REMOTE FILE : " << remote_file;

    /*
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, urlInfo.name());
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, urlInfo.owner());
    item->setText(3, urlInfo.group());
    item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

    QPixmap pixmap(urlInfo.isDir() ? ":/images/dir.png" : ":/images/file.png");
    item->setIcon(0, pixmap);

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    fileList->addTopLevelItem(item);
    if (!fileList->currentItem()) {
        fileList->setCurrentItem(fileList->topLevelItem(0));
        fileList->setEnabled(true);
    }*/
}

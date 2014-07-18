/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2014 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "obsaccess.h"

OBSaccess::OBSaccess()
{
    authenticated = false;
    manager = NULL;
    xmlReader = new OBSxmlReader();
    obsPackage = new OBSpackage();
}

void OBSaccess::createManager()
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
    SLOT(provideAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> &)));
}

void OBSaccess::setCredentials(const QString& username, const QString& password)
{
//    Allow login with another username/password
    delete manager;
    createManager();

    curUsername = username;
    curPassword = password;
}

QString OBSaccess::getUsername()
{
    return curUsername;
}

void OBSaccess::makeRequest()
{

//    QByteArray version(appVersion().toAscii());

    QNetworkRequest request;
    request.setUrl(url);
//    request.setRawHeader("User-Agent",version);
    request.setRawHeader("User-Agent","Qactus 0.1.3");
    manager->get(request);

//    Don't make a new request until we get a reply
    QEventLoop *loop = new QEventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply *)),loop, SLOT(quit()));
    loop->exec();
}

void OBSaccess::setUrl(QUrl url)
{
    this->url = url;
}


void OBSaccess::provideAuthentication(QNetworkReply *reply, QAuthenticator *ator)
{
    static QString prevPassword = "";
    static QString prevUsername = "";
//    qDebug() << reply->readAll();

    if (reply->error()!=QNetworkReply::NoError)
    {
            qDebug() << "Request failed! " << reply->errorString();
//            statusBar()->showMessage(tr("Connection failed"), 0);
    }
    else
    {
        if ((curPassword != prevPassword) || (curUsername != prevUsername)) {
            prevPassword = curPassword;
            prevUsername = curUsername;
            ator->setUser(curUsername);
            ator->setPassword(curPassword);
//            statusBar()->showMessage(tr("Authenticating..."), 5000);
        } else {
            qDebug() << "Authentication failed";
            prevPassword = "";
            prevUsername = "";

        }
    }

    if (reply->error()==QNetworkReply::NoError) {
        authenticated = true;
    }
}

//bool OBSaccess::isAuthenticated()
//{
//    return authenticated;
//}

void OBSaccess::replyFinished(QNetworkReply *reply)
{
      // QNetworkReply is a sequential-access QIODevice, which means that
      // once data is read from the object, it no longer kept by the device.
      // It is therefore the application's responsibility to keep this data if it needs to.
      // See http://doc.qt.nokia.com/latest/qnetworkreply.html for more info

    data = (QString) reply->readAll();
    qDebug () << "reply" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "networkreply: " << data;
    qDebug() << "url: " << reply->url();

    if (reply->error() != QNetworkReply::NoError) {
        authenticated = false;
        emit isAuthenticated(authenticated);
//        packageErrors += reply->errorString() + "\n\n";
//        qDebug() << "Request failed, " << packageErrors;
////        QMessageBox::critical(this,tr("Error"), tr("An error has occured:\n") + reply->errorString(), QMessageBox::Ok );
////        statusBar()->showMessage(tr("Error ") + reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString(), 500000);
//        return;
    } else {
        authenticated = true;
        emit isAuthenticated(authenticated);
        qDebug() << "Request succeeded!";

        if (data.contains("status")) {
            xmlReader->parse(data);
            obsPackage = xmlReader->getPackage();
            qDebug() << "obsaccess data.contains pkg: " +  obsPackage->getName();

        } else if (data.contains("action")) {
            xmlReader->parseRequests(data);
            obsRequests = xmlReader->getRequests();
        }
    }
}

OBSpackage* OBSaccess::getPackage()
{
    qDebug() << "obsaccess::getpackage: " + obsPackage->getName();
    return obsPackage;
}

QList<OBSrequest*> OBSaccess::getRequests()
{
    return obsRequests;
}

int OBSaccess::getRequestNumber()
{
    return xmlReader->getRequestNumber();
}


void OBSaccess::onSslErrors(QNetworkReply* reply, const QList<QSslError> &list)
{
    QString errorString;
    QString message;

    foreach (const QSslError &error, list) {
        if (list.count() >= 1) {
            errorString += ", ";
            errorString = error.errorString();
        }
    }
    qDebug() << "SSL Errors:" << errorString;

    if (list.count() == 1) {
        message=tr("An SSL error has occured: %1");
    } else {
        message=list.count()+tr(" SSL errors have occured: %1");
    }

//    if (QMessageBox::warning(this,tr("Warning"),
//                             message.arg(errorString),
//                             QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
////        Ignore SSL error(s) if the user presses ignore
//        reply->ignoreSslErrors();
//    }

}

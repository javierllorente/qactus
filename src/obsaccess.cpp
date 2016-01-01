/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2015 Javier Llorente <javier@opensuse.org>
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

OBSAccess* OBSAccess::instance = NULL;

OBSAccess::OBSAccess()
{
    authenticated = false;
    xmlReader = OBSXmlReader::getInstance();
    createManager();
}

void OBSAccess::createManager()
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
    SLOT(provideAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)),
            this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> &)));
}

OBSAccess* OBSAccess::getInstance()
{
    if (!instance) {
        instance = new OBSAccess();
    }
    return instance;
}

void OBSAccess::setCredentials(const QString& username, const QString& password)
{
//    Allow login with another username/password
    delete manager;
    createManager();

    curUsername = username;
    curPassword = password;
}

QString OBSAccess::getUsername()
{
    return curUsername;
}

void OBSAccess::request(const QString &urlStr)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    const QString userAgent = QCoreApplication::applicationName() + " " +
            QCoreApplication::applicationVersion();
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    manager->get(request);

//    Don't make a new request until we get a reply
    QEventLoop *loop = new QEventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply *)),loop, SLOT(quit()));
    loop->exec();
}

void OBSAccess::request(const QString &urlStr, const int &row)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    const QString userAgent = QCoreApplication::applicationName() + " " +
            QCoreApplication::applicationVersion();
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    QNetworkReply *reply = manager->get(request);
    reply->setProperty("row", row);
}

void OBSAccess::postRequest(const QString &urlStr, const QByteArray &data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    const QString userAgent = QCoreApplication::applicationName() + " " +
            QCoreApplication::applicationVersion();
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, data);

//    Make it a synchronous call
    QEventLoop *loop = new QEventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply *)),loop, SLOT(quit()));
    loop->exec();
}

void OBSAccess::provideAuthentication(QNetworkReply *reply, QAuthenticator *ator)
{
    qDebug() << "provideAuthentication() called";
    static QString prevPassword = "";
    static QString prevUsername = "";
//    qDebug() << reply->readAll();

    if (reply->error()!=QNetworkReply::NoError)
    {
            qDebug() << "Request failed!" << reply->errorString();
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

bool OBSAccess::isAuthenticated()
{
    return authenticated;
}

void OBSAccess::replyFinished(QNetworkReply *reply)
{
    // QNetworkReply is a sequential-access QIODevice, which means that
    // once data is read from the object, it no longer kept by the device.
    // It is therefore the application's responsibility to keep this data if it needs to.
    // See http://doc.qt.nokia.com/latest/qnetworkreply.html for more info

    data = (QString) reply->readAll();
    int row = reply->property("row").toInt();
    qDebug() << "Reply row property:" << QString::number(row);
    xmlReader->setPackageRow(row);

    qDebug() << "URL:" << reply->url();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP status code:" << httpStatusCode;
//    qDebug() << "Network Reply: " << data;

    // Package/Project not found
    if (httpStatusCode==404 && isAuthenticated()) {
        xmlReader->addData(data);
    } else if (httpStatusCode==401) {
        qDebug() << "Authentication failed!";
        authenticated = false;
        emit isAuthenticated(authenticated);
    } else if (reply->error() != QNetworkReply::NoError) {
        authenticated = false;
        qDebug() << "Request failed! Error:" << reply->errorString();
        emit networkError(reply->errorString());
    } else {
        authenticated = true;
        emit isAuthenticated(authenticated);
        qDebug() << "Request succeeded!";

        if(data.startsWith("Index")) {
            // Don't process diffs
            requestDiff = data;
        } else {
            xmlReader->addData(data);
        }
    }
    reply->deleteLater();
}

QString OBSAccess::getRequestDiff()
{
    return requestDiff;
}

void OBSAccess::onSslErrors(QNetworkReply* reply, const QList<QSslError> &list)
{
    QString errorString;
    QString message;

    foreach (const QSslError &sslError, list) {
        if (list.count() >= 1) {
            errorString += ", ";
            errorString = sslError.errorString();
            if (sslError.error() == QSslError::SelfSignedCertificateInChain) {
                qDebug() << "Self signed certificate!";
                emit selfSignedCertificate(reply);
            }
        }
    }
    qDebug() << "SSL Errors:" << errorString;

    if (list.count() == 1) {
        message=tr("An SSL error has occured: %1");
    } else {
        message=list.count()+tr(" SSL errors have occured: %1");
    }

   qDebug() << "onSslErrors() url:" << reply->url() << "row:" << reply->property("row").toInt();
}

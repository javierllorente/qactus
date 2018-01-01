/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2018 Javier Llorente <javier@opensuse.org>
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

OBSAccess *OBSAccess::instance = NULL;
const QString userAgent = APP_NAME + QString(" ") + QACTUS_VERSION;

OBSAccess::OBSAccess()
{
    authenticated = false;
    xmlReader = OBSXmlReader::getInstance();
    manager = nullptr;
}

void OBSAccess::createManager()
{
    qDebug() << "OBSAccess::createManager()";
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            SLOT(provideAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)),
            this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> &)));
}

OBSAccess *OBSAccess::getInstance()
{
    if (!instance) {
        instance = new OBSAccess();
    }
    return instance;
}

void OBSAccess::setCredentials(const QString& username, const QString& password)
{
    qDebug() << "OBSAccess::setCredentials()";
//    Allow login with another username/password
    if (manager!=nullptr) {
        delete manager;
        manager = nullptr;
    }
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
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());  
    manager->get(request);
}

QNetworkReply *OBSAccess::browseRequest(const QString &urlStr)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    QNetworkReply *reply = manager->get(request);
    return reply;
}

void OBSAccess::getProjects(const QString &urlStr)
{
    QNetworkReply *reply = browseRequest(urlStr);
    reply->setProperty("reqtype", OBSAccess::ProjectList);
}

void OBSAccess::getProjectMetadata(const QString &urlStr)
{
    QNetworkReply *reply = browseRequest(urlStr);
    reply->setProperty("reqtype", OBSAccess::ProjectMetadata);
}

void OBSAccess::getPackages(const QString &urlStr)
{
    QNetworkReply *reply = browseRequest(urlStr);
    reply->setProperty("reqtype", OBSAccess::PackageList);
}

void OBSAccess::getFiles(const QString &urlStr)
{
    QNetworkReply *reply = browseRequest(urlStr);
    reply->setProperty("reqtype", OBSAccess::FileList);
}

void OBSAccess::getAllBuildStatus(const QString &urlStr)
{
    QNetworkReply *reply = browseRequest(urlStr);
    reply->setProperty("reqtype", OBSAccess::BuildStatusList);
}

void OBSAccess::request(const QString &urlStr, const int &row)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    QNetworkReply *reply = manager->get(request);
    reply->setProperty("row", row);
}

QNetworkReply *OBSAccess::postRequest(const QString &urlStr, const QByteArray &data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = manager->post(request, data);

    return reply;
}

void OBSAccess::changeSubmitRequest(const QString &urlStr, const QByteArray &data)
{
    QNetworkReply *reply = postRequest(urlStr, data);
    reply->setProperty("reqtype", OBSAccess::SubmitRequest);
}

void OBSAccess::provideAuthentication(QNetworkReply *reply, QAuthenticator *ator)
{
    qDebug() << "OBSAccess::provideAuthentication() for" << reply->request().url().toString();
    static QString prevPassword = "";
    static QString prevUsername = "";
//    qDebug() << reply->readAll();


    if ((curPassword != prevPassword) || (curUsername != prevUsername)) {
        prevPassword = curPassword;
        prevUsername = curUsername;
        ator->setUser(curUsername);
        ator->setPassword(curPassword);;
        //            statusBar()->showMessage(tr("Authenticating..."), 5000);
    } else {
        prevPassword = "";
        prevUsername = "";
        //            authenticated = false;
        //            emit isAuthenticated(authenticated);
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

    QString data = QString::fromUtf8(reply->readAll());
    qDebug() << "OBSAccess::replyFinished()" << reply->url().toString();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "OBSAccess::replyFinished() HTTP status code:" << httpStatusCode;
//    qDebug() << "Network Reply: " << data;

    if (httpStatusCode==302) {
        authenticated = true;
        emit isAuthenticated(authenticated);
    } else if (httpStatusCode==401) {
        authenticated = false;
        emit isAuthenticated(authenticated);
    }

    /* Set package row always (error/no error) if property is valid.
     * Needed for inserting the build status
     */
    if(reply->property("row").isValid()) {
        int row = reply->property("row").toInt();
        qDebug() << "Reply row property:" << QString::number(row);
        xmlReader->setPackageRow(row);
    }

    switch (reply->error()) {

    case QNetworkReply::NoError:
        qDebug() << "OBSAccess::replyFinished() Request succeeded! Status code:" << httpStatusCode;

        if (reply->property("reqtype").isValid()) {
            QString reqType = "RequestType";

            switch(reply->property("reqtype").toInt()) {

            case OBSAccess::ProjectList: // <directory>
                qDebug() << reqType << "ProjectList";
                xmlReader->parseProjectList(data);
                break;

            case OBSAccess::ProjectMetadata: // <project>
                qDebug() << reqType << "ProjectList";
                xmlReader->parseProjectMetadata(data);
                break;

            case OBSAccess::PackageList: // <directory>
                qDebug() << reqType << "PackageList";
                xmlReader->parsePackageList(data);
                break;

            case OBSAccess::FileList: // <directory>
                qDebug() << reqType << "FileList";
                xmlReader->parseFileList(data);
                break;

            case OBSAccess::BuildStatusList: // <resultlist>
                qDebug() << reqType << "BuildStatusList";
                xmlReader->parseResultList(data);
                break;

            case OBSAccess::SubmitRequest:
                qDebug() << reqType << "SubmitRequest";
                xmlReader->parseSubmitRequest(data);
                break;

            case OBSAccess::SRDiff:
                qDebug() << reqType << "SRDiff";
                emit srDiffFetched(data);
                break;
            }
            return;
        }
        xmlReader->addData(data);
        break; // end of case QNetworkReply::NoError

    case QNetworkReply::ContentNotFoundError: // 404
        // Package/Project not found
        if (isAuthenticated()) {
            xmlReader->addData(data);
        }
        break;

    case QNetworkReply::ContentAccessDenied: // 401
        qDebug() << "OBSAccess::replyFinished() Access denied!";
        break;

    case QNetworkReply::AuthenticationRequiredError:
        qDebug() << "OBSAccess::replyFinished() Credentials provided not accepted!";
        break;

    default: // Other errors
        qDebug() << "OBSAccess::replyFinished() Request failed! Error:" << reply->errorString();
        emit networkError(reply->errorString());
        break;
    }

    reply->deleteLater();
}

void OBSAccess::getSRDiff(const QString &urlStr)
{
    QNetworkReply *reply = postRequest(urlStr, "");
    reply->setProperty("reqtype", OBSAccess::SRDiff);
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
                qDebug() << "OBSAccess::onSslErrors() Self signed certificate!";
                emit selfSignedCertificate(reply);
            }
        }
    }
    qDebug() << "OBSAccess::onSslErrors() SSL Errors:" << errorString;

    if (list.count() == 1) {
        message=tr("An SSL error has occured: %1");
    } else {
        message=list.count()+tr(" SSL errors have occured: %1");
    }

   qDebug() << "OBSAccess::onSslErrors() url:" << reply->url() << "row:" << reply->property("row").toInt();
}

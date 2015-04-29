/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2015 Javier Llorente <javier@opensuse.org>
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

OBSaccess* OBSaccess::instance = NULL;

OBSaccess::OBSaccess()
{
    authenticated = false;
    manager = NULL;
    xmlReader = OBSxmlReader::getInstance();
}

void OBSaccess::createManager()
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
    SLOT(provideAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> &)));
}

OBSaccess* OBSaccess::getInstance()
{
    if (!instance) {
        instance = new OBSaccess();
    }
    return instance;
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

void OBSaccess::request(const QString &urlStr)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    const QString userAgent = QCoreApplication::applicationName() + " " +
            QCoreApplication::applicationVersion();
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toAscii());
    manager->get(request);

//    Don't make a new request until we get a reply
    QEventLoop *loop = new QEventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply *)),loop, SLOT(quit()));
    loop->exec();
}

void OBSaccess::postRequest(const QString &urlStr, const QUrl &data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    const QString userAgent = QCoreApplication::applicationName() + " " +
            QCoreApplication::applicationVersion();
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toAscii());
    manager->post(request, data.encodedQuery());
}

void OBSaccess::setApiUrl(const QString &apiUrl)
{
    this->apiUrl = apiUrl;
}

void OBSaccess::provideAuthentication(QNetworkReply *reply, QAuthenticator *ator)
{
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

bool OBSaccess::isAuthenticated()
{
    return authenticated;
}

void OBSaccess::replyFinished(QNetworkReply *reply)
{
      // QNetworkReply is a sequential-access QIODevice, which means that
      // once data is read from the object, it no longer kept by the device.
      // It is therefore the application's responsibility to keep this data if it needs to.
      // See http://doc.qt.nokia.com/latest/qnetworkreply.html for more info

    data = (QString) reply->readAll();
    qDebug() << "URL:" << reply->url();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP status code:" << httpStatusCode;
//    qDebug() << "Network Reply: " << data;

    if (httpStatusCode==404 && isAuthenticated()) {
        xmlReader->addData(data);
    } else if (reply->error() != QNetworkReply::NoError) {
        authenticated = false;
        emit isAuthenticated(authenticated);
        qDebug() << "Request failed!";

//        packageErrors += reply->errorString() + "\n\n";
//        qDebug() << "Request failed, " << packageErrors;
////        QMessageBox::critical(this,tr("Error"), tr("An error has occured:\n") + reply->errorString(), QMessageBox::Ok );
////        statusBar()->showMessage(tr("Error ") + reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString(), 500000);
//        return;
    } else {
        authenticated = true;
        emit isAuthenticated(authenticated);
        qDebug() << "Request succeeded!";
        xmlReader->addData(data);
    }
}

void OBSaccess::login()
{
    request(apiUrl + "/");
}

OBSpackage* OBSaccess::getBuildStatus(const QStringList &stringList)
{
//    URL format: https://api.opensuse.org/build/KDE:Extra/openSUSE_13.2/x86_64/qrae/_status
    request(apiUrl + "/build/"
                 + stringList[0] + "/"
            + stringList[1] + "/"
            + stringList[2] + "/"
            + stringList[3] + "/_status");
    return xmlReader->getPackage();
}

QList<OBSrequest*> OBSaccess::getRequests()
{
    request(apiUrl + "/request?view=collection&states=new&roles=maintainer&user=" + getUsername());
    return xmlReader->getRequests();
}

int OBSaccess::getRequestNumber()
{
    return xmlReader->getRequestNumber();
}

void OBSaccess::acceptRequest(const QString &id, const QString &comments)
{
    QUrl data;
    data.addQueryItem("comments", comments);
    postRequest(apiUrl + "/request/" + id + "?cmd=changestate&newstate=accepted", data);
}

void OBSaccess::declineRequest(const QString &id, const QString &comments)
{
    QUrl data;
    data.addQueryItem("comments", comments);
    postRequest(apiUrl + "/request/" + id + "?cmd=changestate&newstate=declined", data);
}

QStringList OBSaccess::getProjectList()
{
    request(apiUrl + "/source");
    return xmlReader->getList();
}

QStringList OBSaccess::getPackageListForProject(const QString &projectName)
{
    request(apiUrl + "/source/" + projectName);
    return xmlReader->getList();
}

QStringList OBSaccess::getMetadataForProject(const QString &projectName)
{
    request(apiUrl + "/source/" + projectName + "/_meta");
    return xmlReader->getList();
}

void OBSaccess::onSslErrors(QNetworkReply* /*reply*/, const QList<QSslError> &list)
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

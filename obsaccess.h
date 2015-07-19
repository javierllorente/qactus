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

#ifndef OBSACCESS_H
#define OBSACCESS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QSslError>
#include <QDebug>
#include <QEventLoop>
#include <QCoreApplication>
#include "obsxmlreader.h"
#include "obspackage.h"

class OBSXmlReader;
class OBSPackage;

class OBSAccess : public QObject
{
     Q_OBJECT

public:
    static OBSAccess* getInstance();
    bool isAuthenticated();
    void setApiUrl(const QString &apiUrl);
    void login();
    OBSPackage* getBuildStatus(const QStringList &list, const int &row);
    QString getUsername();
    QList<OBSRequest*> getRequests();
    int getRequestNumber();
    QString acceptRequest(const QString &id, const QString &comments);
    QString declineRequest(const QString &id, const QString &comments);
    QString diffRequest(const QString &source);
    QStringList getProjectList();
    QStringList getPackageListForProject(const QString &projectName);
    QStringList getMetadataForProject(const QString &projectName);

signals:
    void isAuthenticated(bool authenticated);

public slots:
    void setCredentials(const QString&, const QString&);
    void provideAuthentication(QNetworkReply* reply, QAuthenticator* ator);
    void replyFinished(QNetworkReply* reply);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError> &list);
    void packageIsReady(OBSPackage*, const int&);
    void requestsAreReady(QList<OBSRequest*>);

private:
/*
 * We need to instantiate QNAM only once, as it should be used as
 * as singleton or a utility instead of recreating it for each
 * request. This class (OBSaccess) uses the singleton pattern to
 * achieve this.
 *
 */
    QNetworkAccessManager* manager;
    void createManager();
    OBSAccess();
    static OBSAccess* instance;
    QString apiUrl;
    void request(const QString &urlStr);
    void request(const QString &urlStr, const int &row);
    void postRequest(const QString &urlStr, const QByteArray &data);
    QString curUsername;
    QString curPassword;
    QString data;
    bool authenticated;
    OBSXmlReader *xmlReader;
    QList<OBSRequest*> obsRequests;

signals:
    void finishedParsingPackage(OBSPackage*, const int&);
    void finishedParsingRequests(QList<OBSRequest*>);
};

#endif // OBSACCESS_H

/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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

#ifndef OBS_H
#define OBS_H

#include <QObject>
#include "obsaccess.h"
#include "obsxmlreader.h"
#include "obspackage.h"

class OBS : public QObject
{
    Q_OBJECT

public:
    explicit OBS(QObject *parent = 0);
    void setCredentials(const QString &username, const QString &password);
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    QString getApiUrl() const;
    void request(const QString& urlStr);
    void request(const QString &urlStr, const int &row);
    void postRequest(const QString &urlStr, const QByteArray &data);
    bool isAuthenticated();
    void login();
    void getBuildStatus(const QStringList &stringList, const int &row);
    void getRequests();
    int getRequestCount();
    QString acceptRequest(const QString &id, const QString &comments);
    QString declineRequest(const QString &id, const QString &comments);
    QString getRequestDiff(const QString &source);
    QStringList getProjectList();
    QStringList getProjectPackageList(const QString &projectName);
    QStringList getProjectMetadata(const QString &projectName);
    QStringList getRepositoryArchs(const QString &repository);
    QStringList readXmlFile(const QString &xmlFile);
    OBSXmlReader* getXmlReader();   

private:
    OBSAccess *obsAccess;
    OBSXmlReader *xmlReader;
    QString apiUrl;

signals:
    void isAuthenticated(bool);
    void selfSignedCertificate(QNetworkReply*);
    void networkError(const QString&);
    void finishedParsingPackage(OBSPackage*, const int&);
    void finishedParsingRequests(QList<OBSRequest*>);
    void finishedParsingList(QStringList);

public slots:

};

#endif // OBS_H

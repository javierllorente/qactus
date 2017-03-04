/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2017 Javier Llorente <javier@opensuse.org>
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
    bool isAuthenticated();
    void login();
    void getBuildStatus(const QStringList &stringList, const int &row);
    void getRevisions(const QString &project, const QString &package);
    void getRequests();
    int getRequestCount();
    void getRequestDiff(const QString &source);
    void getProjects();
    void getPackages(const QString &project);
    void getProjectMetadata(const QString &project);
    void getFiles(const QString &project, const QString &package);
    QStringList getRepositoryArchs(const QString &repository);
    QStringList readXmlFile(const QString &xmlFile);
    void changeSubmitRequest(const QString &urlStr, const QByteArray &data);
    OBSXmlReader* getXmlReader();   

private:
    OBSAccess *obsAccess;
    OBSXmlReader *xmlReader;
    QString apiUrl;
    void postRequest(const QString &urlStr, const QByteArray &data);

signals:
    void isAuthenticated(bool);
    void selfSignedCertificate(QNetworkReply*);
    void networkError(const QString&);
    void finishedParsingPackage(OBSPackage*, const int&);
    void finishedParsingResult(OBSResult*);
    void finishedParsingResultList();
    void finishedParsingRevision(OBSRevision*);
    void finishedParsingRequest(OBSRequest*);
    void removeRequest(const QString&);
    void projectListIsReady();
    void projectMetadataIsReady();
    void packageListIsReady();
    void finishedParsingList(QStringList);
    void finishedParsingFile(OBSFile*);
    void srStatus(const QString &);
    void srDiffFetched(const QString &);

public slots:
    void getAllBuildStatus(const QString &project, const QString &package);
    void changeSubmitRequestSlot(const QString &id, const QString &comments, bool accepted);
    void srChangeResult(OBSPackage *obsPackage);
};

#endif // OBS_H

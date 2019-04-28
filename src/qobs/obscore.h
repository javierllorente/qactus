/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2013-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSCORE_H
#define OBSCORE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QSslError>
#include <QDebug>
#include <QEventLoop>
#include "obsxmlreader.h"

class OBSXmlReader;

class OBSCore : public QObject
{
     Q_OBJECT

public:
    static OBSCore* getInstance();
    bool isAuthenticated();
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    QString getApiUrl() const;
    void login();
    QNetworkReply *request(const QString &resource);
    void request(QNetworkReply *reply);
    QNetworkReply *requestBuild(const QString &resource);
    void getBuildStatus(const QStringList &build, int row);
    QNetworkReply *requestSource(const QString &resource);
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    QNetworkReply *postRequest(const QString &resource, const QByteArray &data, const QString &contentTypeHeader);
    QNetworkReply *putRequest(const QString &resource, const QByteArray &data);
    QNetworkReply *deleteRequest(const QString &resource);
    bool isIncludeHomeProjects() const;
    void setIncludeHomeProjects(bool value);
    void getProjects();
    void getProjectMetadata(const QString &resource);
    void getPackages(const QString &resource);
    void getFiles(const QString &project, const QString &package);
    void getLink(const QString &project, const QString &package);
    void getAllBuildStatus(const QString &resource);
    void changeSubmitRequest(const QString &resource, const QByteArray &data);
    void request(const QString &resource, int row);
    void getSRDiff(const QString &resource);
    void branchPackage(const QString &project, const QString &package);
    void createRequest(const QByteArray &data);
    void createProject(const QString &project, const QByteArray &data);
    void createPackage(const QString &project, const QString &package, const QByteArray &data);
    void uploadFile(const QString &project, const QString &package, const QString &fileName, const QByteArray &data);
    void downloadFile(const QString &project, const QString &package, const QString &fileName);
    void getBuildLog(const QString &project, const QString &repository, const QString &arch, const QString &package);
    void deleteProject(const QString &project);
    void deletePackage(const QString &project, const QString &package);
    void deleteFile(const QString &project, const QString &package, const QString &fileName);
    void about();
    void getPerson();
    void updatePerson(const QByteArray &data);

signals:
    void apiNotFound(QUrl url);
    void isAuthenticated(bool authenticated);
    void selfSignedCertificate(QNetworkReply *reply);
    void networkError(const QString &error);
    void srDiffFetched(const QString &diff);
    void fileFetched(const QString &fileName, const QByteArray &data);
    void buildLogFetched(const QString &buildLog);
    void buildLogNotFound();
    void projectNotFound(OBSStatus *status);
    void cannotCreateProject(OBSStatus *obsStatus);
    void cannotCreatePackage(OBSStatus *obsStatus);
    void cannotUploadFile(OBSStatus *obsStatus);
    void cannotDeleteProject(OBSStatus *obsStatus);
    void cannotDeletePackage(OBSStatus *obsStatus);
    void cannotDeleteFile(OBSStatus *obsStatus);

public slots:
    void setCredentials(const QString&, const QString&);

private slots:
    void provideAuthentication(QNetworkReply *reply, QAuthenticator* ator);
    void replyFinished(QNetworkReply *reply);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &list);

private:
/*
 * We need to instantiate QNAM only once, as it should be used as
 * as singleton or a utility instead of recreating it for each
 * request. This class (OBSCore) uses the singleton pattern to
 * achieve this.
 *
 */
    QNetworkAccessManager *manager;
    void createManager();
    OBSCore();
    static OBSCore *instance;
    QString curUsername;
    QString curPassword;
    QString prevPassword;
    QString prevUsername;
    QString apiUrl;
    enum RequestType {
        Login,
        ProjectList,
        ProjectMetadata,
        PackageList,
        FileList,
        Link,
        BuildStatus,
        BuildStatusList,
        IncomingRequests,
        OutgoingRequests,
        DeclinedRequests,
        ChangeRequestState,
        SRDiff,
        BranchPackage,
        CreateRequest,
        CreateProject,
        CreatePackage,
        UploadFile,
        DownloadFile,
        BuildLog,
        DeleteProject,
        DeletePackage,
        DeleteFile,
        About,
        Person,
        UpdatePerson
    };
    bool authenticated;
    OBSXmlReader *xmlReader;
    bool includeHomeProjects;
    QString createReqResourceStr(const QString &states, const QString &roles) const;
    void getRequests(OBSCore::RequestType type);
};

#endif // OBSCORE_H

/*
 * Copyright (C) 2013-2025 Javier Llorente <javier@opensuse.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
#include "obslinkhelper.h"

class OBSCore : public QObject
{
     Q_OBJECT

public:
    static OBSCore* getInstance();
    bool isAuthenticated() const;
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    QString getApiUrl() const;
    void login();
    QNetworkReply *request(const QString &resource);
    void request(QNetworkReply *reply);
    QNetworkReply *requestBuild(const QString &resource);
    void getBuildStatus(const QStringList &build, int row);
    QNetworkReply *requestSource(const QString &resource);
    QNetworkReply *requestRequest(const QString &resource);
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    void getProjectRequests(const QString &project);
    void getPackageRequests(const QString &project, const QString &package);
    QNetworkReply *postRequest(const QString &resource, const QByteArray &data, const QString &contentTypeHeader);
    QNetworkReply *putRequest(const QString &resource, const QByteArray &data);
    QNetworkReply *deleteRequest(const QString &resource);
    bool isIncludeHomeProjects() const;
    void setIncludeHomeProjects(bool value);
    void getProjects();
    void getProjectMetaConfig(const QString &resource);
    void getPackageMetaConfig(const QString &resource);
    void getPackages(const QString &resource);
    void getFiles(const QString &project, const QString &package);
    void getRevisions(const QString &project, const QString &package);
    void getLatestRevision(const QString &project, const QString &package);
    void getLink(const QString &project, const QString &package);
    void getAllBuildStatus(const QString &resource);
    void changeSubmitRequest(const QString &resource, const QByteArray &data);
    void packageSearch(const QString &package);
    void request(const QString &resource, int row);
    void getRequestDiff(const QString &resource);
    void branchPackage(const QString &project, const QString &package);
    void linkPackage(const QString &srcProject, const QString &srcPackage, const QString &dstProject);
    void copyPackage(const QString &originProject, const QString &originPackage,
                     const QString &destProject, const QString &destPackage, const QString &comments);
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
    void getDistributions();

signals:
    void apiNotFound(const QUrl &url);
    void authenticated(bool authenticated);
    void selfSignedCertificateError(QNetworkReply *reply);
    void networkError(const QString &error);
    void requestDiffFetched(const QString &diff);
    void fileFetched(const QString &fileName, const QByteArray &data);
    void buildLogFetched(const QString &buildLog);
    void buildLogNotFound();
    void projectNotFound(QSharedPointer<OBSStatus> status);
    void packageNotFound(QSharedPointer<OBSStatus> status);
    void cannotLinkPackage(QSharedPointer<OBSStatus> status);
    void cannotCopyPackage(QSharedPointer<OBSStatus> status);
    void cannotCreateProject(QSharedPointer<OBSStatus> status);
    void cannotCreatePackage(QSharedPointer<OBSStatus> status);
    void cannotUploadFile(QSharedPointer<OBSStatus> status);
    void cannotDeleteProject(QSharedPointer<OBSStatus> status);
    void cannotDeletePackage(QSharedPointer<OBSStatus> status);
    void cannotDeleteFile(QSharedPointer<OBSStatus> status);

public slots:
    void setCredentials(const QString&, const QString&);
    void onReadyToLinkPackage(const QString &dstProject, const QString &dstPackage, const QByteArray &data);

private slots:
    void provideAuthentication(QNetworkReply *reply, QAuthenticator *authenticator);
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
    QString username;
    QString password;
    QString apiUrl;
    enum RequestType {
        Login,
        ProjectList,
        PrjMetaConfig,
        PkgMetaConfig,
        PackageList,
        FileList,
        RevisionList,
        LatestRevision,
        Link,
        BuildStatus,
        BuildStatusList,
        IncomingRequests,
        OutgoingRequests,
        DeclinedRequests,
        ProjectRequests,
        PackageRequests,
        ChangeRequestState,
        PackageSearch,
        SRDiff,
        BranchPackage,
        LinkPackage,
        CopyPackage,
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
        UpdatePerson,
        Distributions
    };
    bool m_authenticated;
    OBSXmlReader *xmlReader;
    bool includeHomeProjects;
    OBSLinkHelper *linkHelper;
    QString createReqResourceStr(const QString &states, const QString &roles) const;
    void getRequests(OBSCore::RequestType type);
};

#endif // OBSCORE_H

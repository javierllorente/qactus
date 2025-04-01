/*
 * Copyright (C) 2015-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef OBS_H
#define OBS_H

#include <QObject>
#include "obscore.h"
#include "obsxmlreader.h"
#include "obsstatus.h"

#if defined(QOBS_LIBRARY)
#  define QOBS_EXPORT Q_DECL_EXPORT
#else
#  define QOBS_EXPORT Q_DECL_IMPORT
#endif

class QOBS_EXPORT OBS : public QObject
{
    Q_OBJECT

public:
    explicit OBS(QObject *parent = 0);
    void setCredentials(const QString &username, const QString &password);
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    QString getApiUrl() const;
    void request(const QString& resource);
    void request(QNetworkReply *reply);
    bool isAuthenticated();
    void login();
    void getBuildStatus(const QStringList &stringList, int row);
    void getProjectResults(const QString &project);
    void getLatestRevision(const QString &project, const QString &package);
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    int getRequestCount();
    void getProjectRequests(const QString &project);
    void getPackageRequests(const QString &project, const QString &package);
    void getRequestDiff(const QString &source);
    bool isIncludeHomeProjects() const;
    void setIncludeHomeProjects(bool value);
    void getProjects();
    void getPackages(const QString &project);
    void getProjectMetaConfig(const QString &project);
    void getPackageMetaConfig(const QString &project, const QString &package);
    void getFiles(const QString &project, const QString &package);
    void getRevisions(const QString &project, const QString &package);
    void getLink(const QString &project, const QString &package);
    void changeSubmitRequest(const QString &resource, const QByteArray &data);
    void packageSearch(const QString &package);
    OBSXmlReader *getXmlReader();
    void branchPackage(const QString &project, const QString &package);
    void linkPackage(const QString &srcProject, const QString &srcPackage,
                     const QString &dstProject);
    void copyPackage(const QString &originProject, const QString &originPackage,
                     const QString &destProject, const QString &destPackage, const QString &comments);
    void deleteProject(const QString &project);
    void deletePackage(const QString &project, const QString &package);
    void deleteFile(const QString &project, const QString &package, const QString &fileName);
    void about();
    void getPerson();
    void updatePerson(const QByteArray &data);
    void getDistributions();

private:
    OBSCore *obsCore;
    OBSXmlReader *xmlReader;
    bool includeHomeProjects;
    void postRequest(const QString &resource, const QByteArray &data, const QString &contentTypeHeader);
    void deleteRequest(const QString &resource);

signals:
    void apiNotFound(const QUrl &url);
    void authenticated(bool authenticated);
    void selfSignedCertificateError(QNetworkReply *reply);
    void networkError(const QString &error);
    void finishedParsingPackage(QSharedPointer<OBSStatus> status, int row);
    void finishedParsingBranchPackage(QSharedPointer<OBSStatus> status);
    void finishedParsingLinkPkgRevision(QSharedPointer<OBSRevision> revision);
    void finishedParsingCopyPkgRevision(QSharedPointer<OBSRevision> revision);
    void finishedParsingCreateRequest(QSharedPointer<OBSRequest> request);
    void finishedParsingCreateRequestStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingCreatePrjStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingCreatePkgStatus(QSharedPointer<OBSStatus> status);
    void cannotLinkPackage(QSharedPointer<OBSStatus> status);
    void cannotCopyPackage(QSharedPointer<OBSStatus> status);
    void cannotCreateProject(QSharedPointer<OBSStatus> status);
    void cannotCreatePackage(QSharedPointer<OBSStatus> status);
    void finishedParsingUploadFileRevision(QSharedPointer<OBSRevision> revision);
    void cannotUploadFile(QSharedPointer<OBSStatus> status);
    void fileFetched(const QString &fileName, const QByteArray &data);
    void buildLogFetched(const QString &buildLog);
    void buildLogNotFound();
    void projectNotFound(QSharedPointer<OBSStatus> status);
    void packageNotFound(QSharedPointer<OBSStatus> status);
    void finishedParsingDeletePrjStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingDeletePkgStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingDeleteFileStatus(QSharedPointer<OBSStatus> status);
    void cannotDeleteProject(QSharedPointer<OBSStatus> status);
    void cannotDeletePackage(QSharedPointer<OBSStatus> status);
    void cannotDeleteFile(QSharedPointer<OBSStatus> status);
    void finishedParsingResult(QSharedPointer<OBSResult> result);
    void finishedParsingResultList(QList<QSharedPointer<OBSResult>> resultList);
    void finishedParsingRevision(QSharedPointer<OBSRevision> revision);
    void finishedParsingRequest(QSharedPointer<OBSRequest> request);
    void finishedParsingRequestList(const QString &project, const QString &package);
    void finishedParsingLatestRevision(QSharedPointer<OBSRevision> revision);
    void finishedParsingIncomingRequest(QSharedPointer<OBSRequest> request);
    void finishedParsingIncomingRequestList();
    void finishedParsingOutgoingRequest(QSharedPointer<OBSRequest> request);
    void finishedParsingOutgoingRequestList();
    void finishedParsingDeclinedRequest(QSharedPointer<OBSRequest> request);
    void finishedParsingDeclinedRequestList();
    void finishedParsingProjectList(const QStringList &projectList);
    void projectFetched(const QString &project);
    void finishedParsingProjectMetaConfig(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void finishedParsingPackageMetaConfig(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig);
    void finishedParsingPackageList(const QStringList &packageList);
    void finishedParsingList(const QStringList &list);
    void finishedParsingFile(QSharedPointer<OBSFile> file);
    void finishedParsingFileList(const QString &project, const QString &package);
    void finishedParsingRevisionList(const QString &project, const QString &package);
    void finishedParsingLink(QSharedPointer<OBSLink> link);
    void finishedParsingRequestStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingPackageSearch(const QStringList &results);
    void requestDiffFetched(const QString &diff);
    void finishedParsingAbout(QSharedPointer<OBSAbout> about);
    void finishedParsingPerson(QSharedPointer<OBSPerson> person);
    void finishedParsingUpdatePerson(QSharedPointer<OBSStatus> status);
    void finishedParsingDistribution(QSharedPointer<OBSDistribution> distribution);

public slots:
    void getAllBuildStatus(const QString &project, const QString &package);
    void onChangeRequest(const QString &id, const QString &comments, bool accepted);
    void createRequest(const QByteArray &data);
    void createProject(const QString &project, const QByteArray &data);
    void createPackage(const QString &project, const QString &package, const QByteArray &data);
    void uploadFile(const QString &project, const QString &package, const QString &fileName, const QByteArray &data);
    void downloadFile(const QString &project, const QString &package, const QString &fileName);
    void getBuildLog(const QString &project, const QString &repository, const QString &arch, const QString &package);
};

#endif // OBS_H

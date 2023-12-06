/*
 * Copyright (C) 2015-2021 Javier Llorente <javier@opensuse.org>
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
    void getRevisions(const QString &project, const QString &package);
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    int getRequestCount();
    void getRequestDiff(const QString &source);
    bool isIncludeHomeProjects() const;
    void setIncludeHomeProjects(bool value);
    void getProjects();
    void getPackages(const QString &project);
    void getProjectMetaConfig(const QString &project);
    void getPackageMetaConfig(const QString &project, const QString &package);
    void getFiles(const QString &project, const QString &package);
    void getLink(const QString &project, const QString &package);
    void changeSubmitRequest(const QString &resource, const QByteArray &data);
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
    void apiNotFound(QUrl url);
    void isAuthenticated(bool);
    void selfSignedCertificate(QNetworkReply*);
    void networkError(const QString&);
    void finishedParsingPackage(OBSStatus*, int);
    void finishedParsingBranchPackage(OBSStatus*);
    void finishedParsingLinkPkgRevision(OBSRevision *revision);
    void finishedParsingCopyPkgRevision(OBSRevision *revision);
    void finishedParsingCreateRequest(OBSRequest*);
    void finishedParsingCreateRequestStatus(OBSStatus*);
    void finishedParsingCreatePrjStatus(OBSStatus*);
    void finishedParsingCreatePkgStatus(OBSStatus*);
    void cannotLinkPackage(OBSStatus *obsStatus);
    void cannotCopyPackage(OBSStatus *obsStatus);
    void cannotCreateProject(OBSStatus *obsStatus);
    void cannotCreatePackage(OBSStatus *obsStatus);
    void finishedParsingUploadFileRevision(OBSRevision*);
    void cannotUploadFile(OBSStatus *obsStatus);
    void fileFetched(const QString &fileName, const QByteArray &data);
    void buildLogFetched(const QString &buildLog);
    void buildLogNotFound();
    void projectNotFound(OBSStatus *status);
    void packageNotFound(OBSStatus *status);
    void finishedParsingDeletePrjStatus(OBSStatus*);
    void finishedParsingDeletePkgStatus(OBSStatus*);
    void finishedParsingDeleteFileStatus(OBSStatus*);
    void cannotDeleteProject(OBSStatus *obsStatus);
    void cannotDeletePackage(OBSStatus *obsStatus);
    void cannotDeleteFile(OBSStatus *obsStatus);
    void finishedParsingResult(OBSResult*);
    void finishedParsingResultList(QList<OBSResult *> resultList);
    void finishedParsingRevision(OBSRevision*);
    void finishedParsingIncomingRequest(OBSRequest *request);
    void finishedParsingIncomingRequestList();
    void finishedParsingOutgoingRequest(OBSRequest *request);
    void finishedParsingOutgoingRequestList();
    void finishedParsingDeclinedRequest(OBSRequest *request);
    void finishedParsingDeclinedRequestList();
    void finishedParsingProjectList(QStringList);
    void projectFetched(const QString &project);
    void finishedParsingProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig);
    void finishedParsingPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig);
    void finishedParsingPackageList(QStringList);
    void finishedParsingList(QStringList);
    void finishedParsingFile(OBSFile*);
    void finishedParsingFileList();
    void finishedParsingLink(OBSLink *obsLink);
    void finishedParsingRequestStatus(OBSStatus *status);
    void srDiffFetched(const QString &);
    void finishedParsingAbout(OBSAbout *obsAbout);
    void finishedParsingPerson(OBSPerson *obsPerson);
    void finishedParsingUpdatePerson(OBSStatus *obsStatus);
    void finishedParsingDistribution(OBSDistribution *distribution);

public slots:
    void getAllBuildStatus(const QString &project, const QString &package);
    void slotChangeSubmitRequest(const QString &id, const QString &comments, bool accepted);
    void createRequest(const QByteArray &data);
    void createProject(const QString &project, const QByteArray &data);
    void createPackage(const QString &project, const QString &package, const QByteArray &data);
    void uploadFile(const QString &project, const QString &package, const QString &fileName, const QByteArray &data);
    void downloadFile(const QString &project, const QString &package, const QString &fileName);
    void getBuildLog(const QString &project, const QString &repository, const QString &arch, const QString &package);
};

#endif // OBS_H

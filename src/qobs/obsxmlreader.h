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
#ifndef OBSXMLREADER_H
#define OBSXMLREADER_H

#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSharedPointer>
#include "obsrequest.h"
#include "obsfile.h"
#include "obslink.h"
#include "obsresult.h"
#include "obsrevision.h"
#include "obsstatus.h"
#include "obsabout.h"
#include "obsperson.h"
#include "obsprjmetaconfig.h"
#include "obspkgmetaconfig.h"
#include "obsdistribution.h"

class OBSXmlReader : public QObject
{
    Q_OBJECT

public:
    static OBSXmlReader *getInstance();
    void addData(const QString &data);
    void setPackageRow(int row);
    void parseProjectList(const QString &userHome, const QString &data);
    void parsePrjMetaConfig(const QString &data);
    void parsePkgMetaConfig(const QString &data);
    void parseBuildStatus(const QString &data);
    QSharedPointer<OBSStatus> parseNotFoundStatus(const QString &data);
    void parsePackageList(const QString &data);
    void parseFileList(const QString &project, const QString &package, const QString &data);
    void parseRevisionList(const QString &project, const QString &package, const QString &data);
    void parseLatestRevision(const QString &project, const QString &package, const QString &data);
    void parseLink(const QString &data);
    void parseResultList(const QString &data);
    void parseIncomingRequests(const QString &data);
    void parseOutgoingRequests(const QString &data);
    void parseDeclinedRequests(const QString &data);
    void parseRequestStatus(const QString &data);
    void parseRequests(const QString &project, const QString &package, const QString &data);
    void parseBranchPackage(const QString &data);
    void parseLinkPackage(const QString &project, const QString &package, const QString &data);
    void parseCopyPackage(const QString &project, const QString &package, const QString &data);
    void parseCreateRequest(const QString &data);
    void parseCreateRequestStatus(const QString &data);
    void parseCreateProject(const QString &project, const QString &data);
    void parseCreatePackage(const QString &package, const QString &project, const QString &data);
    void parseUploadFile(const QString &project, const QString &package, const QString &file, const QString &data);
    void parseDeleteProject(const QString &project, const QString &data);
    void parseDeletePackage(const QString &project, const QString &package, const QString &data);
    void parseDeleteFile(const QString &project, const QString &package, const QString &fileName, const QString &data);
    int getRequestNumber();
    void parseAbout(const QString &data);
    void parsePerson(const QString &data);
    void parseUpdatePerson(const QString &data);
    void parseDistributions(const QString &data);

private:
    static OBSXmlReader *instance;
    OBSXmlReader();
    void parseStatus(QXmlStreamReader &xml, QSharedPointer<OBSStatus> status);
    int row;
    void parseRevision(QXmlStreamReader &xml, OBSRevision *obsRevision);
    QList<QString> requestIdList;
    QList<QString> oldRequestIdList;
    void parseCollection(QXmlStreamReader &xml);
    OBSRequest *parseRequest(QXmlStreamReader &xml);
    QStringList parseList(QXmlStreamReader &xml);
    void parseMetaConfig(QXmlStreamReader &xml, OBSMetaConfig *metaConfig);
    QHash<QString, bool> parseRepositoryFlags(QXmlStreamReader &xml);
    OBSDistribution *parseDistribution(QXmlStreamReader &xml);
    QString requestNumber;

signals:
    void finishedParsingPackage(QSharedPointer<OBSStatus> status, int row);
    void finishedParsingBranchPackage(QSharedPointer<OBSStatus> status);
    void finishedParsingLinkPkgRevision(OBSRevision *revision);
    void finishedParsingCopyPkgRevision(OBSRevision *revision);
    void finishedParsingCreateRequest(OBSRequest*);
    void finishedParsingCreateRequestStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingCreatePrjStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingCreatePkgStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingUploadFileRevision(OBSRevision*);
    void finishedParsingDeletePrjStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingDeletePkgStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingDeleteFileStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingResult(OBSResult*);
    void finishedParsingResultList(const QList<OBSResult *> &resultList);
    void finishedParsingRevision(OBSRevision*);
    void finishedParsingLatestRevision(OBSRevision *revision);
    void finishedParsingIncomingRequest(OBSRequest *request);
    void finishedParsingIncomingRequestList();
    void finishedParsingOutgoingRequest(OBSRequest *request);
    void finishedParsingOutgoingRequestList();
    void finishedParsingDeclinedRequest(OBSRequest *request);
    void finishedParsingDeclinedRequestList();
    void finishedParsingList(QStringList);
    void finishedParsingProjectList(QStringList);
    void projectFetched(const QString &project);
    void finishedParsingProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig);
    void finishedParsingPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig);
    void finishedParsingPackageList(QStringList);
    void finishedParsingFile(OBSFile*);
    void finishedParsingFileList(const QString &project, const QString &package);
    void finishedParsingRevisionList(const QString &project, const QString &package);
    void finishedParsingLink(OBSLink *obsLink);
    void finishedParsingRequestStatus(QSharedPointer<OBSStatus> status);
    void finishedParsingRequest(OBSRequest *obsRequest);
    void finishedParsingRequestList(const QString &project, const QString &package);
    void finishedParsingAbout(OBSAbout *obsAbout);
    void finishedParsingPerson(OBSPerson *obsPerson);
    void finishedParsingUpdatePerson(QSharedPointer<OBSStatus> status);
    void finishedParsingDistribution(OBSDistribution *distribution);
};

#endif // OBSXMLREADER_H

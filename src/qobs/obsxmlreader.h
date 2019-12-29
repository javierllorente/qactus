/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2013-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSXMLREADER_H
#define OBSXMLREADER_H

#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
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
    OBSStatus *parseNotFoundStatus(const QString &data);
    void parsePackageList(const QString &data);
    void parseFileList(const QString &project, const QString &package, const QString &data);
    void parseLink(const QString &data);
    void parseResultList(const QString &data);
    void parseIncomingRequests(const QString &data);
    void parseOutgoingRequests(const QString &data);
    void parseDeclinedRequests(const QString &data);
    void parseSubmitRequest(const QString &data);
    void parseBranchPackage(const QString &project, const QString &package, const QString &data);
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
    void parseStatus(QXmlStreamReader &xml, OBSStatus *obsStatus);
    int row;
    void parseRevision(QXmlStreamReader &xml, OBSRevision *obsRevision);
    void parseRevisionList(const QString &data);
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
    void finishedParsingPackage(OBSStatus*, int);
    void finishedParsingBranchPackage(OBSStatus*);
    void finishedParsingLinkPkgRevision(OBSRevision *revision);
    void finishedParsingCopyPkgRevision(OBSRevision *revision);
    void finishedParsingCreateRequest(OBSRequest*);
    void finishedParsingCreateRequestStatus(OBSStatus*);
    void finishedParsingCreatePrjStatus(OBSStatus*);
    void finishedParsingCreatePkgStatus(OBSStatus*);
    void finishedParsingUploadFileRevision(OBSRevision*);
    void finishedParsingDeletePrjStatus(OBSStatus*);
    void finishedParsingDeletePkgStatus(OBSStatus*);
    void finishedParsingDeleteFileStatus(OBSStatus*);
    void finishedParsingResult(OBSResult*);
    void finishedParsingResultList();
    void finishedParsingRevision(OBSRevision*);
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
    void finishedParsingFileList();
    void finishedParsingLink(OBSLink *obsLink);
    void finishedParsingSR(OBSStatus *obsStatus);
    void finishedParsingAbout(OBSAbout *obsAbout);
    void finishedParsingPerson(OBSPerson *obsPerson);
    void finishedParsingUpdatePerson(OBSStatus *obsStatus);
    void finishedParsingDistribution(OBSDistribution *distribution);
};

#endif // OBSXMLREADER_H

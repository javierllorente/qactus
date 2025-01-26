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
#include "obs.h"

OBS::OBS(QObject *parent) : QObject(parent)
{
    obsCore = OBSCore::getInstance();
    xmlReader = OBSXmlReader::getInstance();
    includeHomeProjects = false;

    // Forward signals
    connect(obsCore, SIGNAL(apiNotFound(QUrl)), this, SIGNAL(apiNotFound(QUrl)));
    connect(obsCore, SIGNAL(isAuthenticated(bool)),
            this, SIGNAL(isAuthenticated(bool)));
    connect(obsCore, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SIGNAL(selfSignedCertificate(QNetworkReply*)));
    connect(obsCore, SIGNAL(networkError(QString)),
            this, SIGNAL(networkError(QString)));
    connect(xmlReader, SIGNAL(finishedParsingPackage(OBSStatus*,int)),
            this, SIGNAL(finishedParsingPackage(OBSStatus*,int)));

    connect(xmlReader, SIGNAL(finishedParsingBranchPackage(OBSStatus*)),
            this, SIGNAL(finishedParsingBranchPackage(OBSStatus*)));
    connect(xmlReader, &OBSXmlReader::finishedParsingLinkPkgRevision,
            this, &OBS::finishedParsingLinkPkgRevision);
    connect(xmlReader, &OBSXmlReader::finishedParsingCopyPkgRevision,
            this, &OBS::finishedParsingCopyPkgRevision);
    connect(xmlReader, SIGNAL(finishedParsingCreateRequest(OBSRequest*)),
            this, SIGNAL(finishedParsingCreateRequest(OBSRequest*)));
    connect(xmlReader, SIGNAL(finishedParsingCreateRequestStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreateRequestStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingCreatePrjStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreatePrjStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingCreatePkgStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreatePkgStatus(OBSStatus*)));
    connect(obsCore, &OBSCore::cannotLinkPackage, this, &OBS::cannotLinkPackage);
    connect(obsCore, &OBSCore::cannotCopyPackage, this, &OBS::cannotCopyPackage);
    connect(obsCore, SIGNAL(cannotCreateProject(OBSStatus*)),
            this, SIGNAL(cannotCreateProject(OBSStatus*)));
    connect(obsCore, SIGNAL(cannotCreatePackage(OBSStatus*)),
            this, SIGNAL(cannotCreatePackage(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingUploadFileRevision(OBSRevision*)),
            this, SIGNAL(finishedParsingUploadFileRevision(OBSRevision*)));
    connect(obsCore, SIGNAL(cannotUploadFile(OBSStatus*)),
            this, SIGNAL(cannotUploadFile(OBSStatus*)));
    connect(obsCore, SIGNAL(fileFetched(QString,QByteArray)), this, SIGNAL(fileFetched(QString,QByteArray)));
    connect(obsCore, SIGNAL(buildLogFetched(QString)), this, SIGNAL(buildLogFetched(QString)));
    connect(obsCore, SIGNAL(buildLogNotFound()), this, SIGNAL(buildLogNotFound()));
    connect(obsCore, &OBSCore::projectNotFound, this, &OBS::projectNotFound);
    connect(obsCore, &OBSCore::packageNotFound, this, &OBS::packageNotFound);
    connect(xmlReader, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingDeleteFileStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingDeleteFileStatus(OBSStatus*)));
    connect(obsCore, SIGNAL(cannotDeleteProject(OBSStatus*)),
            this, SIGNAL(cannotDeleteProject(OBSStatus*)));
    connect(obsCore, SIGNAL(cannotDeletePackage(OBSStatus*)),
            this, SIGNAL(cannotDeletePackage(OBSStatus*)));
    connect(obsCore, SIGNAL(cannotDeleteFile(OBSStatus*)),
            this, SIGNAL(cannotDeleteFile(OBSStatus*)));

    connect(xmlReader, SIGNAL(finishedParsingResult(OBSResult*)),
            this, SIGNAL(finishedParsingResult(OBSResult*)));
    connect(xmlReader, &OBSXmlReader::finishedParsingResultList, this, &OBS::finishedParsingResultList);
    connect(xmlReader, &OBSXmlReader::finishedParsingRevision,
            this, &OBS::finishedParsingRevision);
    connect(xmlReader, &OBSXmlReader::finishedParsingLatestRevision,
            this, &OBS::finishedParsingLatestRevision);

    connect(xmlReader, &OBSXmlReader::finishedParsingRequest,
            this, &OBS::finishedParsingRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingRequestList,
            this, &OBS::finishedParsingRequestList);

    connect(xmlReader, &OBSXmlReader::finishedParsingIncomingRequest, this, &OBS::finishedParsingIncomingRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingIncomingRequestList, this, &OBS::finishedParsingIncomingRequestList);
    connect(xmlReader, &OBSXmlReader::finishedParsingOutgoingRequest, this, &OBS::finishedParsingOutgoingRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingOutgoingRequestList, this, &OBS::finishedParsingOutgoingRequestList);
    connect(xmlReader, &OBSXmlReader::finishedParsingDeclinedRequest, this, &OBS::finishedParsingDeclinedRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingDeclinedRequestList, this, &OBS::finishedParsingDeclinedRequestList);

    connect(xmlReader, &OBSXmlReader::projectFetched, this, &OBS::projectFetched);

    connect(xmlReader, SIGNAL(finishedParsingProjectList(QStringList)),
            this, SIGNAL(finishedParsingProjectList(QStringList)));
    connect(xmlReader, &OBSXmlReader::finishedParsingProjectMetaConfig, this, &OBS::finishedParsingProjectMetaConfig);
    connect(xmlReader, &OBSXmlReader::finishedParsingPackageMetaConfig, this, &OBS::finishedParsingPackageMetaConfig);
    connect(xmlReader, SIGNAL(finishedParsingPackageList(QStringList)),
            this, SIGNAL(finishedParsingPackageList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingList(QStringList)),
            this, SIGNAL(finishedParsingList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingFile(OBSFile*)),
            this, SIGNAL(finishedParsingFile(OBSFile*)));
    connect(xmlReader, &OBSXmlReader::finishedParsingFileList,
            this, &OBS::finishedParsingFileList);
    connect(xmlReader, &OBSXmlReader::finishedParsingRevisionList,
            this, &OBS::finishedParsingRevisionList);
    connect(xmlReader, &OBSXmlReader::finishedParsingLink, this, &OBS::finishedParsingLink);
    connect(xmlReader, &OBSXmlReader::finishedParsingRequestStatus, this, &OBS::finishedParsingRequestStatus);
    connect(obsCore, SIGNAL(srDiffFetched(QString)),
            this, SIGNAL(srDiffFetched(QString)));
    connect(xmlReader, SIGNAL(finishedParsingAbout(OBSAbout*)),
            SIGNAL(finishedParsingAbout(OBSAbout*)));
    connect(xmlReader, &OBSXmlReader::finishedParsingPerson, this, &OBS::finishedParsingPerson);
    connect(xmlReader, &OBSXmlReader::finishedParsingUpdatePerson, this, &OBS::finishedParsingUpdatePerson);
    connect(xmlReader, &OBSXmlReader::finishedParsingDistribution, this, &OBS::finishedParsingDistribution);
}

void OBS::setCredentials(const QString &username, const QString &password)
{
    obsCore->setCredentials(username, password);
}

QString OBS::getUsername()
{
    return obsCore->getUsername();
}

void OBS::setApiUrl(const QString &apiUrl)
{
    obsCore->setApiUrl(apiUrl);
}

QString OBS::getApiUrl() const
{
    return obsCore->getApiUrl();
}

void OBS::request(const QString &resource)
{
    obsCore->request(resource);
}

void OBS::request(QNetworkReply *reply)
{
    obsCore->request(reply);
}

void OBS::postRequest(const QString &resource, const QByteArray &data, const QString &contentTypeHeader)
{
    obsCore->postRequest(resource, data, contentTypeHeader);
}

void OBS::deleteRequest(const QString &resource)
{
    obsCore->deleteRequest(resource);
}

bool OBS::isAuthenticated()
{
    return obsCore->isAuthenticated();
}

void OBS::login()
{
    obsCore->login();
}

void OBS::getBuildStatus(const QStringList &stringList, int row)
{
    //    URL format: https://api.opensuse.org/build/<project>/<repository>/<arch>/<package>/_status
    obsCore->getBuildStatus(stringList, row);
}

void OBS::getAllBuildStatus(const QString &project, const QString &package)
{
    //    URL format: https://api.opensuse.org/build/<project>/_result?package=<package>
    QString resource = QString("%1/%2%3").arg(project, "_result?package=", package);
    obsCore->getAllBuildStatus(resource);
}

void OBS::getProjectResults(const QString &project)
{
    //    URL format: https://api.opensuse.org/build/<project>/_result
    QString resource = QString("%1/%2").arg(project, "_result");
    obsCore->getAllBuildStatus(resource);
}

void OBS::getLatestRevision(const QString &project, const QString &package)
{
    obsCore->getLatestRevision(project, package);
}

void OBS::getIncomingRequests()
{
    obsCore->getIncomingRequests();
}

void OBS::getOutgoingRequests()
{
    obsCore->getOutgoingRequests();
}

void OBS::getDeclinedRequests()
{
    obsCore->getDeclinedRequests();
}

int OBS::getRequestCount()
{
    return xmlReader->getRequestNumber();
}

void OBS::getProjectRequests(const QString &project)
{
    obsCore->getProjectRequests(project);
}

void OBS::getPackageRequests(const QString &project, const QString &package)
{
    obsCore->getPackageRequests(project, package);
}

void OBS::slotChangeSubmitRequest(const QString &id, const QString &comments, bool accepted)
{
    qDebug() << Q_FUNC_INFO << "id:" << id << " comments:" << comments << " accept:" << accepted;
    QString newState = accepted ? "accepted" : "declined";
    QString resource = QString("/request/%1?cmd=changestate&newstate=%2").arg(id, newState);
    QByteArray data;
    data.append(comments.toUtf8());
    changeSubmitRequest(resource, data);
}

void OBS::createRequest(const QByteArray &data)
{
    obsCore->createRequest(data);
}

void OBS::getRequestDiff(const QString &source)
{
    qDebug() << Q_FUNC_INFO;
    QString resource = QString("/request/%1?cmd=diff").arg(source);
    obsCore->getSRDiff(resource);
}

bool OBS::isIncludeHomeProjects() const
{
    return includeHomeProjects;
}

void OBS::setIncludeHomeProjects(bool value)
{
    includeHomeProjects = value;
}

void OBS::getProjects()
{
    obsCore->setIncludeHomeProjects(includeHomeProjects);
    obsCore->getProjects();
}

void OBS::getPackages(const QString &project)
{
    obsCore->getPackages(project);
}

void OBS::getProjectMetaConfig(const QString &project)
{
    obsCore->getProjectMetaConfig(project + "/_meta");
}

void OBS::getPackageMetaConfig(const QString &project, const QString &package)
{
    obsCore->getPackageMetaConfig("/" + project + "/" + package + "/_meta");
}

void OBS::getFiles(const QString &project, const QString &package)
{
    obsCore->getFiles(project, package);
}

void OBS::getRevisions(const QString &project, const QString &package)
{
    obsCore->getRevisions(project, package);
}

void OBS::getLink(const QString &project, const QString &package)
{
    obsCore->getLink(project, package);
}

void OBS::changeSubmitRequest(const QString &resource, const QByteArray &data)
{
    obsCore->changeSubmitRequest(resource, data);
}

OBSXmlReader* OBS::getXmlReader()
{
    return xmlReader;
}

void OBS::branchPackage(const QString &project, const QString &package)
{
    obsCore->branchPackage(project, package);
}

void OBS::linkPackage(const QString &srcProject, const QString &srcPackage, const QString &dstProject)
{
    obsCore->linkPackage(srcProject, srcPackage, dstProject);
}

void OBS::copyPackage(const QString &originProject, const QString &originPackage,
                      const QString &destProject, const QString &destPackage, const QString &comments)
{
    obsCore->copyPackage(originProject, originPackage, destProject, destPackage, comments);
}

void OBS::createProject(const QString &project, const QByteArray &data)
{
    obsCore->createProject(project, data);
}

void OBS::createPackage(const QString &project, const QString &package, const QByteArray &data)
{
    obsCore->createPackage(project, package, data);
}

void OBS::uploadFile(const QString &project, const QString &package, const QString &fileName, const QByteArray &data)
{
    obsCore->uploadFile(project, package, fileName, data);
}

void OBS::downloadFile(const QString &project, const QString &package, const QString &fileName)
{
    obsCore->downloadFile(project, package, fileName);
}

void OBS::getBuildLog(const QString &project, const QString &repository, const QString &arch, const QString &package)
{
    obsCore->getBuildLog(project, repository, arch, package);
}

void OBS::deleteProject(const QString &project)
{
    obsCore->deleteProject(project);
}

void OBS::deletePackage(const QString &project, const QString &package)
{
    obsCore->deletePackage(project, package);
}

void OBS::deleteFile(const QString &project, const QString &package, const QString &fileName)
{
    obsCore->deleteFile(project, package, fileName);
}

void OBS::about()
{
    obsCore->about();
}

void OBS::getPerson()
{
    obsCore->getPerson();
}

void OBS::updatePerson(const QByteArray &data)
{
    obsCore->updatePerson(data);
}

void OBS::getDistributions()
{
    obsCore->getDistributions();
}

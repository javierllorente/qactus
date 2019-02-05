/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2015-2019 Javier Llorente <javier@opensuse.org>
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
    connect(xmlReader, SIGNAL(finishedParsingCreateRequest(OBSRequest*)),
            this, SIGNAL(finishedParsingCreateRequest(OBSRequest*)));
    connect(xmlReader, SIGNAL(finishedParsingCreateRequestStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreateRequestStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingCreatePrjStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreatePrjStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingCreatePkgStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingCreatePkgStatus(OBSStatus*)));
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
    connect(xmlReader, SIGNAL(finishedParsingResultList()),
            this, SIGNAL(finishedParsingResultList()));
    connect(xmlReader, SIGNAL(finishedParsingRevision(OBSRevision*)),
            this, SIGNAL(finishedParsingRevision(OBSRevision*)));

    connect(xmlReader, &OBSXmlReader::finishedParsingIncomingRequest, this, &OBS::finishedParsingIncomingRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingIncomingRequestList, this, &OBS::finishedParsingIncomingRequestList);
    connect(xmlReader, &OBSXmlReader::finishedParsingOutgoingRequest, this, &OBS::finishedParsingOutgoingRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingOutgoingRequestList, this, &OBS::finishedParsingOutgoingRequestList);
    connect(xmlReader, &OBSXmlReader::finishedParsingDeclinedRequest, this, &OBS::finishedParsingDeclinedRequest);
    connect(xmlReader, &OBSXmlReader::finishedParsingDeclinedRequestList, this, &OBS::finishedParsingDeclinedRequestList);

    connect(xmlReader, SIGNAL(finishedParsingProjectList(QStringList)),
            this, SIGNAL(finishedParsingProjectList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingProjectMetadata(QStringList)),
            this, SIGNAL(finishedParsingProjectMetadata(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingPackageList(QStringList)),
            this, SIGNAL(finishedParsingPackageList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingList(QStringList)),
            this, SIGNAL(finishedParsingList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingFile(OBSFile*)),
            this, SIGNAL(finishedParsingFile(OBSFile*)));
    connect(xmlReader, SIGNAL(finishedParsingFileList()),
            this, SIGNAL(finishedParsingFileList()));
    connect(xmlReader, &OBSXmlReader::finishedParsingLink, this, &OBS::finishedParsingLink);
    connect(xmlReader, SIGNAL(finishedParsingSR(OBSStatus *)),
            this, SLOT(srChangeResult(OBSStatus *)));
    connect(obsCore, SIGNAL(srDiffFetched(QString)),
            this, SIGNAL(srDiffFetched(QString)));
    connect(xmlReader, SIGNAL(finishedParsingAbout(OBSAbout*)),
            SIGNAL(finishedParsingAbout(OBSAbout*)));
    connect(xmlReader, &OBSXmlReader::finishedParsingPerson, this, &OBS::finishedParsingPerson);
    connect(xmlReader, &OBSXmlReader::finishedParsingUpdatePerson, this, &OBS::finishedParsingUpdatePerson);
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

void OBS::getRevisions(const QString &project, const QString &package)
{
    //    URL format: https://api.opensuse.org/source/<project>/<package>/_history
    QString resource = QString("%1/%2/_history").arg(project, package);
    obsCore->requestSource(resource);
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

void OBS::slotChangeSubmitRequest(const QString &id, const QString &comments, bool accepted)
{
    qDebug() << "OBS::changeSubmitRequest() id:" << id << " comments:" << comments << " accept:" << accepted;
    QString newState = accepted ? "accepted" : "declined";
    QString resource = QString("/request/%1?cmd=changestate&newstate=%2").arg(id, newState);
    QByteArray data;
    data.append(comments);
    changeSubmitRequest(resource, data);
}

void OBS::srChangeResult(OBSStatus *obsStatus)
{
    QString code = obsStatus->getCode();
    delete obsStatus;
    obsStatus = nullptr;
    emit srStatus(code);
}

void OBS::createRequest(const QByteArray &data)
{
    obsCore->createRequest(data);
}

void OBS::getRequestDiff(const QString &source)
{
    qDebug() << "OBS::getRequestDiff()";
    QString resource = QString("/source/%1?unified=1&tarlimit=0&cmd=diff&filelimit=0&expand=1")
            .arg(source);
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
    obsCore->getPackages("/" + project);
}

void OBS::getProjectMetadata(const QString &project)
{
    obsCore->getProjectMetadata("/" + project + "/_meta");
}

void OBS::getFiles(const QString &project, const QString &package)
{
    obsCore->getFiles(project, package);
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

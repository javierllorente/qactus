/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2018 Javier Llorente <javier@opensuse.org>
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

    // Forward signals
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
    connect(xmlReader, SIGNAL(finishedParsingRequest(OBSRequest*)),
            this, SIGNAL(finishedParsingRequest(OBSRequest*)));
    connect(xmlReader, SIGNAL(removeRequest(const QString&)),
            this, SIGNAL(removeRequest(const QString&)));
    connect(xmlReader, SIGNAL(projectListIsReady()),
            this, SIGNAL(projectListIsReady()));
    connect(xmlReader, SIGNAL(projectMetadataIsReady()),
            this, SIGNAL(projectMetadataIsReady()));
    connect(xmlReader, SIGNAL(packageListIsReady()),
            this, SIGNAL(packageListIsReady()));
    connect(xmlReader, SIGNAL(finishedParsingList(QStringList)),
            this, SIGNAL(finishedParsingList(QStringList)));
    connect(xmlReader, SIGNAL(finishedParsingFile(OBSFile*)),
            this, SIGNAL(finishedParsingFile(OBSFile*)));
    connect(xmlReader, SIGNAL(finishedParsingFileList()),
            this, SIGNAL(finishedParsingFileList()));
    connect(xmlReader, SIGNAL(finishedParsingSR(OBSStatus *)),
            this, SLOT(srChangeResult(OBSStatus *)));
    connect(obsCore, SIGNAL(srDiffFetched(QString)),
            this, SIGNAL(srDiffFetched(QString)));
    connect(xmlReader, SIGNAL(finishedParsingAbout(OBSAbout*)),
            SIGNAL(finishedParsingAbout(OBSAbout*)));
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

void OBS::request(const QString &resource, int row)
{
    obsCore->request(resource, row);
}

void OBS::postRequest(const QString &resource, const QByteArray &data)
{
    obsCore->postRequest(resource, data);
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
    request("/");
}

void OBS::getBuildStatus(const QStringList &stringList, int row)
{
    //    URL format: https://api.opensuse.org/build/<project>/<repository>/<arch>/<package>/_status
    QString resource = QString("%1/%2/%3/%4/_status").arg(stringList[0], stringList[1], stringList[2], stringList[3]);
    obsCore->requestBuild(resource, row);
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

void OBS::getRequests()
{
    QString resource = QString("?view=collection&states=new&roles=maintainer&user=%1").arg(getUsername());
    obsCore->requestRequest(resource);
}

int OBS::getRequestCount()
{
    return xmlReader->getRequestNumber();
}

void OBS::changeSubmitRequestSlot(const QString &id, const QString &comments, bool accepted)
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

void OBS::getRequestDiff(const QString &source)
{
    qDebug() << "OBS::getRequestDiff()";
    QString resource = QString("/source/%1?unified=1&tarlimit=0&cmd=diff&filelimit=0&expand=1")
            .arg(source);
    obsCore->getSRDiff(resource);
}

void OBS::getProjects()
{
    xmlReader->setFileName("projects.xml");
    obsCore->getProjects();
}

void OBS::getPackages(const QString &project)
{
    xmlReader->setFileName(project + ".xml");
    obsCore->getPackages("/" + project);
}

void OBS::getProjectMetadata(const QString &project)
{
    xmlReader->setFileName(project + "_meta.xml");
    obsCore->getProjectMetadata("/" + project + "/_meta");
}

void OBS::getFiles(const QString &project, const QString &package)
{
    QString fileName = QString("%1_%2.xml").arg(project, package);
    xmlReader->setFileName(fileName);
    obsCore->getFiles(project, package);
}

QStringList OBS::getRepositoryArchs(const QString &repository)
{
    xmlReader->getRepositoryArchs(repository);
    return xmlReader->getList();
}

void OBS::changeSubmitRequest(const QString &resource, const QByteArray &data)
{
    obsCore->changeSubmitRequest(resource, data);
}

QStringList OBS::readXmlFile(const QString &xmlFile)
{
    xmlReader->setFileName(xmlFile);
    xmlReader->readList();
    return xmlReader->getList();
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

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
    obsAccess = OBSAccess::getInstance();
    xmlReader = OBSXmlReader::getInstance();

    // Forward signals
    connect(obsAccess, SIGNAL(isAuthenticated(bool)),
            this, SIGNAL(isAuthenticated(bool)));
    connect(obsAccess, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SIGNAL(selfSignedCertificate(QNetworkReply*)));
    connect(obsAccess, SIGNAL(networkError(QString)),
            this, SIGNAL(networkError(QString)));
    connect(xmlReader, SIGNAL(finishedParsingPackage(OBSPackage*, const int)),
            this, SIGNAL(finishedParsingPackage(OBSPackage*,int)));

    connect(xmlReader, SIGNAL(finishedParsingStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)));
    connect(xmlReader, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)),
            this, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)));
    connect(obsAccess, SIGNAL(cannotDeleteProject(OBSStatus*)),
            this, SIGNAL(cannotDeleteProject(OBSStatus*)));
    connect(obsAccess, SIGNAL(cannotDeletePackage(OBSStatus*)),
            this, SIGNAL(cannotDeletePackage(OBSStatus*)));

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
    connect(xmlReader, SIGNAL(finishedParsingSR(OBSPackage *)),
            this, SLOT(srChangeResult(OBSPackage *)));
    connect(obsAccess, SIGNAL(srDiffFetched(QString)),
            this, SIGNAL(srDiffFetched(QString)));
}

void OBS::setCredentials(const QString &username, const QString &password)
{
    obsAccess->setCredentials(username, password);
}

QString OBS::getUsername()
{
    return obsAccess->getUsername();
}

void OBS::setApiUrl(const QString &apiUrl)
{
    obsAccess->setApiUrl(apiUrl);
}

QString OBS::getApiUrl() const
{
    return obsAccess->getApiUrl();
}

void OBS::request(const QString &resource)
{
    obsAccess->request(resource);
}

void OBS::request(const QString &resource, int row)
{
    obsAccess->request(resource, row);
}

void OBS::postRequest(const QString &resource, const QByteArray &data)
{
    obsAccess->postRequest(resource, data);
}

void OBS::deleteRequest(const QString &resource)
{
    obsAccess->deleteRequest(resource);
}

bool OBS::isAuthenticated()
{
    return obsAccess->isAuthenticated();
}

void OBS::login()
{
    request("/");
}

void OBS::getBuildStatus(const QStringList &stringList, const int &row)
{
    //    URL format: https://api.opensuse.org/build/<project>/<repository>/<arch>/<package>/_status
    QString resource = QString("%1/%2/%3/%4/_status").arg(stringList[0], stringList[1], stringList[2], stringList[3]);
    obsAccess->requestBuild(resource, row);
}

void OBS::getAllBuildStatus(const QString &project, const QString &package)
{
    //    URL format: https://api.opensuse.org/build/<project>/_result?package=<package>
    QString resource = QString("%1/%2%3").arg(project, "_result?package=", package);
    obsAccess->getAllBuildStatus(resource);
}

void OBS::getRevisions(const QString &project, const QString &package)
{
    //    URL format: https://api.opensuse.org/source/<project>/<package>/_history
    QString resource = QString("%1/%2/_history").arg(project, package);
    obsAccess->requestSource(resource);
}

void OBS::getRequests()
{
    QString resource = QString("?view=collection&states=new&roles=maintainer&user=%1").arg(getUsername());
    obsAccess->requestRequest(resource);
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

void OBS::srChangeResult(OBSPackage *obsPackage)
{
    QString status = obsPackage->getStatus();
    delete obsPackage;
    emit srStatus(status);
}

void OBS::getRequestDiff(const QString &source)
{
    qDebug() << "OBS::getRequestDiff()";
    QString resource = QString("/source/%1?unified=1&tarlimit=0&cmd=diff&filelimit=0&expand=1")
            .arg(source);
    obsAccess->getSRDiff(resource);
}

void OBS::getProjects()
{
    xmlReader->setFileName("projects.xml");
    obsAccess->getProjects();
}

void OBS::getPackages(const QString &project)
{
    xmlReader->setFileName(project + ".xml");
    obsAccess->getPackages("/" + project);
}

void OBS::getProjectMetadata(const QString &project)
{
    xmlReader->setFileName(project + "_meta.xml");
    obsAccess->getProjectMetadata("/" + project + "/_meta");
}

void OBS::getFiles(const QString &project, const QString &package)
{
    QString fileName = QString("%1_%2.xml").arg(project, package);
    xmlReader->setFileName(fileName);
    QString resource = QString("/%1/%2").arg(project, package);
    obsAccess->getFiles(resource);
}

QStringList OBS::getRepositoryArchs(const QString &repository)
{
    xmlReader->getRepositoryArchs(repository);
    return xmlReader->getList();
}

void OBS::changeSubmitRequest(const QString &resource, const QByteArray &data)
{
    obsAccess->changeSubmitRequest(resource, data);
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
    QString resource = QString("/source/%1/%2?cmd=branch").arg(project, package);
    obsAccess->branchPackage(resource);
}

void OBS::deleteProject(const QString &project)
{
    obsAccess->deleteProject(project);
}

void OBS::deletePackage(const QString &project, const QString &package)
{
    obsAccess->deletePackage(project, package);
}

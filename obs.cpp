/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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
    connect(xmlReader, SIGNAL(finishedParsingRequests(QList<OBSRequest*>)),
            this, SIGNAL(finishedParsingRequests(QList<OBSRequest*>)));
    connect(xmlReader, SIGNAL(finishedParsingList(QStringList)),
            this, SIGNAL(finishedParsingList(QStringList)));
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
    this->apiUrl = apiUrl;
}

QString OBS::getApiUrl() const
{
    return apiUrl;
}

void OBS::request(const QString &urlStr)
{
    obsAccess->request(urlStr);
}

void OBS::request(const QString &urlStr, const int &row)
{
    obsAccess->request(urlStr, row);
}

void OBS::postRequest(const QString &urlStr, const QByteArray &data)
{
    obsAccess->postRequest(urlStr, data);
}

bool OBS::isAuthenticated()
{
    return obsAccess->isAuthenticated();
}

void OBS::login()
{
    request(apiUrl + "/");
}

void OBS::getBuildStatus(const QStringList &stringList, const int &row)
{
    //    URL format: https://api.opensuse.org/build/KDE:Extra/openSUSE_13.2/x86_64/qrae/_status
    request(apiUrl + "/build/"
            + stringList[0] + "/"
            + stringList[1] + "/"
            + stringList[2] + "/"
            + stringList[3] + "/_status", row);
}

void OBS::getRequests()
{
    request(apiUrl + "/request?view=collection&states=new&roles=maintainer&user=" + getUsername());
}

int OBS::getRequestCount()
{
    return xmlReader->getRequestNumber();
}

QString OBS::acceptRequest(const QString &id, const QString &comments)
{
    QByteArray data;
    data.append(comments);
    postRequest(apiUrl + "/request/" + id + "?cmd=changestate&newstate=accepted", data);
    return xmlReader->getPackage()->getStatus();
}

QString OBS::declineRequest(const QString &id, const QString &comments)
{
    QByteArray data;
    data.append(comments);
    postRequest(apiUrl + "/request/" + id + "?cmd=changestate&newstate=declined", data);
    return xmlReader->getPackage()->getStatus();
}

QString OBS::getRequestDiff(const QString &source)
{
    postRequest(apiUrl + "/source/" + source +
                "?unified=1&tarlimit=0&cmd=diff&filelimit=0&expand=1", "");
    return obsAccess->getRequestDiff();
}

QStringList OBS::getProjectList()
{
    xmlReader->setFileName("projects.xml");
    request(apiUrl + "/source");
    xmlReader->readFile();
    return xmlReader->getList();
}

QStringList OBS::getProjectPackageList(const QString &projectName)
{
    xmlReader->setFileName(projectName + ".xml");
    request(apiUrl + "/source/" + projectName);
    xmlReader->readFile();
    return xmlReader->getList();
}

QStringList OBS::getProjectMetadata(const QString &projectName)
{
    xmlReader->setFileName(projectName + "_meta.xml");
    request(apiUrl + "/source/" + projectName + "/_meta");
    xmlReader->readFile();
    return xmlReader->getList();
}

QStringList OBS::getRepositoryArchs(const QString &repository)
{
    xmlReader->getRepositoryArchs(repository);
    return xmlReader->getList();
}

QStringList OBS::readXmlFile(const QString &xmlFile)
{
    xmlReader->setFileName(xmlFile);
    xmlReader->readFile();
    return xmlReader->getList();
}

OBSXmlReader* OBS::getXmlReader()
{
    return xmlReader;
}

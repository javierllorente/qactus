#include "obs.h"

OBS::OBS(QObject *parent) : QObject(parent)
{
    obsAccess = OBSAccess::getInstance();
    xmlReader = OBSXmlReader::getInstance();

    // Forward signals
    connect(obsAccess, SIGNAL(isAuthenticated(bool)),
            this, SIGNAL(isAuthenticated(bool)));
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

OBSPackage* OBS::getBuildStatus(const QStringList &stringList, const int &row)
{
    //    URL format: https://api.opensuse.org/build/KDE:Extra/openSUSE_13.2/x86_64/qrae/_status
    request(apiUrl + "/build/"
            + stringList[0] + "/"
            + stringList[1] + "/"
            + stringList[2] + "/"
            + stringList[3] + "/_status", row);
    return xmlReader->getPackage();
}

QList<OBSRequest*> OBS::getRequests()
{
    request(apiUrl + "/request?view=collection&states=new&roles=maintainer&user=" + getUsername());
    return xmlReader->getRequests();
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
    request(apiUrl + "/source");
    return xmlReader->getList();
}

QStringList OBS::getProjectPackageList(const QString &projectName)
{
    request(apiUrl + "/source/" + projectName);
    return xmlReader->getList();
}


QStringList OBS::getProjectMetadata(const QString &projectName)
{
    request(apiUrl + "/source/" + projectName + "/_meta");
    return xmlReader->getList();
}

OBSXmlReader* OBS::getXmlReader()
{
    return xmlReader;
}

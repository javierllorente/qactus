/*
 * Copyright (C) 2013-2024 Javier Llorente <javier@opensuse.org>
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
#include "obscore.h"
#include "obsstatus.h"

OBSCore *OBSCore::instance = nullptr;
const QString userAgent = APP_NAME + QString(" ") + QACTUS_VERSION;

OBSCore::OBSCore()
{
    authenticated = false;
    xmlReader = OBSXmlReader::getInstance();
    manager = nullptr;
    includeHomeProjects = false;
    linkHelper = nullptr;
}

void OBSCore::createManager()
{
    qDebug() << Q_FUNC_INFO;
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::authenticationRequired,
            this, &OBSCore::provideAuthentication);
    connect(manager, &QNetworkAccessManager::finished, this, &OBSCore::replyFinished);
    connect(manager, &QNetworkAccessManager::sslErrors, this, &OBSCore::onSslErrors);
}

OBSCore *OBSCore::getInstance()
{
    if (!instance) {
        instance = new OBSCore();
    }
    return instance;
}

void OBSCore::setCredentials(const QString& username, const QString& password)
{
    qDebug() << "OBSCore::setCredentials()";
//    Allow login with another username/password
    if (manager) {
        delete manager;
        manager = nullptr;
    }
    createManager();

    this->username = username;
    this->password = password;
}

void OBSCore::slotLinkPackage(const QString &dstProject, const QString &dstPackage, const QByteArray &data)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString resource = QString("/source/%1/%2/_link").arg(dstProject, dstPackage);

    QNetworkReply *reply = putRequest(resource, data);
    reply->setProperty("reqtype", OBSCore::LinkPackage);
    reply->setProperty("destprj", dstProject);
    reply->setProperty("destpkg", dstPackage);

    if (linkHelper) {
        delete linkHelper;
        linkHelper = nullptr;
    }
}

QString OBSCore::getUsername()
{
    return username;
}

void OBSCore::setApiUrl(const QString &apiUrl)
{
    this->apiUrl = apiUrl;
}

QString OBSCore::getApiUrl() const
{
    return apiUrl;
}

void OBSCore::login()
{
    QNetworkReply *reply = request("/");
    reply->setProperty("reqtype", OBSCore::Login);
}

QNetworkReply *OBSCore::request(const QString &resource)
{
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
    request.setUrl(QUrl(apiUrl + resource));
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());  
    QNetworkReply *reply = manager->get(request);
    return reply;
}

void OBSCore::request(QNetworkReply *reply)
{
    QString resource = reply->url().toString();
    QNetworkReply *newReply = request(resource);
    newReply->setProperty("reqtype", reply->property("reqtype"));

    if (reply->property("row").isValid()) {
        newReply->setProperty("row", reply->property("row").toInt());
    }
}

QNetworkReply *OBSCore::requestBuild(const QString &resource)
{
    return request("/build/" + resource);
}

void OBSCore::getBuildStatus(const QStringList &build, int row)
{
    QString resource = QString("%1/%2/%3/%4/_status").arg(build[0], build[1], build[2], build[3]);
    QNetworkReply *reply = requestBuild(resource);
    reply->setProperty("reqtype", OBSCore::BuildStatus);
    reply->setProperty("row", row);
}

QNetworkReply *OBSCore::requestSource(const QString &resource)
{
    return request("/source/" + resource);
}

QNetworkReply *OBSCore::requestRequest(const QString &resource)
{
    return request("/request/" + resource);
}

QString OBSCore::createReqResourceStr(const QString &states, const QString &roles) const
{
    return  QString("/request/?view=collection&states=%1&roles=%2&user=%3")
            .arg(states).arg(roles).arg(username);
}

void OBSCore::getRequests(OBSCore::RequestType type)
{
    QString resource;
    QNetworkReply *reply = nullptr;

    switch (type) {
    case OBSCore::IncomingRequests:
        resource = createReqResourceStr("new", "maintainer");
        reply = request(resource);
        break;
    case OBSCore::OutgoingRequests:
        resource = createReqResourceStr("new,review", "creator");
        reply = request(resource);
        break;
    case OBSCore::DeclinedRequests:
        resource = createReqResourceStr("declined", "creator");
        reply = request(resource);
        break;
    default:
        qDebug() << " OBSCore::getRequests() request type not handled!";
        break;
    }

    if (reply) {
        reply->setProperty("reqtype", type);
    }
}

void OBSCore::getIncomingRequests()
{
    getRequests(OBSCore::IncomingRequests);
}

void OBSCore::getOutgoingRequests()
{
    getRequests(OBSCore::OutgoingRequests);
}

void OBSCore::getDeclinedRequests()
{
    getRequests(OBSCore::DeclinedRequests);
}

void OBSCore::getPackageRequests(const QString &project, const QString &package)
{
    QString types = "submit,delete,add_role,change_devel,maintenance_incident,maintenance_release,release";
    QString states = "new,review";
    QString resource = QString("?view=collection&types=%1&states=%2&project=%3&package=%4")
            .arg(types, states, project, package);
    QNetworkReply *reply = requestRequest(resource);
    reply->setProperty("reqtype", OBSCore::PackageRequests);
    reply->setProperty("prjreq", project);
    reply->setProperty("pkgreq", package);
}

bool OBSCore::isIncludeHomeProjects() const
{
    return includeHomeProjects;
}

void OBSCore::setIncludeHomeProjects(bool value)
{
    includeHomeProjects = value;
}

void OBSCore::getProjects()
{
    QNetworkReply *reply = requestSource("");
    reply->setProperty("reqtype", OBSCore::ProjectList);
    QString userHome = includeHomeProjects ? "" : "home:" + username;
    reply->setProperty("includehomeprjs", userHome);
}

void OBSCore::getProjectMetaConfig(const QString &resource)
{
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::PrjMetaConfig);
}

void OBSCore::getPackageMetaConfig(const QString &resource)
{
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::PkgMetaConfig);
}

void OBSCore::getPackages(const QString &resource)
{
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::PackageList);
}

void OBSCore::getFiles(const QString &project, const QString &package)
{
    QString resource = QString("/%1/%2").arg(project, package);
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::FileList);
    reply->setProperty("prjfile", project);
    reply->setProperty("pkgfile", package);
}

void OBSCore::getRevisions(const QString &project, const QString &package)
{
    QString resource = QString("/%1/%2/_history").arg(project, package);
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::RevisionList);
    reply->setProperty("prjrev", project);
    reply->setProperty("pkgrev", package);
}

void OBSCore::getLatestRevision(const QString &project, const QString &package)
{
    QString resource = QString("/%1/%2/_history?limit=1").arg(project, package);
    QNetworkReply *reply = requestSource(resource);
    reply->setProperty("reqtype", OBSCore::LatestRevision);
    reply->setProperty("prjrev", project);
    reply->setProperty("pkgrev", package);
}

void OBSCore::getLink(const QString &project, const QString &package)
{
    QString resource = QString("/source/%1/%2/_link").arg(project, package);
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::Link);
}

void OBSCore::getAllBuildStatus(const QString &resource)
{
    QNetworkReply *reply = requestBuild(resource);
    reply->setProperty("reqtype", OBSCore::BuildStatusList);
}

void OBSCore::request(const QString &resource, int row)
{
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::BuildStatus);
    reply->setProperty("row", row);
}

QNetworkReply *OBSCore::postRequest(const QString &resource, const QByteArray &data, const QString &contentTypeHeader)
{
    QNetworkRequest request;
    request.setUrl(QUrl(apiUrl + resource));
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentTypeHeader);
    QNetworkReply *reply = manager->post(request, data);

    return reply;
}

QNetworkReply *OBSCore::putRequest(const QString &resource, const QByteArray &data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(apiUrl + resource));
    qDebug() << "User-Agent:" << userAgent;
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
    QNetworkReply *reply = manager->put(request, data);

    return reply;
}

QNetworkReply *OBSCore::deleteRequest(const QString &resource)
{
    QNetworkRequest request;
    request.setUrl(QUrl(apiUrl + resource));
    request.setRawHeader("User-Agent", userAgent.toLatin1());
    QNetworkReply *reply = manager->deleteResource(request);

    return reply;
}

void OBSCore::changeSubmitRequest(const QString &resource, const QByteArray &data)
{
    QNetworkReply *reply = postRequest(resource, data, "application/x-www-form-urlencoded");
    reply->setProperty("reqtype", OBSCore::ChangeRequestState);
}

void OBSCore::provideAuthentication(QNetworkReply *reply, QAuthenticator *authenticator)
{
    qDebug() << Q_FUNC_INFO << "URL =" << reply->request().url().toString();
//    qDebug() << reply->readAll();
    authenticator->setUser(username);
    authenticator->setPassword(password);
}

bool OBSCore::isAuthenticated()
{
    return authenticated;
}

void OBSCore::replyFinished(QNetworkReply *reply)
{
    // QNetworkReply is a sequential-access QIODevice, which means that
    // once data is read from the object, it no longer kept by the device.
    // It is therefore the application's responsibility to keep this data if it needs to.
    // See http://doc.qt.nokia.com/latest/qnetworkreply.html for more info
    QByteArray data = reply->readAll();

    qDebug() << "OBSCore::replyFinished()" << reply->url().toString();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "OBSCore::replyFinished() HTTP status code:" << httpStatusCode;
//    qDebug() << "Network Reply: " << data;

    if (httpStatusCode==200 && !authenticated) {
        authenticated = true;
        emit isAuthenticated(authenticated);
    } else if (httpStatusCode==401) {
        authenticated = false;
        emit isAuthenticated(authenticated);
    }

    /* Set package row always (error/no error) if property is valid.
     * Needed for inserting the build status
     */
    if(reply->property("row").isValid()) {
        int row = reply->property("row").toInt();
        qDebug() << "Reply row property:" << QString::number(row);
        xmlReader->setPackageRow(row);
    }

    QString reqTypeStr = "RequestType";

    switch (reply->error()) {

    case QNetworkReply::NoError: {
        qDebug() << "OBSCore::replyFinished() Request succeeded! Status code:" << httpStatusCode;

        if (reply->property("reqtype").isValid()) {
            QString reqTypeStr = "RequestType";
            int reqType = reply->property("reqtype").toInt();

            QString dataStr;
            if (reqType != OBSCore::DownloadFile) {
                dataStr = QString::fromUtf8(data);
            }

            switch(reqType) {

            case OBSCore::Login: // <html>
                qDebug() << reqTypeStr << "Login";
                // do nothing
                break;

            case OBSCore::ProjectList: { // <directory>
                qDebug() << reqTypeStr << "ProjectList";
                QString userHome;
                if (reply->property("includehomeprjs").isValid()) {
                    userHome = reply->property("includehomeprjs").toString();
                }
                xmlReader->parseProjectList(userHome, dataStr);
                break;
            }

            case OBSCore::PrjMetaConfig: // <project>
                qDebug() << reqTypeStr << "PrjMetaConfig";
                xmlReader->parsePrjMetaConfig(dataStr);
                break;

            case OBSCore::PkgMetaConfig: // <package>
                qDebug() << reqTypeStr << "PkgMetaConfig";
                xmlReader->parsePkgMetaConfig(dataStr);
                break;

            case OBSCore::PackageList: // <directory>
                qDebug() << reqTypeStr << "PackageList";
                xmlReader->parsePackageList(dataStr);
                break;

            case OBSCore::FileList: { // <directory>
                qDebug() << reqTypeStr << "FileList";
                QString project;
                QString package;
                if (reply->property("prjfile").isValid()) {
                    project = reply->property("prjfile").toString();
                }
                if (reply->property("pkgfile").isValid()) {
                    package = reply->property("pkgfile").toString();
                }
                xmlReader->parseFileList(project, package, dataStr);
                break;
            }

            case OBSCore::RevisionList: { // <revisionlist>
                qDebug() << reqTypeStr << "RevisionList";
                QString project;
                QString package;
                if (reply->property("prjrev").isValid()) {
                    project = reply->property("prjrev").toString();
                }
                if (reply->property("pkgrev").isValid()) {
                    package = reply->property("pkgrev").toString();
                }
                xmlReader->parseRevisionList(project, package, dataStr);
                break;
            }

            case OBSCore::LatestRevision: { // <revision>
                qDebug() << reqTypeStr << "LatestRevision";
                QString project;
                QString package;
                if (reply->property("prjrev").isValid()) {
                    project = reply->property("prjrev").toString();
                }
                if (reply->property("pkgrev").isValid()) {
                    package = reply->property("pkgrev").toString();
                }
                xmlReader->parseLatestRevision(project, package, dataStr);
                break;
            }

            case OBSCore::Link: { // <link>
                qDebug() << reqTypeStr << "Link";
                xmlReader->parseLink(dataStr);
                break;
            }

            case OBSCore::BuildStatus: // <status>
                qDebug() << reqTypeStr << "BuildStatus";
                xmlReader->parseBuildStatus(dataStr);
                break;

            case OBSCore::BuildStatusList: // <resultlist>
                qDebug() << reqTypeStr << "BuildStatusList";
                xmlReader->parseResultList(dataStr);
                break;

            case OBSCore::IncomingRequests: // <collection>
                qDebug() << reqTypeStr << "Collection";
                xmlReader->parseIncomingRequests(dataStr);
                break;

            case OBSCore::OutgoingRequests: // <collection>
                qDebug() << reqTypeStr << "Collection";
                xmlReader->parseOutgoingRequests(dataStr);
                break;

            case OBSCore::DeclinedRequests: // <collection>
                qDebug() << reqTypeStr << "Collection";
                xmlReader->parseDeclinedRequests(dataStr);
                break;

            case OBSCore::PackageRequests: {
                qDebug() << reqTypeStr << "Collection";
                QString project;
                QString package;
                if (reply->property("prjreq").isValid()) {
                    project = reply->property("prjreq").toString();
                }
                if (reply->property("pkgreq").isValid()) {
                    package = reply->property("pkgreq").toString();
                }
                xmlReader->parseRequests(project, package, dataStr);
                break;
            }

            case OBSCore::ChangeRequestState:
                qDebug() << reqTypeStr << "ChangeRequestState";
                xmlReader->parseRequestStatus(dataStr);
                break;

            case OBSCore::SRDiff:
                qDebug() << reqTypeStr << "SRDiff";
                emit srDiffFetched(dataStr);
                break;

            case OBSCore::BranchPackage: {
                qDebug() << reqTypeStr << "BranchPackage";
                xmlReader->parseBranchPackage(dataStr);
                break;
            }

            case OBSCore::LinkPackage: {
                qDebug() << reqTypeStr << "LinkPackage";
                QString project;
                QString package;
                if (reply->property("destprj").isValid()) {
                    project = reply->property("destprj").toString();
                }
                if (reply->property("destpkg").isValid()) {
                    package = reply->property("destpkg").toString();
                }
                xmlReader->parseLinkPackage(project, package, dataStr);
                break;
            }

            case OBSCore::CopyPackage: {
                qDebug() << reqTypeStr << "CopyPackage";
                QString project;
                QString package;
                if (reply->property("destprj").isValid()) {
                    project = reply->property("destprj").toString();
                }
                if (reply->property("destpkg").isValid()) {
                    package = reply->property("destpkg").toString();
                }
                xmlReader->parseCopyPackage(project, package, dataStr);
                break;
            }

            case OBSCore::CreateRequest: {
                qDebug() << reqTypeStr << "CreateRequest";
                xmlReader->parseCreateRequest(dataStr);
                break;
            }

            case OBSCore::CreateProject: {
                qDebug() << reqTypeStr << "CreateProject";
                QString project;
                if (reply->property("createprj").isValid()) {
                    project = reply->property("createprj").toString();
                }
                xmlReader->parseCreateProject(project, dataStr);
                break;
            }

            case OBSCore::CreatePackage: {
                qDebug() << reqTypeStr << "CreatePackage";
                QString project;
                QString package;
                if (reply->property("createprj").isValid()) {
                    project = reply->property("createprj").toString();
                }
                if (reply->property("createpkg").isValid()) {
                    package = reply->property("createpkg").toString();
                }
                xmlReader->parseCreatePackage(project, package, dataStr);
                break;
            }

            case OBSCore::UploadFile: {
                qDebug() << reqTypeStr << "UploadFile";
                QString project;
                QString package;
                QString file;
                if (reply->property("uploadprj").isValid()) {
                    project = reply->property("uploadprj").toString();
                }
                if (reply->property("uploadpkg").isValid()) {
                    package = reply->property("uploadpkg").toString();
                }
                if (reply->property("uploadfile").isValid()) {
                    file = reply->property("uploadfile").toString();
                }
                xmlReader->parseUploadFile(project, package, file, dataStr);
                break;
            }

            case OBSCore::DownloadFile: {
                qDebug() << reqTypeStr << "DownloadFile";
                QString fileName;
                if (reply->property("downloadfile").isValid()) {
                    fileName = reply->property("downloadfile").toString();
                }
                emit fileFetched(fileName, data);
                break;
            }

            case OBSCore::BuildLog: {
                qDebug() << reqTypeStr << "BuildLog";
                emit buildLogFetched(dataStr);
                break;
            }

            case OBSCore::DeleteProject: {
                qDebug() << reqTypeStr << "DeleteProject";
                QString project;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                xmlReader->parseDeleteProject(project, dataStr);
                break;
            }

            case OBSCore::DeletePackage: {
                qDebug() << reqTypeStr << "DeletePackage";
                QString project;
                QString package;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                if (reply->property("deletepkg").isValid()) {
                    package = reply->property("deletepkg").toString();
                }
                xmlReader->parseDeletePackage(project, package, dataStr);
                break;
            }

            case OBSCore::DeleteFile: {
                qDebug() << reqTypeStr << "DeleteFile";
                QString project;
                QString package;
                QString fileName;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                if (reply->property("deletepkg").isValid()) {
                    package = reply->property("deletepkg").toString();
                }
                if (reply->property("deletefile").isValid()) {
                    fileName = reply->property("deletefile").toString();
                }
                xmlReader->parseDeleteFile(project, package, fileName, dataStr);
                break;
            }

            case OBSCore::About:
                qDebug() << reqTypeStr << "About";
                xmlReader->parseAbout(dataStr);
                break;

            case OBSCore::Person:
                qDebug() << reqTypeStr << "Person";
                xmlReader->parsePerson(dataStr);
                break;

            case OBSCore::UpdatePerson:
                qDebug() << reqTypeStr << "UpdatePerson";
                xmlReader->parseUpdatePerson(dataStr);
                break;

            case OBSCore::Distributions:
                qDebug() << reqTypeStr << "Distributions";
                xmlReader->parseDistributions(dataStr);
                break;
            }
            return;
        }
        QString dataStr = QString::fromUtf8(data);
        xmlReader->addData(dataStr);
        break; // end of case QNetworkReply::NoError
    }

    case QNetworkReply::ContentNotFoundError: // 404
        if (reply->property("reqtype").isValid()) {

            switch(reply->property("reqtype").toInt()) {

            case OBSCore::Login:
                qDebug() << reqTypeStr << "Login"; // <hash><status>
                qDebug() << "OBSCore::replyFinished() OBS API not found at" << reply->url().toString();
                emit apiNotFound(reply->url());
                break;

            case OBSCore::PackageList: // <status>
                qDebug() << reqTypeStr << "PackageList";
                if (isAuthenticated()) {
                    QString dataStr = QString::fromUtf8(data);
                    OBSStatus *status = xmlReader->parseNotFoundStatus(dataStr);
                    qDebug() << "OBSCore::replyFinished() Project not found!" << status->getSummary() << status->getCode();
                    emit projectNotFound(status);
                    delete status;
                }
                break;

            case OBSCore::FileList: // <status>
            case OBSCore::RevisionList: // <status>
            case OBSCore::LatestRevision: // <status>
                qDebug() << reqTypeStr << "FileList";
                if (isAuthenticated()) {
                    QString dataStr = QString::fromUtf8(data);
                    OBSStatus *status = xmlReader->parseNotFoundStatus(dataStr);
                    qDebug() << "OBSCore::replyFinished() Package not found!" << status->getSummary() << status->getCode();
                    emit packageNotFound(status);
                    delete status;
                }
                break;

            case OBSCore::BuildStatus: // <status>
                qDebug() << reqTypeStr << "BuildStatus";
                if (isAuthenticated()) {
                    QString dataStr = QString::fromUtf8(data);
                    xmlReader->parseBuildStatus(dataStr);
                }
                break;

            case OBSCore::CreateRequest:
                xmlReader->parseCreateRequestStatus(data);
                break;

            case OBSCore::LinkPackage: {
                qDebug() << reqTypeStr << "LinkPackage";
                QString project;
                QString package;
                if (reply->property("destprj").isValid()) {
                    project = reply->property("destprj").toString();
                }
                if (reply->property("destpkg").isValid()) {
                    package = reply->property("destpkg").toString();
                }
                OBSStatus *status = new OBSStatus();
                QString details = QString("The package %2 in project %1 does NOT exist.").arg(project, package);
                status->setDetails(details);
                status->setSummary("Cannot link");
                status->setCode("error");
                status->setProject(project);
                status->setPackage(package);

                emit cannotLinkPackage(status);
                delete status;
                break;
            }

            case OBSCore::BuildLog: {
                qDebug() << reqTypeStr << "BuildLog";
                emit buildLogNotFound();
                break;
            }

            default:
                qDebug() << "OBSCore Error 404 NOT handled for request type" << reply->property("reqtype").toInt();
            }
        }
        break;

    case QNetworkReply::ProtocolInvalidOperationError: // 400
        if (reply->property("reqtype").isValid()) {

            switch(reply->property("reqtype").toInt()) {
            case OBSCore::CreateRequest:
                xmlReader->parseCreateRequestStatus(data);
                break;
            case OBSCore::BranchPackage: {
                xmlReader->parseBranchPackage(data);
                break;
            }
            default:
                qDebug() << "OBSCore::replyFinished() Request failed! Error:" << reply->errorString();
                qDebug() << data;
                emit networkError(reply->errorString());
                break;
            }

        }
        break;

    case QNetworkReply::ContentAccessDenied: // 401
        qDebug() << "OBSCore::replyFinished() Access denied!";
        if (reply->property("reqtype").isValid()) {
            OBSStatus *obsStatus = nullptr;
            switch(reply->property("reqtype").toInt()) {

            case OBSCore::CopyPackage: {
                obsStatus = new OBSStatus();
                QString project;
                QString package;
                if (reply->property("destprj").isValid()) {
                    project = reply->property("destprj").toString();
                }
                if (reply->property("destpkg").isValid()) {
                    package = reply->property("destpkg").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setPackage(package);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot copy");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to<br>copy %1 to %2<br>")
                                      .arg(obsStatus->getPackage(), obsStatus->getProject()));
                emit cannotCopyPackage(obsStatus);
                delete obsStatus;
                break;
            }
            case OBSCore::CreateProject: {
                obsStatus = new OBSStatus();
                QString project;
                if (reply->property("createprj").isValid()) {
                    project = reply->property("createprj").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot create");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to create<br>%1")
                                      .arg(obsStatus->getProject()));
                emit cannotCreateProject(obsStatus);
                delete obsStatus;
                break;
            }

            case OBSCore::CreatePackage: {
                obsStatus = new OBSStatus();
                QString project;
                QString package;
                if (reply->property("createprj").isValid()) {
                    project = reply->property("createprj").toString();
                }
                if (reply->property("createpkg").isValid()) {
                    package = reply->property("createpkg").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setPackage(package);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot create");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to create<br>%1/%2")
                                      .arg(obsStatus->getProject(), obsStatus->getPackage()));
                emit cannotCreatePackage(obsStatus);
                delete obsStatus;
                break;
            }

            case OBSCore::UploadFile: {
                obsStatus = new OBSStatus();
                QString project;
                QString package;
                if (reply->property("uploadprj").isValid()) {
                    project = reply->property("uploadprj").toString();
                }
                if (reply->property("uploadpkg").isValid()) {
                    package = reply->property("uploadpkg").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setPackage(package);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot upload file");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to upload to <br>%1/%2")
                                      .arg(obsStatus->getProject(), obsStatus->getPackage()));
                emit cannotUploadFile(obsStatus);
                delete obsStatus;
                break;
            }

            case OBSCore::DeleteProject: {
                obsStatus = new OBSStatus();
                QString project;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot delete");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to delete<br>%1")
                                      .arg(obsStatus->getProject()));
                emit cannotDeleteProject(obsStatus);
                delete obsStatus;
                break;
            }

            case OBSCore::DeletePackage: {
                obsStatus = new OBSStatus();
                QString project;
                QString package;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                if (reply->property("deletepkg").isValid()) {
                    package = reply->property("deletepkg").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setPackage(package);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot delete");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to delete<br>%1/%2")
                                      .arg(obsStatus->getProject(), obsStatus->getPackage()));
                emit cannotDeletePackage(obsStatus);
                delete obsStatus;
                break;
            }

            case OBSCore::DeleteFile: {
                obsStatus = new OBSStatus();
                QString project;
                QString package;
                QString fileName;
                if (reply->property("deleteprj").isValid()) {
                    project = reply->property("deleteprj").toString();
                }
                if (reply->property("deletepkg").isValid()) {
                    package = reply->property("deletepkg").toString();
                }
                if (reply->property("deletefile").isValid()) {
                    fileName = reply->property("deletefile").toString();
                }
                obsStatus->setProject(project);
                obsStatus->setPackage(package);
                obsStatus->setCode("error");
                obsStatus->setSummary("Cannot delete");
                obsStatus->setDetails(tr("You don't have the appropriate permissions to delete<br>%1/%2/%3")
                                      .arg(obsStatus->getProject(), obsStatus->getPackage(), fileName));
                emit cannotDeleteFile(obsStatus);
                delete obsStatus;
                break;
            }

            }
        }
        break;

    case QNetworkReply::AuthenticationRequiredError:
        qDebug() << "OBSCore::replyFinished() Credentials provided not accepted!";
        break;

    default: // Other errors
        qDebug() << "OBSCore::replyFinished() Request failed! Error:" << reply->errorString();
        qDebug() << data;
        emit networkError(reply->errorString());
        break;
    }

    reply->deleteLater();
}

void OBSCore::getSRDiff(const QString &resource)
{
    QNetworkReply *reply = postRequest(resource, "", "application/x-www-form-urlencoded");
    reply->setProperty("reqtype", OBSCore::SRDiff);
}

void OBSCore::branchPackage(const QString &project, const QString &package)
{
    QString resource = QString("/source/%1/%2?cmd=branch").arg(project, package);
    QNetworkReply *reply = postRequest(resource, "", "application/x-www-form-urlencoded");
    reply->setProperty("reqtype", OBSCore::BranchPackage);
}

void OBSCore::linkPackage(const QString &srcProject, const QString &srcPackage, const QString &dstProject)
{
    linkHelper = new OBSLinkHelper(this);

    connect(xmlReader, &OBSXmlReader::finishedParsingPackageMetaConfig,
            linkHelper, &OBSLinkHelper::slotFetchedPackageMetaConfig);
    connect(xmlReader, &OBSXmlReader::finishedParsingCreatePkgStatus,
            linkHelper, &OBSLinkHelper::slotFetchedCreatePkgStatus);

    connect(linkHelper, &OBSLinkHelper::getPackageMetaConfig, this, [&](const QString &resource) {
        getPackageMetaConfig(resource);
    });
    connect(linkHelper, &OBSLinkHelper::createPackage, this, [&](const QString &dstProject,
            const QString &dstPackage, const QByteArray &data) {
        createPackage(dstProject, dstPackage, data);
    });
    connect(linkHelper, &OBSLinkHelper::readyToLinkPackage, this, &OBSCore::slotLinkPackage);
    connect(this, &OBSCore::cannotCreatePackage, linkHelper, [&](OBSStatus *status) {
        QString details = QString("You don't have the appropriate permissions to create a link in %1/%2")
                .arg(dstProject, srcPackage);
        status->setDetails(details);
        status->setSummary("Cannot link");
        emit cannotLinkPackage(status);
    });

    linkHelper->linkPackage(srcProject, srcPackage, dstProject);
}

void OBSCore::copyPackage(const QString &originProject, const QString &originPackage,
                          const QString &destProject, const QString &destPackage, const QString &comments)
{
    QString resource = QString("/source/%1/%2?cmd=copy&oproject=%3&opackage=%4&comment=%5")
            .arg(destProject, destPackage, originProject, originPackage, comments);
    QNetworkReply *reply = postRequest(resource, "", "application/x-www-form-urlencoded");
    reply->setProperty("reqtype", OBSCore::CopyPackage);
    reply->setProperty("destprj", destProject);
    reply->setProperty("destpkg", destPackage);
}

void OBSCore::createRequest(const QByteArray &data)
{
    QString resource = QString("/request?cmd=create");
    QNetworkReply *reply = postRequest(resource, data, "application/xml");
    reply->setProperty("reqtype", OBSCore::CreateRequest);
}

void OBSCore::createProject(const QString &project, const QByteArray &data)
{
    QString resource = QString("/source/%1/_meta").arg(project);
    QNetworkReply *reply = putRequest(resource, data);
    reply->setProperty("reqtype", OBSCore::CreateProject);
    reply->setProperty("createprj", project);
}

void OBSCore::createPackage(const QString &project, const QString &package, const QByteArray &data)
{
    QString resource = QString("/source/%1/%2/_meta").arg(project, package);
    QNetworkReply *reply = putRequest(resource, data);
    reply->setProperty("reqtype", OBSCore::CreatePackage);
    reply->setProperty("createprj", project);
    reply->setProperty("createpkg", package);
}

void OBSCore::uploadFile(const QString &project, const QString &package, const QString &fileName, const QByteArray &data)
{
    QString resource = QString("/source/%1/%2/%3").arg(project, package, fileName);
    QNetworkReply *reply = putRequest(resource, data);
    reply->setProperty("reqtype", OBSCore::UploadFile);
    reply->setProperty("uploadprj", project);
    reply->setProperty("uploadpkg", package);
    reply->setProperty("uploadfile", fileName);
}

void OBSCore::downloadFile(const QString &project, const QString &package, const QString &fileName)
{
    QString resource = QString("/source/%1/%2/%3").arg(project, package, fileName);
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::DownloadFile);
    reply->setProperty("downloadprj", project);
    reply->setProperty("downloadpkg", package);
    reply->setProperty("downloadfile", fileName);
}

void OBSCore::getBuildLog(const QString &project, const QString &repository, const QString &arch, const QString &package)
{
    QString resource = QString("/build/%1/%2/%3/%4/_log").arg(project, repository, arch, package);
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::BuildLog);
}

void OBSCore::deleteProject(const QString &project)
{
    QString resource = QString("/source/%1").arg(project);
    QNetworkReply *reply = deleteRequest(resource);
    reply->setProperty("reqtype", OBSCore::DeleteProject);
    reply->setProperty("deleteprj", project);
}

void OBSCore::deletePackage(const QString &project, const QString &package)
{
    QString resource = QString("/source/%1/%2").arg(project, package);
    QNetworkReply *reply = deleteRequest(resource);
    reply->setProperty("reqtype", OBSCore::DeletePackage);
    reply->setProperty("deleteprj", project);
    reply->setProperty("deletepkg", package);
}

void OBSCore::deleteFile(const QString &project, const QString &package, const QString &fileName)
{
    QString resource = QString("/source/%1/%2/%3").arg(project, package, fileName);
    QNetworkReply *reply = deleteRequest(resource);
    reply->setProperty("reqtype", OBSCore::DeleteFile);
    reply->setProperty("deleteprj", project);
    reply->setProperty("deletepkg", package);
    reply->setProperty("deletefile", fileName);
}

void OBSCore::about()
{
    QString resource = "/about";
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::About);
}

void OBSCore::getPerson()
{
    QString resource = "/person/" + username;
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::Person);
}

void OBSCore::updatePerson(const QByteArray &data)
{
    QString resource = "/person/" + username;
    QNetworkReply *reply = putRequest(resource, data);
    reply->setProperty("reqtype", OBSCore::UpdatePerson);
}

void OBSCore::getDistributions()
{
    QString resource = "/distributions";
    QNetworkReply *reply = request(resource);
    reply->setProperty("reqtype", OBSCore::Distributions);
}

void OBSCore::onSslErrors(QNetworkReply *reply, const QList<QSslError> &list)
{
    QString errorString;
    QString message;

    foreach (const QSslError &sslError, list) {
        if (list.count() >= 1) {
            errorString += ", ";
            errorString = sslError.errorString();
            if (sslError.error() == QSslError::SelfSignedCertificateInChain) {
                qDebug() << "OBSCore::onSslErrors() Self signed certificate!";
                emit selfSignedCertificate(reply);
            }
        }
    }
    qDebug() << "OBSCore::onSslErrors() SSL Errors:" << errorString;

    if (list.count() == 1) {
        message=tr("An SSL error has occured: %1");
    } else {
        message=QString::number(list.count())+tr(" SSL errors have occured: %1");
    }

   qDebug() << "OBSCore::onSslErrors() url:" << reply->url() << "row:" << reply->property("row").toInt();
}

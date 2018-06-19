/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2018 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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

#ifndef OBSACCESS_H
#define OBSACCESS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QSslError>
#include <QDebug>
#include <QEventLoop>
#include "obsxmlreader.h"

class OBSXmlReader;

class OBSAccess : public QObject
{
     Q_OBJECT

public:
    static OBSAccess* getInstance();
    bool isAuthenticated();
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    QString getApiUrl() const;
    QNetworkReply *request(const QString &resource);
    QNetworkReply *requestBuild(const QString &resource);
    void requestBuild(const QString &resource, int row);
    QNetworkReply *requestSource(const QString &resource);
    void requestRequest(const QString &resource);
    QNetworkReply *postRequest(const QString &resource, const QByteArray &data);
    QNetworkReply *putRequest(const QString &resource, const QByteArray &data);
    QNetworkReply *deleteRequest(const QString &resource);
    void getProjects();
    void getProjectMetadata(const QString &resource);
    void getPackages(const QString &resource);
    void getFiles(const QString &resource);
    void getAllBuildStatus(const QString &resource);
    void changeSubmitRequest(const QString &resource, const QByteArray &data);
    void request(const QString &resource, int row);
    void getSRDiff(const QString &resource);
    void branchPackage(const QString &resource);
    void createProject(const QString &project, const QByteArray &data);
    void createPackage(const QString &project, const QString &package, const QByteArray &data);
    void deleteProject(const QString &project);
    void deletePackage(const QString &project, const QString &package);
    void deleteFile(const QString &project, const QString &package, const QString &fileName);
    void about();

signals:
    void isAuthenticated(bool authenticated);
    void selfSignedCertificate(QNetworkReply *reply);
    void networkError(const QString &error);
    void srDiffFetched(const QString &diff);
    void cannotCreateProject(OBSStatus *obsStatus);
    void cannotCreatePackage(OBSStatus *obsStatus);
    void cannotDeleteProject(OBSStatus *obsStatus);
    void cannotDeletePackage(OBSStatus *obsStatus);
    void cannotDeleteFile(OBSStatus *obsStatus);

public slots:
    void setCredentials(const QString&, const QString&);

private slots:
    void provideAuthentication(QNetworkReply* reply, QAuthenticator* ator);
    void replyFinished(QNetworkReply* reply);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError> &list);

private:
/*
 * We need to instantiate QNAM only once, as it should be used as
 * as singleton or a utility instead of recreating it for each
 * request. This class (OBSAccess) uses the singleton pattern to
 * achieve this.
 *
 */
    QNetworkAccessManager* manager;
    void createManager();
    OBSAccess();
    static OBSAccess* instance;
    QString curUsername;
    QString curPassword;
    QString prevPassword;
    QString prevUsername;
    QString apiUrl;
    enum RequestType {
        ProjectList,
        ProjectMetadata,
        PackageList,
        FileList,
        BuildStatusList,
        SubmitRequest,
        SRDiff,
        BranchPackage,
        CreateProject,
        CreatePackage,
        DeleteProject,
        DeletePackage,
        DeleteFile,
        About
    };
    bool authenticated;
    OBSXmlReader *xmlReader;
};

#endif // OBSACCESS_H

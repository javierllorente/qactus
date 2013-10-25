/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2013 Javier Llorente <javier@opensuse.org>
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
#include "obspackage.h"

class OBSxmlReader;
class OBSpackage;

class OBSaccess : public QObject
{
     Q_OBJECT

public:
    OBSaccess();
    void setUrl(QUrl url);
    void makeRequest();
    QString getUsername();
    OBSpackage* getPackage();
    QList<OBSrequest*> getRequests();
    int getRequestNumber();

signals:
    void isAuthenticated(bool authenticated);

public slots:
    void setCredentials(const QString&, const QString&);
    void provideAuthentication(QNetworkReply* reply, QAuthenticator* ator);
    void replyFinished(QNetworkReply* reply);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError> &list);

private:
// we need to instantiate QNAM only once, as it should be used as
// as singelton or a utility instead of recreating it for each
// request.
    QNetworkAccessManager* manager;
    void createManager();
    QUrl url;
    QString curUsername;
    QString curPassword;
    QString data;
    bool authenticated;
    OBSxmlReader *xmlReader;
    OBSpackage *obsPackage;
    QList<OBSrequest*> obsRequests;

};

#endif // OBSACCESS_H

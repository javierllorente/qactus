/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSLINKHELPER_H
#define OBSLINKHELPER_H

#include <QObject>
#include "obspkgmetaconfig.h"
#include "obsstatus.h"

class OBSLinkHelper : public QObject
{
    Q_OBJECT

public:
    explicit OBSLinkHelper(QObject *parent = nullptr);
    void linkPackage(const QString &srcProject, const QString &srcPackage, const QString &dstProject);

private:
    OBSPkgMetaConfig *m_pkgMetaConfig;
    QString m_srcProject;
    QString m_dstProject;
    QString m_dstPackage;

signals:
    void getPackageMetaConfig(const QString &resource);
    void readyToLinkPackage(const QString &dstProject, const QString &dstPackage, const QByteArray &data);
    void createPackage(const QString &dstProject, const QString &dstPackage, const QByteArray &data);

public slots:
    void slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig);
    void slotFetchedCreatePkgStatus(OBSStatus *status);

private slots:
};

#endif // OBSLINKHELPER_H

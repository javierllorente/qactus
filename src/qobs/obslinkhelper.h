/*
 * Copyright (C) 2019-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef OBSLINKHELPER_H
#define OBSLINKHELPER_H

#include <QObject>
#include <QSharedPointer>
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
    void slotFetchedPackageMetaConfig(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig);
    void slotFetchedCreatePkgStatus(QSharedPointer<OBSStatus> status);

private slots:
};

#endif // OBSLINKHELPER_H

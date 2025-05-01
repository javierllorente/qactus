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
#include "obslinkhelper.h"
#include "obsxmlwriter.h"
#include <QDebug>

OBSLinkHelper::OBSLinkHelper(QObject *parent) : QObject(parent),
  m_pkgMetaConfig(nullptr)
{
}

void OBSLinkHelper::linkPackage(const QString &srcProject, const QString &srcPackage, const QString &dstProject)
{
    qDebug() << __PRETTY_FUNCTION__ << srcProject << srcPackage << dstProject;
    m_srcProject = srcProject;
    m_dstProject = dstProject;
    m_dstPackage = srcPackage;

    QString resource = QString("/%1/%2/_meta").arg(srcProject, srcPackage);
    emit getPackageMetaConfig(resource);
}

void OBSLinkHelper::onFetchedPackageMetaConfig(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig)
{
    qDebug() << __PRETTY_FUNCTION__;
    QSharedPointer<OBSPkgMetaConfig> newPkgMetaConfig(new OBSPkgMetaConfig());
    newPkgMetaConfig->setName(pkgMetaConfig->getName());
    newPkgMetaConfig->setProject(m_dstProject);
    newPkgMetaConfig->setTitle(pkgMetaConfig->getTitle());
    newPkgMetaConfig->setUrl(pkgMetaConfig->getUrl());
    newPkgMetaConfig->setDescription(pkgMetaConfig->getDescription());
    newPkgMetaConfig->setBuildFlag(pkgMetaConfig->getBuildFlag());

    OBSXmlWriter xmlWriter;
    QByteArray metaConfigData = xmlWriter.createPackageMeta(newPkgMetaConfig);

    emit createPackage(m_dstProject, m_dstPackage, metaConfigData);
}

void OBSLinkHelper::onFetchedCreatePkgStatus(QSharedPointer<OBSStatus> status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();
    if (status->getCode() == "ok") {
        OBSXmlWriter xmlWriter;
        QByteArray data = xmlWriter.createLink(m_srcProject, m_dstPackage);
        emit readyToLinkPackage(m_dstProject, m_dstPackage, data);
    }
}

/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019-2023 Javier Llorente <javier@opensuse.org>
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

void OBSLinkHelper::slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig)
{
    qDebug() << __PRETTY_FUNCTION__;
    OBSPkgMetaConfig *newPkgMetaConfig = new OBSPkgMetaConfig();
    newPkgMetaConfig->setName(pkgMetaConfig->getName());
    newPkgMetaConfig->setProject(m_dstProject);
    newPkgMetaConfig->setTitle(pkgMetaConfig->getTitle());
    newPkgMetaConfig->setUrl(pkgMetaConfig->getUrl());
    newPkgMetaConfig->setDescription(pkgMetaConfig->getDescription());
    newPkgMetaConfig->setBuildFlag(pkgMetaConfig->getBuildFlag());

    OBSXmlWriter *xmlWriter = new OBSXmlWriter();
    QByteArray metaConfigData = xmlWriter->createPackageMeta(newPkgMetaConfig);
    delete xmlWriter;
    delete newPkgMetaConfig;

    emit createPackage(m_dstProject, m_dstPackage, metaConfigData);
}

void OBSLinkHelper::slotFetchedCreatePkgStatus(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();
    if (status->getCode() == "ok") {
        OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
        QByteArray data = xmlWriter->createLink(m_srcProject, m_dstPackage);
        delete xmlWriter;
        emit readyToLinkPackage(m_dstProject, m_dstPackage, data);
    }
}

/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
#include "obsdistribution.h"

OBSDistribution::OBSDistribution()
{

}

QString OBSDistribution::getVendor() const
{
    return vendor;
}

void OBSDistribution::setVendor(const QString &value)
{
    vendor = value;
}

QString OBSDistribution::getVersion() const
{
    return version;
}

void OBSDistribution::setVersion(const QString &value)
{
    version = value;
}

QString OBSDistribution::getId() const
{
    return id;
}

void OBSDistribution::setId(const QString &value)
{
    id = value;
}

QString OBSDistribution::getName() const
{
    return name;
}

void OBSDistribution::setName(const QString &value)
{
    name = value;
}

QString OBSDistribution::getProject() const
{
    return project;
}

void OBSDistribution::setProject(const QString &value)
{
    project = value;
}

QString OBSDistribution::getRepoName() const
{
    return repoName;
}

void OBSDistribution::setRepoName(const QString &value)
{
    repoName = value;
}

QString OBSDistribution::getRepository() const
{
    return repository;
}

void OBSDistribution::setRepository(const QString &value)
{
    repository = value;
}

QUrl OBSDistribution::getLink() const
{
    return link;
}

void OBSDistribution::setLink(const QUrl &value)
{
    link = value;
}

void OBSDistribution::appendIcon(const QUrl &url)
{
    icons.append(url);
}

QList<QUrl> OBSDistribution::getIcons() const
{
    return icons;
}

void OBSDistribution::setIcons(const QList<QUrl> &value)
{
    icons = value;
}

void OBSDistribution::appendArch(const QString &arch)
{
    archs.append(arch);
}

QStringList OBSDistribution::getArchs() const
{
    return archs;
}

void OBSDistribution::setArchs(const QStringList &value)
{
    archs = value;
}

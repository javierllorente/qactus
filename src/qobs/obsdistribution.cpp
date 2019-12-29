/*
 *  Qactus - A Qt based OBS notifier
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

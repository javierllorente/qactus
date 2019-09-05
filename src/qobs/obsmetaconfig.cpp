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

#include "obsmetaconfig.h"

OBSMetaConfig::OBSMetaConfig()
{

}

QString OBSMetaConfig::getName() const
{
    return name;
}

void OBSMetaConfig::setName(const QString &value)
{
    name = value;
}

QString OBSMetaConfig::getTitle() const
{
    return title;
}

void OBSMetaConfig::setTitle(const QString &value)
{
    title = value;
}

QString OBSMetaConfig::getDescription() const
{
    return description;
}

void OBSMetaConfig::setDescription(const QString &value)
{
    description = value;
}

void OBSMetaConfig::insertPerson(const QString &userId, const QString &role)
{
    persons.insert(userId, role);
}

QMultiHash<QString, QString> OBSMetaConfig::getPersons() const
{
    return persons;
}

void OBSMetaConfig::setPersons(const QMultiHash<QString, QString> &value)
{
    persons = value;
}

void OBSMetaConfig::insertGroup(const QString &groupId, const QString &role)
{
    groups.insert(groupId, role);
}

QMultiHash<QString, QString> OBSMetaConfig::getGroups() const
{
    return groups;
}

void OBSMetaConfig::setGroups(const QMultiHash<QString, QString> &value)
{
    groups = value;
}

void OBSMetaConfig::insertBuildFlag(const QString &repository, bool enable)
{
    buildFlag.insert(repository, enable);
}

QHash<QString, bool> OBSMetaConfig::getBuildFlag() const
{
    return buildFlag;
}

void OBSMetaConfig::setBuildFlag(const QHash<QString, bool> &value)
{
    buildFlag = value;
}

void OBSMetaConfig::insertPublishFlag(const QString &repository, bool enable)
{
    publishFlag.insert(repository, enable);
}

QHash<QString, bool> OBSMetaConfig::getPublishFlag() const
{
    return publishFlag;
}

void OBSMetaConfig::setPublishFlag(const QHash<QString, bool> &value)
{
    publishFlag = value;
}

void OBSMetaConfig::insertUseForBuildFlag(const QString &repository, bool enable)
{
    useForBuildFlag.insert(repository, enable);
}

QHash<QString, bool> OBSMetaConfig::getUseForBuildFlag() const
{
    return useForBuildFlag;
}

void OBSMetaConfig::setUseForBuildFlag(const QHash<QString, bool> &value)
{
    useForBuildFlag = value;
}

void OBSMetaConfig::insertDebugInfoFlag(const QString &repository, bool enable)
{
    debugInfoFlag.insert(repository, enable);
}

QHash<QString, bool> OBSMetaConfig::getDebugInfoFlag() const
{
    return debugInfoFlag;
}

void OBSMetaConfig::setDebugInfoFlag(const QHash<QString, bool> &value)
{
    debugInfoFlag = value;
}

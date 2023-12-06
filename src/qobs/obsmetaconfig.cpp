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
#include "obsmetaconfig.h"

OBSMetaConfig::OBSMetaConfig()
{

}

OBSMetaConfig::~OBSMetaConfig()
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

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
#ifndef OBSMETACONFIG_H
#define OBSMETACONFIG_H

#include <QString>
#include <QMultiHash>
#include <QList>
#include "obsrepository.h"

class OBSMetaConfig
{
public:
    OBSMetaConfig();
    virtual ~OBSMetaConfig();

    QString getName() const;
    void setName(const QString &value);

    QString getTitle() const;
    void setTitle(const QString &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    void insertPerson(const QString &userId, const QString &role);
    QMultiHash<QString, QString> getPersons() const;
    void setPersons(const QMultiHash<QString, QString> &value);

    void insertGroup(const QString &groupId, const QString &role);
    QMultiHash<QString, QString> getGroups() const;
    void setGroups(const QMultiHash<QString, QString> &value);

    void insertBuildFlag(const QString &repository, bool enable);
    QHash<QString, bool> getBuildFlag() const;
    void setBuildFlag(const QHash<QString, bool> &value);

    void insertPublishFlag(const QString &repository, bool enable);
    QHash<QString, bool> getPublishFlag() const;
    void setPublishFlag(const QHash<QString, bool> &value);

    void insertUseForBuildFlag(const QString &repository, bool enable);
    QHash<QString, bool> getUseForBuildFlag() const;
    void setUseForBuildFlag(const QHash<QString, bool> &value);

    void insertDebugInfoFlag(const QString &repository, bool enable);
    QHash<QString, bool> getDebugInfoFlag() const;
    void setDebugInfoFlag(const QHash<QString, bool> &value);

private:
    QString name;
    QString title;
    QString description;
    QMultiHash<QString, QString> persons;
    QMultiHash<QString, QString> groups;
    QHash<QString, bool> buildFlag;
    QHash<QString, bool> publishFlag;
    QHash<QString, bool> useForBuildFlag;
    QHash<QString, bool> debugInfoFlag;

};

#endif // OBSMETACONFIG_H

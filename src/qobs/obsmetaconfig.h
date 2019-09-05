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

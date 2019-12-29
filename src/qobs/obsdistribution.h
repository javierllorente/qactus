/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSDISTRIBUTION_H
#define OBSDISTRIBUTION_H
#include <QString>
#include <QList>
#include <QUrl>
#include <QStringList>

class OBSDistribution
{
public:
    OBSDistribution();

    QString getVendor() const;
    void setVendor(const QString &value);

    QString getVersion() const;
    void setVersion(const QString &value);

    QString getId() const;
    void setId(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    QString getProject() const;
    void setProject(const QString &value);

    QString getRepoName() const;
    void setRepoName(const QString &value);

    QString getRepository() const;
    void setRepository(const QString &value);

    QUrl getLink() const;
    void setLink(const QUrl &value);

    void appendIcon(const QUrl &url);
    QList<QUrl> getIcons() const;
    void setIcons(const QList<QUrl> &value);

    void appendArch(const QString &arch);
    QStringList getArchs() const;
    void setArchs(const QStringList &value);

private:
    QString vendor;
    QString version;
    QString id;
    QString name;
    QString project;
    QString repoName;
    QString repository;
    QUrl link;
    QList<QUrl> icons;
    QStringList archs;
};

#endif // OBSDISTRIBUTION_H

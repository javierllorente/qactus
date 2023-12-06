/*
 * Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

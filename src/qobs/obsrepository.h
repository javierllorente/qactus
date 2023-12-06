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
#ifndef OBSREPOSITORY_H
#define OBSREPOSITORY_H

#include <QString>
#include <QStringList>

class OBSRepository
{
public:
    OBSRepository();
    OBSRepository(const QString &name, const QString &project, const QString &repository, const QStringList &archs);
    OBSRepository(const QString &name, const QString &project, const QString &repository, const QString &arch);

    QString getName() const;
    void setName(const QString &value);

    QString getProject() const;
    void setProject(const QString &value);

    QString getRepository() const;
    void setRepository(const QString &value);

    void appendArch(const QString &arch);
    QStringList getArchs() const;
    void setArchs(const QStringList &value);

private:
    QString name;
    QString project;
    QString repository;
    QStringList archs;
};

#endif // OBSREPOSITORY_H

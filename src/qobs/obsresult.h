/*
 * Copyright (C) 2016-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef OBSRESULT_H
#define OBSRESULT_H

#include <QString>
#include <QList>
#include <QSharedPointer>
#include "obsstatus.h"

class OBSResult
{
public:
    OBSResult();
    ~OBSResult();

    inline bool operator==(const OBSResult &other)
    {
        return (project == other.getProject()
                && repository == other.getRepository()
                && arch == other.getArch());
    }

    QString getProject() const;
    void setProject(const QString &value);

    QString getRepository() const;
    void setRepository(const QString &value);

    QString getArch() const;
    void setArch(const QString &value);

    QString getCode() const;
    void setCode(const QString &value);

    QString getState() const;
    void setState(const QString &value);

    QSharedPointer<OBSStatus> getStatus();
    void appendStatus(QSharedPointer<OBSStatus> status);
    QList<QSharedPointer<OBSStatus>> getStatusList() const;

private:
    QString project;
    QString repository;
    QString arch;
    QString code;
    QString state;
    QList<QSharedPointer<OBSStatus>> statusList;
};

#endif // OBSRESULT_H

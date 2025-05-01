/*
 * Copyright (C) 2013-2024 Javier Llorente <javier@opensuse.org>
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
#ifndef OBSREQUEST_H
#define OBSREQUEST_H

#include <QString>
#include <QScopedPointer>
#include "obsobject.h"

class OBSRequest
{
public:
    OBSRequest();
    OBSRequest(const OBSRequest &other);
    ~OBSRequest();
    OBSRequest &operator=(const OBSRequest &other);
    inline bool operator==(const OBSRequest &rhs)
    {
        return this->getId()==rhs.getId();
    }

    void setId(const QString &);
    void setCreator(const QString &);
    void setActionType(const QString &);
    void setSourceProject(const QString &);
    void setSourcePackage(const QString &);
    void setTargetProject(const QString &);
    void setTargetPackage(const QString &);
    void setSourceUpdate(const QString &value);
    void setState(const QString &);
    void setRequester(const QString &);
    void setDate(const QString &);
    void setDescription(const QString &);

    QString getId() const;
    QString getCreator() const;
    QString getActionType() const;
    QString getSourceProject() const;
    QString getSourcePackage() const;
    QString getSource() const;
    QString getTargetProject() const;
    QString getTargetPackage() const;
    QString getTarget() const;
    QString getSourceUpdate() const;
    QString getState() const;
    QString getRequester() const;
    QString getDate() const;
    QString getDescription() const;

private:
    QString id;
    QString creator;
    QString actionType;
    QScopedPointer<OBSObject> source;
    QScopedPointer<OBSObject> target;
    QString sourceUpdate;
    QString state;
    QString requester;
    QString date;
    QString description;
};

#endif // OBSREQUEST_H

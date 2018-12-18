/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2013-2018 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSREQUEST_H
#define OBSREQUEST_H

#include <QString>
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
    void setActionType(const QString &);
    void setSourceProject(const QString &);
    void setSourcePackage(const QString &);
    void setTargetProject(const QString &);
    void setTargetPackage(const QString &);
    void setState(const QString &);
    void setRequester(const QString &);
    void setDate(const QString &);
    void setDescription(const QString &);

    QString getId() const;
    QString getActionType() const;
    QString getSourceProject() const;
    QString getSourcePackage() const;
    QString getSource() const;
    QString getTargetProject() const;
    QString getTargetPackage() const;
    QString getTarget() const;
    QString getState() const;
    QString getRequester() const;
    QString getDate() const;
    QString getDescription() const;

private:
    QString id;
    QString actionType;
    OBSObject *source;
    OBSObject *target;
    QString state;
    QString requester;
    QString date;
    QString description;
};

#endif // OBSREQUEST_H

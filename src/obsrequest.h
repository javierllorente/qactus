/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013, 2015 Javier Llorente <javier@opensuse.org>
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

class OBSRequest
{
public:
    OBSRequest();
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

    QString getId();
    QString getActionType();
    QString getSourceProject();
    QString getSourcePackage();
    QString getSource();
    QString getTargetProject();
    QString getTargetPackage();
    QString getTarget();
    QString getState();
    QString getRequester();
    QString getDate();
    QString getDescription();

private:
    QString id;
    QString actionType;
    QString sourceProject;
    QString sourcePackage;
    QString targetProject;
    QString targetPackage;
    QString state;
    QString requester;
    QString date;
    QString description;
};

#endif // OBSREQUEST_H

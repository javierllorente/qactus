/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2016-2018 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSRESULT_H
#define OBSRESULT_H

#include <QString>
#include "obsstatus.h"

class OBSResult
{
public:
    OBSResult();
    ~OBSResult();

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

    OBSStatus *getStatus() const;

private:
    QString project;
    QString repository;
    QString arch;
    QString code;
    QString state;
    OBSStatus *status;
};

#endif // OBSRESULT_H

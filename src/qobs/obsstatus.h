/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSSTATUS_H
#define OBSSTATUS_H

#include <QString>

class OBSStatus
{
public:
    OBSStatus();
    OBSStatus(const OBSStatus &other);
    OBSStatus &operator=(const OBSStatus &other);
    inline bool operator==(const OBSStatus &rhs)
    {
        return this->getProject()==rhs.getProject() && this->getPackage()==rhs.getPackage() &&
                this->getCode()==rhs.getCode();
    }

    QString getProject() const;
    void setProject(const QString &value);

    QString getPackage() const;
    void setPackage(const QString &value);

    QString getCode() const;
    void setCode(const QString &value);

    QString getSummary() const;
    void setSummary(const QString &value);

    QString getDetails() const;
    void setDetails(const QString &value);

private:
    QString project;
    QString package;
    QString code;
    QString summary;
    QString details;
};

#endif // OBSSTATUS_H

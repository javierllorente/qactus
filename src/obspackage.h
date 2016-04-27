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

#ifndef OBSPACKAGE_H
#define OBSPACKAGE_H

#include <QString>

class OBSPackage
{
public:
    OBSPackage();
    OBSPackage(const OBSPackage &other);
    OBSPackage &operator=(const OBSPackage &other);
    inline bool operator==(const OBSPackage &rhs)
    {
        return this->getName()==rhs.getName();
    }
    void setName(const QString &);
    void setStatus(const QString &);
    void setDetails(const QString &);
    QString getName() const;
    QString getStatus() const;
    QString getDetails() const;

private:
    QString name;
    QString status;
    QString details;
};

#endif // OBSPACKAGE_H

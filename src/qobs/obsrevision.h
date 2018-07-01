/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2017-2018 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSREVISION_H
#define OBSREVISION_H

#include <QString>

class OBSRevision
{
public:
    OBSRevision();

    uint getRev() const;
    void setRev(const uint &value);

    QString getVersion() const;
    void setVersion(const QString &value);

    uint getTime() const;
    void setTime(const uint &value);

    QString getUser() const;
    void setUser(const QString &value);

    QString getComment() const;
    void setComment(const QString &value);

private:
    uint rev;
    QString version;
    uint time;
    QString user;
    QString comment;
};

#endif // OBSREVISION_H

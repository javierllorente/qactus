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

#ifndef OBSFILE_H
#define OBSFILE_H

#include <QString>
#include "obsobject.h"

class OBSFile : public OBSObject
{
public:
    OBSFile();

    QString getName() const;
    void setName(const QString &value);

    QString getSize() const;
    void setSize(const QString &value);

    QString getLastModified() const;
    void setLastModified(const QString &value);

private:
    QString name;
    QString size;
    QString lastModified;
};

#endif // OBSFILE_H

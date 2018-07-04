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

#include "obsrevision.h"

OBSRevision::OBSRevision()
{

}

uint OBSRevision::getRev() const
{
    return rev;
}

void OBSRevision::setRev(const uint &value)
{
    rev = value;
}

QString OBSRevision::getVersion() const
{
    return version;
}

void OBSRevision::setVersion(const QString &value)
{
    version = value;
}

uint OBSRevision::getTime() const
{
    return time;
}

void OBSRevision::setTime(const uint &value)
{
    time = value;
}

QString OBSRevision::getUser() const
{
    return user;
}

void OBSRevision::setUser(const QString &value)
{
    user = value;
}

QString OBSRevision::getComment() const
{
    return comment;
}

void OBSRevision::setComment(const QString &value)
{
    comment = value;
}

QString OBSRevision::getProject() const
{
    return project;
}

void OBSRevision::setProject(const QString &value)
{
    project = value;
}

QString OBSRevision::getPackage() const
{
    return package;
}

void OBSRevision::setPackage(const QString &value)
{
    package = value;
}

QString OBSRevision::getFile() const
{
    return file;
}

void OBSRevision::setFile(const QString &value)
{
    file = value;
}

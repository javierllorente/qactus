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

#include "obsabout.h"

OBSAbout::OBSAbout()
{

}

QString OBSAbout::getTitle() const
{
    return title;
}

void OBSAbout::setTitle(const QString &value)
{
    title = value;
}

QString OBSAbout::getDescription() const
{
    return description;
}

void OBSAbout::setDescription(const QString &value)
{
    description = value;
}

QString OBSAbout::getRevision() const
{
    return revision;
}

void OBSAbout::setRevision(const QString &value)
{
    revision = value;
}

QString OBSAbout::getLastDeployment() const
{
    return lastDeployment;
}

void OBSAbout::setLastDeployment(const QString &value)
{
    lastDeployment = value;
}

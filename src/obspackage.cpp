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

#include "obspackage.h"

OBSPackage::OBSPackage()
{
}

void OBSPackage::setName(const QString& name)
{
    this->name = name;
}

QString OBSPackage::getName()
{
    return name;
}

void OBSPackage::setStatus(const QString& status)
{
    this->status = status;
}

QString OBSPackage::getStatus()
{
    return status;
}

void OBSPackage::setDetails(const QString& details)
{
    this->details = details;
}

QString OBSPackage::getDetails()
{
    return details;
}

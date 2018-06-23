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

#include "obsstatus.h"

OBSStatus::OBSStatus()
{

}

OBSStatus::OBSStatus(const OBSStatus &other)
{
    *this = other;
}

OBSStatus& OBSStatus::operator=(const OBSStatus &other)
{
    this->setProject(other.getProject());
    this->setPackage(other.getPackage());
    this->setCode(other.getCode());
    this->setSummary(other.getSummary());
    this->setDetails(other.getDetails());
    return *this;
}

QString OBSStatus::getProject() const
{
    return project;
}

void OBSStatus::setProject(const QString &value)
{
    project = value;
}

QString OBSStatus::getPackage() const
{
    return package;
}

void OBSStatus::setPackage(const QString &value)
{
    package = value;
}

QString OBSStatus::getCode() const
{
    return code;
}

void OBSStatus::setCode(const QString &value)
{
    code = value;
}

QString OBSStatus::getSummary() const
{
    return summary;
}

void OBSStatus::setSummary(const QString &value)
{
    summary = value;
}

QString OBSStatus::getDetails() const
{
    return details;
}

void OBSStatus::setDetails(const QString &value)
{
    details = value;
}

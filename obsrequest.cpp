/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013 Javier Llorente <javier@opensuse.org>
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

#include "obsrequest.h"

OBSrequest::OBSrequest()
{
}

void OBSrequest::setId(const QString& id)
{
    this->id = id;
}

QString OBSrequest::getId()
{
    return id;
}

void OBSrequest::setActionType(const QString& actionType)
{
    this->actionType = actionType;
}

QString OBSrequest::getActionType()
{
    return actionType;
}

void OBSrequest::setSourceProject(const QString& sourceProject)
{
    this->sourceProject = sourceProject;
}

QString OBSrequest::getSourceProject()
{
    return sourceProject;
}

void OBSrequest::setSourcePackage(const QString& sourcePackage)
{
    this->sourcePackage = sourcePackage;
}

QString OBSrequest::getSourcePackage()
{
    return sourcePackage;
}

QString OBSrequest::getSource()
{
    if (!sourcePackage.isEmpty()) {
        return sourceProject + "/" + sourcePackage;
    } else {
        return "N/A";
    }
}

void OBSrequest::setTargetProject(const QString& targetProject)
{
    this->targetProject = targetProject;
}

QString OBSrequest::getTargetProject()
{
    return targetProject;
}

void OBSrequest::setTargetPackage(const QString& targetPackage)
{
    this->targetPackage = targetPackage;
}

QString OBSrequest::getTargetPackage()
{
    return targetPackage;
}

QString OBSrequest::getTarget()
{
    return targetProject + "/" + targetPackage;
}

void OBSrequest::setState(const QString& state)
{
    this->state = state;
}

QString OBSrequest::getState()
{
    return state;
}

void OBSrequest::setRequester(const QString& requester)
{
    this->requester = requester;
}

QString OBSrequest::getRequester()
{
    return requester;
}

void OBSrequest::setDate(const QString& date)
{
    this->date = date;
}

QString OBSrequest::getDate()
{
    // Replace the "T" (as in 2015-03-13T20:01:33)
    // with a space and return it
    return date.replace(10, 1, " ");
}

void OBSrequest::setDescription(const QString& description)
{
    this->description = description;
}

QString OBSrequest::getDescription()
{
    return description;
}

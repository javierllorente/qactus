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

#include "obsrequest.h"

OBSRequest::OBSRequest()
{
}

void OBSRequest::setId(const QString& id)
{
    this->id = id;
}

QString OBSRequest::getId()
{
    return id;
}

void OBSRequest::setActionType(const QString& actionType)
{
    this->actionType = actionType;
}

QString OBSRequest::getActionType()
{
    return actionType;
}

void OBSRequest::setSourceProject(const QString& sourceProject)
{
    this->sourceProject = sourceProject;
}

QString OBSRequest::getSourceProject()
{
    return sourceProject;
}

void OBSRequest::setSourcePackage(const QString& sourcePackage)
{
    this->sourcePackage = sourcePackage;
}

QString OBSRequest::getSourcePackage()
{
    return sourcePackage;
}

QString OBSRequest::getSource()
{
    if (!sourcePackage.isEmpty()) {
        return sourceProject + "/" + sourcePackage;
    } else {
        return "N/A";
    }
}

void OBSRequest::setTargetProject(const QString& targetProject)
{
    this->targetProject = targetProject;
}

QString OBSRequest::getTargetProject()
{
    return targetProject;
}

void OBSRequest::setTargetPackage(const QString& targetPackage)
{
    this->targetPackage = targetPackage;
}

QString OBSRequest::getTargetPackage()
{
    return targetPackage;
}

QString OBSRequest::getTarget()
{
    return targetProject + "/" + targetPackage;
}

void OBSRequest::setState(const QString& state)
{
    this->state = state;
}

QString OBSRequest::getState()
{
    return state;
}

void OBSRequest::setRequester(const QString& requester)
{
    this->requester = requester;
}

QString OBSRequest::getRequester()
{
    return requester;
}

void OBSRequest::setDate(const QString& date)
{
    this->date = date;
}

QString OBSRequest::getDate()
{
    // Replace the "T" (as in 2015-03-13T20:01:33)
    // with a space and return it
    return date.replace(10, 1, " ");
}

void OBSRequest::setDescription(const QString& description)
{
    this->description = description;
}

QString OBSRequest::getDescription()
{
    return description;
}

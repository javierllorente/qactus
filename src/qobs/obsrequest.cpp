/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2013-2018 Javier Llorente <javier@opensuse.org>
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
    source = new OBSObject();
    target = new OBSObject();
}

OBSRequest::OBSRequest(const OBSRequest &other)
{
    *this = other;
}

OBSRequest::~OBSRequest()
{
    delete source;
    delete target;
}

OBSRequest &OBSRequest::operator=(const OBSRequest &other)
{
    this->setId(other.getId());
    this->setActionType(other.getActionType());
    this->setSourceProject(other.getSourceProject());
    this->setSourcePackage(other.getSourcePackage());
    this->setTargetProject(other.getTargetProject());
    this->setTargetPackage(other.getTargetPackage());
    this->setState(other.getState());
    this->setRequester(other.getRequester());
    this->setDate(other.getDate());
    this->setDescription(other.getDescription());
    return *this;
}

void OBSRequest::setId(const QString &id)
{
    this->id = id;
}

QString OBSRequest::getId() const
{
    return id;
}

void OBSRequest::setActionType(const QString &actionType)
{
    this->actionType = actionType;
}

QString OBSRequest::getActionType() const
{
    return actionType;
}

void OBSRequest::setSourceProject(const QString &sourceProject)
{
    source->setProject(sourceProject);
}

QString OBSRequest::getSourceProject() const
{
    return source->getProject();
}

void OBSRequest::setSourcePackage(const QString &sourcePackage)
{
    source->setPackage(sourcePackage);
}

QString OBSRequest::getSourcePackage() const
{
    return source->getPackage();
}

QString OBSRequest::getSource() const
{
    return source->getPackage().isEmpty() ?
                "N/A" : source->toString();
}

void OBSRequest::setTargetProject(const QString &targetProject)
{
    target->setProject(targetProject);
}

QString OBSRequest::getTargetProject() const
{
    return target->getProject();
}

void OBSRequest::setTargetPackage(const QString &targetPackage)
{
    target->setPackage(targetPackage);
}

QString OBSRequest::getTargetPackage() const
{
    return target->getPackage();
}

QString OBSRequest::getTarget() const
{
    return target->toString();
}

void OBSRequest::setSourceUpdate(const QString &value)
{
    sourceUpdate = value;
}

QString OBSRequest::getSourceUpdate() const
{
    return sourceUpdate;
}

void OBSRequest::setState(const QString &state)
{
    this->state = state;
}

QString OBSRequest::getState() const
{
    return state;
}

void OBSRequest::setRequester(const QString &requester)
{
    this->requester = requester;
}

QString OBSRequest::getRequester() const
{
    return requester;
}

void OBSRequest::setDate(const QString &date)
{
    this->date = date;
}

QString OBSRequest::getDate() const
{
    return date;
}

void OBSRequest::setDescription(const QString &description)
{
    this->description = description;
}

QString OBSRequest::getDescription() const
{
    return description;
}

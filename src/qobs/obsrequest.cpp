/*
 * Copyright (C) 2013-2024 Javier Llorente <javier@opensuse.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "obsrequest.h"

OBSRequest::OBSRequest()
{
    source.reset(new OBSObject());
    target.reset(new OBSObject());
}

OBSRequest::OBSRequest(const OBSRequest &other)
{
    *this = other;
}

OBSRequest::~OBSRequest()
{
}

OBSRequest &OBSRequest::operator=(const OBSRequest &other)
{
    this->setId(other.getId());
    this->setCreator(other.getCreator());
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

void OBSRequest::setCreator(const QString &creator)
{
    this->creator = creator;
}

QString OBSRequest::getCreator() const
{
    return creator;
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

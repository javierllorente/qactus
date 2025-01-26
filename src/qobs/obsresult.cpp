/*
 * Copyright (C) 2016-2025 Javier Llorente <javier@opensuse.org>
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
#include "obsresult.h"

OBSResult::OBSResult()
{

}

OBSResult::~OBSResult()
{

}

QString OBSResult::getProject() const
{
    return project;
}

void OBSResult::setProject(const QString &value)
{
    project = value;
}

QString OBSResult::getRepository() const
{
    return repository;
}

void OBSResult::setRepository(const QString &value)
{
    repository = value;
}

QString OBSResult::getArch() const
{
    return arch;
}

void OBSResult::setArch(const QString &value)
{
    arch = value;
}

QString OBSResult::getCode() const
{
    return code;
}

void OBSResult::setCode(const QString &value)
{
    code = value;
}

QString OBSResult::getState() const
{
    return state;
}

void OBSResult::setState(const QString &value)
{
    state = value;
}

QSharedPointer<OBSStatus> OBSResult::getStatus()
{
    return statusList.first();
}

void OBSResult::appendStatus(QSharedPointer<OBSStatus> status)
{
    statusList.append(status);
}

QList<QSharedPointer<OBSStatus>> OBSResult::getStatusList() const
{
    return statusList;
}

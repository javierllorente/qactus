/*
 * Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

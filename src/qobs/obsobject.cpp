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
#include "obsobject.h"

OBSObject::OBSObject()
{

}

QString OBSObject::getProject() const
{
    return project;
}

void OBSObject::setProject(const QString &value)
{
    project = value;
}

QString OBSObject::getPackage() const
{
    return package;
}

void OBSObject::setPackage(const QString &value)
{
    package = value;
}

QString OBSObject::toString() const
{
     return project + "/" + package;
}

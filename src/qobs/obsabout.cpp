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

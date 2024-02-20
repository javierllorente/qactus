/*
 * Copyright (C) 2017-2024 Javier Llorente <javier@opensuse.org>
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
#include "obsrevision.h"

OBSRevision::OBSRevision()
{
    rev = 0;
    time = 0;
}

uint OBSRevision::getRev() const
{
    return rev;
}

void OBSRevision::setRev(const uint &value)
{
    rev = value;
}

QString OBSRevision::getVersion() const
{
    return version;
}

void OBSRevision::setVersion(const QString &value)
{
    version = value;
}

uint OBSRevision::getTime() const
{
    return time;
}

void OBSRevision::setTime(const uint &value)
{
    time = value;
}

QString OBSRevision::getUser() const
{
    return user;
}

void OBSRevision::setUser(const QString &value)
{
    user = value;
}

QString OBSRevision::getComment() const
{
    return comment;
}

void OBSRevision::setComment(const QString &value)
{
    comment = value;
}

QString OBSRevision::getFile() const
{
    return file;
}

void OBSRevision::setFile(const QString &value)
{
    file = value;
}

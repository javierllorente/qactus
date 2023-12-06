/*
 * Copyright (C) 2016 Javier Llorente <javier@opensuse.org>
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
#include "obsfile.h"

OBSFile::OBSFile()
{

}

QString OBSFile::getName() const
{
    return name;
}

void OBSFile::setName(const QString &value)
{
    name = value;
}

QString OBSFile::getSize() const
{
    return size;
}

void OBSFile::setSize(const QString &value)
{
    size = value;
}

QString OBSFile::getLastModified() const
{
    return lastModified;
}

void OBSFile::setLastModified(const QString &value)
{
    lastModified = value;
}

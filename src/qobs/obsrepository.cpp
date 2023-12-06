/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
#include "obsrepository.h"

OBSRepository::OBSRepository()
{

}

OBSRepository::OBSRepository(const QString &name, const QString &project, const QString &repository, const QStringList &archs)
{
    this->name = name;
    this->project = project;
    this->repository = repository;
    this->archs = archs;
}

OBSRepository::OBSRepository(const QString &name, const QString &project, const QString &repository, const QString &arch)
{
    this->name = name;
    this->project = project;
    this->repository = repository;
    archs.append(arch);
}

QString OBSRepository::getName() const
{
    return name;
}

void OBSRepository::setName(const QString &value)
{
    name = value;
}

QString OBSRepository::getProject() const
{
    return project;
}

void OBSRepository::setProject(const QString &value)
{
    project = value;
}

QString OBSRepository::getRepository() const
{
    return repository;
}

void OBSRepository::setRepository(const QString &value)
{
    repository = value;
}

void OBSRepository::appendArch(const QString &arch)
{
    if (!archs.contains(arch)) {
        archs.append(arch);
    }
}

QStringList OBSRepository::getArchs() const
{
    return archs;
}

void OBSRepository::setArchs(const QStringList &value)
{
    archs = value;
}

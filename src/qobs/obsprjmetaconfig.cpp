/*
 * Copyright (C) 2019-2025 Javier Llorente <javier@opensuse.org>
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
#include "obsprjmetaconfig.h"

OBSPrjMetaConfig::OBSPrjMetaConfig()
{

}

OBSPrjMetaConfig::~OBSPrjMetaConfig()
{

}

void OBSPrjMetaConfig::appendRepository(QSharedPointer<OBSRepository> repository)
{
    repositories.append(repository);
}

QList<QSharedPointer<OBSRepository>> OBSPrjMetaConfig::getRepositories() const
{
    return repositories;
}

void OBSPrjMetaConfig::setRepositories(QList<QSharedPointer<OBSRepository>> value)
{
    repositories = value;
}


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
#ifndef OBSPRJMETACONFIG_H
#define OBSPRJMETACONFIG_H

#include <QString>
#include <QMultiHash>
#include "obsmetaconfig.h"
#include "obsrepository.h"

class OBSPrjMetaConfig : public OBSMetaConfig
{

public:
    OBSPrjMetaConfig();
    ~OBSPrjMetaConfig();

    void appendRepository(OBSRepository *repository);
    QList<OBSRepository *> getRepositories() const;
    void setRepositories(const QList<OBSRepository *> &value);

private:
    QList<OBSRepository *> repositories;
};

#endif // OBSPRJMETACONFIG_H

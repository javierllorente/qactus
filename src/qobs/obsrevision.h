/*
 * Copyright (C) 2017-2020 Javier Llorente <javier@opensuse.org>
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
#ifndef OBSREVISION_H
#define OBSREVISION_H

#include <QString>
#include "obsobject.h"

class OBSRevision : public OBSObject
{
public:
    OBSRevision();

    uint getRev() const;
    void setRev(const uint &value);

    QString getVersion() const;
    void setVersion(const QString &value);

    uint getTime() const;
    void setTime(const uint &value);

    QString getUser() const;
    void setUser(const QString &value);

    QString getComment() const;
    void setComment(const QString &value);

    QString getFile() const;
    void setFile(const QString &value);

private:
    uint rev;
    QString version;
    uint time;
    QString user;
    QString comment;
    QString file;
};

#endif // OBSREVISION_H

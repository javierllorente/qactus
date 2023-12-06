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
#ifndef OBSOBJECT_H
#define OBSOBJECT_H

#include <QString>

class OBSObject
{
public:
    OBSObject();
    inline bool operator==(const OBSObject &other)
    {
        return project==other.getProject() &&
                package==other.getPackage();
    }

    QString getProject() const;
    void setProject(const QString &value);

    QString getPackage() const;
    void setPackage(const QString &value);

    QString toString() const;

private:
    QString project;
    QString package;
};

#endif // OBSOBJECT_H

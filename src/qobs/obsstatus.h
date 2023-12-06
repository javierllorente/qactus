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
#ifndef OBSSTATUS_H
#define OBSSTATUS_H

#include <QString>
#include "obsobject.h"

class OBSStatus : public OBSObject
{
public:
    OBSStatus();
    OBSStatus(const OBSStatus &other);
    OBSStatus &operator=(const OBSStatus &other);
    inline bool operator==(const OBSStatus &rhs)
    {
        return this->getProject()==rhs.getProject() && this->getPackage()==rhs.getPackage() &&
                this->getCode()==rhs.getCode();
    }

    QString getCode() const;
    void setCode(const QString &value);

    QString getSummary() const;
    void setSummary(const QString &value);

    QString getDetails() const;
    void setDetails(const QString &value);

private:
    QString code;
    QString summary;
    QString details;
};

#endif // OBSSTATUS_H

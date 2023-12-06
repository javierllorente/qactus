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
#ifndef OBSABOUT_H
#define OBSABOUT_H

#include <QString>

class OBSAbout
{
public:
    OBSAbout();

    QString getTitle() const;
    void setTitle(const QString &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    QString getRevision() const;
    void setRevision(const QString &value);

    QString getLastDeployment() const;
    void setLastDeployment(const QString &value);

private:
    QString title;
    QString description;
    QString revision;
    QString lastDeployment;
};

#endif // OBSABOUT_H

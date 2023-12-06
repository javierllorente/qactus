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
#ifndef OBSPERSON_H
#define OBSPERSON_H

#include <QString>
#include <QStringList>

class OBSPerson
{
public:
    OBSPerson();

    QString getLogin() const;
    void setLogin(const QString &value);

    QString getEmail() const;
    void setEmail(const QString &value);

    QString getRealName() const;
    void setRealName(const QString &value);

    QString getState() const;
    void setState(const QString &value);

    QStringList getWatchList() const;
    void setWatchList(const QStringList &value);
    void appendWatchItem(const QString &item);
    int removeWatchItem(const QString &item);

private:
    QString login;
    QString email;
    QString realName;
    QString state;
    QStringList watchList;
};

#endif // OBSPERSON_H

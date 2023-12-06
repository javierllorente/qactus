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
#include "obsperson.h"

OBSPerson::OBSPerson()
{

}

QString OBSPerson::getLogin() const
{
    return login;
}

void OBSPerson::setLogin(const QString &value)
{
    login = value;
}

QString OBSPerson::getEmail() const
{
    return email;
}

void OBSPerson::setEmail(const QString &value)
{
    email = value;
}

QString OBSPerson::getRealName() const
{
    return realName;
}

void OBSPerson::setRealName(const QString &value)
{
    realName = value;
}

QString OBSPerson::getState() const
{
    return state;
}

void OBSPerson::setState(const QString &value)
{
    state = value;
}

QStringList OBSPerson::getWatchList() const
{
    return watchList;
}

void OBSPerson::setWatchList(const QStringList &value)
{
    watchList = value;
}

void OBSPerson::appendWatchItem(const QString &item)
{
    watchList.append(item);
}

int OBSPerson::removeWatchItem(const QString &item)
{
    int index = watchList.indexOf(item);
    if (index != -1) {
        watchList.removeAt(index);
    }
    return index;
}

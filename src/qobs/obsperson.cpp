/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

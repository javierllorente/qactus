/*
 *  Qactus - A Qt.based OBS client
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

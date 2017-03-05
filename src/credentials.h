/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2017 Javier Llorente <javier@opensuse.org>
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

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <qt5keychain/keychain.h>
#include <QEventLoop>
#include <QDebug>

class Credentials : public QObject
{
    Q_OBJECT
public:
    explicit Credentials(QObject *parent = 0);

    void readPassword(const QString &username);
    void writeCredentials(const QString &username, const QString &password);
    void deletePassword(const QString &username);

signals:
    void errorReadingPassword(const QString &error);
    void credentialsRestored(const QString &username, const QString &password);
    void errorStoringCredentials(const QString &error);
    void errorDeletingPassword(const QString &error);

public slots:
};

#endif // CREDENTIALS_H

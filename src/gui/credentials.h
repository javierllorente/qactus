/*
 * Copyright (C) 2017-2023 Javier Llorente <javier@opensuse.org>
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
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <qt6keychain/keychain.h>
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

/*
 * Copyright (C) 2017-2025 Javier Llorente <javier@opensuse.org>
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
#include "credentials.h"

Credentials::Credentials(QObject *parent) : QObject(parent)
{

}

void Credentials::readPassword(const QString &username)
{
    qDebug() << Q_FUNC_INFO;
    QKeychain::ReadPasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    QEventLoop loop;
    job.connect(&job, &QKeychain::ReadPasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    const QString pw = job.textData();

    if (job.error()) {
        qDebug() << Q_FUNC_INFO << "Restoring password failed:" << qPrintable(job.errorString());
        emit errorReadingPassword(job.errorString());
    } else {
        qDebug() << Q_FUNC_INFO << "Password restored successfully";
        emit credentialsRestored(job.key(), pw);
    }
}

void Credentials::writeCredentials(const QString &username, const QString &password)
{
    qDebug() << Q_FUNC_INFO;
    QKeychain::WritePasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    job.setTextData(password);
    QEventLoop loop;
    job.connect(&job, &QKeychain::WritePasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        qDebug() << Q_FUNC_INFO << "Storing credentials failed:" << qPrintable(job.errorString());
        emit errorStoringCredentials(job.errorString());
    } else {
        qDebug() << Q_FUNC_INFO << "Credentials stored successfully";
    }
}

void Credentials::deletePassword(const QString &username)
{
    qDebug() << Q_FUNC_INFO;
    QKeychain::DeletePasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    QEventLoop loop;
    job.connect(&job, &QKeychain::DeletePasswordJob::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        qDebug() << Q_FUNC_INFO << "Deleting password failed:" << qPrintable(job.errorString());
        emit errorDeletingPassword(job.errorString());
    } else {
        qDebug() << Q_FUNC_INFO << "Password deleted successfully";
    }
}



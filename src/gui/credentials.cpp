/*
 * Copyright (C) 2017 Javier Llorente <javier@opensuse.org>
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
    qDebug() << "Credentials::readPassword()";
    QKeychain::ReadPasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    QEventLoop loop;
    job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
    job.start();
    loop.exec();
    const QString pw = job.textData();

    if (job.error()) {
        qDebug() << "Restoring password failed:" << qPrintable(job.errorString());
        emit errorReadingPassword(job.errorString());
    } else {
        qDebug() << "Password restored successfully";
        emit credentialsRestored(job.key(), pw);
    }
}

void Credentials::writeCredentials(const QString &username, const QString &password)
{
    qDebug() << "Credentials::writeCredentials()";
    QKeychain::WritePasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    job.setTextData(password);
    QEventLoop loop;
    job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
    job.start();
    loop.exec();

    if (job.error()) {
        qDebug() << "Storing credentials failed:" << qPrintable(job.errorString());
        emit errorStoringCredentials(job.errorString());
    } else {
        qDebug() << "Credentials stored successfully";
    }
}

void Credentials::deletePassword(const QString &username)
{
    qDebug() << "Credentials:deletePassword()";
    QKeychain::DeletePasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(username);
    QEventLoop loop;
    job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
    job.start();
    loop.exec();

    if (job.error()) {
        qDebug() << "Deleting password failed:" << qPrintable(job.errorString());
        emit errorDeletingPassword(job.errorString());
    } else {
        qDebug() << "Password deleted successfully";
    }
}



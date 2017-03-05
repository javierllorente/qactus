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



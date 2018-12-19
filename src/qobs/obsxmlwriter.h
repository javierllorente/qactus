/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#ifndef OBSXMLWRITER_H
#define OBSXMLWRITER_H

#include <QObject>
#include <QXmlStreamWriter>
#include "obsrequest.h"

class OBSXmlWriter : public QObject
{
    Q_OBJECT
public:
    explicit OBSXmlWriter(QObject *parent = nullptr);
    QByteArray createRequest(OBSRequest *obsRequest);
    QByteArray createProjectMeta(const QString &project, const QString &title, const QString &description, const QString &username) const;
    QByteArray createPackageMeta(const QString &project, const QString &package, const QString &title, const QString &description, const QString &username) const;

signals:

public slots:
};

#endif // OBSXMLWRITER_H

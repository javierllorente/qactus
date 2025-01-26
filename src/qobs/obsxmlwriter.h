/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef OBSXMLWRITER_H
#define OBSXMLWRITER_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QSharedPointer>
#include "obsrequest.h"
#include "obsrepository.h"
#include "obsperson.h"
#include "obsprjmetaconfig.h"
#include "obspkgmetaconfig.h"

class OBSXmlWriter : public QObject
{
    Q_OBJECT
public:
    explicit OBSXmlWriter(QObject *parent = nullptr);
    QByteArray createRequest(OBSRequest *obsRequest);
    QByteArray createProjectMeta(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig) const;
    QByteArray createPackageMeta(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig) const;
    QByteArray createLink(const QString &project, const QString &package) const;
    QByteArray createPerson(QSharedPointer<OBSPerson> obsPerson);

private:
    void createRepositoryElement(QXmlStreamWriter &xmlWriter, QSharedPointer<OBSRepository> repository) const;
    void createWatchListElement(QXmlStreamWriter &xmlWriter, const QStringList &watchList);
    void createUserRoles(QXmlStreamWriter &xmlWriter, const QMultiHash<QString, QString> &userRoles, const QString &type) const;
    void createRepositoryFlags(QXmlStreamWriter &xmlWriter, const QHash<QString, bool> &flag, const QString &type) const;

signals:

public slots:
};

#endif // OBSXMLWRITER_H

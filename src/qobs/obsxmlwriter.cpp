/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#include "obsxmlwriter.h"

OBSXmlWriter::OBSXmlWriter(QObject *parent) : QObject(parent)
{

}

QByteArray OBSXmlWriter::createRequest(OBSRequest *obsRequest)
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("request");

    xmlWriter.writeStartElement("action");
    xmlWriter.writeAttribute("type", obsRequest->getActionType());

    xmlWriter.writeEmptyElement("source");
    xmlWriter.writeAttribute("project", obsRequest->getSourceProject());
    xmlWriter.writeAttribute("package", obsRequest->getSourcePackage());

    xmlWriter.writeEmptyElement("target");
    xmlWriter.writeAttribute("project", obsRequest->getTargetProject());
    xmlWriter.writeAttribute("package", obsRequest->getTargetPackage());

    if (!obsRequest->getSourceUpdate().isEmpty()) {
        xmlWriter.writeStartElement("options");
         xmlWriter.writeTextElement("sourceupdate", obsRequest->getSourceUpdate());
        xmlWriter.writeEndElement(); // options
    }

    xmlWriter.writeEndElement(); // action

    xmlWriter.writeTextElement("description", obsRequest->getDescription());

    xmlWriter.writeEndElement(); // request

    return data;
}

void OBSXmlWriter::createRepositoryElement(QXmlStreamWriter &xmlWriter, OBSRepository *repository) const
{
    xmlWriter.writeStartElement("repository");
    xmlWriter.writeAttribute("name", repository->getName());
    xmlWriter.writeEmptyElement("path");
    xmlWriter.writeAttribute("project", repository->getProject());
    xmlWriter.writeAttribute("repository", repository->getRepository());
    for (auto arch : repository->getArchs()) {
        xmlWriter.writeTextElement("arch", arch);
    }
    xmlWriter.writeEndElement();
}

void OBSXmlWriter::createWatchListElement(QXmlStreamWriter &xmlWriter, const QStringList &watchList)
{
    xmlWriter.writeStartElement("watchlist");

    for (QString project : watchList) {
        xmlWriter.writeStartElement("project");
        xmlWriter.writeAttribute("name", project);
        xmlWriter.writeEndElement(); // project
    }

    xmlWriter.writeEndElement(); // watchlist
}

void OBSXmlWriter::createUserRoles(QXmlStreamWriter &xmlWriter, const QMultiHash<QString, QString> &userRoles, const QString &type) const
{
    if (!userRoles.isEmpty()) {
        QString tag = (type == "userid") ? "person" : "group";
        xmlWriter.writeEmptyElement(tag);
        QStringList users = userRoles.keys();

        for (auto user : users) {
            QStringList roles = userRoles.values(user);
            for (auto role : roles) {
                xmlWriter.writeAttribute(type, user);
                xmlWriter.writeAttribute("role", role);
            }
        }
    }
}

QByteArray OBSXmlWriter::createProjectMeta(OBSPrjMetaConfig *prjMetaConfig) const
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("project");
    xmlWriter.writeAttribute("name", prjMetaConfig->getName());

    xmlWriter.writeTextElement("title", prjMetaConfig->getTitle());
    xmlWriter.writeTextElement("description", prjMetaConfig->getDescription());

    createUserRoles(xmlWriter, prjMetaConfig->getPersons(), "userid");
    createUserRoles(xmlWriter, prjMetaConfig->getGroups(), "groupid");


    for (auto repository : prjMetaConfig->getRepositories()) {
        createRepositoryElement(xmlWriter, repository);
    }

    xmlWriter.writeEndElement(); // project

    return data;
}

QByteArray OBSXmlWriter::createPackageMeta(OBSPkgMetaConfig *pkgMetaConfig) const
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("package");
    xmlWriter.writeAttribute("name", pkgMetaConfig->getName());
    xmlWriter.writeAttribute("project", pkgMetaConfig->getProject());

    xmlWriter.writeTextElement("title", pkgMetaConfig->getTitle());
    xmlWriter.writeTextElement("description", pkgMetaConfig->getDescription());

    createUserRoles(xmlWriter, pkgMetaConfig->getPersons(), "userid");
    xmlWriter.writeTextElement("url", pkgMetaConfig->getUrl().toString());

    xmlWriter.writeEndElement(); // package

    return data;
}

QByteArray OBSXmlWriter::createPerson(OBSPerson *obsPerson)
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("person");

    xmlWriter.writeTextElement("login", obsPerson->getLogin());
    xmlWriter.writeTextElement("email", obsPerson->getEmail());
    xmlWriter.writeTextElement("state", obsPerson->getState());

    createWatchListElement(xmlWriter, obsPerson->getWatchList());

    xmlWriter.writeEndElement(); // person

    return data;
}

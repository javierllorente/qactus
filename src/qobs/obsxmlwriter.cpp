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

QByteArray OBSXmlWriter::createProjectMeta(const QString &project, const QString &title, const QString &description, const QString &username) const
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("project");
    xmlWriter.writeAttribute("name", project);

    xmlWriter.writeTextElement("title", title);
    xmlWriter.writeTextElement("description", description);

    xmlWriter.writeEmptyElement("person");
    xmlWriter.writeAttribute("userid", username);
    xmlWriter.writeAttribute("role", "maintainer");

    // openSUSE Tumbleweed
    OBSRepository *twRepository = new OBSRepository();
    twRepository->setName("openSUSE_Tumbleweed");
    twRepository->setProject("openSUSE:Factory");
    twRepository->setRepository("snapshot");
    twRepository->appendArch("x86_64");
    createRepositoryElement(xmlWriter, twRepository);
    delete twRepository;

    // openSUSE Leap   
    OBSRepository *leapRepository = new OBSRepository();
    leapRepository->setName("openSUSE_Current");
    leapRepository->setProject("openSUSE:Current");
    leapRepository->setRepository("standard");
    leapRepository->appendArch("x86_64");
    createRepositoryElement(xmlWriter, leapRepository);
    delete leapRepository;

    xmlWriter.writeEndElement(); // project

    return data;
}

QByteArray OBSXmlWriter::createPackageMeta(const QString &project, const QString &package, const QString &title, const QString &description, const QString &username) const
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("package");
    xmlWriter.writeAttribute("name", package);
    xmlWriter.writeAttribute("project", project);

    xmlWriter.writeTextElement("title", title);
    xmlWriter.writeTextElement("description", description);

    xmlWriter.writeEmptyElement("person");
    xmlWriter.writeAttribute("userid", username);
    xmlWriter.writeAttribute("role", "maintainer");

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

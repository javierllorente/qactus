/*
 *  Qactus - A Qt based OBS notifier
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

#include "obsxmlwriter.h"

OBSXmlWriter::OBSXmlWriter(QObject *parent) : QObject(parent)
{

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
    xmlWriter.writeStartElement("repository");
    xmlWriter.writeAttribute("name", "openSUSE_Tumbleweed");
    xmlWriter.writeEmptyElement("path");
    xmlWriter.writeAttribute("project", "openSUSE:Factory");
    xmlWriter.writeAttribute("repository", "snapshot");
    xmlWriter.writeTextElement("arch", "x86_64");
    xmlWriter.writeEndElement(); // repository

    // openSUSE Leap
    xmlWriter.writeStartElement("repository");
    xmlWriter.writeAttribute("name", "openSUSE_Current");
    xmlWriter.writeEmptyElement("path");
    xmlWriter.writeAttribute("project", "openSUSE:Current");
    xmlWriter.writeAttribute("repository", "standard");
    xmlWriter.writeTextElement("arch", "x86_64");
    xmlWriter.writeEndElement(); // repository

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

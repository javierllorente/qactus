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

void OBSXmlWriter::createRepositoryElement(QXmlStreamWriter &xmlWriter, QSharedPointer<OBSRepository> repository) const
{
    xmlWriter.writeStartElement("repository");
    xmlWriter.writeAttribute("name", repository->getName());
    xmlWriter.writeEmptyElement("path");
    xmlWriter.writeAttribute("project", repository->getProject());
    xmlWriter.writeAttribute("repository", repository->getRepository());
    for (auto&& arch : repository->getArchs()) {
        xmlWriter.writeTextElement("arch", arch);
    }
    xmlWriter.writeEndElement();
}

void OBSXmlWriter::createWatchListElement(QXmlStreamWriter &xmlWriter, const QStringList &watchList)
{
    xmlWriter.writeStartElement("watchlist");

    for (auto&& item : std::as_const(watchList)) {
        if (item.contains("/")) {
            QStringList location = item.split("/");
            xmlWriter.writeStartElement("package");
            xmlWriter.writeAttribute("name", location[1]);
            xmlWriter.writeAttribute("project", location[0]);
            xmlWriter.writeEndElement();
        } else {
            xmlWriter.writeStartElement("project");
            xmlWriter.writeAttribute("name", item);
            xmlWriter.writeEndElement();
        }
    }

    xmlWriter.writeEndElement(); // watchlist
}

void OBSXmlWriter::createUserRoles(QXmlStreamWriter &xmlWriter, const QMultiHash<QString, QString> &userRoles, const QString &type) const
{
    if (!userRoles.isEmpty()) {
        QString tag = (type == "userid") ? "person" : "group";
        QStringList users = userRoles.keys();
        QString userAdded;

        for (auto&& user : std::as_const(users)) {
            QStringList roles = userRoles.values(user);
            if (user!=userAdded) {
                for (auto&& role : std::as_const(roles)) {
                    xmlWriter.writeEmptyElement(tag);
                    xmlWriter.writeAttribute(type, user);
                    xmlWriter.writeAttribute("role", role);
                }
                userAdded=user;
            }
        }
    }
}

void OBSXmlWriter::createRepositoryFlags(QXmlStreamWriter &xmlWriter, const QHash<QString, bool> &flag, const QString &type) const
{
    QStringList repositories = flag.keys();

    if (!flag.isEmpty()) {
        xmlWriter.writeStartElement(type);

        for (auto&& repository : std::as_const(repositories)) {
            bool enabled = flag.value(repository);
            QString enabledStr = enabled ? "enable" : "disable";

            xmlWriter.writeEmptyElement(enabledStr);

            if (repository!="all") {
                xmlWriter.writeAttribute("repository", repository);
            }
        }
        xmlWriter.writeEndElement();
    }
}

QByteArray OBSXmlWriter::createProjectMeta(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig) const
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

    createRepositoryFlags(xmlWriter, prjMetaConfig->getBuildFlag(), "build");
    createRepositoryFlags(xmlWriter, prjMetaConfig->getDebugInfoFlag(), "debuginfo");
    createRepositoryFlags(xmlWriter, prjMetaConfig->getPublishFlag(), "publish");
    createRepositoryFlags(xmlWriter, prjMetaConfig->getUseForBuildFlag(), "useforbuild");

    for (auto&& repository : prjMetaConfig->getRepositories()) {
        createRepositoryElement(xmlWriter, repository);
    }

    xmlWriter.writeEndElement(); // project

    return data;
}

QByteArray OBSXmlWriter::createPackageMeta(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig) const
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
    createUserRoles(xmlWriter, pkgMetaConfig->getGroups(), "groupid");

    createRepositoryFlags(xmlWriter, pkgMetaConfig->getBuildFlag(), "build");
    createRepositoryFlags(xmlWriter, pkgMetaConfig->getDebugInfoFlag(), "debuginfo");
    createRepositoryFlags(xmlWriter, pkgMetaConfig->getPublishFlag(), "publish");
    createRepositoryFlags(xmlWriter, pkgMetaConfig->getUseForBuildFlag(), "useforbuild");

    xmlWriter.writeTextElement("url", pkgMetaConfig->getUrl().toString());

    xmlWriter.writeEndElement(); // package

    return data;
}

QByteArray OBSXmlWriter::createLink(const QString &project, const QString &package) const
{
    QByteArray data;
    QXmlStreamWriter xmlWriter(&data);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartElement("link");
    xmlWriter.writeAttribute("project", project);
    xmlWriter.writeAttribute("package", package);

    xmlWriter.writeStartElement("patches");
    xmlWriter.writeComment("<branch /> for a full copy, default case");
    xmlWriter.writeComment("<apply name=\"patch\" /> apply a patch on the source directory");
    xmlWriter.writeComment("<topadd>%define build_with_feature_x 1</topadd> add a line on the top (spec file only)");
    xmlWriter.writeComment("<add name=\"file.patch\" /> add a patch to be applied after %setup (spec file only)");
    xmlWriter.writeComment("<delete name=\"filename\" /> delete a file");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement(); // link

    return data;
}

QByteArray OBSXmlWriter::createPerson(QSharedPointer<OBSPerson> obsPerson)
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

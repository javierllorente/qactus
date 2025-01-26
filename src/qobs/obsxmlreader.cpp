/*
 * Copyright (C) 2013-2025 Javier Llorente <javier@opensuse.org>
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
#include "obsxmlreader.h"

OBSXmlReader *OBSXmlReader::instance = nullptr;

OBSXmlReader::OBSXmlReader()
{

}

OBSXmlReader *OBSXmlReader::getInstance()
{
    if (!instance) {
        instance = new OBSXmlReader();
    }
    return instance;
}

void OBSXmlReader::addData(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "resultlist" && xml.isStartElement()) {
            parseResultList(data);
        } else if (xml.name().toString() == "status" && xml.isStartElement()) {
            parseBuildStatus(data);
        }
    }
}

void OBSXmlReader::parseProjectList(const QString &userHome, const QString &data)
{
    QXmlStreamReader xml(data);

    QStringList list;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "entry") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                QString entry = attrib.value("name").toString();
                if (!userHome.isEmpty()) {
                    if (entry.startsWith(userHome)) {
                        list.append(entry);
                        emit projectFetched(entry);
                    }
                    if (!entry.startsWith("home")) {
                        list.append(entry);
                        emit projectFetched(entry);
                    }
                } else {
                    list.append(entry);
                    emit projectFetched(entry);
                }
            }
        } // end entry

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
    }

    emit finishedParsingProjectList(list);
}

void OBSXmlReader::parsePrjMetaConfig(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSPrjMetaConfig *prjMetaConfig = nullptr;
    OBSRepository *repository = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "project" && xml.isStartElement()) {
            prjMetaConfig = new OBSPrjMetaConfig();
            QXmlStreamAttributes attrib = xml.attributes();
            prjMetaConfig->setName(attrib.value("name").toString());
        }

        parseMetaConfig(xml, prjMetaConfig);

        if (xml.name().toString() == "repository" && xml.isStartElement()) {
            repository = new OBSRepository();
            QXmlStreamAttributes attrib = xml.attributes();
            repository->setName(attrib.value("name").toString());

            while (xml.readNextStartElement()) {
                if (xml.name().toString() == "path" && xml.isStartElement()) {
                    QXmlStreamAttributes attrib = xml.attributes();
                    repository->setProject(attrib.value("project").toString());
                    repository->setRepository(attrib.value("repository").toString());
                }
            }

            while (xml.readNextStartElement()) {
                if (xml.name().toString() == "arch" && xml.isStartElement()) {
                    repository->appendArch(xml.readElementText());
                }
            }
        }

        if (xml.name().toString() == "repository" && xml.isEndElement()) {
            prjMetaConfig->appendRepository(repository);
        }
    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete prjMetaConfig;
        return;
    }

    emit finishedParsingProjectMetaConfig(prjMetaConfig);
    delete prjMetaConfig;
}

void OBSXmlReader::parsePkgMetaConfig(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSPkgMetaConfig *pkgMetaConfig = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name().toString() == "package" && xml.isStartElement()) {
            pkgMetaConfig = new OBSPkgMetaConfig();
            QXmlStreamAttributes attrib = xml.attributes();
            pkgMetaConfig->setName(attrib.value("name").toString());
            pkgMetaConfig->setProject(attrib.value("project").toString());
        }

        parseMetaConfig(xml, pkgMetaConfig);

        if (xml.name().toString() == "url" && xml.isStartElement()) {
            pkgMetaConfig->setUrl(QUrl(xml.readElementText()));
        }

    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete pkgMetaConfig;
        return;
    }

    emit finishedParsingPackageMetaConfig(pkgMetaConfig);
    delete pkgMetaConfig;
}

void OBSXmlReader::parsePackageList(const QString &data)
{
    QXmlStreamReader xml(data);
    QStringList list = parseList(xml);
    emit finishedParsingPackageList(list);
}

void OBSXmlReader::parseStatus(QXmlStreamReader &xml, QSharedPointer<OBSStatus> status)
{
    if (xml.name().toString() == "status") {
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            if (attrib.hasAttribute("package")) {
                status->setPackage(attrib.value("package").toString());
            }
            status->setCode(attrib.value("code").toString());
        }
    } // end status

    if (xml.name().toString() == "summary" && xml.isStartElement()) {
        xml.readNext();
        status->setSummary(xml.text().toString());
        // If user doesn't exist, return
        if (xml.text().toString().startsWith("Couldn't find User with login")) {
            return;
        }
    } // end summary

    if (xml.name().toString() == "details" && xml.isStartElement()) {
        xml.readNext();
        status->setDetails(xml.text().toString());
    } // end details

    if (xml.name().toString() == "data" && xml.isStartElement()) {
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            if (attrib.hasAttribute("name")) {
                xml.readNext();
                if (attrib.value("name").toString() == "targetproject") {
                    status->setProject(xml.text().toString());
                } else if (attrib.value("name").toString() == "targetpackage") {
                    status->setPackage(xml.text().toString());
                }
            }
        }
    } // end data
}

void OBSXmlReader::parseBuildStatus(const QString &data)
{
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }
    emit finishedParsingPackage(status, row);
}

QSharedPointer<OBSStatus> OBSXmlReader::parseNotFoundStatus(const QString &data)
{
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
    }
    return status;
}

void OBSXmlReader::setPackageRow(int row)
{
    this->row = row;
}

void OBSXmlReader::parseResultList(const QString &data)
{
    qDebug() << Q_FUNC_INFO;

    QXmlStreamReader xml(data);
    QList<OBSResult *> resultList;
    OBSResult *obsResult = nullptr;
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {

            if (xml.name().toString() == "resultlist") {
                xml.readNextStartElement();
            }

            if (xml.name().toString() == "result") {
                obsResult = new OBSResult();
                QXmlStreamAttributes attrib = xml.attributes();
                obsResult->setProject(attrib.value("project").toString());
                obsResult->setRepository(attrib.value("repository").toString());
                obsResult->setArch(attrib.value("arch").toString());
                obsResult->setCode(attrib.value("code").toString());
                obsResult->setState(attrib.value("state").toString());
                resultList.append(obsResult);
            }

            if (xml.name().toString() == "status") {
                status = QSharedPointer<OBSStatus>(new OBSStatus());
                obsResult->appendStatus(status);
            }

            parseStatus(xml, status);
        }

        if (xml.name().toString() == "result" && xml.isEndElement()) {
            emit finishedParsingResult(obsResult);
        }

        if (xml.name().toString() == "resultlist" && xml.isEndElement()) {
            emit finishedParsingResultList(resultList);
            qDeleteAll(resultList);
            resultList.clear();
        }
    }
}

void OBSXmlReader::parseRequestStatus(const QString &data)
{
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingRequestStatus(status);
}


void OBSXmlReader::parseRequests(const QString &project, const QString &package, const QString &data)
{
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        parseCollection(xml);

        if (xml.name().toString() == "request") {
                obsRequest = parseRequest(xml);
        }

        if (xml.name().toString() == "request" && xml.isEndElement()) {
            emit finishedParsingRequest(obsRequest);
            delete obsRequest;
        }
    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete obsRequest;
        return;
    }

    emit finishedParsingRequestList(project, package);
}

void OBSXmlReader::parseBranchPackage(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << data;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingBranchPackage(status);
}

void OBSXmlReader::parseLinkPackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSRevision> revision = QSharedPointer<OBSRevision>(new OBSRevision());
    revision->setProject(project);
    revision->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseRevision(xml, revision);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingLinkPkgRevision(revision);
}

void OBSXmlReader::parseCopyPackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSRevision> revision = QSharedPointer<OBSRevision>(new OBSRevision());
    revision->setProject(project);
    revision->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseRevision(xml, revision);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCopyPkgRevision(revision);
}

void OBSXmlReader::parseCreateRequest(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.name().toString() == "request") {
            obsRequest = parseRequest(xml);
        }
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete obsRequest;
        return;
    }

    emit finishedParsingCreateRequest(obsRequest);
    delete obsRequest;
}

void OBSXmlReader::parseCreateRequestStatus(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreateRequestStatus(status);
}

void OBSXmlReader::parseCreateProject(const QString &project, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());
    status->setProject(project);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreatePrjStatus(status);
}

void OBSXmlReader::parseCreatePackage(const QString &project, const QString &package,const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());
    status->setProject(project);
    status->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreatePkgStatus(status);
}

void OBSXmlReader::parseUploadFile(const QString &project, const QString &package, const QString &file, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSRevision> revision = QSharedPointer<OBSRevision>(new OBSRevision());
    revision->setProject(project);
    revision->setPackage(package);
    revision->setFile(file);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseRevision(xml, revision);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingUploadFileRevision(revision);
}

void OBSXmlReader::parseDeleteProject(const QString &project, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());
    status->setProject(project);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeletePrjStatus(status);
}

void OBSXmlReader::parseDeletePackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());
    status->setProject(project);
    status->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeletePkgStatus(status);
}

void OBSXmlReader::parseDeleteFile(const QString &project, const QString &package, const QString &fileName, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());
    status->setProject(project);
    status->setPackage(package);
    status->setDetails(fileName);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeleteFileStatus(status);
}

void OBSXmlReader::parseRevision(QXmlStreamReader &xml, QSharedPointer<OBSRevision> revision)
{
    if (xml.name().toString() == "revision") {
        QXmlStreamAttributes attrib = xml.attributes();
        revision->setRev(attrib.value("rev").toUInt());
    }
    if (xml.name().toString() == "version") {
        xml.readNext();
        revision->setVersion(xml.text().toString());
    }
    if (xml.name().toString() == "time") {
        xml.readNext();
        revision->setTime(xml.text().toUInt());
    }
    if (xml.name().toString() == "user") {
        xml.readNext();
        revision->setUser(xml.text().toString());
    }
    if (xml.name().toString() == "comment") {
        xml.readNext();
        revision->setComment(xml.text().toString());
    }
}

void OBSXmlReader::parseRevisionList(const QString &project, const QString &package, const QString &data)
{
    QXmlStreamReader xml(data);
    QSharedPointer<OBSRevision> revision = QSharedPointer<OBSRevision>();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == "revisionlist") {
                xml.readNextStartElement();
            }

            if (xml.name().toString() == "revision") {
                revision = QSharedPointer<OBSRevision>(new OBSRevision());
                revision->setProject(project);
                revision->setPackage(package);
            }
            parseRevision(xml, revision);
        }

        if (xml.name().toString() == "revision" && xml.isEndElement()) {
            emit finishedParsingRevision(revision);
        }
    }
    emit finishedParsingRevisionList(project, package);
}

void OBSXmlReader::parseLatestRevision(const QString &project, const QString &package, const QString &data)
{
    QXmlStreamReader xml(data);
    QSharedPointer<OBSRevision> revision = QSharedPointer<OBSRevision>();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == "revision") {
                revision = QSharedPointer<OBSRevision>(new OBSRevision());
                revision->setProject(project);
                revision->setPackage(package);
            }
            parseRevision(xml, revision);
        }

        if (xml.name().toString() == "revision" && xml.isEndElement()) {
            emit finishedParsingLatestRevision(revision);
        }
    }
}

void OBSXmlReader::parseCollection(QXmlStreamReader &xml)
{
    if (xml.name().toString() == "collection") {
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            QStringView matches = attrib.value("matches");
            QStringView code = attrib.value("code");
            requestNumber = matches.toString();

            qDebug() << Q_FUNC_INFO << "Collection matches:" << requestNumber;
        }
    } // collection
}

void OBSXmlReader::parseIncomingRequests(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        parseCollection(xml);

        if (xml.name().toString() == "request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name().toString() == "request" && xml.isEndElement()) {
                emit finishedParsingIncomingRequest(obsRequest);
                delete obsRequest;
            }
        } // request

        if (xml.name().toString() == "collection" && xml.isEndElement()) {
            emit finishedParsingIncomingRequestList();
        }
    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }
}

void OBSXmlReader::parseOutgoingRequests(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        parseCollection(xml);

        if (xml.name().toString() == "request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name().toString() == "request" && xml.isEndElement()) {
                emit finishedParsingOutgoingRequest(obsRequest);
                delete obsRequest;
            }
        } // request

        if (xml.name().toString() == "collection" && xml.isEndElement()) {
            emit finishedParsingOutgoingRequestList();
        }

    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete obsRequest;
        return;
    }
}

void OBSXmlReader::parseDeclinedRequests(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        parseCollection(xml);

        if (xml.name().toString() == "request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name().toString() == "request" && xml.isEndElement()) {
                emit finishedParsingDeclinedRequest(obsRequest);
                delete obsRequest;
            }
        } // request

        if (xml.name().toString() == "collection" && xml.isEndElement()) {
            emit finishedParsingDeclinedRequestList();
        }

    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete obsRequest;
        return;
    }
}

OBSRequest *OBSXmlReader::parseRequest(QXmlStreamReader &xml)
{
    OBSRequest *obsRequest = nullptr;

    if (xml.name().toString() == "request") {
        if (xml.isStartElement()) {
            obsRequest = new OBSRequest();
            QXmlStreamAttributes attrib = xml.attributes();
            obsRequest->setId(attrib.value("id").toString());
            obsRequest->setCreator(attrib.value("creator").toString());
        }
    } // request

    while (!(xml.name().toString() == "request" && xml.isEndElement())) {
        xml.readNext();

        if (xml.name().toString() == "action")  {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setActionType(attrib.value("type").toString());
            }
        } // action

        if (xml.name().toString() == "source") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setSourceProject(attrib.value("project").toString());
                obsRequest->setSourcePackage(attrib.value("package").toString());
            }
        } // source

        if (xml.name().toString() == "target") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setTargetProject(attrib.value("project").toString());
                obsRequest->setTargetPackage(attrib.value("package").toString());
            }
        } // target

        if (xml.name().toString() == "state") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setState(attrib.value("name").toString());
                obsRequest->setRequester(attrib.value("who").toString());
                QString date = attrib.value("when").toString();
                // Replace the "T" (as in 2015-03-13T20:01:33)
                date.replace(10, 1, " ");
                obsRequest->setDate(date);
            }
        } // state

        if (xml.name().toString() == "description") {
            if (xml.isStartElement()) {
                xml.readNext();
                obsRequest->setDescription(xml.text().toString());
                // if tag is not empty (ie: <description/>), read next start element
                if (!xml.text().isEmpty()) {
                    xml.readNextStartElement();
                }
            }
        } // description
    }
    return obsRequest;
}

QStringList OBSXmlReader::parseList(QXmlStreamReader &xml)
{
    qDebug() << Q_FUNC_INFO;
    QStringList list;
    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name().toString() == "entry") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("code").toString() == "unregistered_ichain_user") {
                    qDebug() << Q_FUNC_INFO << "Unregistered username!";
                } else {
                    list.append(attrib.value("name").toString());
                }
            }
        } // end entry

        if (xml.name().toString() == "repository") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("code").toString() == "unregistered_ichain_user") {
                    qDebug() << Q_FUNC_INFO << "Unregistered username!";
                } else {
                    list.append(attrib.value("name").toString());
                }
            }
        } // end repository

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
    }

    return list;
}

void OBSXmlReader::parseMetaConfig(QXmlStreamReader &xml, OBSMetaConfig *metaConfig)
{
    if (xml.name().toString() == "title" && xml.isStartElement()) {
        metaConfig->setTitle(xml.readElementText());
    }

    if (xml.name().toString() == "description" && xml.isStartElement()) {
        metaConfig->setDescription(xml.readElementText());
    }

    if (xml.name().toString() == "person" && xml.isStartElement()) {
        QXmlStreamAttributes attrib = xml.attributes();
        metaConfig->insertPerson(attrib.value("userid").toString(),
                                 attrib.value("role").toString());
    }

    if (xml.name().toString() == "group" && xml.isStartElement()) {
        QXmlStreamAttributes attrib = xml.attributes();
        metaConfig->insertGroup(attrib.value("groupid").toString(),
                                attrib.value("role").toString());
    }

    if (xml.name().toString() == "build" && xml.isStartElement()) {
        metaConfig->setBuildFlag(parseRepositoryFlags(xml));
    }

    if (xml.name().toString() == "publish" && xml.isStartElement()) {
        metaConfig->setPublishFlag(parseRepositoryFlags(xml));
    }

    if (xml.name().toString() == "useforbuild" && xml.isStartElement()) {
        metaConfig->setUseForBuildFlag(parseRepositoryFlags(xml));
    }

    if (xml.name().toString() == "debuginfo" && xml.isStartElement()) {
        metaConfig->setDebugInfoFlag(parseRepositoryFlags(xml));
    }
}

QHash<QString, bool> OBSXmlReader::parseRepositoryFlags(QXmlStreamReader &xml)
{
    QHash<QString, bool> flagHash;
    while (xml.readNextStartElement()) {
        if (xml.name().toString() == "enable") {
            QXmlStreamAttributes attrib = xml.attributes();
            if (!attrib.hasAttribute("repository")) {
                flagHash.insert("all", true);
            } else {
                QString repository = attrib.value("repository").toString();
                flagHash.insert(repository, true);
            }
            xml.readNext();
        }

        if (xml.name().toString() == "disable") {
            QXmlStreamAttributes attrib = xml.attributes();
            if (!attrib.hasAttribute("repository")) {
                flagHash.insert("all", false);
            } else {
                QString repository = attrib.value("repository").toString();
                flagHash.insert(repository, false);
            }
            xml.readNext();
        }
    }

    return flagHash;
}

OBSDistribution *OBSXmlReader::parseDistribution(QXmlStreamReader &xml)
{
    OBSDistribution *distribution = nullptr;

    if (xml.name().toString() == "distribution" && xml.isStartElement()) {
        distribution = new OBSDistribution();
        QXmlStreamAttributes attrib = xml.attributes();
        distribution->setVendor(attrib.value("vendor").toString());
        distribution->setVersion(attrib.value("version").toString());
        distribution->setId(attrib.value("id").toString());
    }

    while (!(xml.name().toString() == "distribution" && xml.isEndElement())) {
        xml.readNext();

        if (xml.name().toString() == "name" && xml.isStartElement()) {
            distribution->setName(xml.readElementText());
        }

        if (xml.name().toString() == "project" && xml.isStartElement()) {
            distribution->setProject(xml.readElementText());
        }

        if (xml.name().toString() == "reponame" && xml.isStartElement()) {
            distribution->setRepoName(xml.readElementText());
        }

        if (xml.name().toString() == "repository" && xml.isStartElement()) {
            distribution->setRepository(xml.readElementText());
        }

        if (xml.name().toString() == "link" && xml.isStartElement()) {
            distribution->setLink(QUrl(xml.readElementText()));
        }

        if (xml.name().toString() == "icon" && xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            distribution->appendIcon(QUrl(attrib.value("url").toString()));
        }

        if (xml.name().toString() == "architecture" && xml.isStartElement()) {
            distribution->appendArch(xml.readElementText());
        }
    }

    return distribution;
}

void OBSXmlReader::parseFileList(const QString &project, const QString &package, const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name().toString() == "entry") {
            OBSFile *obsFile = nullptr;
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsFile = new OBSFile();
                obsFile->setProject(project);
                obsFile->setPackage(package);
                obsFile->setName(attrib.value("name").toString());
                obsFile->setSize(attrib.value("size").toString());
                obsFile->setLastModified(attrib.value("mtime").toString());
                emit finishedParsingFile(obsFile);
                delete obsFile;
            }
        } // end entry

    } // end while
    emit finishedParsingFileList(project, package);
}

void OBSXmlReader::parseLink(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    OBSLink *obsLink = new OBSLink();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name().toString() == "link") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsLink->setProject(attrib.value("project").toString());
                obsLink->setPackage(attrib.value("package").toString());
            }
        } // end link

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        delete obsLink;
        return;
    }

    emit finishedParsingLink(obsLink);
}

void OBSXmlReader::parseAbout(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSAbout *obsAbout = new OBSAbout();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name().toString() == "title" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setTitle(xml.text().toString());
        }

        if (xml.name().toString() == "description" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setDescription(xml.text().toString());
        }

        if (xml.name().toString() == "revision" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setRevision(xml.text().toString());
        }

        if (xml.name().toString() == "last_deployment" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setLastDeployment(xml.text().toString());
        }

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingAbout(obsAbout);
}

void OBSXmlReader::parsePerson(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSPerson *obsPerson = new OBSPerson();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name().toString() == "login" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setLogin(xml.text().toString());
        }

        if (xml.name().toString() == "email" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setEmail(xml.text().toString());
        }

        if (xml.name().toString() == "realname" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setRealName(xml.text().toString());
        }

        if (xml.name().toString() == "state" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setState(xml.text().toString());
        }

        if (xml.name().toString() == "project") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsPerson->appendWatchItem(attrib.value("name").toString());
            }
        }

        if (xml.name().toString() == "package") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                QString location = attrib.value("project").toString()
                        + "/" + attrib.value("name").toString();
                obsPerson->appendWatchItem(location);
            }
        }

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingPerson(obsPerson);
}

void OBSXmlReader::parseUpdatePerson(const QString &data)
{
    qDebug() << Q_FUNC_INFO;
    QXmlStreamReader xml(data);
    QSharedPointer<OBSStatus> status = QSharedPointer<OBSStatus>(new OBSStatus());

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, status);
    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingUpdatePerson(status);
}

void OBSXmlReader::parseDistributions(const QString &data)
{
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.name().toString() == "distributions" && xml.isStartElement()) {
            xml.readNextStartElement();
        }
        if (xml.name().toString() == "distribution" && xml.isStartElement()) {
            OBSDistribution *distribution = parseDistribution(xml);
            emit finishedParsingDistribution(distribution);
        }

    } // end while

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << "Error parsing XML!" << xml.errorString();
        return;
    }
}

int OBSXmlReader::getRequestNumber()
{
    return requestNumber.toInt();
}

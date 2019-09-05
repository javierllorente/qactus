/* 
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2013-2019 Javier Llorente <javier@opensuse.org>
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
    qDebug() << "OBSXmlReader::addData()";
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name()=="resultlist" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: resultlist tag found";
            parseResultList(data);
            break;

        } else if (xml.name()=="revisionlist" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: revisionlist tag found";
            parseRevisionList(data);
        } else if (xml.name()=="status" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: status tag found";
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

        if (xml.name()=="entry") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                QString entry = attrib.value("name").toString();
//                qDebug() << "Entry name: " << entry;
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
        qDebug() << "Error parsing XML!" << xml.errorString();
    }

    emit finishedParsingProjectList(list);
}

void OBSXmlReader::parseProjectMetadata(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSPrjMetaConfig *prjMetaConfig = nullptr;
    OBSRepository *repository = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name()=="project" && xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            prjMetaConfig = new OBSPrjMetaConfig();
            prjMetaConfig->setName(attrib.value("name").toString());
        }

        if (xml.name()=="title" && xml.isStartElement()) {
            prjMetaConfig->setTitle(xml.readElementText());
        }

        if (xml.name()=="description" && xml.isStartElement()) {
            prjMetaConfig->setDescription(xml.readElementText());
        }

        if (xml.name()=="person" && xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            prjMetaConfig->insertPerson(attrib.value("userid").toString(),
                                        attrib.value("role").toString());
        }

        if (xml.name()=="group" && xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            prjMetaConfig->insertGroup(attrib.value("groupid").toString(),
                                       attrib.value("role").toString());
        }

        if (xml.name()=="build" && xml.isStartElement()) {
            prjMetaConfig->setBuildFlag(parseRepositoryFlags(xml));
        }

        if (xml.name()=="publish" && xml.isStartElement()) {
            prjMetaConfig->setPublishFlag(parseRepositoryFlags(xml));
        }

        if (xml.name()=="useforbuild" && xml.isStartElement()) {
            prjMetaConfig->setUseForBuildFlag(parseRepositoryFlags(xml));
        }

        if (xml.name()=="debuginfo" && xml.isStartElement()) {
            prjMetaConfig->setDebugInfoFlag(parseRepositoryFlags(xml));
        }


        if (xml.name()=="repository" && xml.isStartElement()) {
            repository = new OBSRepository();
            QXmlStreamAttributes attrib = xml.attributes();
            repository->setName(attrib.value("name").toString());

            while (xml.readNextStartElement()) {
                if (xml.name()=="path" && xml.isStartElement()) {
                    QXmlStreamAttributes attrib = xml.attributes();
                    repository->setProject(attrib.value("project").toString());
                    repository->setRepository(attrib.value("repository").toString());
                }
            }

            while (xml.readNextStartElement()) {
                if (xml.name()=="arch" && xml.isStartElement()) {
                    repository->appendArch(xml.readElementText());
                }
            }
        }

        if (xml.name()=="repository" && xml.isEndElement()) {
            prjMetaConfig->appendRepository(repository);
        }
    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingProjectMetadata(prjMetaConfig);
}

void OBSXmlReader::parsePackageList(const QString &data)
{
    QXmlStreamReader xml(data);
    QStringList list = parseList(xml);
    emit finishedParsingPackageList(list);
}

void OBSXmlReader::parseStatus(QXmlStreamReader &xml, OBSStatus *obsStatus)
{
    qDebug() << "OBSXmlReader::parseStatus()";

    if (xml.name()=="status") {
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            if (attrib.hasAttribute("package")) {
                obsStatus->setPackage(attrib.value("package").toString());
            }
            obsStatus->setCode(attrib.value("code").toString());
            qDebug() << "Package:" << obsStatus->getPackage() << "Status code:" << obsStatus->getCode();

        }
    } // end status

    if (xml.name()=="summary" && xml.isStartElement()) {
        xml.readNext();
        obsStatus->setSummary(xml.text().toString());
        qDebug() << "Status summary:" << obsStatus->getSummary();
        // If user doesn't exist, return
        if (xml.text().toString().startsWith("Couldn't find User with login")) {
            return;
        }
    } // end summary

    if (xml.name()=="details" && xml.isStartElement()) {
        xml.readNext();
        obsStatus->setDetails(xml.text().toString());
        qDebug() << "Status details:" << obsStatus->getDetails();
    } // end details

}

void OBSXmlReader::parseBuildStatus(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }
    emit finishedParsingPackage(obsStatus, row);
}

OBSStatus *OBSXmlReader::parseNotFoundStatus(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
    }
    return obsStatus;
}

void OBSXmlReader::setPackageRow(int row)
{
    this->row = row;
}

void OBSXmlReader::parseResultList(const QString &data)
{
    qDebug() << "OBSXmlReader::parseResultList()";
    QXmlStreamReader xml(data);
    OBSResult *obsResult = nullptr;
    OBSStatus *obsStatus = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {

            if (xml.name()=="resultlist") {
                xml.readNextStartElement();
            }

            if (xml.name()=="result") {
                obsResult = new OBSResult();
                obsStatus = obsResult->getStatus();
                QXmlStreamAttributes attrib = xml.attributes();
                obsResult->setProject(attrib.value("project").toString());
                obsResult->setRepository(attrib.value("repository").toString());
                obsResult->setArch(attrib.value("arch").toString());
                obsResult->setCode(attrib.value("code").toString());
                obsResult->setState(attrib.value("state").toString());
                qDebug() << obsResult->getProject()
                         << obsResult->getRepository()
                         << obsResult->getArch()
                         << obsResult->getCode()
                         << obsResult->getState();
            }

            parseStatus(xml, obsStatus);

            if (xml.name()=="details") {
                xml.readNext();
                obsResult->getStatus()->setDetails(xml.text().toString());
                qDebug() << "Details:" << obsResult->getStatus()->getDetails();
            }
        }

        if (xml.name()=="result" && xml.isEndElement()) {
            emit finishedParsingResult(obsResult);
        }

        if (xml.name()=="resultlist" && xml.isEndElement()) {
            emit finishedParsingResultList();
        }
    }
}

void OBSXmlReader::parseSubmitRequest(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus );
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingSR(obsStatus);
}

void OBSXmlReader::parseBranchPackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << "OBSXmlReader::parseBranchPackage()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);
    obsStatus->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingBranchPackage(obsStatus);
}

void OBSXmlReader::parseCopyPackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << __PRETTY_FUNCTION__;
    QXmlStreamReader xml(data);
    OBSRevision *obsRevision = new OBSRevision();
    obsRevision->setProject(project);
    obsRevision->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseRevision(xml, obsRevision);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCopyPkgRevision(obsRevision);
}

void OBSXmlReader::parseCreateRequest(const QString &data)
{
    qDebug() << "OBSXmlReader::parseCreateRequest()";
    QXmlStreamReader xml(data);
    OBSRequest *obsRequest = nullptr;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.name()=="request") {
            obsRequest = parseRequest(xml);
        }
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreateRequest(obsRequest);
}

void OBSXmlReader::parseCreateRequestStatus(const QString &data)
{
    qDebug() << "OBSXmlReader::parseCreateRequestStatus()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreateRequestStatus(obsStatus);
}

void OBSXmlReader::parseCreateProject(const QString &project, const QString &data)
{
    qDebug() << "OBSXmlReader::parseCreateProject()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreatePrjStatus(obsStatus);
}

void OBSXmlReader::parseCreatePackage(const QString &project, const QString &package,const QString &data)
{
    qDebug() << "OBSXmlReader::parseCreatePackage()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);
    obsStatus->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingCreatePkgStatus(obsStatus);
}

void OBSXmlReader::parseUploadFile(const QString &project, const QString &package, const QString &file, const QString &data)
{
    qDebug() << "OBSXmlReader::parseUploadFile()";
    QXmlStreamReader xml(data);
    OBSRevision *obsRevision = new OBSRevision();
    obsRevision->setProject(project);
    obsRevision->setPackage(package);
    obsRevision->setFile(file);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseRevision(xml, obsRevision);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingUploadFileRevision(obsRevision);
}

void OBSXmlReader::parseDeleteProject(const QString &project, const QString &data)
{
    qDebug() << "OBSXmlReader::parseDeleteProject()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeletePrjStatus(obsStatus);
}

void OBSXmlReader::parseDeletePackage(const QString &project, const QString &package, const QString &data)
{
    qDebug() << "OBSXmlReader::parseDeletePackage()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);
    obsStatus->setPackage(package);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeletePkgStatus(obsStatus);
}

void OBSXmlReader::parseDeleteFile(const QString &project, const QString &package, const QString &fileName, const QString &data)
{
    qDebug() << "OBSXmlReader::parseDeleteFile()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();
    obsStatus->setProject(project);
    obsStatus->setPackage(package);
    obsStatus->setDetails(fileName);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingDeleteFileStatus(obsStatus);
}

void OBSXmlReader::parseRevision(QXmlStreamReader &xml, OBSRevision *obsRevision)
{
    if (xml.name()=="revision") {
        QXmlStreamAttributes attrib = xml.attributes();
        obsRevision->setRev(attrib.value("rev").toUInt());
    }
    if (xml.name()==("version")) {
        xml.readNext();
        obsRevision->setVersion(xml.text().toString());
    }
    if (xml.name()==("time")) {
        xml.readNext();
        obsRevision->setTime(xml.text().toUInt());
    }
    if (xml.name()==("user")) {
        xml.readNext();
        obsRevision->setUser(xml.text().toString());
    }
    if (xml.name()==("comment")) {
        xml.readNext();
        obsRevision->setComment(xml.text().toString());
    }
}

void OBSXmlReader::parseRevisionList(const QString &data)
{
    QXmlStreamReader xml(data);
    OBSRevision *obsRevision = NULL;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement()) {

            if (xml.name()=="revisionlist") {
                xml.readNextStartElement();
            }

            obsRevision = new OBSRevision();
            parseRevision(xml, obsRevision);

        }

        if (xml.name()=="revision" && xml.isEndElement()) {
            emit finishedParsingRevision(obsRevision);
        }
    }
}

void OBSXmlReader::parseCollection(QXmlStreamReader &xml)
{
    if (xml.name()=="collection") {
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrib = xml.attributes();
            QStringRef matches = attrib.value("matches");
            QStringRef code = attrib.value("code");
            requestNumber = matches.toString();

            qDebug() << "Collection matches:" << requestNumber;
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

        if (xml.name()=="request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name()=="request" && xml.isEndElement()) {
                emit finishedParsingIncomingRequest(obsRequest);
            }
        } // request

        if (xml.name()=="collection" && xml.isEndElement()) {
            emit finishedParsingIncomingRequestList();
        }
    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
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

        if (xml.name()=="request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name()=="request" && xml.isEndElement()) {
                emit finishedParsingOutgoingRequest(obsRequest);
            }
        } // request

        if (xml.name()=="collection" && xml.isEndElement()) {
            emit finishedParsingOutgoingRequestList();
        }

    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
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

        if (xml.name()=="request" && xml.isStartElement()) {
            obsRequest = parseRequest(xml);
            if (xml.name()=="request" && xml.isEndElement()) {
                emit finishedParsingDeclinedRequest(obsRequest);
            }
        } // request

        if (xml.name()=="collection" && xml.isEndElement()) {
            emit finishedParsingDeclinedRequestList();
        }

    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }
}

OBSRequest *OBSXmlReader::parseRequest(QXmlStreamReader &xml)
{
    OBSRequest *obsRequest = nullptr;

    if (xml.name()=="request") {
        if (xml.isStartElement()) {
            obsRequest = new OBSRequest();
            QXmlStreamAttributes attrib = xml.attributes();
            obsRequest->setId(attrib.value("id").toString());
        }
    } // request

    while (!(xml.name()=="request" && xml.isEndElement())) {
        xml.readNext();

        if (xml.name()=="action")  {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setActionType(attrib.value("type").toString());
                qDebug() << "Action type:" <<  obsRequest->getActionType();
            }
        } // action

        if (xml.name()=="source") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setSourceProject(attrib.value("project").toString());
                obsRequest->setSourcePackage(attrib.value("package").toString());
                qDebug() << "Source: " <<  obsRequest->getSource();
            }
        } // source

        if (xml.name()=="target") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setTargetProject(attrib.value("project").toString());
                obsRequest->setTargetPackage(attrib.value("package").toString());
                qDebug() << "Target: " <<  obsRequest->getTarget();
            }
        } // target

        if (xml.name()=="state") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setState(attrib.value("name").toString());
                qDebug() << "State: " <<  obsRequest->getState();
                obsRequest->setRequester(attrib.value("who").toString());
                qDebug() << "Requester: " <<  obsRequest->getRequester();
                QString date = attrib.value("when").toString();
                // Replace the "T" (as in 2015-03-13T20:01:33)
                date.replace(10, 1, " ");
                obsRequest->setDate(date);
                qDebug() << "Date: " <<  obsRequest->getDate();
            }
        } // state

        if (xml.name()=="description") {
            if (xml.isStartElement()) {
                xml.readNext();
                obsRequest->setDescription(xml.text().toString());
                qDebug() << "Description:\n" <<  obsRequest->getDescription();
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
    qDebug() << "OBSXmlReader::parseList()";
    QStringList list;
    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name()=="entry") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("code").toString() == "unregistered_ichain_user") {
                    qDebug() << "Unregistered username!";
                } else {
//                    qDebug() << "Name: " << attrib.value("name").toString();
                    list.append(attrib.value("name").toString());
                }
            }
        } // end entry

        if (xml.name()=="repository") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("code").toString() == "unregistered_ichain_user") {
                    qDebug() << "Unregistered username!";
                } else {
//                    qDebug() << "Repository: " << attrib.value("name").toString();
                    list.append(attrib.value("name").toString());
                }
            }
        } // end repository

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
    }

    return list;
}

QHash<QString, bool> OBSXmlReader::parseRepositoryFlags(QXmlStreamReader &xml)
{
    QHash<QString, bool> flagHash;
    while (xml.readNextStartElement()) {
        if (xml.name() == "enable") {
            QXmlStreamAttributes attrib = xml.attributes();
            if (!attrib.hasAttribute("repository")) {
                flagHash.insert("all", true);
            } else {
                QString repository = attrib.value("repository").toString();
                flagHash.insert(repository, true);
            }
            xml.readNext();
        }

        if (xml.name() == "disable") {
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

void OBSXmlReader::parseFileList(const QString &project, const QString &package, const QString &data)
{
    qDebug() << "OBSXmlReader::parseFileList()";
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name()=="entry") {
            OBSFile *obsFile = nullptr;
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                qDebug() << "Name: " << attrib.value("name").toString();
                qDebug() << "Size: " << attrib.value("size").toString();
                qDebug() << "Mtime: " << attrib.value("mtime").toString();
                obsFile = new OBSFile();
                obsFile->setProject(project);
                obsFile->setPackage(package);
                obsFile->setName(attrib.value("name").toString());
                obsFile->setSize(attrib.value("size").toString());
                obsFile->setLastModified(attrib.value("mtime").toString());
                emit finishedParsingFile(obsFile);
            }
        } // end entry

    } // end while
    emit finishedParsingFileList();
}

void OBSXmlReader::parseLink(const QString &data)
{
    qDebug() << "OBSXmlReader::parseLink()";
    QXmlStreamReader xml(data);
    OBSLink *obsLink = new OBSLink();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name()=="link") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsLink->setProject(attrib.value("project").toString());
                obsLink->setPackage(attrib.value("package").toString());
            }
        } // end link

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
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

        if (xml.name()=="title" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setTitle(xml.text().toString());
        }

        if (xml.name()=="description" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setDescription(xml.text().toString());
        }

        if (xml.name()=="revision" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setRevision(xml.text().toString());
        }

        if (xml.name()=="last_deployment" && xml.isStartElement()) {
            xml.readNext();
            obsAbout->setLastDeployment(xml.text().toString());
        }

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
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

        if (xml.name()=="login" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setLogin(xml.text().toString());
        }

        if (xml.name()=="email" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setEmail(xml.text().toString());
        }

        if (xml.name()=="realname" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setRealName(xml.text().toString());
        }

        if (xml.name()=="state" && xml.isStartElement()) {
            xml.readNext();
            obsPerson->setState(xml.text().toString());
        }

        if (xml.name()=="project") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsPerson->appendWatchItem(attrib.value("name").toString());
            }
        }

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingPerson(obsPerson);
}

void OBSXmlReader::parseUpdatePerson(const QString &data)
{
    qDebug() << "OBSXmlReader::parseUpdatePerson()";
    QXmlStreamReader xml(data);
    OBSStatus *obsStatus = new OBSStatus();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        parseStatus(xml, obsStatus);
    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingUpdatePerson(obsStatus);
}

int OBSXmlReader::getRequestNumber()
{
    return requestNumber.toInt();
}

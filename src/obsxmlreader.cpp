/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2013-2015 Javier Llorente <javier@opensuse.org>
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

OBSXmlReader* OBSXmlReader::instance = NULL;

OBSXmlReader::OBSXmlReader()
{

}

OBSXmlReader* OBSXmlReader::getInstance()
{
    if (!instance) {
        instance = new OBSXmlReader();
    }
    return instance;
}

void OBSXmlReader::addData(const QString& data)
{
    list.clear();
    qDebug() << "OBSXmlReader addData()";
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.name()=="status" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: status tag found";
            parsePackage(data);
            obsPackage = getPackage();
        } else if (xml.name()=="collection" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: collection tag found";
            parseRequests(data);
            obsRequests = getRequests();
        } else if (xml.name()=="directory" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: directory tag found";
            stringToFile(data);
        } else if (xml.name()=="project" && xml.isStartElement()) {
            qDebug() << "OBSXmlReader: project tag found";
            stringToFile(data);
        }
    }
}

void OBSXmlReader::parsePackage(const QString &data)
{
    QXmlStreamReader xml(data);
    obsPackage = new OBSPackage();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name()=="status") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("code").toString() == "unregistered_ichain_user") {
                    qDebug() << "Unregistered username!";
                }
                else {
                    obsPackage->setName(attrib.value("package").toString());
                    obsPackage->setStatus(attrib.value("code").toString());
                    qDebug() << "Package:" << obsPackage->getName() << "Status:" << obsPackage->getStatus();
                }
            }
        } // end status

        if (xml.name()=="details") {
            if (xml.tokenType() != QXmlStreamReader::StartElement) {
                // emit signal if we get a 404
                emit finishedParsingPackage(obsPackage, row);
                return;
            }
            xml.readNext();
            obsPackage->setDetails(xml.text().toString());
//               qDebug() << "details:" << details;
        }  else {
//            no details
        } // end details

        if (xml.name()=="summary" && xml.isStartElement()) {
            xml.readNext();
            qDebug() << "Summary:" << xml.text().toString();
            // If user doesn't exist, return
            if (xml.text().toString().startsWith("Couldn't find User with login")) {
                return;
            }
        } // end summary

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingPackage(obsPackage, row);
}

void OBSXmlReader::setPackageRow(const int &row)
{
    this->row = row;
}

OBSPackage* OBSXmlReader::getPackage()
{
    return obsPackage;
}

void OBSXmlReader::parseRequests(const QString &data)
{
    QXmlStreamReader xml(data);
    obsRequests.clear();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name()=="collection") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                QStringRef matches = attrib.value("matches");
                QStringRef code = attrib.value("code");
                requestNumber = matches.toString();

                qDebug() << "Matches:" << requestNumber;

                if (code.toString() == "unregistered_ichain_user") {
                    qDebug() << "Unregistered username!";
                } else {
//                    data = code.toString();
                }
            }
        } // collection

        if (xml.name()=="request") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest = new OBSRequest;
                obsRequest->setId(attrib.value("id").toString());
            } else if (xml.isEndElement()){
                obsRequests.append(obsRequest);
                qDebug() << "added " << obsRequest->getId();
            }
        } // request

        if (xml.name()=="action")  {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();
                obsRequest->setActionType(attrib.value("type").toString());
                qDebug() << "Action type:" <<  obsRequest->getActionType();
//                if (obsRequest->getActionType()=="delete") {
//                    obsRequest->setSourceProject("N/A");
//                }
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
                obsRequest->setDate(attrib.value("when").toString());
                qDebug() << "Date: " <<  obsRequest->getDate();
            }
        } // state

        if (xml.name()=="description") {
            if (xml.isStartElement()) {
                xml.readNext();
                obsRequest->setDescription(xml.text().toString());
                qDebug() << "Description:\n" <<  obsRequest->getDescription();
                // if tag is not empty (ie: <description/>), read next start element
                if(!xml.text().isEmpty()) {
                    xml.readNextStartElement();
                }
            }
        } // description
    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }

    emit finishedParsingRequests(obsRequests);
}

void OBSXmlReader::parseList(QXmlStreamReader &xml)
{
    qDebug() << "OBSXmlReader parseList()";
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
        return;
    }

    emit finishedParsingList(list);
}

void OBSXmlReader::stringToFile(const QString &data)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
            "/data/" + QCoreApplication::applicationName();
    QDir dir(dataDir);

    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    QFile file(fileName);
    QDir::setCurrent(dataDir);

    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << data;
    file.close();
}

QFile* OBSXmlReader::openFile()
{
    qDebug() << "OBSXmlReader openFile()";
    list.clear();
    QFile* file = new QFile(fileName);
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
            "/data/" + QCoreApplication::applicationName();
    QDir::setCurrent(dataDir);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot read file " << dataDir << fileName << "(" << file->errorString() << ")";
    }
    return file;
}

void OBSXmlReader::readFile()
{
    QXmlStreamReader xml;
    xml.setDevice(openFile());
    parseList(xml);
}

void OBSXmlReader::getRepositoryArchs(const QString &repository)
{
    qDebug() << "OBSXmlReader getRepositoryArchs()";
    list.clear();
    QXmlStreamReader xml;
    xml.setDevice(openFile());
    bool repositoryFound = false;

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        if (xml.name()=="repository") {
            if (xml.isStartElement()) {
                QXmlStreamAttributes attrib = xml.attributes();

                if (attrib.value("name")==repository) {
//                    qDebug() << "Repository: " << attrib.value("name").toString();
                    repositoryFound = true;
                } else {
                    repositoryFound = false;
                }
            }
        } // end repository

        if (xml.name()=="arch" && repositoryFound) {
            xml.readNext();
//            qDebug() << "Arch:" << xml.text().toString();
            list.append(xml.text().toString());
            xml.readNextStartElement();

        } // end arch

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
        return;
    }
}

QList<OBSRequest*> OBSXmlReader::getRequests()
{
    return obsRequests;
}

int OBSXmlReader::getRequestNumber()
{
    return requestNumber.toInt();
}

QStringList OBSXmlReader::getList()
{
    return list;
}

void OBSXmlReader::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

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

OBSxmlReader* OBSxmlReader::instance = NULL;

OBSxmlReader::OBSxmlReader()
{

}

OBSxmlReader* OBSxmlReader::getInstance()
{
    if (!instance) {
        instance = new OBSxmlReader();
    }
    return instance;
}

void OBSxmlReader::addData(const QString& data)
{
    list.clear();
    qDebug() << "OBSxmlReader addData()";
    QXmlStreamReader xml(data);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.name()=="status" && xml.isStartElement()) {
            qDebug() << "OBSxmlReader: status tag found";
            parsePackage(data);
            obsPackage = getPackage();
        } else if (xml.name()=="collection" && xml.isStartElement()) {
            qDebug() << "OBSxmlReader: collection tag found";
            parseRequests(data);
            obsRequests = getRequests();
        } else if (xml.name()=="directory" && xml.isStartElement()) {
            qDebug() << "OBSxmlReader: directory tag found";
            stringToFile(data);
        } else if (xml.name()=="project" && xml.isStartElement()) {
            qDebug() << "OBSxmlReader: project tag found";
            stringToFile(data);
        }
    }
}

void OBSxmlReader::parsePackage(const QString &data)
{
    QXmlStreamReader xml(data);
    obsPackage = new OBSpackage();

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
                return;
            }
            xml.readNext();
            obsPackage->setDetails(xml.text().toString());
//               qDebug() << "details:" << details;
        }  else {
//            no details
        } // end details

    } // end while

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
    }
}

OBSpackage* OBSxmlReader::getPackage()
{
    return obsPackage;
}

void OBSxmlReader::parseRequests(const QString &data)
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
                obsRequest = new OBSrequest;
                obsRequest->setId(attrib.value("id").toString());
            }
        }

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
            if (xml.tokenType() != QXmlStreamReader::StartElement) {
                return;
            }
            xml.readNext();
            obsRequest->setDescription(xml.text().toString());
            qDebug() << "Description:\n" <<  obsRequest->getDescription();
            xml.readNextStartElement();
            obsRequests.append(obsRequest);
        } // description
    }

    if (xml.hasError()) {
        qDebug() << "Error parsing XML!" << xml.errorString();
    }
}

void OBSxmlReader::parseList(QXmlStreamReader &xml)
{
    qDebug() << "OBSxmlReader parseList()";
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
}

void OBSxmlReader::stringToFile(const QString &data)
{
    QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
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

QFile* OBSxmlReader::openFile()
{
    qDebug() << "OBSxmlReader openFile()";
    list.clear();
    QFile* file = new QFile(fileName);
    QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir::setCurrent(dataDir);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot read file " << dataDir << fileName << "(" << file->errorString() << ")";
    }
    return file;
}

void OBSxmlReader::readFile()
{
    QXmlStreamReader xml;
    xml.setDevice(openFile());
    parseList(xml);
}

void OBSxmlReader::getArchsForRepository(const QString &repository)
{
    qDebug() << "OBSxmlReader getArchsForRepository()";
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
    }
}

QList<OBSrequest*> OBSxmlReader::getRequests()
{
    return obsRequests;
}

int OBSxmlReader::getRequestNumber()
{
    return requestNumber.toInt();
}

QStringList OBSxmlReader::getList()
{
    return list;
}

void OBSxmlReader::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

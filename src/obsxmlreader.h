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

#ifndef OBSXMLREADER_H
#define OBSXMLREADER_H

#include <QXmlStreamReader>
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include "obspackage.h"
#include "obsrequest.h"

class OBSXmlReader : public QObject
{
    Q_OBJECT

public:

    static OBSXmlReader* getInstance();
    void addData(const QString &data);
    void setPackageRow(const int &row);
    OBSPackage* getPackage();
    QList<OBSRequest*> getRequests();
    int getRequestNumber();
    QStringList getList();
    void readFile();
    void setFileName(const QString &fileName);
    void getRepositoryArchs(const QString &repository);

private:
    static OBSXmlReader* instance;
    OBSXmlReader();
    void parsePackage(const QString &data);
    int row;
    void parseRequests(const QString &data);
    void parseList(QXmlStreamReader &xml);
    OBSPackage *obsPackage;
    QList<OBSRequest*> obsRequests;
    OBSRequest *obsRequest;
    QString requestNumber;
    QStringList list;
    void stringToFile(const QString &data);
    QString fileName;
    QFile* openFile();

signals:
    void finishedParsingPackage(OBSPackage*, const int&);
    void finishedParsingRequests(QList<OBSRequest*>);
    void finishedParsingList(QStringList);
};

#endif // OBSXMLREADER_H

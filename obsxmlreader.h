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
#include "obspackage.h"
#include "obsrequest.h"

class OBSxmlReader : public QXmlStreamReader
{
public:
    static OBSxmlReader* getInstance();
    void addData(const QString &data);

    OBSpackage* getPackage();
    QList<OBSrequest*> getRequests();
    int getRequestNumber();
    QStringList getList();
    void readFile();
    void setFileName(const QString &fileName);

private:
    static OBSxmlReader* instance;
    OBSxmlReader();
    void parsePackage(const QString &data);
    void parseRequests(const QString &data);
    void parseList(QXmlStreamReader &xml);
    OBSpackage *obsPackage;
    QList<OBSrequest*> obsRequests;
    OBSrequest *obsRequest;
    QString requestNumber;
    QStringList list;
    void stringToFile(const QString &data);
    QString fileName;
};

#endif // OBSXMLREADER_H

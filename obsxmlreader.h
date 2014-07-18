/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2014 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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
#include "obspackage.h"
#include "obsrequest.h"

class OBSxmlReader : public QXmlStreamReader
{
public:
    OBSxmlReader();
    void parse(const QString &data);
    void parseRequests(const QString& data);
    OBSpackage* getPackage();
    QList<OBSrequest*> getRequests();
    int getRequestNumber();

private:
    OBSpackage *obsPackage;
    QList<OBSrequest*> obsRequests;
    OBSrequest *obsRequest;
    QString requestNumber;
};

#endif // OBSXMLREADER_H

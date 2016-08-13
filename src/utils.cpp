/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2016 Javier Llorente <javier@opensuse.org>
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

#include "utils.h"

QString Utils::unixTimeToDate(const QString &unixTime)
{
    QDateTime dateTime = QDateTime::fromTime_t(unixTime.toUInt());
    return dateTime.toString("dd/MM/yyyy H:mm");
}

QString Utils::breakLine(QString &details, int maxSize)
{
    int i = maxSize;
    if (details.size()>i) {
        for (; i<details.size(); i++) {
            if (details[i]==QChar(',') || details[i]==QChar('-') || details[i]==QChar(' ')) {
                details.insert(++i,QString("<br>"));
                break;
            }
        }
    }
    return details;
}

QColor Utils::getColorForStatus(const QString &status)
{
//    Change the status' colour according to the status itself
    QColor color;
    color = Qt::black;

    if (status=="succeeded")
    {
        color = Qt::darkGreen;
    }
    else if (status=="blocked")
    {
        color = Qt::gray;
    }
    else if (status=="scheduled"||status=="building")
    {
        color = Qt::darkBlue;
    }
    else if (status=="disabled")
    {
        color = Qt::gray;
    }
    else if (status=="failed")
    {
        color = Qt::red;
    }
    else if (status=="unresolvable")
    {
        color = Qt::darkRed;
    }
    else if (status.contains("unknown")||status=="404")
    {
        color = Qt::red;
    }

    return color;
}

void Utils::setItemBoldFont(QTreeWidgetItem *item, bool bold)
{
    QFont font = item->font(0);
    font.setBold(bold);
    for (int i=0; i<5; i++) {
        item->setFont(i, font);
    }
}

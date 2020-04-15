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

QString Utils::fileSizeHuman(float fileSize)
{
    QStringList unitList = (QStringList() << "Bytes" << "KB" << "MB" << "GB" << "TB");
    QStringListIterator i(unitList);
    QString unit = i.next();
    int precision = (fileSize < 1024) ? 0 : 2;
    while (fileSize >= 1024 && i.hasNext()) {
        unit = i.next();
        fileSize /= 1024;
    }
    return QString().setNum(fileSize,'f',precision) + " " + unit;
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
        color.setRgb(110, 185, 39);
    }
    else if (status=="blocked")
    {
        color.setRgb(187, 187, 187); // colour should be white and bg should be rgb(187, 187, 187)
    }
    else if (status=="scheduled") {
        color.setRgb(17, 17, 221);//  bg should be rgb(187, 187, 187)
    }
    else if (status=="building")
    {
        color.setRgb(0, 0, 255); // bg should be rgb(187, 187, 187)
    }
    else if (status=="disabled")
    {
        color.setRgb(187, 187, 187);
    }
    else if (status=="excluded")
    {
        color.setRgb(0, 102, 153);
    }
    else if (status=="failed")
    {
        color.setRgb(255, 0, 0);
    }    
    else if(status=="broken") {

        color.setRgb(238, 0, 0);
    }
    else if (status=="unresolvable")
    {
        color.setRgb(204, 0, 0);
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

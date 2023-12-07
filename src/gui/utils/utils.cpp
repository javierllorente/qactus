/*
 * Copyright (C) 2016-2023 Javier Llorente <javier@opensuse.org>
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
#include "utils.h"

QString Utils::unixTimeToDate(const QString &unixTime)
{
    QDateTime dateTime = QDateTime:: fromSecsSinceEpoch(unixTime.toUInt());
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

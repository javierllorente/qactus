/*
 * Copyright (C) 2016 Javier Llorente <javier@opensuse.org>
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
#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>
#include <QColor>
#include <QTreeWidgetItem>

class Utils
{
public:
    static QString unixTimeToDate(const QString &unixTime);
    static QString fileSizeHuman(float fileSize);
    static QString breakLine(QString &details, int maxSize);
    static QColor getColorForStatus(const QString &status);
    static void setItemBoldFont(QTreeWidgetItem *item, bool bold);

private:
    Utils();
};

#endif // UTILS_H

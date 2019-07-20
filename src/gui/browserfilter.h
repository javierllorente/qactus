/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef BROWSERFILTER_H
#define BROWSERFILTER_H

#include <QWidget>
#include <QStringListModel>
#include <QCompleter>

namespace Ui {
class BrowserFilter;
}

class BrowserFilter : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserFilter(QWidget *parent = 0);
    ~BrowserFilter();

    QString getText() const;
    void clear();
    void setFocus();

public slots:
    void addProjectList(const QStringList &projectList);

private:
    Ui::BrowserFilter *ui;
    QStringListModel *m_projectModel;
    QCompleter *m_projectCompleter;

private slots:
    void autocompletedProject_clicked(const QString &project);

signals:
    void setCurrentProject(const QString &project);
};

#endif // BROWSERFILTER_H

/*
 * Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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
    explicit BrowserFilter(QWidget *parent = nullptr);
    ~BrowserFilter();

    QString text() const;
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

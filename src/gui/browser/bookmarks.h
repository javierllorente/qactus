/*
 * Copyright (C) 2019-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QMenu>
#include <QSharedPointer>
#include "obsperson.h"

class Bookmarks : public QMenu
{
    Q_OBJECT

public:
    Bookmarks(QWidget *parent = nullptr);
    ~Bookmarks();

public slots:
    void loadBookmarks(QSharedPointer<OBSPerson> person);
    void addBookmark(const QString &project);
    void deleteBookmark(const QString &project);
    void toggleActions(const QString &project);

signals:
    void addBookmarkClicked();
    void deleteBookmarkClicked();
    void clicked(const QString &bookmark);
    void bookmarkAdded(QSharedPointer<OBSPerson> person);
    void bookmarkDeleted(QSharedPointer<OBSPerson> person);
    void updateStatusBar(QString message, bool progressBarHidden);

private:
    QSharedPointer<QAction> m_actionAddBookmark;
    QSharedPointer<QAction> m_actionDeleteBookmark;
    int m_initialCount;
    QSharedPointer<OBSPerson> m_person;
    void addMenuActions();
    void addItem(const QString &entry);

};

#endif // BOOKMARKS_H

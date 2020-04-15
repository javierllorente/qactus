/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QMenu>
#include "obsperson.h"

class Bookmarks : public QMenu
{
    Q_OBJECT

public:
    Bookmarks(QWidget *parent = nullptr);
    ~Bookmarks();

public slots:
    void slotLoadBookmarks(OBSPerson *person);
    void addBookmark(const QString &project);
    void deleteBookmark(const QString &project);
    void toggleActions(const QString &project);

signals:
    void addBookmarkClicked();
    void deleteBookmarkClicked();
    void clicked(const QString &bookmark);
    void bookmarkAdded(OBSPerson *person);
    void bookmarkDeleted(OBSPerson *person);

private:
    QAction *m_actionAddBookmark;
    QAction *m_actionDeleteBookmark;
    int m_initialCount;
    OBSPerson *m_person;
    void addItem(const QString &entry);

};

#endif // BOOKMARKS_H

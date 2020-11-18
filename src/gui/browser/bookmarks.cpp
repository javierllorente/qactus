/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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

#include "bookmarks.h"

Bookmarks::Bookmarks(QWidget *parent)
    : QMenu(parent),
      m_person(nullptr)
{
    m_actionAddBookmark = new QAction(tr("Add bookmark"));
    m_actionAddBookmark->setIcon(QIcon::fromTheme("list-add"));
    m_actionAddBookmark->setVisible(false);
    addAction(m_actionAddBookmark);
    connect(m_actionAddBookmark, &QAction::triggered, this, &Bookmarks::addBookmarkClicked);

    m_actionDeleteBookmark = new QAction(tr("Delete Bookmark"));
    m_actionDeleteBookmark->setIcon(QIcon::fromTheme("trash-empty"));
    m_actionDeleteBookmark->setVisible(false);
    addAction(m_actionDeleteBookmark);
    connect(m_actionDeleteBookmark, &QAction::triggered, this, &Bookmarks::deleteBookmarkClicked);

    addSeparator();
    m_initialCount = actions().count();
}

Bookmarks::~Bookmarks()
{
    delete m_person;
}

void Bookmarks::slotLoadBookmarks(OBSPerson *person)
{
    m_person = person;
    clear();

    for (QString entry : person->getWatchList()) {
        addItem(entry);
    }
}

void Bookmarks::addBookmark(const QString &project)
{
    addItem(project);
    m_person->appendWatchItem(project);
    toggleActions(project);
    emit bookmarkAdded(m_person);
}

void Bookmarks::deleteBookmark(const QString &project)
{
    int index = m_person->removeWatchItem(project);
    if (index != -1) {
        QAction *action = actions().at(index + m_initialCount);
        removeAction(action);
        toggleActions(project);
        emit bookmarkDeleted(m_person);
    }
}

void Bookmarks::toggleActions(const QString &project)
{
    if (project.isEmpty()) {
        m_actionAddBookmark->setVisible(false);
        m_actionDeleteBookmark->setVisible(false);
    } else {
        bool found = m_person->getWatchList().contains(project);
        m_actionAddBookmark->setVisible(!found);
        m_actionDeleteBookmark->setVisible(found);
    }
}

void Bookmarks::addItem(const QString &entry)
{
    QAction *action = new QAction(entry);
    action->setIcon(QIcon::fromTheme("project-development"));
    addAction(action);
    connect(action, &QAction::triggered, this, [=]() {
        emit clicked(entry);
    });
}

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
#include "bookmarks.h"

Bookmarks::Bookmarks(QWidget *parent)
    : QMenu(parent),
      m_person(nullptr)
{
    addMenuActions();
    m_initialCount = actions().count();
}

Bookmarks::~Bookmarks()
{

}

void Bookmarks::addMenuActions()
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
}


void Bookmarks::slotLoadBookmarks(QSharedPointer<OBSPerson> person)
{
    m_person = person;
    clear();
    addMenuActions();

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

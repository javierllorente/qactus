/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2014 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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

#include "trayicon.h"

TrayIcon::TrayIcon(QWidget *parent) :
    QSystemTrayIcon(parent)
{
    createTrayIcon();
    trayIconChanged = false;
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), parent, SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon();
    trayIconMenu = new QMenu();
    trayIcon->setIcon(QIcon(":/obs.png"));
    trayIcon->setToolTip("Qactus");
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

void TrayIcon::change()
{
    trayIcon->setIcon(QIcon(":/obs_change.png"));
    trayIconChanged = true;
}

bool TrayIcon::hasChangedIcon()
{
    return trayIconChanged;
}

void TrayIcon::setTrayIcon(const QString& iconName)
{
    trayIcon->setIcon(QIcon(":/" + iconName));
}

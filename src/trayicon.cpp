/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2015 Javier Llorente <javier@opensuse.org>
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
    trayIconMenu = new QMenu();
    normal();
    setToolTip("Qactus");
    setContextMenu(trayIconMenu);
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), parent,
            SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));
    show();
}

void TrayIcon::notify()
{
    setIcon(QIcon(":/obs_change.png"));
    trayIconChanged = true;
    qDebug() << "TrayIcon notify()";
}

void TrayIcon::normal()
{
    setIcon(QIcon(":/obs.png"));
    trayIconChanged = false;
    qDebug() << "TrayIcon normal()";
}

bool TrayIcon::hasChangedIcon()
{
    return trayIconChanged;
}

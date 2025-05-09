/*
 * Copyright (C) 2010-2025 Javier Llorente <javier@opensuse.org>
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
#include "trayicon.h"

TrayIcon::TrayIcon(QWidget *parent) :
    QSystemTrayIcon(parent),
    trayIconMenu(new QMenu(parent))
{
    setIcon(QIcon(NORMAL_ICON));
    setToolTip(QCoreApplication::applicationName());
    setContextMenu(trayIconMenu);
    show();
}

void TrayIcon::toggleIcon(bool toggle)
{
    qDebug() << Q_FUNC_INFO << toggle;
    notifyIcon = toggle;
    QString icon = toggle ? NOTIFY_ICON : NORMAL_ICON;
    setIcon(QIcon(icon));
}

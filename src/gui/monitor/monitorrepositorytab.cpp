/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2021-2023 Javier Llorente <javier@opensuse.org>
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

#include "monitorrepositorytab.h"
#include "ui_monitortab.h"

MonitorRepositoryTab::MonitorRepositoryTab(QWidget *parent, const QString &title, OBS *obs) :
    MonitorTab(parent, title, obs)
{
    connect(m_obs, &OBS::finishedParsingResultList, this, &MonitorRepositoryTab::slotAddResultList);
}

MonitorRepositoryTab::~MonitorRepositoryTab()
{

}


void MonitorRepositoryTab::refresh()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_obs->getProjectResults(m_title);
}

bool MonitorRepositoryTab::hasSelection()
{
    QItemSelectionModel *treeWidgetSelectionModel = ui->treeWidget->selectionModel();
    if (treeWidgetSelectionModel) {
        return treeWidgetSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void MonitorRepositoryTab::slotAddResultList(const QList<OBSResult *> &resultList)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (!resultList.isEmpty() && m_title == resultList.first()->getProject()) {

        if (ui->treeWidget->topLevelItemCount() > 0) {
            checkForResultListChanges(resultList);
            ui->treeWidget->clear();
        }

        m_resultList = resultList;
        QTreeWidgetItem *item = nullptr;

        for (OBSResult *result : resultList) {
            for (OBSStatus *status : result->getStatusList()) {

                item = new QTreeWidgetItem(ui->treeWidget);
                item->setText(0, result->getProject());
                item->setText(1, status->getPackage());
                item->setText(2, result->getRepository());
                item->setText(3, result->getArch());
                item->setText(4, status->getCode());

                if (!status->getDetails().isEmpty()) {
                    QString details = status->getDetails();
                    details = Utils::breakLine(details, 250);
                    item->setToolTip(4, details);
                }
                item->setForeground(4, Utils::getColorForStatus(status->getCode()));

                ui->treeWidget->addTopLevelItem(item);
            }
        }
    }
}

void MonitorRepositoryTab::checkForResultListChanges(const QList<OBSResult *> &resultList)
{
    // FIXME: intersect always returns 0
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QSet<OBSResult *> old_resultSet(m_resultList.begin(), m_resultList.end());
    QSet<OBSResult *> new_resultSet(resultList.begin(), resultList.end());
#else
    QSet<OBSResult *> old_resultSet = m_resultList.toSet();
    QSet<OBSResult *> new_resultSet= resultList.toSet();
#endif

    QList<OBSResult *> commonResults = old_resultSet.intersect(new_resultSet).values();

    for (OBSResult *oldResult : commonResults) {
        for (OBSResult *newResult : resultList) {

            if (oldResult == newResult) {

                for (OBSStatus *oldStatus : oldResult->getStatusList()) {
                    for (OBSStatus *newStatus : newResult->getStatusList()) {

                        if (oldStatus == newStatus
                                && hasStatusChanged(oldStatus->getCode(), newStatus->getCode())) {

                            // get row index
                            int resultIndex = resultList.indexOf(newResult);
                            int statusIndex = newResult->getStatusList().indexOf(newStatus);

                            if (resultIndex != 0) {
                                int count = 0;
                                for (int i=resultIndex; i<resultList.count(); i++) {
                                    count += resultList.at(i)->getStatusList().count();
                                }
                                statusIndex += count;
                            }

                            QTreeWidgetItem *item = ui->treeWidget->topLevelItem(statusIndex);
                            Utils::setItemBoldFont(item, true);
                            break;
                        }
                    }
                }

                break;
            }
        }
    }
}



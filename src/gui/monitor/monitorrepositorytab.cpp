/*
 * Copyright (C) 2021-2025 Javier Llorente <javier@opensuse.org>
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

void MonitorRepositoryTab::slotAddResultList(QList<QSharedPointer<OBSResult>> resultList)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (!resultList.isEmpty() && m_title == resultList.first()->getProject()) {

        if (ui->treeWidget->topLevelItemCount() > 0) {
            checkForResultListChanges(resultList);
            ui->treeWidget->clear();
        }

        m_resultList = resultList;
        QTreeWidgetItem *item = nullptr;

        for (QSharedPointer<OBSResult> result : resultList) {
            for (QSharedPointer<OBSStatus> status : result->getStatusList()) {

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

void MonitorRepositoryTab::checkForResultListChanges(QList<QSharedPointer<OBSResult>> resultList)
{
    // FIXME: intersect always returns 0
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QSet<QSharedPointer<OBSResult>> old_resultSet(m_resultList.begin(), m_resultList.end());
    QSet<QSharedPointer<OBSResult>> new_resultSet(resultList.begin(), resultList.end());
#else
    QSet<QSharedPointer<OBSResult>> old_resultSet = m_resultList.toSet();
    QSet<QSharedPointer<OBSResult>> new_resultSet= resultList.toSet();
#endif

    QList<QSharedPointer<OBSResult>> commonResults = old_resultSet.intersect(new_resultSet).values();

    for (QSharedPointer<OBSResult> oldResult : commonResults) {
        for (QSharedPointer<OBSResult> newResult : resultList) {

            if (oldResult == newResult) {

                for (QSharedPointer<OBSStatus> oldStatus : oldResult->getStatusList()) {
                    for (QSharedPointer<OBSStatus> newStatus : newResult->getStatusList()) {

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



#include "packagetreewidget.h"

PackageTreeWidget::PackageTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
}

void PackageTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PackageTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void PackageTreeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString urlStr = urlList.at(0).toString();

        qDebug () << "Dropped url:" << urlStr;
        QRegExp rx("^(?:http|https)://(\\S+)/package/show/(\\S+)/(\\S+)");
        if(urlStr.contains(rx)) {
            qDebug () << "Valid OBS URL found!";
            QStringList list = rx.capturedTexts();
            emit obsUrlDropped(list);
        }
    }


}

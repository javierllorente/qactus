#ifndef PACKAGETREEWIDGET_H
#define PACKAGETREEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

class PackageTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    PackageTreeWidget(QWidget *parent = 0);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void obsUrlDropped(const QStringList&);

};

#endif // PACKAGETREEWIDGET_H

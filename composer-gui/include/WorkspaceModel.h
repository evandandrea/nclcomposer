#ifndef WORKSPACEMODEL_H
#define WORKSPACEMODEL_H

#include <QSortFilterProxyModel>
#include <QDebug>

class WorkspaceModel : public QSortFilterProxyModel
{
private:
    QModelIndex topIndex;
public:
    WorkspaceModel(QObject *parent = 0);
    /* RootIndex from sourceModel() */
    inline void setTopIndex(QModelIndex index) { topIndex =index;}
    inline QModelIndex getTopIndex() {return topIndex;}

protected:
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

};

#endif // WORKSPACEMODEL_H
#ifndef QNLYGRAPHICSREGION_H
#define QNLYGRAPHICSREGION_H

#include <QGraphicsItem>
#include <QWidget>
#include <QObject>
#include <QPainterPath>
#include <QSizeF>
#include <QActionGroup>
#include <QAction>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QRectF>
#include <QPointF>
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QLineEdit>
#include <QTextDocument>
#include <QKeyEvent>

#include "qnlygraphicsregionbase.h"

class QnlyGraphicsRegionBase;

#define ROUND_DOUBLE(x) x<0.0?x=0.0:(x>100.0?x=0.0:x)

class QnlyGraphicsRegion : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    QnlyGraphicsRegion(QMenu* switchMenu, QnlyGraphicsRegion* parent = 0);

    virtual ~QnlyGraphicsRegion();

    enum QnlyResizeType {
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        TopLeft,
    };

    bool isMoving() const;

    void setMoving(bool moving);

    bool isResizing() const;

    void setParent(QnlyGraphicsRegion* region);

    void setResizing(bool resizing);

    bool isSelected() const;

    void setSelected(bool selected);

    bool isPainted() const;

    void setPainted(bool painted);

    bool hasChanged() const;

    void setChanged(bool changed);

    bool isValidated() const;

    void setValidated(bool validated);

    QString getId() const;

    void setId(const QString &id);

    QString getUid() const;

    void setUid(const QString &uid);

    QString getTitle() const;

    void setTitle(const QString &title);

    QString getColor() const;

    void setColor(const QString &color);

    qreal getzIndex() const;

    void setzIndex(qreal zindex);

    qreal getzIndexTop() const;

    qreal getTop() const;

    qreal getMoveTop() const;

    qreal getPressTop() const;

    qreal getResizeTop() const;

    qreal getRelativeTop() const;

    void setRelativeTop(qreal relativeTop);

    qreal getLeft() const;

    qreal getMoveLeft() const;

    qreal getPressLeft() const;

    qreal getResizeLeft() const;

    qreal getRelativeLeft() const;

    void setRelativeLeft(qreal relativeLeft);

    qreal getRight() const;

    qreal getRelativeRight() const;

    void setRelativeRight(qreal relativeRight);

    qreal getBottom() const;

    qreal getRelativeBottom() const;

    void setRelativeBottom(qreal relativeBottom);

    qreal getWidth() const;

    qreal getPressWidth() const;

    qreal getResizeWidth() const;

    qreal getRelativeWidth() const;

    void setRelativeWidth(qreal relativeWidth);

    qreal getHeight() const;

    qreal getPressHeight() const;

    qreal getResizeHeight() const;

    qreal getRelativeHeight() const;

    void setRelativeHeight(qreal relativeHeight);

    QnlyResizeType getResizeType() const;

    void adjust(bool repaint = true);

    void addRegion(QnlyGraphicsRegion* region);

    void removeRegion(QnlyGraphicsRegion* region);

    void hideRegion(QnlyGraphicsRegion* region);

signals:
    void regionAdditionRequested(QnlyGraphicsRegion* parent);

    void regionSelectionRequested(QnlyGraphicsRegion* region);

    void regionDeletionRequested(QnlyGraphicsRegion* region);

    void regionChangeRequested(QnlyGraphicsRegion* region,
                               QMap<QString, QString> attributes);

    void regionbasePerformed();

protected:
    void setTop(qreal top);

    void setMoveTop(qreal moveTop);

    void setPressTop(qreal pressTop);

    void setResizeTop(qreal resizeTop);

    void setLeft(qreal left);

    void setMoveLeft(qreal moveLeft);

    void setPressLeft(qreal pressLeft);

    void setResizeLeft(qreal resizeLeft);

    void setRight(qreal right);

    void setBottom(qreal bottom);

    void setWidth(qreal width);

    void setPressWidth(qreal pressWidth);

    void setResizeWidth(qreal resizeWidth);

    void setHeight(qreal height);

    void setPressHeight(qreal pressHeight);

    void setResizeHeight(qreal resizeHeight);

    void setzIndexTop(qreal zindexTop);

    void setResizeType(const QnlyResizeType &resizeType);

    virtual void move(QGraphicsSceneMouseEvent* event);

    virtual void resize(QGraphicsSceneMouseEvent* event);

    virtual QPainterPath shape() const;

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

    void keyReleaseEvent(QKeyEvent * event);

    void keyPressEvent(QKeyEvent * event);

protected slots:
    void performShow(QAction* action);

    void performRegion();

    void performHide();

    void performDelete();

private:
    void createActions();

    void createMenus();

    void createConnections();

    bool moving;

    bool resizing;

    bool selected;

    bool validated;

    bool painted;

    bool changed;

    qreal top;

    qreal moveTop;

    qreal pressTop;

    qreal resizeTop;

    qreal relativeTop;

    qreal left;

    qreal moveLeft;

    qreal pressLeft;

    qreal resizeLeft;

    qreal relativeLeft;

    qreal right;

    qreal relativeRight;

    qreal bottom;

    qreal relativeBottom;

    qreal width;

    qreal pressWidth;

    qreal resizeWidth;

    qreal relativeWidth;

    qreal height;

    qreal pressHeight;

    qreal resizeHeight;

    qreal relativeHeight;

    qreal zindex;

    qreal zindexTop;

    QMenu* viewMenu;

    QMenu* insertMenu;

    QMenu* showMenu;

    QMenu* arrangeMenu;

    QMenu* switchMenu;

    QMenu* contextMenu;

    QAction* helpAction;

    QAction* undoAction;

    QAction* redoAction;

    QAction* cutAction;

    QAction* copyAction;

    QAction* pasteAction;

    QAction* deleteAction;

    QAction* zoominAction;

    QAction* zoomoutAction;

    QAction* zoomresetAction;

    QAction* hideAction;

    QAction* fullscreenAction;

    QAction* exportAction;

    QAction* regionAction;

    QAction* regionbaseAction;

    QAction* bringfrontAction;

    QAction* bringforwardAction;

    QAction* sendbackwardAction;

    QAction* sendbackAction;

    QAction* propertiesAction;

    QString id;

    QString uid;

    QString title;

    QString color;

    QnlyResizeType resizeType;

    QActionGroup* regionActionGroup;

    QMap<QString, QAction*> regionActions;

    QMap<QString, QnlyGraphicsRegion*> regions;
};

#endif // QNLYGRAPHICSREGION_H
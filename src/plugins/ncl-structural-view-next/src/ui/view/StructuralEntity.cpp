#include "StructuralEntity.h"
#include "StructuralContent.h"
#include "StructuralComposition.h"

StructuralEntity::StructuralEntity(StructuralEntity* parent)
  : QObject(parent), QGraphicsItem(parent)
{
  setStructuralCategory(Structural::NoCategory);
  setStructuralType(Structural::NoType);

  setStructuralUid(StructuralUtil::CreateUid());

  setStructuralParent(parent);

  setMoveable(true);
  setResizable(true);
  setSelectable(true);
  setHoverable(true);
  setDraggable(false);

  setMoving(false);
  setHidden(false);
  setResizing(false);
  setSelected(false);
  setDragging(false);
  setHovering(false);

  setTop(0);
  setLeft(0);
  setWidth(0);
  setHeight(0);

  setMoveTop(0);
  setMoveLeft(0);

  setPressTop(0);
  setPressLeft(0);
  setPressWidth(0);
  setPressHeight(0);

  setResizeTop(0);
  setResizeLeft(0);
  setResizeWidth(0);
  setResizeHeight(0);

  setUncollapedTop(0);
  setUncollapedLeft(0);
  setUncollapedWidth(0);
  setUncollapedHeight(0);

  setzIndex(0);

  createMenus();
  createConnections();

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);

  setAcceptHoverEvents(true);
  setAcceptDrops(true);

  setSelected(false);
  setCollapsed(false);

  hasError = false;
}

StructuralEntity::~StructuralEntity()
{
  // TODO
}

QString StructuralEntity::getStructuralUid() const
{
  return _uid;
}

void StructuralEntity::setStructuralUid(const QString &uid)
{
  _uid = uid;
  _properties.insert(PLG_ENTITY_UID, uid);
}

QString StructuralEntity::getStructuralId() const
{
  return _id;
}

void StructuralEntity::setStructuralId(const QString &id)
{
  _id = id;
  _properties.insert(PLG_ENTITY_ID, id) ;

  refresh();
}

StructualCategory StructuralEntity::getStructuralCategory() const
{
  return _type;
}

void StructuralEntity::setStructuralCategory(const StructualCategory type)
{
  _type = type;
  _properties.insert(PLG_ENTITY_CATEGORY,StructuralUtil::convertType(type));
}

StructuralType StructuralEntity::getStructuralType() const
{
  return _subtype;
}

void StructuralEntity::setStructuralType(const StructuralType subtype)
{
  _subtype = subtype;
  _properties.insert(PLG_ENTITY_TYPE,StructuralUtil::convertSubtype(subtype));
}

QMap<QString, QString> StructuralEntity::getStructuralProperties() const
{
  return _properties;
}

void StructuralEntity::setStructuralProperties(const QMap<QString, QString> &properties)
{
  foreach (QString name, properties.keys()){
    setStructuralProperty(name, properties.value(name));
  }
}

QString StructuralEntity::getStructuralProperty(const QString &name) const
{
  return _properties.value(name,"");
}

void StructuralEntity::setStructuralProperty(const QString &name, const QString &value)
{
  if (name == PLG_ENTITY_TOP)
    setTop(value.toDouble());

  else if (name == PLG_ENTITY_LEFT)
    setLeft(value.toDouble());

  else if (name == PLG_ENTITY_WIDTH)
    setWidth(value.toDouble());

  else if (name == PLG_ENTITY_HEIGHT)
    setHeight(value.toDouble());

  else if (name == PLG_ENTITY_UNCOLLAPSED_TOP)
    setUncollapedTop(value.toDouble());

  else if (name == PLG_ENTITY_UNCOLLAPSED_LEFT)
    setUncollapedLeft(value.toDouble());

  else if (name == PLG_ENTITY_UNCOLLAPSED_WIDTH)
    setUncollapedWidth(value.toDouble());

  else if (name == PLG_ENTITY_UNCOLLAPSED_HEIGHT)
    setUncollapedHeight(value.toDouble());

  else if (name == PLG_ENTITY_ZINDEX)
    setzIndex(value.toInt());

  else if (name == PLG_ENTITY_ID)
    setStructuralId(value);

  else if (name == PLG_ENTITY_UID)
    setStructuralUid(value);

  else if (name == PLG_ENTITY_CATEGORY)
    setStructuralCategory((StructualCategory)value.toInt());

  else if (name == PLG_ENTITY_TYPE)
    setStructuralType((StructuralType)value.toInt());

  else if (name == PLG_ENTITY_HIDDEN)
    setHidden((value == "1" ? true : false));
  else if (name == PLG_ENTITY_COLLAPSED)
    setCollapsed((value == "1" ? true : false));


  else
    _properties.insert(name, value);
}

bool StructuralEntity::isMoveable() const
{
  return _moveable;
}

void StructuralEntity::setMoveable(bool moveable)
{
  _moveable = moveable;
}

bool StructuralEntity::isSelectable() const
{
  return _selectable;
}

void StructuralEntity::setSelectable(bool selectable)
{
  _selectable = selectable;
}

bool StructuralEntity::isHoverable() const
{
  return _hoverable;
}

void StructuralEntity::setHoverable(bool hoverable)
{
  _hoverable = hoverable;
}

bool StructuralEntity::isResizable() const
{
  return _resizable;
}

void StructuralEntity::setResizable(bool resizable)
{
  _resizable = resizable;
}

bool StructuralEntity::isDraggable()
{
  return _draggable;
}

void StructuralEntity::setDraggable(bool draggable)
{
  _draggable = draggable;
}

bool StructuralEntity::isMoving() const
{
  return _moving;
}

void StructuralEntity::setMoving(bool moving)
{
  _moving = moving;
}

bool StructuralEntity::isHidden() const
{
  return _hidden;
}

void StructuralEntity::setHidden(bool hidden)
{
  _hidden = hidden;
  _properties.insert(PLG_ENTITY_HIDDEN,(hidden ? "1" : "0"));

  setVisible(!hidden);
}

bool StructuralEntity::isResizing() const
{
  return _resizing;
}

void StructuralEntity::setResizing(bool resizing)
{
  _resizing = resizing;
}

bool StructuralEntity::isHovering() const
{
  return _hovering;
}

void StructuralEntity::setHovering(bool hovering)
{
  _hovering = hovering;
}

bool StructuralEntity::isDragging() const
{
  return _dragging;
}

void StructuralEntity::setDragging(bool dragging)
{
  _dragging = dragging;
}

bool StructuralEntity::isSelected() const
{
  return _selected;
}

void StructuralEntity::setSelected(bool selected)
{
  _selected = selected;
}

bool StructuralEntity::isCollapsed() const
{
  return _collapsed;
}

void StructuralEntity::setCollapsed(bool collapsed)
{
  _collapsed = collapsed;
  _properties.insert(PLG_ENTITY_COLLAPSED,(collapsed ? "1" : "0"));
}

qreal StructuralEntity::getTop() const
{
  return _top;
}

void StructuralEntity::setTop(qreal top)
{
  _top = top;
  _properties.insert(PLG_ENTITY_TOP,QString::number(top));

  setY(top-4);
}

qreal StructuralEntity::getMoveTop() const
{
  return _moveTop;
}

void StructuralEntity::setMoveTop(qreal moveTop)
{
  _moveTop = moveTop;
}

qreal StructuralEntity::getPressTop() const
{
  return _pressTop;
}

void StructuralEntity::setPressTop(qreal pressTop)
{
  _pressTop = pressTop;
}

qreal StructuralEntity::getResizeTop() const
{
  return _resizeTop;
}

void StructuralEntity::setResizeTop(qreal resizeTop)
{
  _resizeTop = resizeTop;
}

qreal StructuralEntity::getLeft() const
{
  return _left;
}

void StructuralEntity::setLeft(qreal left)
{
  _left = left;
  _properties.insert(PLG_ENTITY_LEFT,QString::number(left));

  setX(left-4);
}

qreal StructuralEntity::getMoveLeft() const
{
  return _moveLeft;
}

void StructuralEntity::setMoveLeft(qreal moveLeft)
{
  _moveLeft = moveLeft;
}

qreal StructuralEntity::getPressLeft() const
{
  return _pressLeft;
}

void StructuralEntity::setPressLeft(qreal pressLeft)
{
  _pressLeft = pressLeft;
}

qreal StructuralEntity::getResizeLeft() const
{
  return _resizeLeft;
}

void StructuralEntity::setResizeLeft(qreal resizeLeft)
{
  _resizeLeft = resizeLeft;
}

qreal StructuralEntity::getWidth() const
{
  return _width;
}

void StructuralEntity::setWidth(qreal width)
{
  _width = width;
  _properties.insert(PLG_ENTITY_WIDTH,QString::number(width));
}

qreal StructuralEntity::getPressWidth() const
{
  return _pressWidth;
}

void StructuralEntity::setPressWidth(qreal pressWidth)
{
  _pressWidth = pressWidth;
}

qreal StructuralEntity::getResizeWidth() const
{
  return _resizeWidth;
}

void StructuralEntity::setResizeWidth(qreal resizeWidth)
{
  _resizeWidth = resizeWidth;
}

qreal StructuralEntity::getHeight() const
{
  return _height;
}

void StructuralEntity::setHeight(qreal height)
{
  _height = height;
  _properties.insert(PLG_ENTITY_HEIGHT,QString::number(height));
}

qreal StructuralEntity::getPressHeight() const
{
  return _pressHeight;
}

void StructuralEntity::setPressHeight(qreal pressHeight)
{
  _pressHeight = pressHeight;
}

qreal StructuralEntity::getResizeHeight() const
{
  return _resizeHeight;
}

void StructuralEntity::setResizeHeight(qreal resizeHeight)
{
  _resizeHeight = resizeHeight;
}

qreal StructuralEntity::getUncollapedTop() const
{
  return _uncollapsedTop;
}

void StructuralEntity::setUncollapedTop(qreal uncollapedTop)
{
  _uncollapsedTop = uncollapedTop;
  _properties.insert(PLG_ENTITY_UNCOLLAPSED_TOP,QString::number(uncollapedTop));
}

qreal StructuralEntity::getUncollapedLeft() const
{
  return _uncollapsedLeft;
}

void StructuralEntity::setUncollapedLeft(qreal uncollapedLeft)
{
  _uncollapsedLeft = uncollapedLeft;
  _properties.insert(PLG_ENTITY_UNCOLLAPSED_LEFT,QString::number(uncollapedLeft));
}

qreal StructuralEntity::getUncollapedWidth() const
{
  return _uncollapsedWidth;
}

void StructuralEntity::setUncollapedWidth(qreal uncollapedWidth)
{
  _uncollapsedWidth = uncollapedWidth;
  _properties.insert(PLG_ENTITY_UNCOLLAPSED_WIDTH,QString::number(uncollapedWidth));
}

qreal StructuralEntity::getUncollapedHeight() const
{
  return _uncollapsedHeight;
}

void StructuralEntity::setUncollapedHeight(qreal uncollapedHeight)
{
  _uncollapsedHeight = uncollapedHeight;
  _properties.insert(PLG_ENTITY_UNCOLLAPSED_HEIGHT,QString::number(uncollapedHeight));
}

qreal StructuralEntity::getzIndex() const
{
  return _zIndex;
}

void StructuralEntity::setzIndex(qreal zIndex)
{
  _zIndex = zIndex;
  _properties.insert(PLG_ENTITY_ZINDEX,QString::number(zIndex));

  setZValue(zIndex);
}

StructuralEntity* StructuralEntity::getStructuralParent() const
{
  return _parent;
}

void StructuralEntity::setStructuralParent(StructuralEntity* parent)
{
  _parent = parent;

  setParent(parent);
  setParentItem(parent);

  if (parent != NULL)
    parent->addStructuralEntity(this);
}

QVector<StructuralEntity*> StructuralEntity::getStructuralEntities()
{
  return _children;
}

void StructuralEntity::createMenus()
{
  menu = new StructuralMenu();
}

void StructuralEntity::createConnections()
{
  connect(menu, SIGNAL(insert(StructuralType)),SLOT(performInsert(StructuralType)));
}

void StructuralEntity::addStructuralEntity(StructuralEntity* entity)
{
  if (entity != NULL)
    if(!_children.contains(entity))
      _children.append(entity);
}

void StructuralEntity::removeStructuralEntity(StructuralEntity* entity)
{
  if (entity != NULL)
  {
    int index = _children.indexOf(entity);

    if (index >= 0)
      _children.remove(index);

    entity->setStructuralParent(NULL);
  }
}

void StructuralEntity::newChild(Structural::StructuralType type)
{
  QString uid = QUuid::createUuid().toString();
  QString parent = getStructuralUid();
  QMap<QString, QString> properties;
  properties[PLG_ENTITY_CATEGORY] = QString::number(type);

  QMap<QString, QString> settings;
  settings[PLG_SETTING_UNDO] = "1";
  settings[PLG_SETTING_NOTIFY] = "1";
  settings[PLG_SETTING_CODE] = QUuid::createUuid().toString();

  emit inserted(uid, parent, properties, settings);
}

StructuralResize StructuralEntity::getStructuralResize() const
{
  return _resize;
}

void StructuralEntity::setStructuralResize(const StructuralResize resize)
{
  _resize = resize;
}

QPainterPath StructuralEntity::shape() const
{
  QPainterPath painter;

  delineate(&painter);

  if (_selectable && _selected && _resizable){
    painter.setFillRule(Qt::WindingFill);

    qreal W = DEFAULT_ANCHOR_WIDTH;
    qreal H = DEFAULT_ANCHOR_HEIGHT;

    painter.addRect(0,0,W,H);                                // topleft
    painter.addRect((_width+W)/2 - W/2,0,W,H);               // top
    painter.addRect((_width+W) - W,0,W,H);                   // topright
    painter.addRect((_width+W) - W,(_height+H)/2 - H/2,W,H); // right
    painter.addRect((_width+W) - W,(_height+H) - H,W,H);     // bottomright
    painter.addRect((_width+W)/2 - W/2,(_height+H) - H,W,H); // bottom
    painter.addRect(0,(_height+H) - H,W,H);                  // bottomleft
    painter.addRect(0,(_height+H)/2 - H/2,W,H);              // left
  }

  return painter;
}

QRectF StructuralEntity::boundingRect() const
{
  QRectF bounds;

  bounds.setX(0);
  bounds.setY(0);
  bounds.setWidth(_width+DEFAULT_ANCHOR_WIDTH);
  bounds.setHeight(_height+DEFAULT_ANCHOR_HEIGHT);

  return bounds;
}

void StructuralEntity::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  draw(painter);

  if (_selectable && _selected){
    painter->setRenderHint(QPainter::Antialiasing,false);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QBrush(Qt::black), 0, Qt::DashLine));

    painter->drawRect(DEFAULT_ANCHOR_WIDTH/2, DEFAULT_ANCHOR_HEIGHT/2, getWidth()-1, getHeight()-1);

    if (_resizable){
      painter->setBrush(QBrush(Qt::white));
      painter->setPen(QPen(QBrush(Qt::black), 0));

      qreal W = DEFAULT_ANCHOR_WIDTH;
      qreal H = DEFAULT_ANCHOR_HEIGHT;

      painter->drawRect(0,0,W,H);                                // topleft
      painter->drawRect((_width+W)/2-H/2-1,0,W,H);               // top
      painter->drawRect((_width+W)-W-1,0,W,H);                   // topright
      painter->drawRect((_width+W)-W-1,(_height+H)/2-H/2-1,W,H); // right
      painter->drawRect((_width+W)-W-1,(_height+H)-H-1,W,H);     // bottomright
      painter->drawRect((_width+W)/2-W/2-1,(_height+H)-H-1,W,H); // bottom
      painter->drawRect(0,(_height+H)-H-1,W,H);                  // bottomleft
      painter->drawRect(0,(_height+H)/2-H/2-1,W,H);              // left
    }
  }
}

void StructuralEntity::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  if (_moveable && _moving){
    move(event);

  } else if (_resizable && _resizing){
    resize(event);
  }else if(_draggable && _dragging){
    QDrag *drag = new QDrag(event->widget());
    QMimeData* minedata = new QMimeData();

    minedata->setObjectName(QString::number(_subtype));
    minedata->setText(_uid);
    drag->setMimeData(minedata);
/*
    QPixmap pixmap(boundingRect().size().toSize());
    pixmap.fill(Qt::white);

    QPainter painter (&pixmap);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setOpacity(0.5);
    QStyleOptionGraphicsItem styleOption;
    paint(&painter,&styleOption,0);
    painter.end();

    drag->setPixmap(pixmap);
*/

    drag->exec();
  }
}

void StructuralEntity::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  if (event->button() == Qt::RightButton){
    event->ignore();

  }else if (event->button() == Qt::LeftButton){
    if (_selectable && !_selected){
      setSelected(true); QMap<QString, QString> settings; emit selected(getStructuralUid(), settings);
    }

    setPressTop(event->pos().y());
    setPressLeft(event->pos().x());
    setPressWidth(_width);
    setPressHeight(_height);

    if (_draggable){
      setDragging(true);

      QGraphicsItem::mousePressEvent(event);

      return;
    }

    // avoid flickering on first move
    if (_moveable){
      setMoveTop(_top);
      setMoveLeft(_left);
      setCursor(Qt::SizeAllCursor);

    }

    // avoid flickering on first resize
    if (_resizable){
      setResizeTop(_top);
      setResizeLeft(_left);
      setResizeWidth(_width);
      setResizeHeight(_height);
    }

    qreal W = DEFAULT_ANCHOR_WIDTH;
    qreal H = DEFAULT_ANCHOR_HEIGHT;

    if (_resizable){
      // if over TOPLEFT resize region
      if (QRectF(0,0,W,H).contains(event->pos())){
        setStructuralResize(Structural::TopLeft);
        setResizing(true);

        // if over TOP resize region
      }else if (QRectF((_width+W)/2 - W/2,0,W,H).contains(event->pos())){
        setStructuralResize(Structural::Top);
        setResizing(true);

        // if over TOPRIGHT resize region
      }else if (QRectF((_width+W) - W,0,W,H).contains(event->pos())){
        setStructuralResize(Structural::TopRight);
        setResizing(true);

        // if over RIGHT resize region
      }else if (QRectF((_width+W) - W,(_height+H)/2 - H/2,W,H).contains(event->pos())){
        setStructuralResize(Structural::Right);
        setResizing(true);

        // if over BOTTOMRIGHT resize region
      }else if (QRectF((_width+W) - W,(_height+H) - H,W,H).contains(event->pos())){
        setStructuralResize(Structural::BottomRight);
        setResizing(true);

        // if over BOTTOM resize region
      }else if (QRectF((_width+W)/2 - W/2,(_height+H) - H,W,H).contains(event->pos())){
        setStructuralResize(Structural::Bottom);
        setResizing(true);

        // if over BOTTOMLEFT resize region
      }else if (QRectF(0,(_height+H) - H,W,H).contains(event->pos())){
        setStructuralResize(Structural::BottomLeft);
        setResizing(true);

        // if over LEFT resize region
      }else if (QRectF(0,(_height+H)/2 - H/2,W,H).contains(event->pos())){
        setStructuralResize(Structural::Left);
        setResizing(true);

        // if not over any resize region
      }else if (_moveable){
        setMoving(true);
      }

    }else if (_moveable){
      setMoving(true);
    }
  }

  QGraphicsItem::mousePressEvent(event);
}

void StructuralEntity::mouseReleaseEvent(QGraphicsSceneMouseEvent*event)
{
  if (_moving){
    setMoving(false);

    if ((_top != _moveTop || _left != _moveLeft)){


      QMap<QString, QString> previous = getStructuralProperties();


      setTop(_moveTop);
      setLeft(_moveLeft);

      QMap<QString, QString> properties = getStructuralProperties();

      QMap<QString, QString> settings;
      settings[PLG_SETTING_UNDO] = "1";
      settings[PLG_SETTING_CODE] = StructuralUtil::CreateUid();
      settings["LOCATION"] = "INTERNAL MOVE";

      emit changed(getStructuralUid(), properties, previous,settings);


      //            emit entityChanged();


      /**************/

    }else{
      adjust();
    }

  }else if (_resizing){
    setResizing(false);

    QMap<QString, QString> previous = getStructuralProperties();

    if ((_top != _resizeTop || _left != _resizeLeft || _width != _resizeWidth || _height != _resizeHeight)){
      if (_resizeTop > _top + _height){
        setResizeHeight(_resizeTop - (_top + _height));
        setResizeTop(_top + _height);
      }

      if (_resizeLeft > _left + _width){
        setResizeWidth(_resizeLeft - (_left + _width));
        setResizeLeft(_left + _width);
      }

      if (_resizeWidth < 0){
        setResizeLeft(_resizeLeft + _resizeWidth);
        setResizeWidth(-_resizeWidth);
      }

      if (_resizeHeight < 0){
        setResizeTop(_resizeTop + _resizeHeight);
        setResizeHeight(-_resizeHeight);
      }

      foreach(StructuralEntity* entity, _children){
        entity->setTop(entity->getTop() - (_resizeTop - _top));
        entity->setLeft(entity->getLeft() - (_resizeLeft - _left));
      }

      //            QMap<QString, QString> properties;

      //            properties["top"] = QString::number(moveTop);
      //            properties["left"] = QString::number(moveLeft);
      //            properties["width"] = QString::number(resizeWidth);
      //            properties["height"] = QString::number(resizeHeight);

      //            emit entityAboutToChange(properties);

      setTop(_resizeTop);
      setLeft(_resizeLeft);
      setWidth(_resizeWidth);
      setHeight(_resizeHeight);

      adjust();

      QMap<QString, QString> properties = getStructuralProperties();

      //            emit entityChanged();

      QMap<QString, QString> settings;
      settings[PLG_SETTING_UNDO] = "1";
      settings["LOCATION"] = "INTERNAL MOVE";
      settings[PLG_SETTING_CODE] = StructuralUtil::CreateUid();

      emit changed(getStructuralUid(), properties, previous,settings);

    }else{
      adjust();
    }
  }

  QGraphicsItem::mouseReleaseEvent(event);
}

void StructuralEntity::updateCursor(QGraphicsSceneHoverEvent* event)
{
  if (_selected){
    if (_resizable){
      qreal W = DEFAULT_ANCHOR_WIDTH;
      qreal H = DEFAULT_ANCHOR_HEIGHT;

      // if over TOPLEFT resize region
      if (QRectF(0,0,W,H).contains(event->pos())){
        setCursor(Qt::SizeFDiagCursor);

        // if over TOP resize region
      }else if (QRectF((_width+W)/2 - W/2,0,W,H).contains(event->pos())){
        setCursor(Qt::SizeVerCursor);

        // if over TOPRIGHT resize region
      }else if (QRectF((_width+W) - W,0,W,H).contains(event->pos())){
        setCursor(Qt::SizeBDiagCursor);

        // if over RIGHT resize region
      }else if (QRectF((_width+W) - W,(_height+H)/2 - H/2,W,H).contains(event->pos())){
        setCursor(Qt::SizeHorCursor);

        // if over BOTTOMRIGHT resize region
      }else if (QRectF((_width+W) - W,(_height+H) - H,W,H).contains(event->pos())){
        setCursor(Qt::SizeFDiagCursor);

        // if over BOTTOM resize region
      }else if (QRectF((_width+W)/2 - W/2,(_height+H) - H,W,H).contains(event->pos())){
        setCursor(Qt::SizeVerCursor);

        // if over BOTTOMLEFT resize region
      }else if (QRectF(0,(_height+H) - H,W,H).contains(event->pos())){
        setCursor(Qt::SizeBDiagCursor);

        // if over LEFT resize region
      }else if (QRectF(0,(_height+H)/2 - H/2,W,H).contains(event->pos())){
        setCursor(Qt::SizeHorCursor);

        // if not over any resize region
      }else if (_moveable){
        setCursor(Qt::SizeAllCursor);
      }

    }else if (_moveable){
      setCursor(Qt::SizeAllCursor);

    }else{
      setCursor(Qt::ArrowCursor);
    }
  }else{
    setCursor(Qt::ArrowCursor);
  }

}

void StructuralEntity::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  updateCursor(event);

  QGraphicsItem::hoverEnterEvent(event);
}


void StructuralEntity::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  QGraphicsItem::contextMenuEvent(event);

  if (!event->isAccepted())
  {
    if (!isSelected())
    {
      setSelected(true);
      //emit entitySelected(this);
      QMap<QString, QString> settings; emit selected(getStructuralUid(), settings);
    }

    _insertPoint = event->pos();

    if (menu != NULL)
    {
      menu->updateInsertAction(_subtype);



      menu->exec(event->screenPos());
    }

    event->accept();
  }
}

bool StructuralEntity::hasMouseHover()
{
  return hover;
}

void StructuralEntity::setMouseHover(bool hover)
{
  this->hover = hover;
}

void StructuralEntity::setMouseHoverHighlight(bool enable)
{
  this->enableMouseHoverHighlight = enable;
}

void StructuralEntity::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
//  updateCursor(event);

  QGraphicsItem::hoverEnterEvent(event);

  hover = true;
}

void StructuralEntity::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
//  updateCursor(event);

  QGraphicsItem::hoverLeaveEvent(event);

  hover = false;
}

void StructuralEntity::setError(bool hasError)
{
  this->hasError = hasError;
}

void StructuralEntity::setErrorMsg(QString erroMsg)
{
  this->erroMsg = erroMsg;
}

void StructuralEntity::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{


  StructuralType dropEntity = (StructuralType) event->mimeData()->objectName().toInt();

  if (StructuralUtil::hasValidKinshp(dropEntity,_subtype)){
    if (event->mimeData()->text() == _uid)
      event->accept();
    else
      event->acceptProposedAction();
  }else{

    event->ignore();
  }
}
void StructuralEntity::dropEvent(QGraphicsSceneDragDropEvent* event)
{

  StructuralType dropEntity = (StructuralType) event->mimeData()->objectName().toInt();

  if (StructuralUtil::hasValidKinshp(dropEntity,_subtype)){
    if (event->mimeData()->text() == _uid)
      event->accept();
    else
      event->acceptProposedAction();

    emit move(event->mimeData()->text(), _uid);

  }else{
    event->accept();
  }
}

void StructuralEntity::drawMouseHoverHighlight(QPainter *painter)
{
  if(_hovering)
  {
    // Draw MouseHover rectangle
    if (!isSelected() && hasMouseHover())
    {
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QPen(QBrush(QColor("#999999")), 0, Qt::DashLine)); // 0px = cosmetic border

      painter->drawRect(DEFAULT_ANCHOR_WIDTH/2, DEFAULT_ANCHOR_HEIGHT/2, getWidth(), getHeight());
    }
  }
}

void StructuralEntity::performInsert(Structural::StructuralType name)
{

  QMap<QString, QString> properties;
  properties[PLG_ENTITY_CATEGORY] = QString::number(name);

  switch (name) {
    case Structural::Body:
    {
      properties[PLG_ENTITY_TOP] = QString::number(_insertPoint.y() - DEFAULT_BODY_HEIGHT/2);
      properties[PLG_ENTITY_LEFT] = QString::number(_insertPoint.x() - DEFAULT_BODY_WIDTH/2);
      properties[PLG_ENTITY_HEIGHT] = QString::number(DEFAULT_BODY_HEIGHT);
      properties[PLG_ENTITY_WIDTH] = QString::number(DEFAULT_BODY_WIDTH);

      break;
    }
    case Structural::Context:
    case Structural::Switch:
    {

      properties[PLG_ENTITY_TOP] = QString::number(_insertPoint.y() - DEFAULT_CONTEXT_HEIGHT/2);
      properties[PLG_ENTITY_LEFT] = QString::number(_insertPoint.x() - DEFAULT_CONTEXT_WIDTH/2);
      properties[PLG_ENTITY_HEIGHT] = QString::number(DEFAULT_CONTEXT_HEIGHT);
      properties[PLG_ENTITY_WIDTH] = QString::number(DEFAULT_CONTEXT_WIDTH);

      break;
    }

    case Structural::Media:
    {

      properties[PLG_ENTITY_TOP] = QString::number(_insertPoint.y() - DEFAULT_MEDIA_HEIGHT/2);
      properties[PLG_ENTITY_LEFT] = QString::number(_insertPoint.x() - DEFAULT_MEDIA_WIDTH/2);
      properties[PLG_ENTITY_HEIGHT] = QString::number(DEFAULT_MEDIA_HEIGHT);
      properties[PLG_ENTITY_WIDTH] = QString::number(DEFAULT_MEDIA_WIDTH);

      break;
    }

    default:
      break;
  }

  QMap<QString, QString> settings;
  settings[PLG_SETTING_UNDO] = "1";
  settings[PLG_SETTING_NOTIFY] = "1";
  settings[PLG_SETTING_CODE] = QUuid::createUuid().toString();

  inserted(QUuid::createUuid().toString(),getStructuralUid(), properties, settings);
}

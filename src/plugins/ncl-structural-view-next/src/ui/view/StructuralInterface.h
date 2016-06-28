#ifndef QNSTGRAPHICSINTERFACE_H
#define QNSTGRAPHICSINTERFACE_H

#include "StructuralEntity.h"

class StructuralInterface : public StructuralEntity
{
public:
  StructuralInterface(StructuralEntity* parent = 0);

  ~StructuralInterface();

  virtual void setStructuralType(const StructuralType _subtype);

  virtual void adjust(bool avoidCollision = true);

  bool isRefer();

  void setRefer(bool refer);

  void setHexColor(QString hexColor);
  void setHexBorderColor(QString hexBorderColor);

  virtual void refresh();

protected:
  virtual void draw(QPainter* painter);

  virtual void delineate(QPainterPath* painter) const;

  virtual void move(QGraphicsSceneMouseEvent* event);

  virtual void resize(QGraphicsSceneMouseEvent* event);

private:
  void adjustToBorder();

  bool _isRefer;

  QString hexColor;
  QString hexBorderColor;

  QPixmap icon;
};

#endif // QNSTGRAPHICSINTERFACE_H
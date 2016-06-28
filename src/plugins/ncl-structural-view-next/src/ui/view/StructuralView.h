#ifndef QNSTVIEW_H
#define QNSTVIEW_H

#include <QGraphicsView>
#include <QVector>
#include <QMap>
#include <QDomDocument>
#include <QDomElement>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWheelEvent>
#include <QSet>
#include <QUndoStack>
#include <QDir>
#include <QGraphicsScene>

#include <QDebug>

#include "StructuralScene.h"
#include "StructuralEntity.h"
#include "StructuralContent.h"
#include "StructuralInterface.h"
#include "StructuralComposition.h"

#include "Remove.h"
#include "Change.h"
#include "Insert.h"

#include "StructuralMinimap.h"

class QnstAddCommand;
class StructuralScene;
class StructuralMiniMap;

class StructuralView : public QGraphicsView
{
  Q_OBJECT

public:
  StructuralView(QWidget* _parent = 0);
  virtual ~StructuralView();

public:
  StructuralScene* getScene();

  bool hasEntity(QString uid);
  StructuralEntity* getEntity(QString uid);
  QMap<QString, StructuralEntity*> getEntities();

  void switchMinimapVis();

public slots:
  void insert(QString uid, QString parent, QMap<QString, QString> properties, QMap<QString, QString> settings);
  void remove(QString uid, QMap<QString, QString> settings);
  void change(QString uid, QMap<QString, QString> properties, QMap<QString, QString> previous, QMap<QString, QString> settings);
  void select(QString uid, QMap<QString, QString> settings);
  void move(QString uid, QString parent);

  void create(StructuralType type);
  void create(StructuralType type, QMap<QString, QString> &properties, QMap<QString, QString> &settings);

  void performHelp();

  void performCut();
  void performCopy();
  void performPaste();


  void performZoomIn();
  void performZoomOut();
  void performZoomReset();

  void performUndo();
  void performRedo();

  void performSnapshot();

  void performDelete();

  void performProperties();

public:
  void load(QString &data);

  void read(QDomElement element, QDomElement parent);

  QString serialize(QString data = "");
  void exportDataFromEntity(StructuralEntity* entity, QDomDocument* doc, QDomElement _parent);

  void unSelect();

   StructuralEntity* getBody();

   void clearErrors();
   void markError(QString uid, QString msg);

   void setMiniMapVisible(bool enable);


signals:
  void inserted(QString uid, QString _parent, QMap<QString, QString> properties, QMap<QString, QString> settings);
  void removed(QString uid, QMap<QString, QString> settings);
  void changed(QString uid, QMap<QString, QString> properties, QMap<QString, QString> previous, QMap<QString, QString> settings);
  void selected(QString uid, QMap<QString, QString> settings);


  void undoStateChange(bool state);
  void redoStateChange(bool state);
  void cutStateChange(bool state);
  void copyStateChange(bool state);
  void pasteStateChange(bool state);

  void zoominStateChange(bool state);

  void bodyStateChange(bool state);

  void selectChange(QString uid);

  void viewChanged();

protected:

  bool updateEntityWithUniqueNstId(StructuralEntity *entity);

  void resizeEvent(QResizeEvent *event);

  virtual void mouseMoveEvent(QMouseEvent* event);

  virtual void mousePressEvent(QMouseEvent* event);

  virtual void mouseReleaseEvent(QMouseEvent*event);

  virtual void keyPressEvent(QKeyEvent *event);

  virtual void keyReleaseEvent(QKeyEvent *event);

  virtual void focusOutEvent(QFocusEvent *event);

  void wheelEvent(QWheelEvent * event);

public slots:
  void clearAllData();

private:
  StructuralEntity* clone(StructuralEntity* e, StructuralEntity * p);

  void performPaste(StructuralEntity* entity, StructuralEntity* _parent, QString CODE, bool newPos);

  bool isChild(StructuralEntity* e , StructuralEntity* p);

//  void rec_clip(StructuralEntity* e, StructuralEntity* parent);

  void createObjects();

  void createConnection();

  void collapseCompositions(QDomElement element, QDomElement _parent);

  void deletePendingEntities();

  bool modified;

  bool linking;

  int zoomStep;

  bool hasCutted;



  QMap<QString, StructuralEntity*> entities;

  QAction* undoAct;

  QAction* redoAct;

  StructuralScene* scene;

  QString _selected_UID;

  StructuralEntity* clipboard;

  QString clip_cut;
  QString clip_copy;

  QSet<QString> linkWriterAux;

  QMap<QString, QString> importBases; // importBaseUid - ConnUid

  QMap<QString, QString> bindParamUIDToBindUID;

  //QMap<QString, QnstConnector*> connectors2; // uid - conn

  //QMap<QString, QnstConnector*> connectors; // id - conn

  QVector<StructuralEntity*> toDelete;

  QUndoStack commnads;

  StructuralEntity* lastLinkMouseOver;

  std::map < Structural::StructuralType, int > entityCounter;

  static std::map <Structural::StructuralType, QString> mediaTypeToXMLStr;

  StructuralMiniMap *minimap;


  StructuralEntity* e_clip;
};

#endif // QNSTVIEW_H
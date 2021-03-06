/*
 * Copyright 2011 TeleMidia/PUC-Rio.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "StructuralViewPlugin.h"

StructuralViewPlugin::StructuralViewPlugin(QObject* parent)
{
  createWidgets();
  createConnections();

  _synching = false;
  _waiting = false;

  _notified = "";     // last entity uid notified by the view

  _selected = "";
}

StructuralViewPlugin::~StructuralViewPlugin()
{
  delete(_window);
}

void StructuralViewPlugin::createWidgets()
{
  _window = new StructuralWindow();
}

void StructuralViewPlugin::createConnections()
{
  StructuralView* view = _window->getView();

  connect(view, SIGNAL(inserted(QString,QString,QMap<QString,QString>,QMap<QString,QString>)), SLOT(insertInCore(QString,QString,QMap<QString,QString>,QMap<QString,QString>)));
  connect(view, SIGNAL(removed(QString,QMap<QString,QString>)), SLOT(removeInCore(QString,QMap<QString,QString>)));
  connect(view, SIGNAL(changed(QString,QMap<QString,QString>,QMap<QString,QString>,QMap<QString,QString>)),SLOT(changeInCore(QString,QMap<QString,QString>,QMap<QString,QString>,QMap<QString,QString>)));
  connect(view, SIGNAL(selected(QString,QMap<QString,QString>)),SLOT(selectInCore(QString,QMap<QString,QString>)));

  connect(view, SIGNAL(requestedUpdate()), SLOT(adjustConnectors()));
}

void StructuralViewPlugin::init()
{
  QString data = project->getPluginData("br.puc-rio.telemidia.composer.structural");

  QString sc = "-- EXTRA DATA";
  QString ec = "-- END OF PLUGIN DATA";
  QString c = "";

  int si = data.indexOf(sc);
  int ei = data.indexOf(ec);
  int ci = si + sc.length();

  c = data.mid(ci, ei - ci);

  int li = 0;
  int ni = c.indexOf(":");

  while(ni > 0)
  {
    QStringList list = (c.mid(li,ni - li)).split("=");

    _mapCoreToView[list.at(0)] = list.at(1);
    _mapViewToCore[list.at(1)] = list.at(0);

    li = ni+1;
    ni = c.indexOf(":", ni+1);
  }

  sc = "-- VIEW DATA";
  ec = "-- EXTRA DATA";
  c = "";

  si = data.indexOf(sc);
  ei = data.indexOf(ec);
  ci = si + sc.length();

  c = data.mid(ci, ei - ci);

  StructuralView* view = _window->getView();
  view->load(c);
}

QWidget* StructuralViewPlugin::getWidget()
{
  return _window;
}

bool StructuralViewPlugin::saveSubsession()
{
  QByteArray data;

  data.append("-- BEGIN OF PLUGIN DATA");

  data.append("-- VIEW DATA");
  data.append(_window->getView()->save());

  data.append("-- EXTRA DATA");
  foreach(QString key, _mapCoreToView.keys())
  {
    data.append(key+"="+_mapCoreToView[key]+":");
  }

  data.append("-- END OF PLUGIN DATA");

  emit setPluginData(data);

  return true;
}

void StructuralViewPlugin::updateFromModel()
{
  //
  // Caching...
  //

  QMap <QString, QMap<QString,QString> > cache;

  foreach (const QString &key, _mapCoreToView.keys())
  {
    StructuralEntity* e = _window->getView()->getEntity(_mapCoreToView.value(key));

    if (e != NULL)
    {
      if (!e->getStructuralId().isEmpty())
      {
        QString pId = "";

        if (e->getStructuralParent() != NULL)
          pId = e->getStructuralParent()->getStructuralId();

        QMap<QString,QString> properties = e->getStructuralProperties();

        foreach (const QString &key, e->getStructuralProperties().keys())
        {
          if (key.contains(STR_PROPERTY_LINKPARAM_NAME) ||
              key.contains(STR_PROPERTY_LINKPARAM_VALUE) ||
              key.contains(STR_PROPERTY_BINDPARAM_NAME) ||
              key.contains(STR_PROPERTY_BINDPARAM_VALUE))

            properties.remove(key);
        }

        cache.insert(e->getStructuralId() + pId, properties);
      }
    }
  }

  //
  // Cleaning...
  //

  _window->getView()->clean();
  clean();

  //
  // Inserting...
  //

  QStack <Entity *> stack;
  stack.push(project);

  while(stack.size())
  {
    Entity *current = stack.top();
    stack.pop();

    foreach (Entity *child, current->getEntityChildren())
    {
      insertInView(child, false);
      stack.push(child);
    }
  }

  //
  // Settings...
  //
  QMap<QString, QString> references;
  references[STR_PROPERTY_REFERENCE_REFER_ID] = STR_PROPERTY_REFERENCE_REFER_UID;
  references[STR_PROPERTY_REFERENCE_COMPONENT_ID] = STR_PROPERTY_REFERENCE_COMPONENT_UID;
  references[STR_PROPERTY_REFERENCE_INTERFACE_ID] = STR_PROPERTY_REFERENCE_INTERFACE_UID;

  foreach (StructuralEntity* e, _window->getView()->getEntities().values())
  {
    foreach (const QString &r, references.keys())
    {
      QString pId = r;
      QString pUid = references.value(r);

      if (!e->getStructuralProperty(pId).isEmpty()) {
        QString coreUid = getUidById(e->getStructuralProperty(pId));

        if (_mapCoreToView.contains(coreUid))
          e->setStructuralProperty(pUid, _mapCoreToView.value(coreUid));
      }
    }

    _window->getView()->adjustReferences(e);
  }

  QStack<StructuralEntity*> s;

  foreach (StructuralEntity* current, _window->getView()->getEntities().values())
  {
    if (current->getStructuralParent() == nullptr)
      s.push(current);
  }

  while (!s.isEmpty())
  {
    StructuralEntity* e = s.pop();

    QString pId = "";

    if (e->getStructuralParent() != nullptr)
       pId = e->getStructuralParent()->getStructuralId();

    QMap<QString, QString> settings = StructuralUtil::createSettings(false, false);

    // Setting cached data...
    if (cache.contains(e->getStructuralId() + pId))
    {
      QMap<QString, QString> properties = cache.value(e->getStructuralId()+pId);
      properties.insert(STR_PROPERTY_ENTITY_UID, e->getStructuralUid());

      QMap<QString, QString> translations = StructuralUtil::createCoreTranslations(e->getStructuralType());

      foreach (const QString &translation, translations.values())
      {
        if (e->getStructuralProperty(translation).isEmpty())
          properties.remove(translation);

        if (e->getStructuralProperty(translation) != properties.value(translation))
          properties.insert(translation, e->getStructuralProperty(translation));
      }

      if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_REFER_ID).isEmpty())
      {
        properties[STR_PROPERTY_REFERENCE_REFER_ID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_REFER_ID);

        if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_REFER_UID).isEmpty())
          properties[STR_PROPERTY_REFERENCE_REFER_UID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_REFER_UID);
        else
          properties.remove(STR_PROPERTY_REFERENCE_REFER_UID);
      }

      if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_COMPONENT_ID).isEmpty())
      {
        properties[STR_PROPERTY_REFERENCE_COMPONENT_ID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_COMPONENT_ID);

        if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
          properties[STR_PROPERTY_REFERENCE_COMPONENT_UID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_COMPONENT_UID);
        else
          properties.remove(STR_PROPERTY_REFERENCE_COMPONENT_UID);
      }

      if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_INTERFACE_ID).isEmpty())
      {
        properties[STR_PROPERTY_REFERENCE_INTERFACE_ID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_INTERFACE_ID);

        bool hasReferInstead = false;

        if (_window->getView()->hasEntity(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID)))
        {
          StructuralEntity* re = _window->getView()->getEntity(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));

          foreach (StructuralEntity* cre, re->getStructuralEntities()) {
            if (cre->isReference())
              if (cre->getStructuralId() == properties.value(STR_PROPERTY_REFERENCE_INTERFACE_ID)){
                properties.insert(STR_PROPERTY_REFERENCE_INTERFACE_UID, cre->getStructuralUid());
                hasReferInstead = true;
                break;
              }
          }
        }

        if (!hasReferInstead)
        {
          if (!e->getStructuralProperty(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty())
            properties[STR_PROPERTY_REFERENCE_INTERFACE_UID] = e->getStructuralProperty(STR_PROPERTY_REFERENCE_INTERFACE_UID);
          else
            properties.remove(STR_PROPERTY_REFERENCE_INTERFACE_UID);
        }
      }

      // Setting 'bind' cached data...
      if (e->getStructuralType() == Structural::Bind)
      {
        if (!properties.value(STR_PROPERTY_ENTITY_ID).isEmpty())
        {
          StructuralRole role = StructuralUtil::translateStringToRole(properties.value(STR_PROPERTY_ENTITY_ID));

          QString coreBindUID = _mapViewToCore.value(e->getStructuralUid());
          QString viewLinkUID = _mapCoreToView.value(getProject()->getEntityById(coreBindUID)->getParentUniqueId());

          properties.insert(STR_PROPERTY_REFERENCE_LINK_UID, viewLinkUID);

          if (StructuralUtil::isCondition(role))
          {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()) {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_LINK_UID));
            }
            else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_LINK_UID));
            }

          }
          else if (StructuralUtil::isAction(role))
          {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_LINK_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
            }
            else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_LINK_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
            }
          }

          foreach (QString key, e->getStructuralProperties().keys())
          {
            if (key.contains(STR_PROPERTY_BINDPARAM_NAME) ||
                key.contains(STR_PROPERTY_BINDPARAM_VALUE))

              properties.insert(key, e->getStructuralProperty(key));
          }
        }

      // Setting 'link' cached data...
      }
      else if (e->getStructuralType() == Structural::Link)
      {
        foreach (QString key, e->getStructuralProperties().keys())
        {
          if (key.contains(STR_PROPERTY_LINKPARAM_NAME) ||
              key.contains(STR_PROPERTY_LINKPARAM_VALUE))

            properties.insert(key, e->getStructuralProperty(key));
        }
      }

      _window->getView()->change(e->getStructuralUid(),
                                 properties,
                                 e->getStructuralProperties(),settings);

    // Setting non cached data...
    }
    else
    {
      QMap<QString, QString> properties = e->getStructuralProperties();

      bool hasChanged = false;

      if (e->getStructuralType() == Structural::Port ||
          e->getStructuralType() == Structural::Bind ||
          e->getStructuralType() == Structural::Mapping)
      {

        if (properties.contains(STR_PROPERTY_REFERENCE_COMPONENT_ID))
        {
          QString coreUID = getUidById(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_ID));

          if (_mapCoreToView.contains(coreUID))
          {
            if (e->getStructuralProperty(STR_PROPERTY_REFERENCE_COMPONENT_UID) != _mapCoreToView.value(coreUID))
            {
              properties.insert(STR_PROPERTY_REFERENCE_COMPONENT_UID, _mapCoreToView.value(coreUID));
              hasChanged = true;
            }
          }
        }

        if (properties.contains(STR_PROPERTY_REFERENCE_INTERFACE_ID))
        {
          QString coreUID = getUidById(properties.value(STR_PROPERTY_REFERENCE_INTERFACE_ID));

          if (_mapCoreToView.contains(coreUID))
          {
            if (e->getStructuralProperty(STR_PROPERTY_REFERENCE_INTERFACE_UID) != _mapCoreToView.value(coreUID))
            {
              properties.insert(STR_PROPERTY_REFERENCE_INTERFACE_UID, _mapCoreToView.value(coreUID));
              hasChanged = true;
            }
          }
        }

        if (e->getStructuralType() == Structural::Bind)
        {
          if (StructuralUtil::isCondition(e->getStructuralProperty(STR_PROPERTY_ENTITY_ID)))
          {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, e->getStructuralProperty(STR_PROPERTY_REFERENCE_LINK_UID));
            }
            else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, e->getStructuralProperty(STR_PROPERTY_REFERENCE_LINK_UID));
            }

          }
          else if (StructuralUtil::isAction(e->getStructuralProperty(STR_PROPERTY_ENTITY_ID)))
          {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, e->getStructuralProperty(STR_PROPERTY_REFERENCE_LINK_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));

            }
            else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
            {
              properties.insert(STR_PROPERTY_EDGE_TAIL, e->getStructuralProperty(STR_PROPERTY_REFERENCE_LINK_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
            }
          }
        }
        else if (e->getStructuralType() == Structural::Mapping)
        {
          if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty())
          {
            properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
          }
          else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty())
          {
            properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
          }
        }
      }

      if (hasChanged)
        _window->getView()->change(e->getStructuralUid(),properties,e->getStructuralProperties(),settings);
    }

    foreach (StructuralEntity* c, e->getStructuralEntities())
      s.push(c);
  }

  foreach (QString key, _window->getView()->getEntities().keys())
  {
    if (_window->getView()->hasEntity(key))
    {
      StructuralEntity* r = _window->getView()->getEntity(key);

      if (r->getStructuralCategory() == Structural::Edge ||
          r->getStructuralType() == Structural::Port)
      {
        _window->getView()->adjustReferences(r);
      }
    }
  }
}

void StructuralViewPlugin::setReferences(QMap<QString, QString> &properties)
{
  if (properties.contains(STR_PROPERTY_REFERENCE_REFER_ID))
  {
    QString coreUID = getUidById(properties.value(STR_PROPERTY_REFERENCE_REFER_ID));

    if (_mapCoreToView.contains(coreUID))
      properties.insert(STR_PROPERTY_REFERENCE_REFER_UID, _mapCoreToView.value(coreUID));
  }

  if (properties.contains(STR_PROPERTY_REFERENCE_COMPONENT_ID))
  {
    QString coreUID = getUidById(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_ID));

    if (_mapCoreToView.contains(coreUID))
      properties.insert(STR_PROPERTY_REFERENCE_COMPONENT_UID, _mapCoreToView.value(coreUID));
  }

  if (properties.contains(STR_PROPERTY_REFERENCE_INTERFACE_ID))
  {
    bool hasReferInstead = false;

    QMap<QString,StructuralEntity*> entities = _window->getView()->getEntities();

    if (entities.contains(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID)))
    {
      StructuralEntity* entity = entities.value(properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));

      foreach (StructuralEntity* e, entity->getStructuralEntities())
      {
        if (e->isReference())
          if (e->getStructuralId() == properties.value(STR_PROPERTY_REFERENCE_INTERFACE_ID))
	  {
            properties.insert(STR_PROPERTY_REFERENCE_INTERFACE_UID, e->getStructuralUid());
            hasReferInstead = true;
            break;
          }
      }
    }

    if (!hasReferInstead)
    {
      QString coreUID = getUidById(properties.value(STR_PROPERTY_REFERENCE_INTERFACE_ID));

      if (_mapCoreToView.contains(coreUID))
      {
        properties.insert(STR_PROPERTY_REFERENCE_INTERFACE_UID, _mapCoreToView.value(coreUID));
      }
    }
  }
}

void StructuralViewPlugin::clean()
{
  _mapCoreToView.clear();
  _mapViewToCore.clear();
}

void StructuralViewPlugin::onEntityAdded(QString pluginID, Entity *entity)
{
  if(_synching)
    return;

  if (pluginID != getPluginInstanceID())
  {
    insertInView(entity);
  }
  else if (_waiting)
  {
    _mapCoreToView[entity->getUniqueId()] = _notified;
    _mapViewToCore[_notified] = entity->getUniqueId();

    _waiting = false;
  }
}

void StructuralViewPlugin::errorMessage(QString error)
{

}

void StructuralViewPlugin::onEntityChanged(QString pluginID, Entity *entity)
{
  if(_synching)
    return;

  if (pluginID != getPluginInstanceID() && !_waiting) {
    changeInView(entity);
  }

  _waiting = false;
}

void StructuralViewPlugin::onEntityRemoved(QString pluginID, QString entityID)
{
  if(_synching)
    return;

  if (pluginID != getPluginInstanceID()) {
      removeInView(getProject()->getEntityById(entityID));
      _mapCoreToView.remove(entityID);
  }
}

void StructuralViewPlugin::changeSelectedEntity(QString pluginID, void* param)
{
  if(_synching)
    return;

  if (pluginID != getPluginInstanceID()) {
    QString* entityUID = (QString*) param;

    if(entityUID != NULL) {
      Entity *entity = getProject()->getEntityById(*entityUID);

      if (entity != NULL)
        selectInView(entity);
    }
  }
}

void StructuralViewPlugin::insertInView(Entity* entity, bool undo)
{
  QMap<QString, QString> properties;

  Structural::StructuralType type = StructuralUtil::translateStringToType(entity->getType());

  if (type != Structural::NoType) {
    properties[STR_PROPERTY_ENTITY_TYPE] = entity->getType();

    _mapCoreToView[entity->getUniqueId()] = entity->getUniqueId();
    _mapViewToCore[entity->getUniqueId()] = entity->getUniqueId();

    QMap<QString, QString> translations = StructuralUtil::createCoreTranslations(type);

    if (!translations.isEmpty()) {
      foreach (QString key, translations.keys())
        if (!entity->getAttribute(key).isEmpty())
          properties.insert(translations.value(key),entity->getAttribute(key));

      QMap<QString,QString> settings = StructuralUtil::createSettings(undo, false);

      QString parentUID = entity->getParentUniqueId();
      parentUID = _mapCoreToView.value(entity->getParentUniqueId(),"");

      if (!undo)
        setReferences(properties);

      if (type == Structural::Bind) {
        parentUID = entity->getParent()->getParentUniqueId();
        assert (parentUID != NULL);

        if (!properties.value(STR_PROPERTY_ENTITY_ID).isEmpty()) {
          StructuralRole role = StructuralUtil::translateStringToRole(properties.value(STR_PROPERTY_ENTITY_ID));

          properties.insert(STR_PROPERTY_BIND_ROLE, StructuralUtil::translateRoleToString(role));
          properties.insert(STR_PROPERTY_REFERENCE_LINK_UID, _mapCoreToView.value(entity->getParentUniqueId()));

          if (StructuralUtil::isCondition(role)) {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, _mapCoreToView.value(entity->getParentUniqueId()));

            }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, _mapCoreToView.value(entity->getParentUniqueId()));

            }

          }else if (StructuralUtil::isAction(role)) {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));

            }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
            }
          }
        }
      } else if (type == Structural::Mapping) {
        parentUID = entity->getParent()->getParentUniqueId();

        if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
          properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
          properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));

        }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
          properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
          properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
        }
      }

      _window->getView()->insert(entity->getUniqueId(), parentUID, properties, settings);
    }

  } else if (entity->getType() == "linkParam" ||
             entity->getType() == "bindParam") {

    if (!entity->getAttribute("name").isEmpty() &&
        !entity->getAttribute("value").isEmpty()) {

      StructuralEntity* e = _window->getView()->getEntity(_mapCoreToView.value(entity->getParentUniqueId()));

      QString uid = entity->getUniqueId();
      QMap<QString, QString> previous = e->getStructuralProperties();
      QMap<QString, QString> next = e->getStructuralProperties();

      QString name;
      QString value;

      if (entity->getType() == "linkParam") {
        name = QString(STR_PROPERTY_LINKPARAM_NAME);
        value = QString(STR_PROPERTY_LINKPARAM_VALUE);

      } else {
        name = QString(STR_PROPERTY_BINDPARAM_NAME);
        value = QString(STR_PROPERTY_BINDPARAM_VALUE);
      }

      next[name+":"+uid] = entity->getAttribute("name");
      next[value+":"+uid] = entity->getAttribute("value");

      _mapCoreToView[uid] = uid;
      _mapViewToCore[uid] = uid;

      _window->getView()->change(e->getStructuralUid(), next, previous, StructuralUtil::createSettings(true, false));
    }
  }
}

void StructuralViewPlugin::removeInView(Entity* entity, bool undo)
{
    if(_mapCoreToView.contains(entity->getUniqueId())) {
      QMap<QString,QString> settings = StructuralUtil::createSettings(undo,false);

      if (entity->getType() == "linkParam" ||
          entity->getType() == "bindParam") {

        QString name;
        QString value;

        if (entity->getType() == "linkParam") {
          name = QString(STR_PROPERTY_LINKPARAM_NAME);
          value = QString(STR_PROPERTY_LINKPARAM_VALUE);

        } else {
          name = QString(STR_PROPERTY_BINDPARAM_NAME);
          value = QString(STR_PROPERTY_BINDPARAM_VALUE);
        }

        StructuralEntity* e = _window->getView()->getEntity(_mapCoreToView.value(entity->getParentUniqueId()));

        if (entity != NULL){
          QString uid = _mapCoreToView.value(entity->getUniqueId());
          QMap<QString, QString> previous = e->getStructuralProperties();
          QMap<QString, QString> next = e->getStructuralProperties();

          if (next.contains(name+":"+uid) ||
              next.contains(value+":"+uid)) {
            next.remove(name+":"+uid);
            next.remove(value+":"+uid);

            _window->getView()->change(e->getStructuralUid(), next, previous, StructuralUtil::createSettings(undo, false));
          }
        }
      } else {

        _window->getView()->remove(_mapCoreToView[entity->getUniqueId()], settings);
      }

      _mapViewToCore.remove(_mapCoreToView.value(entity->getUniqueId()));
      _mapCoreToView.remove(entity->getUniqueId());
    }
}

void StructuralViewPlugin::changeInView(Entity* entity)
{
  QMap<QString, QString> properties;

  Structural::StructuralType type = StructuralUtil::translateStringToType(entity->getType());

  if (type != Structural::NoType) {
    properties[STR_PROPERTY_ENTITY_TYPE] = entity->getType();

    QMap<QString, QString> translations = StructuralUtil::createCoreTranslations(type);

    foreach (QString key, translations.keys()) {
      if (!entity->getAttribute(key).isEmpty())
        properties.insert(translations.value(key), entity->getAttribute(key));
    }

    if(_mapCoreToView.contains(entity->getUniqueId())) {

      setReferences(properties);

      if (type == Structural::Bind) {
        if (!properties.value(STR_PROPERTY_ENTITY_ID).isEmpty()){
          StructuralRole role = StructuralUtil::translateStringToRole(properties.value(STR_PROPERTY_ENTITY_ID));

          properties.insert(STR_PROPERTY_BIND_ROLE, StructuralUtil::translateRoleToString(role));
          properties.insert(STR_PROPERTY_REFERENCE_LINK_UID, _mapCoreToView.value(entity->getParentUniqueId()));

          if (StructuralUtil::isCondition(role))
          {
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, _mapCoreToView.value(entity->getParentUniqueId()));

            }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
              properties.insert(STR_PROPERTY_EDGE_HEAD, _mapCoreToView.value(entity->getParentUniqueId()));
            }

          }else if (StructuralUtil::isAction(role)){
            if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));

            }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
              properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
              properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
            }
          }
        }
      } else if (type == Structural::Mapping) {
        if (!properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID).isEmpty()){
          properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
          properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_INTERFACE_UID));

        }else if (!properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID).isEmpty()){
          properties.insert(STR_PROPERTY_EDGE_TAIL, _mapCoreToView.value(entity->getParentUniqueId()));
          properties.insert(STR_PROPERTY_EDGE_HEAD, properties.value(STR_PROPERTY_REFERENCE_COMPONENT_UID));
        }
      }

      QMap<QString,QString> settings = StructuralUtil::createSettings(true,false);

      QString uid = _mapCoreToView[entity->getUniqueId()];

      if (_window->getView()->hasEntity(uid)){
        _window->getView()->change(uid, properties, _window->getView()->getEntity(uid)->getStructuralProperties(), settings);
      }
    }

  } else if (entity->getType() == "linkParam" ||
             entity->getType() == "bindParam") {

    StructuralEntity* e = _window->getView()->getEntity(_mapCoreToView.value(entity->getParentUniqueId()));

    QString uid;
    QMap<QString, QString> previous = e->getStructuralProperties();
    QMap<QString, QString> next = e->getStructuralProperties();

    QString name;
    QString value;

    if (entity->getType() == "linkParam") {
      name = QString(STR_PROPERTY_LINKPARAM_NAME);
      value = QString(STR_PROPERTY_LINKPARAM_VALUE);

    } else {
      name = QString(STR_PROPERTY_BINDPARAM_NAME);
      value = QString(STR_PROPERTY_BINDPARAM_VALUE);
    }

    if (!entity->getAttribute("name").isEmpty() &&
        !entity->getAttribute("value").isEmpty()) {

      if (_mapCoreToView.contains(entity->getUniqueId()))
        uid = _mapCoreToView.value(entity->getUniqueId());
      else
        uid = entity->getUniqueId();

      next[name+":"+uid] = entity->getAttribute("name");
      next[value+":"+uid] = entity->getAttribute("value");

      _mapCoreToView[uid] = uid;
      _mapViewToCore[uid] = uid;

      _window->getView()->change(e->getStructuralUid(), next, previous, StructuralUtil::createSettings(true, false));

    } else if (_mapCoreToView.contains(entity->getUniqueId())){
      uid = _mapCoreToView.value(entity->getUniqueId());

      next[name+":"+uid] = entity->getAttribute("name");
      next[value+":"+uid] = entity->getAttribute("value");

      _window->getView()->change(e->getStructuralUid(), next, previous, StructuralUtil::createSettings(true, false));
    }
  }
}

void StructuralViewPlugin::selectInView(Entity* entity)
{
  if (_mapCoreToView.contains(entity->getUniqueId()) && _selected != entity->getUniqueId()) {
    _window->getView()->select(_mapCoreToView[entity->getUniqueId()], StructuralUtil::createSettings());

  } else {
    _window->getView()->select("", StructuralUtil::createSettings());
  }
}

void StructuralViewPlugin::insertInCore(QString uid, QString parent, QMap<QString, QString> properties,QMap<QString, QString> settings)
{
  StructuralType type = StructuralUtil::translateStringToType(properties[STR_PROPERTY_ENTITY_TYPE]);

  Entity* entityParent = NULL;

  if (type == Structural::Bind)
  {
    entityParent = getProject()->getEntityById(_mapViewToCore.value(properties.value(STR_PROPERTY_REFERENCE_LINK_UID)));

  }
  else if (type == Structural::Mapping)
  {
    entityParent = getProject()->getEntityById(_mapViewToCore.value(properties.value(STR_PROPERTY_EDGE_TAIL)));

  }
  else if (type == Structural::Body)
  {
    QList<Entity*> list;

    // Check if core already has a 'body' entity.
    // If so, update references.
    list = getProject()->getEntitiesbyType("body");

    if (!list.isEmpty())
    {
      Entity* body = list.first();

      // Cleaning
      _mapViewToCore.remove(_mapCoreToView[body->getUniqueId()]);

      // Updating
      _mapCoreToView[body->getUniqueId()] = uid;
      _mapViewToCore[uid] = body->getUniqueId();

      // Finishing
      return;
    }

    // Check if core already has a 'ncl' entity.
    // If don't, adds and set entity as parent.
    list = getProject()->getEntitiesbyType("ncl");

    if (list.isEmpty())
    {
      Entity* project = getProject();
      if (project != NULL)
      {
        QMap<QString, QString> attributes;
        emit addEntity("ncl", project->getUniqueId(), attributes);
      }

      list = getProject()->getEntitiesbyType("ncl");

      if (!list.isEmpty())
        entityParent = list.first();
    }
  }
  else if (parent.isEmpty() && !STR_DEFAULT_WITH_BODY)
  {
    QList<Entity*> list = getProject()->getEntitiesbyType("body");

    if (!list.isEmpty())
      entityParent = list.first();
  }
  else
  {
    entityParent = getProject()->getEntityById(_mapViewToCore.value(parent));
  }

  if(entityParent != NULL) {
    QMap<QString, QString> attributes;

    QMap<QString, QString> translations = StructuralUtil::createPluginTranslations(type);

    foreach (QString key, translations.keys()) {
      if (!properties.value(key).isEmpty())
        attributes.insert(translations.value(key), properties.value(key));
    }

    _waiting = true;
    _notified = uid;

    emit addEntity(StructuralUtil::translateTypeToString(type), entityParent->getUniqueId(), attributes);

    if (type == Structural::Link ||
        type == Structural::Bind) {

      QString tag;
      QString name;
      QString value;

      if (type == Structural::Link)
      {
        tag = "linkParam";
        name = QString(STR_PROPERTY_LINKPARAM_NAME);
        value = QString(STR_PROPERTY_LINKPARAM_VALUE);

      } else {
        tag = "bindParam";
        name = QString(STR_PROPERTY_BINDPARAM_NAME);
        value = QString(STR_PROPERTY_BINDPARAM_VALUE);
      }

      foreach (QString key, properties.keys()) {
        if (key.contains(name)){
          QString pUid = key.right(key.length() - key.lastIndexOf(':') - 1);

          QMap<QString, QString> pAttr;
          pAttr.insert("name", properties.value(key));
          pAttr.insert("value", properties.value(value+":"+pUid));

          _waiting = true;
          _notified = pUid;

          emit addEntity(tag, _mapViewToCore.value(uid), pAttr);
        }
      }
    }
  }
}

void StructuralViewPlugin::removeInCore(QString uid, QMap<QString, QString> settings)
{
  if (!_mapViewToCore.value(uid, "").isEmpty()) {
    emit removeEntity(getProject()->getEntityById(_mapViewToCore.value(uid)));

    _mapCoreToView.remove(_mapViewToCore.value(uid));
    _mapViewToCore.remove(uid);
  }
}

void StructuralViewPlugin::selectInCore(QString uid, QMap<QString, QString> settings)
{
  Q_UNUSED(settings);

  if (!_mapViewToCore.value(uid, "").isEmpty()) {
    emit sendBroadcastMessage("changeSelectedEntity", new QString(_mapCoreToView.key(uid)));
  }
}

void StructuralViewPlugin::changeInCore(QString uid, QMap<QString, QString> properties, QMap<QString, QString> previous, QMap<QString, QString> settings)
{
  Entity* entity = getProject()->getEntityById(_mapViewToCore.value(uid));

  if (entity != NULL) {
    QMap<QString, QString> attributes;

    Structural::StructuralType type = StructuralUtil::translateStringToType(properties[STR_PROPERTY_ENTITY_TYPE]);

    QMap<QString, QString> translations = StructuralUtil::createPluginTranslations(type);

    foreach (QString key, translations.keys()) {
      if (!properties.value(key).isEmpty())
        attributes.insert(translations.value(key), properties.value(key));
    }

    if (!STR_DEFAULT_WITH_BODY &&
        !STR_DEFAULT_WITH_FLOATING_INTERFACES)
    {
      if (properties.contains(STR_PROPERTY_ENTITY_AUTOSTART))
      {
        bool isInterface = false;

        if (properties[STR_PROPERTY_ENTITY_CATEGORY] ==
            StructuralUtil::translateCategoryToString(Structural::Interface))
        {
          isInterface = true;
        }

        bool hasPort = false;

        QList<Entity*> list = getProject()->getEntitiesbyType("port");

        foreach (Entity* current, list)
        {
          if (isInterface && current->getAttribute("interface") == properties[STR_PROPERTY_ENTITY_ID])
          {
            hasPort = true;

            if (properties[STR_PROPERTY_ENTITY_AUTOSTART] == STR_VALUE_FALSE)
               _window->getView()->remove(_mapCoreToView[current->getUniqueId()], settings);
          }
          else if (current->getAttribute("component") == properties[STR_PROPERTY_ENTITY_ID] &&
                   current->getAttribute("interface").isEmpty())
          {
            hasPort = true;

            if (properties[STR_PROPERTY_ENTITY_AUTOSTART] == STR_VALUE_FALSE)
               _window->getView()->remove(_mapCoreToView[current->getUniqueId()], settings);
          }
        }

        if (!hasPort && (properties[STR_PROPERTY_ENTITY_AUTOSTART] == STR_VALUE_TRUE))
        {
          QString parent = _mapCoreToView.value(entity->getParentUniqueId());

          QMap<QString, QString> pp;
          pp[STR_PROPERTY_ENTITY_TYPE] = StructuralUtil::translateTypeToString(Structural::Port);
          pp.insert(STR_PROPERTY_ENTITY_ID, "p"+properties.value(STR_PROPERTY_ENTITY_ID));

          if (isInterface)
          {
            pp.insert(STR_PROPERTY_REFERENCE_INTERFACE_ID, entity->getAttribute("id"));
            pp.insert(STR_PROPERTY_REFERENCE_COMPONENT_ID, ((Entity *)entity->getParent())->getAttribute("id"));

            parent = _mapCoreToView.value(entity->getParent()->getParentUniqueId());
          }
          else
          {
            pp.insert(STR_PROPERTY_REFERENCE_COMPONENT_ID, properties[STR_PROPERTY_ENTITY_ID]);
          }

          setReferences(pp);

          _window->getView()->insert(StructuralUtil::createUid(),
                                      parent,
                                      pp,
                                      settings);
        }
      }
    }

    if (type == Structural::Link ||
        type == Structural::Bind)
    {
      QString tag;
      QString name;
      QString value;

      if (type == Structural::Link) {
        tag = "linkParam";
        name = QString(STR_PROPERTY_LINKPARAM_NAME);
        value = QString(STR_PROPERTY_LINKPARAM_VALUE);
      } else {
        tag = "bindParam";
        name = QString(STR_PROPERTY_BINDPARAM_NAME);
        value = QString(STR_PROPERTY_BINDPARAM_VALUE);
      }

      QVector<QString> paramUids;
      QVector<QString> paramNames;

      foreach (Entity* c, entity->getEntityChildren()) {
        if (c->getType() == tag) {
          paramUids.append(c->getUniqueId());
          paramNames.append(c->getAttribute("name"));
        }
      }

      foreach (QString key, properties.keys()) {
        if (key.contains(name)){
          QString pUid = key.right(key.length() - key.lastIndexOf(':') - 1);

          QString pName = properties.value(key);
          QString pValue = properties.value(value+":"+pUid);

          QMap<QString, QString> pAttr;
          pAttr.insert("name", pName);
          pAttr.insert("value", pValue);

          Entity* param = getProject()->getEntityById(_mapViewToCore.value(pUid));

          // Change a param entity in core that
          // has been added by the view.
          if (param != NULL){
            if (paramUids.contains(param->getUniqueId()))
              paramUids.remove(paramUids.indexOf(param->getUniqueId()));

            _waiting = true;

            emit setAttributes(param, pAttr);

          // Change a param entity in core that has not been added
          // by the view and contains a empty 'value' attribute.
          }else if (paramNames.contains(pName)){
            int index = paramNames.indexOf(pName);

            param = getProject()->getEntityById(paramUids.at(index));
            paramUids.remove(index);

            _mapCoreToView[param->getUniqueId()] = pUid;
            _mapViewToCore[pUid] = param->getUniqueId();

            _waiting = true;

            emit setAttributes(param, pAttr);

          } else {
            _waiting = true;
            _notified = pUid;

            emit addEntity(tag, entity->getUniqueId(), pAttr);
          }
        }
      }

      foreach (QString pUid, paramUids)
        emit removeEntity(getProject()->getEntityById(pUid));
    }

    _waiting = true;

    emit setAttributes(entity, attributes);
  }
}

void StructuralViewPlugin::textualStartSync(QString, void*)
{
  _synching = true;
}

void StructuralViewPlugin::textualFinishSync(QString, void*)
{
  _synching = false;
  updateFromModel();
}

void StructuralViewPlugin::clearValidationMessages(QString, void *param)
{
  _window->getView()->cleanErrors();
}

void StructuralViewPlugin::validationError(QString pluginID, void *param)
{
  if(_synching)
    return;

  if(param)
  {
    pair <QString, QString> *p = (pair <QString, QString> *)param;

    if (_mapCoreToView.contains(p->first))
      _window->getView()->setError(_mapCoreToView.value(p->first), p->second);
  }
}

QString StructuralViewPlugin::getUidById(const QString &id)
{
  return getUidById(id, getProject());
}

QString StructuralViewPlugin::getUidById(const QString &id, Entity* entity)
{
  QString uid = "";

  if (entity->getType() == "property") {
    if (entity->getAttribute("name") == id)
      return entity->getUniqueId();
  } else {
    if (entity->getAttribute("id") == id)
      return entity->getUniqueId();
  }

  foreach(Entity* child, entity->getEntityChildren ()) {
    QString result = getUidById(id, child);

    if (result != "") {
      uid = result;

      break;
    }
  }

  return uid;
}

QString StructuralViewPlugin::getUidByName(const QString &name, Entity* entity)
{
  QString uid = "";

  if (entity->getAttribute("name") == name)
    return entity->getUniqueId();

  foreach(Entity* child, entity->getEntityChildren()) {
    QString result = getUidByName(name, child);

    if (result != "") {
      uid = result;
      break;
    }
  }

  return uid;
}

void StructuralViewPlugin::adjustConnectors()
{
  // TODO: this function should be calling only when a change occurs in
  // <connectorBase>. Currently, this function is calling every time the
  // link's dialog is displayed.
  if (!getProject()->getEntitiesbyType("connectorBase").isEmpty()){
    Entity* connectorBase = getProject()->getEntitiesbyType("connectorBase").first();

    QMap<QString, QVector<QString> > conditions;
    QMap<QString, QVector<QString> > actions;
    QMap<QString, QVector<QString> > params;

    foreach (Entity* e, connectorBase->getEntityChildren())
    {
      // loading data from local causalConnector
      if (e->getType() == "causalConnector") {
        QString connId;
        QVector<QString> connConditions;
        QVector<QString> connActions;
        QVector<QString> connParams;

        connId = e->getAttribute("id");

        foreach (Entity* ec, e->getEntityChildren()) {
          if (ec->getType() == "simpleCondition") {
            connConditions.append(ec->getAttribute("role"));
          }

          if (ec->getType() == "simpleAction") {
            connActions.append(ec->getAttribute("role"));
          }

          if (ec->getType() == "connectorParam") {
            connParams.append(ec->getAttribute("name"));
          }

          if (ec->getType() == "compoundCondition") {
            foreach (Entity* ecc, ec->getEntityChildren()) {
              if (ecc->getType() == "simpleCondition") {
                connConditions.append(ecc->getAttribute("role"));
              }
            }
          }

          if (ec->getType() == "compoundAction") {
            foreach (Entity* ecc, ec->getEntityChildren()) {
              if (ecc->getType() == "simpleAction") {
                connActions .append(ecc->getAttribute("role"));
              }
            }
          }
        }

        conditions.insert(connId, connConditions);
        actions.insert(connId, connActions);
        params.insert(connId, connParams);

      // loading data from importBases
      }else if (e->getType() == "importBase") {
        QString importAlias;
        QString importURI;

        QString projectURI;

        importAlias = e->getAttribute("alias");
        importURI = e->getAttribute("documentURI");

        // projectURI use '/' as separator
        projectURI = getProject()->getLocation();

        QFile importFile(projectURI.left(projectURI.lastIndexOf("/"))+"/"+importURI);

        if (importFile.open(QIODevice::ReadOnly)){
          QDomDocument d;
          if (d.setContent(&importFile)){

            QDomElement r = d.firstChildElement(); // <ncl>
            QDomElement h = r.firstChildElement(); // <head>
            QDomElement b;                         // <connectorBase>

            bool hasConnBase = false;

            QDomNodeList hc = h.childNodes();
            for (unsigned int i = 0; i < hc.length(); i++){
              if (hc.item(i).isElement()){

                if (hc.item(i).nodeName() == "connectorBase"){
                  b = hc.item(i).toElement();
                  hasConnBase = true;
                  break;
                }
              }
            }

            if (hasConnBase){
              QDomNodeList bc = b.childNodes();
              for (unsigned int i = 0; i < bc.length(); i++){
                if (bc.item(i).isElement()){
                  if (bc.item(i).nodeName() == "causalConnector"){
                    QString connId;
                    QVector<QString> connConditions;
                    QVector<QString> connActions;
                    QVector<QString> connParams;

                    if (bc.item(i).attributes().namedItem("id").isNull())
                      break;

                    connId = bc.item(i).attributes().namedItem("id").nodeValue();
                    connId = importAlias+"#"+connId;

                    QDomNodeList bcc = bc.item(i).childNodes();
                    for (unsigned int j = 0; j < bcc.length(); j++){
                      if (bcc.item(j).nodeName() == "simpleCondition"){
                        if (bcc.item(j).attributes().namedItem("role").isNull())
                          break;

                        connConditions.append(bcc.item(j).attributes().namedItem("role").nodeValue());
                      }

                      if (bcc.item(j).nodeName() == "simpleAction"){
                        if (bcc.item(j).attributes().namedItem("role").isNull())
                          break;

                        connActions.append(bcc.item(j).attributes().namedItem("role").nodeValue());
                      }

                      if (bcc.item(j).nodeName() == "connectorParam"){
                        if (bcc.item(j).attributes().namedItem("name").isNull())
                          break;

                        connParams.append(bcc.item(j).attributes().namedItem("name").nodeValue());
                      }

                      if (bcc.item(j).nodeName() == "compoundCondition"){
                        QDomNodeList bccc = bcc.item(j).childNodes();
                        for (unsigned int k = 0; k < bccc.length(); k++){
                          if (bccc.item(k).nodeName() == "simpleCondition"){
                            if (bccc.item(k).attributes().namedItem("role").isNull())
                              break;

                            connConditions.append(bccc.item(k).attributes().namedItem("role").nodeValue());
                          }
                        }
                      }

                      if (bcc.item(j).nodeName() == "compoundAction"){
                        QDomNodeList bccc = bcc.item(j).childNodes();
                        for (unsigned int k = 0; k < bccc.length(); k++){
                          if (bccc.item(k).nodeName() == "simpleAction"){
                            if (bccc.item(k).attributes().namedItem("role").isNull())
                              break;

                            connActions.append(bccc.item(k).attributes().namedItem("role").nodeValue());
                          }
                        }
                      }
                    }

                    conditions.insert(connId, connConditions);
                    actions.insert(connId, connActions);
                    params.insert(connId, connParams);
                  }
                }
              }
            }
          }
        }
      }
    }

    _window->getView()->getDialog()->setBase(conditions, actions, params);
  }
}

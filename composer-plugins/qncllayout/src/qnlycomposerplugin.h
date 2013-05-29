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
#ifndef QNLYCOMPOSERPLUGIN_H
#define QNLYCOMPOSERPLUGIN_H

#include <QString>
#include <QMap>

#include "ui/qnlymainwindow.h"

#include <core/extensions/IPlugin.h>
using namespace composer::extension;

#include "ui/view/qnlyview.h"

class QnlyComposerPlugin : public IPlugin
{
    Q_OBJECT

public:
    QnlyComposerPlugin(QObject* parent = 0);

    ~QnlyComposerPlugin();

    virtual void init();

    virtual QWidget* getWidget();

    virtual bool saveSubsession();

public slots:

    virtual void onEntityAdded(QString pluginID, Entity *entity);

    virtual void errorMessage(QString error);

    virtual void onEntityChanged(QString pluginID, Entity *entity);

    virtual void onEntityRemoved(QString pluginID, QString entityID);

    virtual void changeSelectedEntity(QString pluginID, void* entityUID);

    virtual void updateFromModel();

protected slots:
    void addRegionToView(Entity* entity);

    void removeRegionFromView(QString entityUID);

    void changeRegionInView(Entity* entity);

    void selectRegionInView(QString entityUID);

    void addRegionBaseToView(Entity* entity);

    void removeRegionBaseFromView(QString entityUID);

    void changeRegionBaseInView(Entity* entity);

    void selectRegionBaseInView(QString entityUID);

    void addRegion(const QString regionUID,
                      const QString parentUID,
                      const QString regionbaseUID,
                      const QMap<QString, QString> attributes);

    void removeRegion(const QString regionUID,
                      const QString regionbaseUID);

    void changeRegion(const QString regionUID,
                      const QString regionbaseUID,
                      const QMap<QString, QString> attributes);

    void selectRegion(const QString regionUID,
                      const QString regionbaseUID);

    void addRegionBase(const QString regionbaseUID,
                       const QMap<QString, QString> attributes);

    void removeRegionBase(const QString regionbaseUID);

    void changeRegionBase(const QString regionbaseUID,
                          const QMap<QString, QString> attributes);

    void selectRegionBase(const QString regionbaseUID);

    void performMediaOverRegionAction(const QString mediaId,
                                      const QString regionUID);

private:
    void clear();

    void loadRegionbase();

    void loadRegion(Entity* region);

    /*!
     * \brief Get the head id of the document. If this head does not exists
     *  this function will create it.
     */
    QString getHeadUid();

    QMap <QString, QString> getRegionAttributes(Entity *region);

    void createView();

    void createConnections();

    QnlyMainWindow *mainWindow;

    QnlyView* view;

    QMap<QString, Entity*> regions;

    QMap<QString, Entity*> regionbases;

    QMap<QString, QString> relations;

    QString *selectedId;
};

#endif // QNLYCOMPOSERPLUGIN_H

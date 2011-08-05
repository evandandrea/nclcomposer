/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef QNLYCOMPOSERPLUGIN_H
#define QNLYCOMPOSERPLUGIN_H

#include <QString>
#include <QMap>

#include <core/extensions/IPlugin.h>
using namespace composer::extension;

#include "ui/view/qnlyview.h"

namespace composer {
    namespace plugin {
        namespace layout {

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

private:
    void createView();

    void createConnections();

    QnlyView* view;

    QMap<QString, Entity*> regions;

    QMap<QString, Entity*> regionbases;

    QMap<QString, QString> relations;

    QString *selectedId;
};

} } } // end namespace

#endif // QNLYCOMPOSERPLUGIN_H

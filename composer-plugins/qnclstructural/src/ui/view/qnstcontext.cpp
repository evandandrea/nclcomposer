/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#include "qnstcontext.h"

QnstContext::QnstContext(QnstNode* parent)
    : QnstComposite(parent)
{
    setColor("#FFD39B");
    setBordertColor("#8B5A2B");

    setnstType(Qnst::Context);

    createConnection();
}

QnstContext::~QnstContext()
{

}

void QnstContext::addContext()
{
    QnstContext* context = new QnstContext(this);
    context->setParentItem(this);
    context->setTop(getHeight()/2-35);
    context->setLeft(getWidth()/2-35);
    context->setWidth(100);
    context->setHeight(100);
    context->setnstParent(this);

    context->adjust();

    connect(context,SIGNAL(entitySelected(QnstEntity*)),
                 SIGNAL(entitySelected(QnstEntity*)));

    connect(context,SIGNAL(entityAdded(QnstEntity*)),
            SIGNAL(entityAdded(QnstEntity*)));

    connect(context,SIGNAL(entityRemoved(QnstEntity*)),
            SIGNAL(entityRemoved(QnstEntity*)));

    emit entityAdded(context);
}

void QnstContext::addSwitch()
{
    QnstSwitch* sswitch = new QnstSwitch(this);
    sswitch->setParentItem(this);
    sswitch->setTop(getHeight()/2-35);
    sswitch->setLeft(getWidth()/2-35);
    sswitch->setWidth(100);
    sswitch->setHeight(100);
    sswitch->setnstParent(this);
    sswitch->adjust();

    connect(sswitch,SIGNAL(entitySelected(QnstEntity*)),
                 SIGNAL(entitySelected(QnstEntity*)));

    connect(sswitch,SIGNAL(entityAdded(QnstEntity*)),
            SIGNAL(entityAdded(QnstEntity*)));

    connect(sswitch,SIGNAL(entityRemoved(QnstEntity*)),
            SIGNAL(entityRemoved(QnstEntity*)));

    emit entityAdded(sswitch);
}

void QnstContext::addPort()
{
    QnstPort* port = new QnstPort(this);
    port->setParentItem(this);
    port->setTop(0);
    port->setLeft(0);
    port->setWidth(28);
    port->setHeight(28);
    port->setnstParent(this);
    port->adjust();

    connect(port,SIGNAL(entitySelected(QnstEntity*)),
                 SIGNAL(entitySelected(QnstEntity*)));

    connect(port,SIGNAL(entityAdded(QnstEntity*)),
            SIGNAL(entityAdded(QnstEntity*)));

    connect(port,SIGNAL(entityRemoved(QnstEntity*)),
            SIGNAL(entityRemoved(QnstEntity*)));

    emit entityAdded(port);
}

void QnstContext::addProperty()
{
    QnstProperty* property = new QnstProperty(this);
    property->setParentItem(this);
    property->setTop(0);
    property->setLeft(0);
    property->setWidth(35);
    property->setHeight(35);
    property->setnstParent(this);
    property->adjust();

    connect(property,SIGNAL(entitySelected(QnstEntity*)),
                 SIGNAL(entitySelected(QnstEntity*)));

    connect(property,SIGNAL(entityAdded(QnstEntity*)),
            SIGNAL(entityAdded(QnstEntity*)));

    connect(property,SIGNAL(entityRemoved(QnstEntity*)),
            SIGNAL(entityRemoved(QnstEntity*)));

    emit entityAdded(property);
}

void QnstContext::addMedia()
{
    QnstMedia* media = new QnstMedia(this);
    media->setParentItem(this);
    media->setTop(getHeight()/2-35);
    media->setLeft(getWidth()/2-35);
    media->setWidth(35);
    media->setHeight(35);
    media->setnstParent(this);
    media->adjust();

    connect(media,SIGNAL(entitySelected(QnstEntity*)),
                 SIGNAL(entitySelected(QnstEntity*)));

    connect(media,SIGNAL(entityAdded(QnstEntity*)),
            SIGNAL(entityAdded(QnstEntity*)));


    connect(media,SIGNAL(entityRemoved(QnstEntity*)),
            SIGNAL(entityRemoved(QnstEntity*)));

    emit entityAdded(media);
}

void QnstContext::createConnection()
{
    connect(switchAction, SIGNAL(triggered()), SLOT(addSwitch()));

    connect(contextAction, SIGNAL(triggered()), SLOT(addContext()));

    connect(portAction, SIGNAL(triggered()), SLOT(addPort()));

    connect(propertyAction, SIGNAL(triggered()), SLOT(addProperty()));

    connect(mediaAction, SIGNAL(triggered()), SLOT(addMedia()));
}

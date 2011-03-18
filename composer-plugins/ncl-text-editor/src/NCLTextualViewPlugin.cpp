#include "NCLTextualViewPlugin.h"

NCLTextualViewPlugin::NCLTextualViewPlugin()
{
    window = new NCLTextEditorMainWindow();
    doc = NULL;
    connect(window, SIGNAL(elementAdded(QString,QString,QMap<QString,QString>&,bool)),
            this, SIGNAL(addEntity(QString,QString,QMap<QString,QString>&,bool)));
}

NCLTextualViewPlugin::~NCLTextualViewPlugin()
{
    delete window;
    window = NULL;
}

QWidget* NCLTextualViewPlugin::getWidget()
{
    return window;
}

void NCLTextualViewPlugin::onEntityAdded(QString pluginID, Entity *entity)
{
    //Return if this is my call to onEntityAdded
    if(pluginID == getPluginID())
        return;

    QString line = "<" + entity->getType() + "";

    int insertAtLine = 0;
    //get the number line where the new element must be inserted
    if(entity->getParentUniqueId() != NULL) {
        insertAtLine = lineOfEntity.value(entity->getParentUniqueId());
    }
    QMap <QString, QString>::iterator begin, end, it;
    entity->getAttributeIterator(begin, end);
    for (it = begin; it != end; ++it)
    {
        line += " " + it.key() + "=\"" + it.value() + "\"";
    }
    line += ">\n";
    line += "</" + entity->getType() + ">\n";

    //update all previous entities line numbers (when necessary)
    QString key;
    foreach(key, lineOfEntity.keys())
    {
        if(lineOfEntity[key] >= insertAtLine+1)
            lineOfEntity[key] += 2;

    }

    window->getTextEditor()->insertAt(line, insertAtLine, 0);
    lineOfEntity[entity->getUniqueId()] = insertAtLine+1;

    //fix indentation
    int lineident = window->getTextEditor()
                        ->SendScintilla( QsciScintilla::SCI_GETLINEINDENTATION,
                                         insertAtLine-1 );

    window->getTextEditor()
            ->SendScintilla( QsciScintilla::SCI_GETLINEINDENTATION,
                             insertAtLine-1 );

    window->getTextEditor()
            ->SendScintilla( QsciScintilla::SCI_SETLINEINDENTATION,
                             insertAtLine,
                             lineident+8);

    window->getTextEditor()
            ->SendScintilla( QsciScintilla::SCI_SETLINEINDENTATION,
                             insertAtLine+1,
                             lineident+8);

    window->getTextEditor()->setCursorPosition(insertAtLine, 0);
    window->getTextEditor()->ensureLineVisible(insertAtLine);
    window->getTextEditor()->SendScintilla(QsciScintilla::SCI_SETFOCUS, true);

    /* qDebug() << "NCLTextualViewPlugin::onEntityAdded" <<
            entity->getType() << " " << insertAtLine; */
}

void NCLTextualViewPlugin::onEntityAddError(QString error)
{
    //qDebug() << "NCLTextualViewPlugin::onEntityAddError(" << error << ")";
}

void NCLTextualViewPlugin::onEntityChanged(QString ID, Entity *entity)
{
    QString line = "PLUGIN (" + ID + ") changed the Entity (" +
                   entity->getType() + " - " + entity->getUniqueId() +")";
    //TODO: All
}

void NCLTextualViewPlugin::onEntityChangeError(QString error)
{
    //TODO: All
}

void NCLTextualViewPlugin::onEntityAboutToRemove(Entity *)
{

}

void NCLTextualViewPlugin::onEntityRemoved(QString pluginID, QString entityID)
{
    //skip if this is my own call to onEntityRemoved
    if(pluginID == getPluginID())
        return;

    int lineOfRemovedEntity = lineOfEntity[entityID];
    window->getTextEditor()->setSelection( lineOfRemovedEntity-1,
                                           0,
                                           lineOfRemovedEntity-1,
                                           window->getTextEditor()
                                             ->lineLength(lineOfRemovedEntity-1)
                                          );
    window->getTextEditor()->removeSelectedText();

    lineOfEntity.remove(entityID);
}

void NCLTextualViewPlugin::onEntityRemoveError(QString error)
{
    //TODO: All
}

bool NCLTextualViewPlugin::save(){
    //TODO: All
}

void NCLTextualViewPlugin::updateFromModel(){

}



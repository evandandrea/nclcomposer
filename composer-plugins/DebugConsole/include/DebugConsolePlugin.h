#ifndef DEBUGCONSOLEPLUGIN_H
#define DEBUGCONSOLEPLUGIN_H

#include <QObject>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include <core/extensions/IPlugin.h>
using namespace composer::core::extension::plugin;

class DebugConsolePlugin : public IPlugin
{
        Q_OBJECT
    private:
        QListWidget *list;
        QWidget *window;
    public:
        explicit DebugConsolePlugin();
        ~DebugConsolePlugin();

        QWidget* getWidget();

    public slots:
        void onEntityAdded(QString ID, Entity *);
        void onEntityAddError(QString error);
        void onEntityChanged(QString ID, Entity *);
        void onEntityChangeError(QString error);
        void onEntityAboutToRemove(Entity *);
        void onEntityRemoved(QString ID, QString entityID);
        void onEntityRemoveError(QString error);

};

#endif // DEBUGCONSOLEPLUGIN_H
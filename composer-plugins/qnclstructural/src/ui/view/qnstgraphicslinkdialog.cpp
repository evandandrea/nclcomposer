#include "qnstgraphicslinkdialog.h"

QnstGraphicsLinkDialog::QnstGraphicsLinkDialog(QWidget* parent)
    : QDialog(parent)
{
    form.setupUi(this);

    connect(form.cbConnector, SIGNAL(currentIndexChanged(QString)), SLOT(adjustBinds(QString)));
}

QnstGraphicsLinkDialog::~QnstGraphicsLinkDialog()
{

}

void QnstGraphicsLinkDialog::init(QMap<QString, QnstConncetor*> connectors)
{
    this->connectors = connectors;

    form.cbConnector->clear();

    form.cbCondition->setEnabled(false);
    form.cbCondition->clear();

    form.cbAction->setEnabled(false);
    form.cbAction->clear();

    foreach(QnstConncetor* conn, connectors.values()){
        form.cbConnector->addItem(conn->getName());
    }

    if (form.cbConnector->count() > 0){
        form.cbConnector->addItem("----------");
    }

    form.cbConnector->addItem("New...");
}

void QnstGraphicsLinkDialog::adjustBinds(QString conn)
{
    if (conn == "" || conn == "----------"){
        form.cbCondition->setEnabled(false);
        form.cbCondition->clear();

        form.cbAction->setEnabled(false);
        form.cbAction->clear();

    }else if (conn == "New..."){
        form.cbCondition->setEnabled(true);
        form.cbCondition->clear();

        form.cbCondition->addItem("onBegin");
        form.cbCondition->addItem("onEnd");
        form.cbCondition->addItem("onSelection");
        form.cbCondition->addItem("onResume");
        form.cbCondition->addItem("onPause");

        form.cbAction->setEnabled(true);
        form.cbAction->clear();

        form.cbAction->addItem("start");
        form.cbAction->addItem("stop");
        form.cbAction->addItem("resume");
        form.cbAction->addItem("pause");
        form.cbAction->addItem("set");
    }else{
        QnstConncetor* oconn = connectors[conn];

        form.cbCondition->setEnabled(true);
        form.cbCondition->clear();

        if (oconn->getName() == conn){
            foreach(QString cond, oconn->getConditions().values()){
                form.cbCondition->addItem(cond);
            }
        }

        form.cbAction->setEnabled(true);
        form.cbAction->clear();

        if (oconn->getName() == conn){
            foreach(QString act, oconn->getActions().values()){
                form.cbAction->addItem(act);
            }
        }
    }
}

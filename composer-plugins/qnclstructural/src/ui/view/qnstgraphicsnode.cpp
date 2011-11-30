#include "qnstgraphicsnode.h"

QnstGraphicsNode::QnstGraphicsNode(QnstGraphicsNode* parent)
    : QncgGraphicsNode(parent), QnstEntity(parent)
{
    setnstType(Qnst::Node);

//    connect(this, SIGNAL(entityChanged()), SLOT(requestEntityChange()));
    connect(this, SIGNAL(entitySelected()), SLOT(requestEntitySelection()));
}

QnstGraphicsNode::~QnstGraphicsNode()
{

}

void QnstGraphicsNode::adjust()
{
    fit(50);

    if (getnstParent() != NULL){
        ((QnstGraphicsNode*) getnstParent())->attract();
        ((QnstGraphicsNode*) getnstParent())->adjust();
    }

    QncgGraphicsNode::adjust();
}

void QnstGraphicsNode::requestEntityChange()
{
    emit entityChanged(this);
}

void QnstGraphicsNode::requestEntitySelection()
{
    setFocus(Qt::MouseFocusReason);

    emit entitySelected(this);
}
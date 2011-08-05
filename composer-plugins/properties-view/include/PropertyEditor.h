/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QObject>
#include <QString>
#include <QWidget>

#include "ui_PropertyEditorWidget.h"
#include "QLineEditFilter.h"

/*!
 \brief PropertyEditor is a Widget that allows to edit individual properties
            (also called attributes) of any entity.

*/
class PropertyEditor : public QWidget
{
    Q_OBJECT

private:
    Ui::PropertyEditorWidget *ui; /*!< TODO */
    QMap <QString, int> propertyToLine; /*!< TODO */
    QMap <QString, QString> propertyToValue; /*!< TODO */

    bool internalPropertyChange;

    QString currentTagname, currentName;
    QString currentFilterString;

public:
    /*!
     \brief Constructor.

     \param parent The QObject parent.
    */
    explicit PropertyEditor(QWidget *parent=0);

    /*!
     \brief Destructor.

    */
    virtual ~PropertyEditor();

    /*!
     \brief Set the current tagname and update the available properties
                according to that tagname.

     The available properties are loaded from the configuration file of the
     current language.

     \param tagname The current tagname.
    */
    void setTagname(QString tagname, QString name);

    /*!
     \brief Set a value of an attribute.

     \param property Which property must be changed.
     \param value The new Value of the property.
    */
    void setAttributeValue(QString property, QString value);

private slots:
    void updateWithItemChanges(QTableWidgetItem *item);
    void filterProperties(const QString&);

signals:
    void propertyChanged(QString property, QString value);


};

#endif // PROPERTYEDITOR_H

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
#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#define WITH_TREEVIEW 0

#include <QComboBox>
#include <QItemDelegate>
#include <QLineEdit>
#include <QObject>
#include <QString>
#include <QStyledItemDelegate>
#include <QWidget>

#include <PropertyButton.h>

#include <NCLStructure.h>
using namespace cpr::nclprofile;

#if WITH_TREEVIEW
#include <QStandardItemModel>
#endif

#include "QLineEditFilter.h"
#include "ui_PropertyEditorWidget.h"

class ComboBoxDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  ComboBoxDelegate (QWidget *parent = 0) : QStyledItemDelegate (parent) {}

  QWidget *
  createEditor (QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex &index) const
  {
    Q_UNUSED (option);

    if (index.column () == 1)
    {
      // \todo References
      NCLStructure *structure = NCLStructure::getInstance ();
      QString datatype = structure->getAttributeDatatype (
          currentTagname, tableWidget->item (index.row (), 0)->text ());

      QStringList defaultSuggestions
          = structure->getDatatypeDefaultSuggestions (datatype);
      if (defaultSuggestions.size ())
      {
        QComboBox *comboEdit = new QComboBox (parent);
        comboEdit->setEditable (true);
        comboEdit->addItems (defaultSuggestions);
        comboEdit->setStyleSheet ("padding: 0,0,0,0;");

        return comboEdit;
      }
      else
      {
        if (datatype == "URI")
        {
          PropertyButtons *urlEdit = new PropertyButtons (parent);
          return urlEdit;
        }
        else
        {
          QLineEdit *lineEdit = new QLineEdit (parent);
          lineEdit->setTextMargins (0, 0, 0, 0);
          return (QWidget *)lineEdit;
        }
      }
    }
    else
    {
      return nullptr;
    }
  }

  void
  setEditorData (QWidget *editor, const QModelIndex &index) const
  {
    if (index.column () == 1)
    {
      QComboBox *combobox = qobject_cast<QComboBox *> (editor);
      if (combobox)
        combobox->setEditText (index.data ().toString ());
      else
      {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *> (editor);
        if (lineEdit)
          lineEdit->setText (index.data ().toString ());
      }
    }
    else
    {
      QStyledItemDelegate::setEditorData (editor, index);
    }
  }

  void
  setModelData (QWidget *editor, QAbstractItemModel *model,
                const QModelIndex &index) const
  {
    if (index.column () == 1)
    {
      QComboBox *combobox = qobject_cast<QComboBox *> (editor);
      if (combobox)
        model->setData (index, qVariantFromValue (combobox->currentText ()));
      else
      {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *> (editor);
        if (lineEdit)
          model->setData (index, qVariantFromValue (lineEdit->text ()));
      }
    }
    else
    {
      QStyledItemDelegate::setModelData (editor, model, index);
    }
  }

  void
  setCurrentTagname (const QString &tagname)
  {
    this->currentTagname = tagname;
  }

  void
  setTableWidget (QTableWidget *tableWidget)
  {
    this->tableWidget = tableWidget;
  }

private:
  QString currentTagname;
  QTableWidget *tableWidget;

private slots:
  void
  commitAndCloseEditor ()
  {
    // CQuantityEditor* editor = qobject_cast<CQuantityEditor*>(sender());
    // emit commitData(editor);
    // emit closeEditor(editor);
  }
};

/*!
 \brief PropertyEditor is a Widget that allows to edit individual properties
            (also called attributes) of any entity.

*/
class PropertyEditor : public QWidget
{
  Q_OBJECT

public:
  /*!
   * \brief Constructor.
   *
   * \param parent The QObject parent.
   */
  explicit PropertyEditor (QWidget *parent = 0);

  /*!
   * \brief Destructor.
   */
  virtual ~PropertyEditor ();
  /*!
   * \brief Set the current tagname and update the available properties
   *  according to that tagname.
   *
   * The available properties are loaded from the configuration file of the
   * current language.
   *
   * \param tagname The current tagname.
   */
  void setTagname (QString tagname, QString name);

  /*!
   * \brief Get the current tagname.
   */
  QString
  getTagname ()
  {
    return this->_currentTagname;
  }

  /*!
   * \brief Set the current name (id, name, ...) of the selected entity.
   */
  void setCurrentName (QString name);

  /*!
   * \brief Set the current error message (and change the color of the text) if
   *  the errorMessage is not empty.
   */
  void setErrorMessage (QString errorMessage);

  /*!
   * \brief Get the current name.
   */
  QString
  getCurrentName ()
  {
    return this->_currentName;
  }

  /*!
   * \brief Set a value of an attribute.
   *
   * \param property Which property must be changed.
   * \param value The new Value of the property.
   */
  void setAttributeValue (QString property, QString value);

  QSize
  sizeHint () const
  {
    return QSize (250, 400);
  }

signals:
  void propertyChanged (QString property, QString value);

private slots:
  /*!
   * \brief This SLOT must be called when the user changes the value of an
   * item.
   *
   * The changes will then be sent to NCL Composer core.
   */
  void updateWithItemChanges (QTableWidgetItem *item);

  /*!
   * \brief This SLOT allows filter the current properties that are been
   * showed.
   */
  void filterProperties (const QString &);

private:
  Ui::PropertyEditorWidget *_ui; /*!< TODO */
  deque<QString> _orderedProperties;
  QMap<QString, int> _propertyToLine;      /*!< TODO */
  QMap<QString, QString> _propertyToValue; /*!< TODO */

  bool _internalPropertyChange;

  QString _currentTagname, _currentName;
  QString _currentFilterString;

#if WITH_TREEVIEW
  QStandardItemModel *standardModel;
  QStandardItem *attributesRootItem, *propertiesRootItem;
#endif
};

#endif // PROPERTYEDITOR_H

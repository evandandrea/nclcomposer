/*
 * Copyright 2011-2014 TeleMidia/PUC-Rio.
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
#ifndef NCLSAXHANDLER_H
#define NCLSAXHANDLER_H

#include <QtXml>

#include <NCLTreeWidget.h>
#include <QList>
#include <QMessageBox>
#include <QtGui>

class NCLTreeWidget;

class NCLParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

public:
  explicit NCLParser (NCLTreeWidget *tree);
  bool startElement (const QString &namespaceURI, const QString &localName,
                     const QString &qName, const QXmlAttributes &attributes);
  bool endElement (const QString &namespaceURI, const QString &localName,
                   const QString &qName);
  bool characters (const QString &str);
  bool fatalError (const QXmlParseException &exception);
  void setDocumentLocator (QXmlLocator *locator);

signals:
  void fatalErrorFound (QString message, QString file, int line, int column,
                        int severity);

private:
  NCLTreeWidget *_treeWidget;
  QTreeWidgetItem *_currentItem;
  QString _currentText;
  QXmlLocator *_locator;
};
#endif

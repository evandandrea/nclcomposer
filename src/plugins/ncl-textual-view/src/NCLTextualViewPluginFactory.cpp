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
#include "NCLTextualViewPluginFactory.h"
#include "ui_TextualPluginPreferencesWidget.h"

NCLTextualViewPluginFactory::NCLTextualViewPluginFactory ()
{
  _prefPageWidget = nullptr;
}

IPlugin *
NCLTextualViewPluginFactory::createPluginInstance ()
{
  return new NCLTextualViewPlugin ();
}

void
NCLTextualViewPluginFactory::releasePluginInstance (IPlugin *plugin)
{
  NCLTextualViewPlugin *textualView
      = qobject_cast<NCLTextualViewPlugin *> (plugin);

  if (textualView)
  {
    delete textualView;
    textualView = nullptr;
  }
}

QString
NCLTextualViewPluginFactory::id () const
{
  return "br.puc-rio.telemidia.NCLTextualView";
}

QIcon
NCLTextualViewPluginFactory::icon () const
{
  return QIcon (":/images/ncl.png");
}

QWidget *
NCLTextualViewPluginFactory::getPreferencePageWidget ()
{
  /*
  if (prefPageWidget == nullptr)
  {
    prefPageWidget = new QFrame();
    prefPageUi = new Ui::TextPluginPreferencesWidget();
    prefPageUi->setupUi(prefPageWidget);
   }
   return prefPageWidget;
  */
  return nullptr;
}

void
NCLTextualViewPluginFactory::setDefaultValues ()
{
  // TODO
}

void
NCLTextualViewPluginFactory::applyValues ()
{
  // TODO
}

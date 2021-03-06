#ifndef RULESVIEWFACTORY_H
#define RULESVIEWFACTORY_H

#include <extensions/IPluginFactory.h>

#include "RulesViewPlugin.h"

using namespace cpr::core;

/*!
 * \brief Handles the creation and deletion of RulesViewPlugin objects.
 */
class RulesViewFactory : public QObject, public IPluginFactory
{
  Q_OBJECT
  Q_INTERFACES (IPluginFactory)
  Q_PLUGIN_METADATA (IID IPluginFactory_iid FILE "ncl-rules-view.json")

public:
  explicit RulesViewFactory ();
  ~RulesViewFactory ();

  IPlugin *
  createPluginInstance () override
  {
    return new RulesViewPlugin ();
  }
  void releasePluginInstance (IPlugin *) override;
  QList<LanguageType> getSupportedLanguages () override;
  QString
  id () const override
  {
    return "br.puc-rio.telemidia.RulesView";
  }
  QIcon
  icon () const override
  {
    return QIcon ();
  }
};

#endif // RULESVIEWFACTORY_H

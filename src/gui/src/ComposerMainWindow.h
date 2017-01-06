/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef COMPOSERMAINWINDOW_H
#define COMPOSERMAINWINDOW_H

#include <QtDebug>
#include <QStringList>
#include <QMap>
#include <QFileSystemModel>

#include <QCoreApplication>
#include <QApplication>
#include <QTimer>

#include <QToolButton>
#include <QDockWidget>
#include <QAtomicInt>

#include <QMainWindow>
#include <QTabWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QMenuBar>
#include <QTreeWidget>
#include <QWizardPage>
#include <QWizard>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QToolBar>
#include <QListWidget>
#include <QDialog>
#include <QBitmap>
#include <QPainter>
#include <QTreeView>

#include <modules/PluginControl.h>
#include <modules/ProjectControl.h>
#include <modules/LanguageControl.h>
using namespace composer::core;

#include <util/ComposerSettings.h>
using namespace composer::core::util;

#include "ComposerSplashScreen.h"

#include "PerspectiveManager.h"
#include "PreferencesDialog.h"
#include "PluginDetailsDialog.h"
#include "WelcomeWidget.h"
#include "AboutDialog.h"
// #include "ComposerHelpWidget.h"
#include "ClickableDockWidget.h"

#include <qtoolwindowmanager.h>

using namespace composer::gui;

namespace Ui {
class ComposerMainWindow;
}

namespace composer {
namespace gui {

/*!
 * \brief The main Window of NCL Composer.
 *
 * This class is the main window of NCL Composer.
 * \fixme This class is too big it must be refactored
 */
class ComposerMainWindow : public QMainWindow {
  Q_OBJECT

private:
  static const int _maximumRecentProjectsSize = 10;

  Ui::ComposerMainWindow *_ui;

  QListWidget *_listWidgetProfilesExt; /*!< Shows a list of the current loaded language
                                   profiles. */
  QStringList _extensionsPaths; /*!< TODO */


  QTabWidget  *_tabProjects; /*!< Each open project is show in a different tab.
                                  The tabProjects variable keeps the list of the
                                  projects open tabs. */

  QToolButton  *_tbLanguageDropList; /*!< Action that shows the list of
                                          languages as a menu. */

  QMenu        *_menuPerspective; /*!< Contains the list of perspectives. */
  QMenu        *_menuMultidevice; /*!<todo */
  QMenu        *_menuLanguage;    /*!< Contains the list of supported
                                       languages.*/

  QAction      *_projectViewAction; /*!< TODO */

  QMap<QString, QToolWindowManager*> _projectsWidgets; /*!< Keeps a reference
                                                            to each project
                                                            widget. */
  QTreeWidget *_treeWidgetPlugins; /*!< Shows a list with the current loaded
                                  plugins. */

  QMap <QTreeWidgetItem*, IPluginFactory*> treeWidgetItem2plFactory;

  PreferencesDialog *_preferencesDialog; /*!< TODO */

  QDialog *_aboutPluginsDialog; /*!< TODO */
  QPushButton *_detailsButton;

  WelcomeWidget *_welcomeWidget; /*!< TODO */

  PerspectiveManager *_perspectiveManager;
  PluginDetailsDialog *_pluginDetailsDialog;

  QProgressDialog *_taskProgressBar;
  QTimer *_autoSaveTimer; // auto save timer

  // ComposerHelpWidget composerHelpWidget;

#if WITH_WIZARD
  QProcess wizardProcess;
  QProcess talProcess;
#endif

private:
  void readSettings();
  void initModules();

  void loadStyleSheets();
  void initGUI();

  void createAboutPluginsWidgets();
  void createMenus();
  void createActions();

  void readExtensions();
  void closeEvent(QCloseEvent *event);
  void cleanUp();
  void updateRecentProjectsWidgets();

  void updateTabWithProject(int index, QString newLocation);

  QTranslator _translator;   /**< contains the translations for this application */
  QTranslator _translatorQt; /**< contains the translations for qt */
  QString     _currLang;     /**< contains the currently loaded language */
  QString     _langPath;     /**< Path of language files. This is always fixed to
                                  /languages. */
  void loadLanguage(const QString& rLanguage);
  void createLanguageMenu(void);
  void switchTranslator(QTranslator& translator, const QString& filename);

  /*!
   * \brief Shows a prompt where the user can choose where its plugins are
   *    located.
   *
   * \return QString the path to the choosen directory.
   */
  QString promptChooseExtDirectory();


  /*!
   * \brief
   * \todo Move this function from here!
   */
  void checkTemporaryFileLastModified(const QString &filename);

  /*!
   * \brief Remove the temporary file related to location.
   * \todo Move this function from here!
   */
  bool removeTemporaryFile(QString location);

protected:
  void changeEvent(QEvent*);
  void keyPressEvent(QKeyEvent *event);

protected slots:
  void slotLanguageChanged(QAction* action);
  void saveLoadPluginData(int);

private slots:
  void about();
  void aboutPlugins();

  void updateViewMenu();
  void showEditPreferencesDialog();

  void tabClosed(int index);
  void closeCurrentTab();
  void showCurrentWidgetFullScreen();
  void closeAllFiles();

  void startOpenProject(QString projectLoc);
  void endOpenProject(QString projectLoc);

  void saveCurrentGeometryAsPerspective();
  void restorePerspective();
  void savePerspective(QString layoutName);
  void saveDefaultPerspective(QString defaultPerspectiveName);
  void restorePerspective(QString layoutName);

  void launchProjectWizard();
  void addToRecentProjects(QString projectUrl);
  void userPressedRecentProject(QString src);
  void userPressedRecentProject();
  void clearRecentProjects (void);
  void importFromDocument();

  void selectedAboutCurrentPluginFactory();

  void showPluginDetails();

  void updateMenuPerspectives();
  void updateMenuLanguages();

  void restorePerspectiveFromMenu();

  void currentTabChanged(int n);

  void setProjectAsDirty(QString location, bool isDirty);

  void gotoNCLClubWebsite();

  void autoSaveCurrentProjects();

  void addDefaultStructureToProject ( Project *project,
                                      bool shouldCopyDefaultConnBase = true,
                                      bool shouldCreateADefaultRegion = true,
                                      bool save = true );

  void on_actionReport_Bug_triggered();

  void pluginWidgetViewToggled(bool);

#ifdef WITH_SERV_PUB
  void on_actionPublish_triggered();
#endif

#if WITH_WIZARD
  void on_actionProject_from_Wizard_triggered();
  void wizardFinished(int resp);
#endif

public:
  explicit ComposerMainWindow(QWidget *parent = 0);
  virtual ~ComposerMainWindow();

  void init(const QApplication &app);

public slots:
  void errorDialog(QString);
  void addPluginWidget(IPluginFactory *fac, IPlugin *plugin, Project *project);
  void openProject();
  void onOpenProjectTab(QString location);

  void saveCurrentProject();
  void saveAsCurrentProject();

  bool showHelp();

  void undo();
  void redo();

  void openProjects(const QStringList &projects);
};

} } //end namespace

#endif // COMPOSERMAINWINDOW_H

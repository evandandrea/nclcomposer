/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#include "ComposerMainWindow.h"
#include "ui_ComposerMainWindow.h"

#include <QPixmap>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QToolButton>
#include <QApplication>

#include <QDesktopServices>

#include "GeneralPreferences.h"

#include "NewProjectWizard.h"

#ifdef USE_MDI
#include <QMdiArea>
#endif

// #define SHOW_PROFILES
namespace composer {
namespace gui {

const int autoSaveInterval = 1 * 60 * 1000; //ms

ComposerMainWindow::ComposerMainWindow(QApplication &app, QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::ComposerMainWindow)
{
  ui->setupUi(this);

  /* The following code could be in another function */
  QPixmap mPix(":/mainwindow/nclcomposer-splash");
  QSplashScreen splash(mPix);
  splash.setMask(mPix.mask());
  splash.showMessage(tr("Loading NCL Composer..."), Qt::AlignRight, Qt::gray);

  //splash.blockSignals(true);
  splash.show();
  app.processEvents();

  splash.showMessage(tr("Starting GUI..."), Qt::AlignRight, Qt::gray);
  initGUI();
  app.processEvents();

  splash.showMessage(tr("Starting Modules and Plugins..."), Qt::AlignRight,
                      Qt::gray);
  initModules();
  app.processEvents();

  splash.showMessage(tr("Reloading last session..."), Qt::AlignRight,
                      Qt::gray);
  readSettings();
  app.processEvents();

  splash.finish(this);
  connect(&app, SIGNAL(focusChanged(QWidget *, QWidget *)),
          this, SLOT(focusChanged(QWidget *, QWidget *)),
          Qt::DirectConnection);

  proc = NULL;

#ifdef WITH_LIBSSH2
  runRemoteGingaVMAction.moveToThread(&runRemoteGingaVMThread);

  connect(&runRemoteGingaVMThread, SIGNAL(started()),
          &runRemoteGingaVMAction, SLOT(runCurrentProject()));

  connect(&runRemoteGingaVMAction, SIGNAL(finished()),
          &runRemoteGingaVMThread, SLOT(quit()));

  SimpleSSHClient::init(); // Initializes the libssh2 library
#endif

  autoSaveTimer = new QTimer(this);
  connect(autoSaveTimer, SIGNAL(timeout()),
          this, SLOT(autoSaveCurrentProjects()));

  autoSaveTimer->start(autoSaveInterval);
}

ComposerMainWindow::~ComposerMainWindow()
{
  delete ui;
  delete menu_Perspective;
#ifdef WITH_LIBSSH2
  SimpleSSHClient::exit();
#endif
}

void ComposerMainWindow::initModules()
{
  PluginControl  *pgControl = PluginControl::getInstance();
  LanguageControl *lgControl = LanguageControl::getInstance();
  ProjectControl *projectControl = ProjectControl::getInstance();

  connect(pgControl,SIGNAL(notifyError(QString)),
          SLOT(errorDialog(QString)));

  connect(pgControl,SIGNAL(addPluginWidgetToWindow(IPluginFactory*,
                                                   IPlugin*, Project*, int)),
          SLOT(addPluginWidget(IPluginFactory*, IPlugin*, Project*, int)));

  connect(lgControl,SIGNAL(notifyError(QString)),
          SLOT(errorDialog(QString)));

  connect(projectControl, SIGNAL(notifyError(QString)),
          SLOT(errorDialog(QString)));

  connect(projectControl,SIGNAL(projectAlreadyOpen(QString)),
          SLOT(onOpenProjectTab(QString)));

  connect(projectControl, SIGNAL(startOpenProject(QString)),
          this, SLOT(startOpenProject(QString)));

  connect(projectControl, SIGNAL(endOpenProject(QString)), this,
          SLOT(endOpenProject(QString)));

  connect(projectControl,SIGNAL(endOpenProject(QString)),
          SLOT(addToRecentProjects(QString)));

  connect(projectControl, SIGNAL(endOpenProject(QString)),
          welcomeWidget, SLOT(addToRecentProjects(QString)));

  connect(welcomeWidget, SIGNAL(userPressedRecentProject(QString)),
          this, SLOT(userPressedRecentProject(QString)));

  connect(projectControl,SIGNAL(projectAlreadyOpen(QString)),
          SLOT(onOpenProjectTab(QString)));

  connect(projectControl, SIGNAL(dirtyProject(QString, bool)),
          this, SLOT(setProjectDirty(QString, bool)));

  readExtensions();
}

void ComposerMainWindow::readExtensions()
{
  ComposerSettings settings;

  settings.beginGroup("extensions");
  extensions_paths.clear();

  //Remember: The dafault paths are been added in main.cpp
  if (settings.contains("path"))
    extensions_paths << settings.value("path").toStringList();

  extensions_paths.removeDuplicates(); // Remove duplicate paths

  // add all the paths to LibraryPath, i.e., plugins are allowed to install
  // dll dependencies in the extensions path.
  for(int i = 0; i < extensions_paths.size(); i++)
  {
    qDebug() << "Adding library " << extensions_paths.at(i);
    QApplication::addLibraryPath(extensions_paths.at(i) + "/");
  }

  // foreach path where extensions can be installed, try to load profiles.
  for(int i = 0; i < extensions_paths.size(); i++)
  {
    LanguageControl::getInstance()->loadProfiles(extensions_paths.at(i));
  }

  // foreach path where extensions can be installed, try to load plugins.
  for(int i = 0; i < extensions_paths.size(); i++)
  {
    PluginControl::getInstance()->loadPlugins(extensions_paths.at(i));
  }
  settings.endGroup();

  preferences->addPreferencePage(new GeneralPreferences());

  /* Load the preferences page */
  preferences->addPreferencePage(new RunGingaConfig());

  // preferences->addPreferencePage(new ImportBasePreferences());

  /* Load PreferencesPages from Plugins */
  QList<IPluginFactory*> list =
      PluginControl::getInstance()->getLoadedPlugins();

  IPluginFactory *currentFactory;
  foreach(currentFactory, list)
  {
    preferences->addPreferencePage(currentFactory);
  }
}

QString ComposerMainWindow::promptChooseExtDirectory()
{
  QMessageBox mBox;

  mBox.setText(tr("The Extension Directory is not set"));
  mBox.setInformativeText(tr("Do you want to try the default"
                             "directory (%1)?").arg(QDir::homePath()));
  mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  mBox.setDefaultButton(QMessageBox::Yes);
  mBox.setIcon(QMessageBox::Question);
  if (mBox.exec() == QMessageBox::No)
  {
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Select Directory"),
                                                  getLastFileDialogPath(),
                                                  QFileDialog::ShowDirsOnly);
    return dirName;
  } else {
    return "";
  }
}

void ComposerMainWindow::readSettings()
{
  ComposerSettings settings;
  settings.beginGroup("openfiles");
  QStringList openfiles = settings.value("openfiles").toStringList();
  settings.endGroup();

  openProjects(openfiles);

  QApplication::processEvents();

  settings.beginGroup("mainwindow");
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  settings.endGroup();
}

void ComposerMainWindow::openProjects(const QStringList &projects)
{
  qDebug() << "Openning files:" << projects;
  for(int i = 0; i < projects.size(); i++)
  {
    QString src = projects.at(i);
#ifdef WIN32
    src = src.replace(QDir::separator(), "/");
#endif

    QFile file(src);
    bool openCurrentFile = true;

    if(!file.exists())
    {
      int resp =
          QMessageBox::question(this,
                                tr("File does not exists anymore."),
                                tr("The File %1 does not exists, but "
                                   "the last time you have closed NCL"
                                   " Composer this files was open. "
                                   "Do you want to create this file "
                                   "again?").arg(src),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No);

      if(resp != QMessageBox::Yes) openCurrentFile = false;
    }

    if (openCurrentFile)
    {
      checkTemporaryFileLastModified(src);

      ProjectControl::getInstance()->launchProject(src);
    }
  }

  /* Update Recent Projects on Menu */
  ComposerSettings settings;
  QStringList recentProjects = settings.value("recentprojects").toStringList();
  updateRecentProjectsMenu(recentProjects);
  welcomeWidget->updateRecentProjects(recentProjects);
}

void ComposerMainWindow::initGUI()
{
#ifndef Q_WS_MAC
  setWindowIcon(QIcon(":/mainwindow/icon"));
#endif
  setWindowTitle(tr("NCL Composer"));
  tabProjects = new QTabWidget(0);

  ui->frame->layout()->addWidget(tabProjects);

  //    tabProjects->setMovable(true);
  tabProjects->setTabsClosable(true);

  /* tbLanguageDropList = new QToolButton(this);
  tbLanguageDropList->setIcon(QIcon(":/mainwindow/language"));
  tbLanguageDropList->setToolTip(tr("Change your current language"));
  tbLanguageDropList->setPopupMode(QToolButton::InstantPopup); */

  tbPerspectiveDropList = new QToolButton(this);
  tbPerspectiveDropList->setIcon(QIcon(":/mainwindow/perspective"));
  tbPerspectiveDropList->setToolTip(tr("Change your current perspective"));
  tbPerspectiveDropList->setPopupMode(QToolButton::InstantPopup);

  connect( tabProjects, SIGNAL(tabCloseRequested(int)),
          this, SLOT(tabClosed(int)), Qt::DirectConnection);

  connect(tabProjects, SIGNAL(currentChanged(int)),
          this, SLOT(currentTabChanged(int)));

//  createStatusBar();
  createActions();
  createMenus();
//  createLanguageMenu();
  createAboutPlugins();

  preferences = new PreferencesDialog(this);
  perspectiveManager = new PerspectiveManager(this);
  pluginDetailsDialog = new PluginDetailsDialog(aboutPluginsDialog);

  connect(ui->action_RunNCL, SIGNAL(triggered()), this, SLOT(runNCL()));

#ifdef WITH_LIBSSH2
  connect(ui->action_StopRemoteNCL, SIGNAL(triggered()),
          this, SLOT(stopRemoteNCL()));
#else
  // ui->action_RunNCL->setEnabled(true);
  // ui->action_RunNCL->setToolTip(tr("Run NCL Remotely: Your program was not built with this option!!!"));
#endif

// UNDO/REDO
  // connect(ui->action_Undo, SIGNAL(triggered()), this, SLOT(undo()));
  // connect(ui->action_Redo, SIGNAL(triggered()), this, SLOT(redo()));

  welcomeWidget = new WelcomeWidget(this);
  tabProjects->addTab(welcomeWidget, tr("Welcome"));
  tabProjects->setTabIcon(0, QIcon());

  connect(welcomeWidget, SIGNAL(userPressedOpenProject()),
          this, SLOT(openProject()));

  connect(welcomeWidget, SIGNAL(userPressedNewProject()),
          this, SLOT(launchProjectWizard()));

  connect(welcomeWidget, SIGNAL(userPressedSeeInstalledPlugins()),
          this, SLOT(aboutPlugins()));


  //Task Progress Bar

#ifdef WITH_LIBSSH2
  taskProgressBar = new QProgressDialog(this);
  taskProgressBar->setWindowTitle(tr("Copy content to Ginga VM."));
  taskProgressBar->setModal(true);

  // start taskProgressBar
  connect(&runRemoteGingaVMAction, SIGNAL(startTask()),
          taskProgressBar, SLOT(show()));

  connect(&runRemoteGingaVMAction, SIGNAL(taskDescription(QString)),
          taskProgressBar, SLOT(setLabelText(QString)));

  connect(&runRemoteGingaVMAction, SIGNAL(taskMaximumValue(int)),
          taskProgressBar, SLOT(setMaximum(int)));

  connect(&runRemoteGingaVMAction, SIGNAL(taskValue(int)),
          taskProgressBar, SLOT(setValue(int)));

  connect(&runRemoteGingaVMAction, SIGNAL(copyFinished()),
          taskProgressBar, SLOT(hide()));

  connect(&runRemoteGingaVMAction, SIGNAL(finished()),
          taskProgressBar, SLOT(hide()));

  connect(taskProgressBar, SIGNAL(canceled()),
          &runRemoteGingaVMAction, SLOT(stopExecution()), Qt::DirectConnection);

  disconnect(taskProgressBar, SIGNAL(canceled()),
             taskProgressBar, SLOT(cancel()));

  connect(&runRemoteGingaVMAction, SIGNAL(finished()),
          taskProgressBar, SLOT(hide()));

// This shows the taskBar inside the toolBar. In the future, this can
//  taskProgressBarAction = ui->toolBar->insertWidget(ui->action_Save,
//                                                    taskProgressBar);
// taskProgressBarAction->setVisible(false);
#endif

}

void ComposerMainWindow::keyPressEvent(QKeyEvent *event)
{
  if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Z)
  {
    undo();
    event->accept();
  }
  else if(event->modifiers() == Qt::ControlModifier &&
          event->key() == Qt::Key_Y)
  {
    redo();
    event->accept();
  }
}

void ComposerMainWindow::addPluginWidget(IPluginFactory *fac, IPlugin *plugin,
                                         Project *project, int n)
{
  QMainWindow *w;
  QString location = project->getLocation();
  QString projectId = project->getAttribute("id");

#ifdef USE_MDI
  QMdiArea *mdiArea;
#endif
  if (projectsWidgets.contains(location))
  {
    w = projectsWidgets[location];
#ifdef USE_MDI
    mdiArea = (QMdiArea *)w->centralWidget();
    mdiArea->setBackground(QBrush(QColor("#FFFFFF")));
#endif
  } else {
    w = new QMainWindow(tabProjects);
#ifdef USE_MDI
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    w->setCentralWidget(mdiArea);
#else
    w->setDockNestingEnabled(true);
    w->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
#endif
    int index = tabProjects->addTab(w, projectId);
    updateTabWithProject(index, location);
    projectsWidgets[location] = w;
  }
  QWidget *pW = plugin->getWidget();

#ifdef USE_MDI
  mdiArea->addSubWindow(pW);
  pW->setWindowModified(true);
  pW->setWindowTitle(projectId + " - " + fac->name());
  pW->show();
  pW->setObjectName(fac->id());
#else
  ClickableQDockWidget *dock = new ClickableQDockWidget(fac->name());

  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setFeatures(QDockWidget::AllDockWidgetFeatures);

  connect(dock, SIGNAL(clicked()), pW, SLOT(setFocus()));
  pW->setFocusPolicy(Qt::StrongFocus);

  QFrame *borderFrame = new QFrame();
  borderFrame->setFrameShape(QFrame::NoFrame);
  borderFrame->setFrameShadow(QFrame::Plain);

  QVBoxLayout *layoutBorderFrame = new QVBoxLayout();
  layoutBorderFrame->setMargin(0);
  layoutBorderFrame->addWidget(pW);
  borderFrame->setLayout(layoutBorderFrame);

  dock->setWidget(borderFrame);
  dock->setObjectName(fac->id());

  if (n%2)
    w->addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Vertical);
  else
    w->addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);

  dock->setMinimumSize(0, 0);
  tabProjects->setCurrentWidget(w);

  if(firstDock.contains(location)) {
    // w->tabifyDockWidget(firstDock[location], dock);
  }
  else firstDock[location] = dock;

  QFrame *titleBar = new QFrame();
  titleBar->setContentsMargins(0,0,0,0);

  QLabel *titleLabel = new QLabel(fac->name());
  titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout *layout = new QHBoxLayout(titleBar);
  layout->setMargin(0);
  layout->setSpacing(0);

  layout->addWidget(titleLabel);

  titleBar->setStyleSheet(" ");

  dock->setTitleBarWidget(titleBar);
  allDocksMutex.lock();
  allDocks.insert(0, dock);
  allDocksMutex.unlock();

  QPushButton *refresh = new QPushButton(titleBar);
  refresh->setIcon(QIcon(":/mainwindow/refreshplugin"));
  refresh->setToolTip(tr("Reload View Model"));
  addButtonToDockTitleBar(titleBar, refresh);
  connect(refresh, SIGNAL(pressed()), plugin, SLOT(updateFromModel()));

  QPushButton *hide = new QPushButton(titleBar);
  connect(hide, SIGNAL(pressed()), dock, SLOT(close()));
  hide->setIcon(QIcon(":/mainwindow/closeplugin"));
  hide->setToolTip(tr("Hide View"));
  addButtonToDockTitleBar(titleBar, hide);

  // dock->installEventFilter(this);

  updateDockStyle(dock, false);
#endif
}

void ComposerMainWindow::updateDockStyle(QDockWidget *dock, bool selected)
{
  QList <QTabBar*> tabBars = this->findChildren <QTabBar *>();

  foreach (QTabBar * tabBar, tabBars)
  {
    for(int index = 0; index < tabBar->count(); index++)
    {
      QVariant tmp = tabBar->tabData(index);
      QDockWidget * dockWidget = reinterpret_cast<QDockWidget *>(tmp.toULongLong());
      if(dockWidget == dock)
      {
        if(selected)
          tabBar->setProperty("activePlugin", "true");
        else
          tabBar->setProperty("activePlugin", "false");

        tabBar->setStyleSheet(styleSheet());
      }
    }
  }

  QFrame *titleBar = (QFrame*) dock->titleBarWidget();
  if(!selected)
  {
    titleBar->setProperty("activePluginTitleBar", "false");
    dock->setProperty("activePluginBorder", "false");
  }
  else
  {
    titleBar->setProperty("activePluginTitleBar", "true");
    dock->setProperty("activePluginBorder", "true");
  }

  dock->setStyleSheet(styleSheet());
}

void ComposerMainWindow::addButtonToDockTitleBar(QFrame *titleBar,
                                                 QPushButton *button)
{
  button->setIconSize(QSize(18, 18));

  titleBar->layout()->addWidget(button);
}

void ComposerMainWindow::tabClosed(int index)
{
  if(index == 0)
    return; // Do nothing

  QString location = tabProjects->tabToolTip(index);
  Project *project = ProjectControl::getInstance()->getOpenProject(location);
  qDebug() << location << project;
  if(project != NULL && project->isDirty())
  {
    int ret = QMessageBox::warning(this, project->getAttribute("id"),
                                   tr("The project has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No,
                                   QMessageBox::Cancel|QMessageBox::Escape);
    if (ret == QMessageBox::Yes)
      saveCurrentProject();
    else if (ret == QMessageBox::Cancel)
      return;
  }

  ProjectControl::getInstance()->closeProject(location);

  //Remove temporary file
  removeTemporaryFile(location);

  if(projectsWidgets.contains(location))
  {
    QMainWindow *w = projectsWidgets[location];

    allDocksMutex.lock();
      QList<QDockWidget*> newAllDocks;
      //Remove from allDocks
      for( int i = 0; i < allDocks.size(); i++)
      {
        if(w->isAncestorOf(allDocks.at(i)))
          allDocks.removeAt(i);
        else
          newAllDocks.push_back(allDocks.at(i));
      }
      allDocks = newAllDocks;
    allDocksMutex.unlock();

    // Delete QMainWindow
    if (w)
    {
        w->close();
        w->deleteLater();
    }
    projectsWidgets.remove(location);
    firstDock.remove(location);
  }
}

void ComposerMainWindow::closeCurrentTab()
{
  if(tabProjects->currentIndex())
  {
    int currentIndex = tabProjects->currentIndex();
    tabClosed(currentIndex);
  }
}
void ComposerMainWindow::closeAllFiles()
{
  while(tabProjects->count() > 1)
  {
    tabClosed(1);
    tabProjects->removeTab(1);
  }
}

void ComposerMainWindow::onOpenProjectTab(QString location)
{
  if (!projectsWidgets.contains(location)) return;
  QMainWindow *w = projectsWidgets[location];
  tabProjects->setCurrentWidget(w);
}

void ComposerMainWindow::createMenus()
{
#ifdef Q_WS_MAC
  ui->menubar->setNativeMenuBar(true);
#endif

  ui->menu_Edit->addAction(editPreferencesAct);

  menuBar()->addSeparator();

  connect( ui->menu_Window, SIGNAL(aboutToShow()),
          this, SLOT(updateViewMenu()));

  connect(ui->action_CloseProject, SIGNAL(triggered()),
          this, SLOT(closeCurrentTab()));

  connect(ui->action_CloseAll, SIGNAL(triggered()),
          this, SLOT(closeAllFiles()));

  connect( ui->action_Save, SIGNAL(triggered()),
          this, SLOT(saveCurrentProject()));

  connect( ui->action_SaveAs, SIGNAL(triggered()),
          this, SLOT(saveAsCurrentProject()));

  connect ( ui->action_NewProject, SIGNAL(triggered()),
           this, SLOT(launchProjectWizard()));

  /* menu_Language = new QMenu(0);
  tbLanguageDropList->setMenu(menu_Language);
  ui->toolBar->addWidget(tbLanguageDropList);*/

  menu_Perspective = new QMenu(0);
  // assing menu_Perspective to tbPerspectiveDropList
  tbPerspectiveDropList->setMenu(menu_Perspective);
  ui->toolBar->addWidget(tbPerspectiveDropList);

  // tabProjects->setCornerWidget(tbPerspectiveDropList, Qt::TopRightCorner);
  tabProjects->setCornerWidget(ui->toolBar, Qt::TopRightCorner);
  // tabProjects->setCornerWidget(ui->menu_Window, Qt::TopLeftCorner);

//  updateMenuLanguages();
  updateMenuPerspectives();
}

void ComposerMainWindow::createAboutPlugins()
{
  aboutPluginsDialog = new QDialog(this);
  aboutPluginsDialog->setWindowTitle(tr("Installed Plugins"));

#ifdef SHOW_PROFILES
  profilesExt = new QListWidget(aboutPluginsDialog);
  profilesExt->setAlternatingRowColors(true);
#endif

  /* This should be a new Widget and change some code for there */
  pluginsExt = new QTreeWidget(aboutPluginsDialog);
  pluginsExt->setAlternatingRowColors(true);

  connect(pluginsExt, SIGNAL(itemSelectionChanged()),
          this, SLOT(selectedAboutCurrentPluginFactory()));

  QStringList header;
  header << tr("Name") << tr("Load") << tr("Version") << tr("Vendor");
  pluginsExt->setHeaderLabels(header);

  QDialogButtonBox *bOk = new QDialogButtonBox(QDialogButtonBox::Ok |
                                               QDialogButtonBox::Close,
                                               Qt::Horizontal,
                                               aboutPluginsDialog);

  detailsButton = bOk->button(QDialogButtonBox::Ok);
  detailsButton->setText(tr("Details"));
  detailsButton->setIcon(QIcon());
  detailsButton->setEnabled(false);

  connect(bOk, SIGNAL(rejected()), aboutPluginsDialog, SLOT(close()));

  connect( detailsButton, SIGNAL(pressed()), this, SLOT(showPluginDetails()) );

  QGridLayout *gLayout = new QGridLayout(aboutPluginsDialog);
  gLayout->addWidget(new QLabel(tr("The <b>Composer</b> is an IDE for"
                                   " Declarative Multimedia languages."),
                                aboutPluginsDialog));

#ifdef SHOW_PROFILES
  gLayout->addWidget(new QLabel(tr("<b>Installed Language Profiles</b>"),
                                aboutPluginsDialog));
  gLayout->addWidget(profilesExt);
#endif
  gLayout->addWidget(new QLabel(tr("<b>Installed Plug-ins</b>")));
  gLayout->addWidget(pluginsExt);
  gLayout->addWidget(bOk);
  aboutPluginsDialog->setLayout(gLayout);

  aboutPluginsDialog->setModal(true);

  connect(aboutPluginsDialog, SIGNAL(finished(int)),
          this, SLOT(saveLoadPluginData(int)));
}

void ComposerMainWindow::about()
{
  AboutDialog dialog(this);
  dialog.exec();
}

void ComposerMainWindow::aboutPlugins()
{
  QList<IPluginFactory*>::iterator it;
  QList<IPluginFactory*> pList = PluginControl::getInstance()->
      getLoadedPlugins();
  pluginsExt->clear();

  //search for categories
  QTreeWidgetItem *treeWidgetItem;
  QMap <QString, QTreeWidgetItem*> categories;
  for (it = pList.begin(); it != pList.end(); it++) {
    IPluginFactory *pF = *it;
    QString category = pF->category();
    if(!categories.contains(category)){
      treeWidgetItem = new QTreeWidgetItem(pluginsExt);
      categories.insert(category, treeWidgetItem);
      treeWidgetItem->setText(0, category);
      treeWidgetItem->setTextColor(0, QColor("#0000FF"));
    }
  }

  treeWidgetItem2plFactory.clear();
  for (it = pList.begin(); it != pList.end(); it++) {
    IPluginFactory *pF = *it;
    treeWidgetItem = new QTreeWidgetItem(categories.value(pF->category()));
    treeWidgetItem2plFactory.insert(treeWidgetItem, pF);

    treeWidgetItem->setText(0, pF->name());

    // Set checked (or not) based on the settings
    ComposerSettings settings;
    settings.beginGroup("loadPlugins");
    if(!settings.contains(pF->id()) || settings.value(pF->id()).toBool())
      treeWidgetItem->setCheckState(1, Qt::Checked);
    else
      treeWidgetItem->setCheckState(1, Qt::Unchecked);

    settings.endGroup();
    treeWidgetItem->setText(2, pF->version());
    treeWidgetItem->setText(3, pF->vendor());
  }

  pluginsExt->expandAll();

  pluginsExt->setColumnWidth(0, 150);
  pluginsExt->resizeColumnToContents(1);
  pluginsExt->resizeColumnToContents(2);
  pluginsExt->resizeColumnToContents(3);

  /* PROFILE LANGUAGE */
#ifdef SHOW_PROFILES
  QList<ILanguageProfile*>::iterator itL;
  QList<ILanguageProfile*> lList = LanguageControl::getInstance()->
      getLoadedProfiles();
  profilesExt->clear();

  for(itL = lList.begin(); itL != lList.end(); itL++)
  {
    ILanguageProfile *lg = *itL;
    profilesExt->addItem(new QListWidgetItem(lg->getProfileName()));
  }
#endif

  detailsButton->setEnabled(false);
  aboutPluginsDialog->show();
}

void ComposerMainWindow::errorDialog(QString message)
{
  //QMessageBox::warning(this,tr("Error!"),message);
  qWarning() << message;
}

void ComposerMainWindow::createActions() {

  connect(ui->action_About, SIGNAL(triggered()), this, SLOT(about()));

  connect( ui->action_AboutPlugins, SIGNAL(triggered()),
           this, SLOT(aboutPlugins()));

  fullScreenViewAct = new QAction(tr("&FullScreen"),this);
  fullScreenViewAct->setShortcut(tr("F11"));

  connect(fullScreenViewAct,SIGNAL(triggered()),this,
          SLOT(showCurrentWidgetFullScreen()));

  editPreferencesAct = new QAction(tr("&Preferences"), this);
  editPreferencesAct->setStatusTip(tr("Edit preferences"));
  connect (editPreferencesAct, SIGNAL(triggered()), this,
           SLOT(showEditPreferencesDialog()));

  connect(ui->action_Preferences, SIGNAL(triggered()),
          this, SLOT(showEditPreferencesDialog()));

  connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));
  saveCurrentPluginsLayoutAct = new QAction(tr("Save current perspective..."),
                                            this);

  connect(    saveCurrentPluginsLayoutAct, SIGNAL(triggered()),
          this, SLOT(saveCurrentGeometryAsPerspective()));

  restorePluginsLayoutAct = new QAction(tr("Restore a perspective"), this);
  connect(restorePluginsLayoutAct, SIGNAL(triggered()),
          this, SLOT(restorePerspective()));

  QWidget* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  //Add a separator to the toolbar. All actions after preferences (including
  // it will be aligned in the bottom.
  ui->toolBar->insertWidget(ui->action_Preferences, spacer);

  connect (ui->action_OpenProject, SIGNAL(triggered()),
           this, SLOT(openProject()));

  connect (ui->action_ImportFromExistingNCL, SIGNAL(triggered()),
           this, SLOT(importFromDocument()));

  connect (ui->action_GoToClubeNCLWebsite, SIGNAL(triggered()),
           this, SLOT(gotoNCLClubWebsite()));

  connect (ui->action_Help, SIGNAL(triggered()), this, SLOT(showHelp()));
}

void ComposerMainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

void ComposerMainWindow::showCurrentWidgetFullScreen()
{
  tabProjects->addAction(fullScreenViewAct);

  if(!tabProjects->isFullScreen())
  {
    tabProjects->setWindowFlags(Qt::Window);
    tabProjects->showFullScreen();
  }
  else
  {
    tabProjects->setParent(ui->frame, Qt::Widget);
    tabProjects->show();
  }
}

void ComposerMainWindow::updateViewMenu()
{
  ui->menu_Window->clear();
  ui->menu_Window->addAction(fullScreenViewAct);
  ui->menu_Window->addSeparator();
  ui->menu_Window->addAction(saveCurrentPluginsLayoutAct);
  ui->menu_Window->addAction(restorePluginsLayoutAct);
}

void ComposerMainWindow::closeEvent(QCloseEvent *event)
{

  /** Save any dirty project. \todo This must be a function. */
  for(int index = 1; index < tabProjects->count(); index++)
  {
    QString location = tabProjects->tabToolTip(index);
    Project *project = ProjectControl::getInstance()->getOpenProject(location);

    qDebug() << location << project;
    if(project != NULL && project->isDirty())
    {
      tabProjects->setCurrentIndex(index);
      int ret = QMessageBox::warning(this, project->getAttribute("id"),
                                     tr("The project %1 has been modified.\n"
                                        "Do you want to save your changes?").
                                     arg(location),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No,
                                     QMessageBox::Cancel|QMessageBox::Escape);
      if (ret == QMessageBox::Yes)
        saveCurrentProject();
      else if (ret == QMessageBox::Cancel)
      {
        event->ignore();
        return;
      }
    }
  }

  ComposerSettings settings;
  settings.beginGroup("extensions");
  settings.setValue("path", extensions_paths);
  settings.endGroup();

  settings.beginGroup("mainwindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.endGroup();

  QStringList openfiles;
  QString key;
  foreach (key, projectsWidgets.keys())
    openfiles << key;

  settings.beginGroup("openfiles");
  if(openfiles.size())
  {
    settings.setValue("openfiles", openfiles);
  }
  else {
    /* If there aren't any openfile, remove this settings, otherwise it will
       try to load the current path */
    settings.remove("openfiles");
  }
  settings.endGroup();
  settings.sync();
  cleanUp();

  qApp->quit(); // close the application
}

void ComposerMainWindow::cleanUp()
{
  LanguageControl::releaseInstance();
  ProjectControl::releaseInstance();
  PluginControl::releaseInstance();
}

void ComposerMainWindow::showEditPreferencesDialog()
{
  preferences->show();
}

void ComposerMainWindow::startOpenProject(QString project)
{
  (void) project;

  this->setCursor(QCursor(Qt::WaitCursor));
  update();
}

void ComposerMainWindow::endOpenProject(QString project)
{
  (void) project;
  this->setCursor(QCursor(Qt::ArrowCursor));

  ComposerSettings settings;

  if(settings.contains("default_perspective"))
  {
    QString defaultPerspective =
        settings.value("default_perspective").toString();
    restorePerspective(defaultPerspective);
    update();
  }
}

void ComposerMainWindow::saveCurrentProject()
{
  int index = tabProjects->currentIndex();
  bool saveAlsoNCLDocument = true;

  if(index != 0)
  {
    QString location = tabProjects->tabToolTip(index);
    Project *project = ProjectControl::getInstance()->getOpenProject(location);

    PluginControl::getInstance()->savePluginsData(project);
    ProjectControl::getInstance()->saveProject(location);
    ui->action_Save->setEnabled(false);

    if(saveAlsoNCLDocument)
    {

      QString nclfilepath = location.mid(0, location.lastIndexOf(".")) + ".ncl";
      QFile file(nclfilepath);
      if(file.open(QFile::WriteOnly | QIODevice::Truncate))
      {
        // Write FILE!!
        if(project->getChildren().size())
          file.write(project->getChildren().at(0)->toString(0, false).toAscii());

        file.close();
      }
    }
  }
  else
  {
    QMessageBox box(QMessageBox::Warning,
                    tr("Information"),
                    tr("There aren't a project to be saved."),
                    QMessageBox::Ok
                    );
    box.exec();
  }
}

void ComposerMainWindow::saveAsCurrentProject()
{
  int index = tabProjects->currentIndex();
  bool saveAlsoNCLDocument = true;

  if(index != 0)
  {
    QString location = tabProjects->tabToolTip(index);

    QString destFileName = QFileDialog::getSaveFileName(
          this,
          tr("Save as NCL Composer Project"),
          getLastFileDialogPath(),
          tr("NCL Composer Projects (*.cpr)") );

    if(!destFileName.isNull() && !destFileName.isEmpty())
    {
      updateLastFileDialogPath(destFileName);

      if(!destFileName.endsWith(".cpr"))
        destFileName  = destFileName + ".cpr";

      /* Move the location of the current project to destFileName */
      ProjectControl::getInstance()->moveProject(location, destFileName);

      /* Get the project */
      Project *project =
          ProjectControl::getInstance()->getOpenProject(destFileName);

      PluginControl::getInstance()->savePluginsData(project);
      ProjectControl::getInstance()->saveProject(destFileName);

      /* Update Tab Text and Index */
      updateTabWithProject(index, destFileName);

      ui->action_Save->setEnabled(false);

      if(saveAlsoNCLDocument)
      {
        QString nclfilepath =
            location.mid(0, destFileName.lastIndexOf(".")) + ".ncl";

        QFile file(nclfilepath);
        if(file.open(QFile::WriteOnly | QIODevice::Truncate))
        {
          // Write FILE!!
          if(project->getChildren().size())
            file.write(project->getChildren().at(0)->toString(0, false).toAscii());

          file.close();
        }
      }

      addToRecentProjects(destFileName);
    }
  }
  else
  {
    QMessageBox box(QMessageBox::Warning,
                    tr("Information"),
                    tr("There aren't a project to be saved."),
                    QMessageBox::Ok
                    );
    box.exec();
  }
}

void ComposerMainWindow::saveCurrentGeometryAsPerspective()
{
  if(tabProjects->count()) // If there is a document open
  {
    perspectiveManager->setBehavior(PERSPEC_SAVE);
    if(perspectiveManager->exec())
    {
      savePerspective(perspectiveManager->getSelectedName());
      saveDefaultPerspective(perspectiveManager->getDefaultPerspective());
    }
  }
  else
  {
    QMessageBox box(QMessageBox::Warning,
                    tr("Information"),
                    tr("There aren't a layout open to be saved."),
                    QMessageBox::Ok
                    );
    box.exec();
  }
  /* Update the elements in MENU PERSPECTIVE*/
  updateMenuPerspectives();
}

void ComposerMainWindow::restorePerspective()
{
  perspectiveManager->setBehavior(PERSPEC_LOAD);
  if(perspectiveManager->exec())
  {
    restorePerspective(perspectiveManager->getSelectedName());
  }

  /* Update the elements in MENU PERSPECTIVE*/
  updateMenuPerspectives();
}

void ComposerMainWindow::savePerspective(QString layoutName)
{
  if(tabProjects->count()) //see if there is any open document
  {
    QString location = tabProjects->tabToolTip(tabProjects->currentIndex());

    QMainWindow *window = projectsWidgets[location];
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       "telemidia",
                       "composer");

    settings.beginGroup("pluginslayout");
    settings.setValue(layoutName, window->saveState(0));
    settings.endGroup();
  }
}

void ComposerMainWindow::saveDefaultPerspective(QString defaultPerspectiveName)
{
  ComposerSettings settings;
  settings.setValue("default_perspective", defaultPerspectiveName);
}

void ComposerMainWindow::restorePerspective(QString layoutName)
{
  if(tabProjects->count()) //see if there is any open project
  {
    QString location = tabProjects->tabToolTip(
          tabProjects->currentIndex());

    QMainWindow *window = projectsWidgets[location];
    ComposerSettings settings;
    settings.beginGroup("pluginslayout");
#ifndef USE_MDI
    window->restoreState(settings.value(layoutName).toByteArray());
#endif
    settings.endGroup();
  }
}

void ComposerMainWindow::runNCL()
{
  bool runRemote = false;
  ComposerSettings settings;
  settings.beginGroup("runginga");
  if(settings.contains("run_remote") && settings.value("run_remote").toBool())
    runRemote = true;
  else
    runRemote = false;
  settings.endGroup();

  if(runRemote)
    runOnRemoteGingaVM();
  else
    runOnLocalGinga();
}

void ComposerMainWindow::runOnLocalGinga()
{
  ComposerSettings settings;
  QString command, args;
  settings.beginGroup("runginga");
  command = settings.value("local_ginga_cmd").toString();
  args = settings.value("local_ginga_args").toString();
  settings.endGroup();

  // TODO: Ask to Save current project before send it to Ginga VM.
  QProcess *ginga = new QProcess(this);
  QStringList args_list;
  QString location = tabProjects->tabToolTip(tabProjects->currentIndex());

  if(location.isEmpty())
  {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, tr("Warning!"),
                                 tr("There aren't a current NCL project."),
                                 QMessageBox::Ok);
    return;
  }

  Project *project = ProjectControl::getInstance()->getOpenProject(location);
  QString nclpath = location.mid(0, location.lastIndexOf("/")) +  "/tmp.ncl";
  qDebug() << "Running NCL File: " << nclpath;

  QFile file(nclpath);
  if(file.open(QFile::WriteOnly | QIODevice::Truncate))
  {
    /* Write FILE!! */
    if(project->getChildren().size())
      file.write(project->getChildren().at(0)->toString(0, false).toAscii());

    file.close();

    /* PARAMETERS */
    //\todo Other parameters
    args.replace("${nclpath}", nclpath);
    args_list << args.split("\n");

    /* RUNNING GINGA */
    ginga->start(command, args_list);
    QByteArray result = ginga->readAll();
  }
  else
  {
    qWarning() << "Error trying to running NCL. Could not create : "
               << nclpath << " !";
  }
}

void ComposerMainWindow::runOnRemoteGingaVM()
{
#ifdef WITH_LIBSSH2
  if(runRemoteGingaVMThread.isRunning())
  {
    // There aren't a current project.
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(NULL, tr("Warning!"),
                                 tr("You already have an NCL application "
                                    "running. Please, stop it before you start "
                                    "a new one."),
                                 QMessageBox::Ok);
    return;
  }

  int currentTab = tabProjects->currentIndex();
  if(currentTab != 0)
  {
    QString location = tabProjects->tabToolTip(currentTab);
    Project *currentProject = ProjectControl::getInstance()->
                                                      getOpenProject(location);

    if(currentProject->isDirty())
    {
      /*QMessageBox::StandardButton reply;
      reply = QMessageBox::warning(this, tr("Warning!"),
                                     tr("Your document is not saved. "
                                        "Do you want to save it now?"),
                                     QMessageBox::Yes, QMessageBox::No);*/

      int reply;
      reply = QMessageBox::warning(NULL, tr("Warning!"),
                                   tr("Your document is not saved."
                                      "Do you want to save it now?"),
                                   QMessageBox::Yes, QMessageBox::No);

      if(reply == QMessageBox::Yes)
      {
        // save the current project before send it to Ginga VM
        saveCurrentProject();
      }
    }

    runRemoteGingaVMAction.setCurrentProject(currentProject);

    runRemoteGingaVMThread.start();
  }
  else
  {
    // There aren't a current project.
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(NULL, tr("Warning!"),
                                 tr("There aren't a current NCL project."),
                                 QMessageBox::Ok);
  }

#else
  QMessageBox::StandardButton reply;
  reply = QMessageBox::warning(NULL, tr("Warning!"),
                               tr("Your NCL Composer was not build with Remote "
                                  "Run support."),
                               QMessageBox::Ok);
#endif
}

void ComposerMainWindow::stopRemoteNCL()
{
#ifdef WITH_LIBSSH2
  stopRemoteGingaVMAction.stopRunningApplication();
#endif
}

void ComposerMainWindow::launchProjectWizard()
{
  NewProjectWizard wizard (this);
  wizard.setModal(true);
  wizard.exec();

  if(wizard.result() == QWizard::Accepted)
  {
    QString filename = wizard.getProjectFullPath();

    filename.replace("\\", "/"); //Force the use of "/"

    if( !filename.isNull() && !filename.isEmpty())
    {

      updateLastFileDialogPath(filename);

      if(!filename.endsWith(".cpr"))
        filename = filename + QString(".cpr");

      QFileInfo info(filename);
      // If the file already exist ask the user if it want to overwrite it
      if(info.exists())
      {
          if(QMessageBox::question(this, tr("File already exists!"),
                                    tr("The file \"%1\" already exists. Do you want overwrite it?").arg(filename),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return; // Do not overwrite!

          QFile::remove(filename); // Remove the OLD file (only if the user accept it)
      }

      // We dont need to check this, when creating a new project
      // checkTemporaryFileLastModified(filename);


      if(ProjectControl::getInstance()->launchProject(filename))
      {
        // After launch the project we will insert NCL, HEAD and BODY elements
        // by default
        Project *project = ProjectControl::getInstance()
                                               ->getOpenProject(filename);

        addDefaultStructureToProject( project,
                                      wizard.shouldCopyDefaultConnBase() );
      }
      else
      {
        // \todo a report to this problem (we should track the error message).
      }
    }
  }
}

void ComposerMainWindow::addDefaultStructureToProject(Project *project,
                                                 bool shouldCopyDefaultConnBase,
                                                 bool save)
{
  const QString defaultNCLID = "myNCLDocID";
  const QString defaultBodyID = "myBodyID";
  const QString defaultConnBaseID = "connBaseId";

  QMap <QString, QString> nclAttrs, headAttrs, bodyAttrs;
  nclAttrs.insert("id", defaultNCLID);
  nclAttrs.insert("xmlns", "http://www.ncl.org.br/NCL3.0/EDTVProfile");

  Entity *nclEntity;
  MessageControl *msgControl = PluginControl::getInstance()
      ->getMessageControl(project);
  msgControl->anonymousAddEntity("ncl", project->getUniqueId(), nclAttrs);

  nclEntity = project->getEntitiesbyType("ncl").first();

  if(nclEntity != NULL)
  {
    QString nclEntityId = nclEntity->getUniqueId();
    msgControl->anonymousAddEntity("head", nclEntityId, headAttrs);

    bodyAttrs.insert("id", defaultBodyID);
    msgControl->anonymousAddEntity("body", nclEntityId, bodyAttrs);
  }

  // Copy the default connectro
  if(shouldCopyDefaultConnBase)
  {
    ComposerSettings settings;
    settings.beginGroup("importBases");
    QString defaultConnBase =
        settings.value("default_conn_base").toString();
    settings.endGroup();

    qDebug() << "[GUI] DefaultConnBase " << defaultConnBase;

    QFileInfo defaultConnBaseInfo(defaultConnBase);
    if(defaultConnBaseInfo.exists())
    {
      QString filename = project->getLocation();
      QString newConnBase = filename. mid(0, filename.lastIndexOf("/")+1) +
                            defaultConnBaseInfo.fileName();

      qDebug() << "[GUI] Copy " << defaultConnBase << " to "
               << newConnBase;

      //remove the file if it already exists
      if(QFile::exists(newConnBase))
      {
        QFile::remove(newConnBase);
      }

      //copy the defaultConnBase to the project dir
      if(QFile::copy(defaultConnBase, newConnBase))
      {
        //If everything is OK we import the new defaultConnBase to NCL
        // document.

        QMap <QString, QString> connBaseAttrs, importBaseAttrs;
        connBaseAttrs.insert("id", defaultConnBaseID);
        importBaseAttrs.insert("alias", "conn");
        importBaseAttrs.insert("documentURI",
                               defaultConnBaseInfo.fileName());

        //add connectorBase element
        Entity *head = project->getEntitiesbyType("head").at(0);
        msgControl->anonymousAddEntity("connectorBase",
                                       head->getUniqueId(),
                                       connBaseAttrs);

        //add importBase element
        Entity *connectorBase =
            project->getEntitiesbyType("connectorBase").at(0);
        msgControl->anonymousAddEntity("importBase",
                                       connectorBase->getUniqueId(),
                                       importBaseAttrs);
      }
      else //error
      {
        QMessageBox::warning(this, tr("Error!"),
                             tr("There was an error copying the default"
                                "Connector Base. You will need to add a "
                                "Connector Base by hand in your NCL "
                                "code."),
                             tr("Ok"));
      }
    }
    else //error
    {
      QMessageBox::warning(this, tr("Error!"),
                           tr("The default Connect Base %1 does not"
                              "exists").arg(defaultConnBase),
                           tr("Ok"));
    }
  }

  if(save)
    saveCurrentProject(); //Save the just created basic file!
}

void ComposerMainWindow::openProject()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                               tr("Open NCL Composer Project"),
                                               getLastFileDialogPath(),
                                           tr("NCL Composer Projects (*.cpr)"));
  if(filename != "")
  {
#ifdef WIN32
    filename = filename.replace("\\", "/");
#endif

    checkTemporaryFileLastModified(filename);

    ProjectControl::getInstance()->launchProject(filename);

    updateLastFileDialogPath(filename);
  }
}

void ComposerMainWindow::checkTemporaryFileLastModified(QString filename)
{
  QFileInfo temporaryFileInfo(filename + "~");
  QFileInfo fileInfo(filename);

  if(temporaryFileInfo.exists() &&
     temporaryFileInfo.lastModified() > fileInfo.lastModified())
  {
      bool replace = QMessageBox::question(this,
                            tr("Temporary file is newer."),
                            tr("There is a temporary file related to %1 that is"
                               " newer. Do you want replace the %1 file with "
                               " this temporary one?").arg(filename),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No);

      if(replace)
      {
          QFile file (filename + "~");
          if(!file.copy(filename))
          {
              QFile oldfile (filename);
              oldfile.remove();
              file.copy(filename);
          }
      }
  }
}

bool ComposerMainWindow::removeTemporaryFile(QString location)
{
  QFile file(location + "~");
  return file.remove();
}

void ComposerMainWindow::updateLastFileDialogPath(QString filepath)
{
  Utilities::updateLastFileDialogPath(filepath);
}

QString ComposerMainWindow::getLastFileDialogPath()
{
  return Utilities::getLastFileDialogPath();
}

void ComposerMainWindow::importFromDocument()
{
  QString docFilename = QFileDialog::getOpenFileName(
        this,
        tr("Choose the NCL file to be imported"),
        getLastFileDialogPath(),
        tr("NCL Documents (*.ncl)") );

  if(docFilename != "")
  {
    updateLastFileDialogPath(docFilename);

    QString projFilename = QFileDialog::getSaveFileName(
          this,
          tr("Choose the NCL Composer Project where the NCL document must be "
             "imported"),
             getLastFileDialogPath(),
             tr("NCL Composer Projects (*.cpr)") );

    //Create the file
    QFile f(projFilename);
    f.open(QIODevice::ReadWrite);
    f.close();

    if(projFilename != "")
    {
#ifdef WIN32
      projFilename = projFilename.replace(QDir::separator(), "/");
#endif
      ProjectControl::getInstance()->importFromDocument(docFilename,
                                                        projFilename);

      updateLastFileDialogPath(projFilename);
    }
  }
}

void ComposerMainWindow::addToRecentProjects(QString projectUrl)
{
  ComposerSettings settings;
  QStringList recentProjects = settings.value("recentProjects").toStringList();

  recentProjects.push_front(projectUrl);
  recentProjects.removeDuplicates();

  //MAXIMUM SIZE
  while(recentProjects.size() > this->maximumRecentProjectsSize)
    recentProjects.pop_back();

  settings.setValue("recentProjects", recentProjects);

  updateRecentProjectsMenu(recentProjects);
  welcomeWidget->updateRecentProjects(recentProjects);
}

void ComposerMainWindow::updateRecentProjectsMenu(QStringList &recentProjects)
{
  ui->menu_Recent_Files->clear();
  if(recentProjects.size() == 0 )
  {
    QAction *act = ui->menu_Recent_Files->addAction(tr("empty"));
    act->setEnabled(false);
  }
  else /* There are at least one element in the recentProject list */
  {
    for(int i = 0; i < recentProjects.size(); i++)
    {
      QAction *act = ui->menu_Recent_Files->addAction(
            recentProjects.at(i));
      act->setData(recentProjects.at(i));
      connect(act, SIGNAL(triggered()), this, SLOT(userPressedRecentProject()));
    }

    ui->menu_Recent_Files->addSeparator();
    QAction *clearRecentProjects =
        ui->menu_Recent_Files->addAction(tr("Clear Recent Projects"));

    connect(clearRecentProjects, SIGNAL(triggered()),
            this, SLOT(clearRecentProjects()));

  }
}

void ComposerMainWindow::userPressedRecentProject(QString src)
{
#ifdef WIN32
  src = src.replace(QDir::separator(), "/");
#endif

  QFile file(src);
  bool openCurrentFile = true, recreateFile = false;
  if(!file.exists())
  {
    int resp =
        QMessageBox::question(this,
                              tr("File does not exists anymore."),
                              tr("The File %1 does not exists anymore. "
                                 "Do you want to create recreate this file "
                                 "again?").arg(src),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No);

    if(resp != QMessageBox::Yes)
      openCurrentFile = false;
    else
      recreateFile = true;
  }

  if(openCurrentFile)
  {
    checkTemporaryFileLastModified(src);

    ProjectControl::getInstance()->launchProject(src);
    if(recreateFile)
    {
      // \todo Ask for the import or not of the defaultConnBase.
      addDefaultStructureToProject(
            ProjectControl::getInstance()->getOpenProject(src),
            false,
            true);
    }
  }
}

void ComposerMainWindow::userPressedRecentProject()
{
  QAction *action = qobject_cast<QAction *> (QObject::sender());

  QString src = action->data().toString();

  userPressedRecentProject(src);
}

void ComposerMainWindow::clearRecentProjects(void)
{
  ComposerSettings settings;

  settings.remove("recentProjects");
  QStringList empty;
  updateRecentProjectsMenu(empty);
}

void ComposerMainWindow::selectedAboutCurrentPluginFactory()
{
  QList<QTreeWidgetItem*> selectedPlugins = pluginsExt->selectedItems();
  if(selectedPlugins.size())
  {
    if(treeWidgetItem2plFactory.value(selectedPlugins.at(0)) != NULL)
    {
      pluginDetailsDialog->setCurrentPlugin(
            treeWidgetItem2plFactory.value(selectedPlugins.at(0)));
      detailsButton->setEnabled(true);
    }
    else
      detailsButton->setEnabled(false);
  }
}

void ComposerMainWindow::showPluginDetails()
{
  pluginDetailsDialog->show();
}

void ComposerMainWindow::restorePerspectiveFromMenu()
{
  QAction *action = qobject_cast<QAction*>(QObject::sender());
  restorePerspective(action->data().toString());
}

void ComposerMainWindow::updateMenuPerspectives()
{
  ComposerSettings settings;
  settings.beginGroup("pluginslayout");
  QStringList keys = settings.allKeys();
  settings.endGroup();

  menu_Perspective->clear();

  for(int i = 0; i < keys.size(); i++)
  {
    QAction *act = menu_Perspective->addAction(keys.at(i),
                                            this,
                                            SLOT(restorePerspectiveFromMenu()));
    act->setData(keys[i]);
  }

  // Add Option to save current Perspective
  menu_Perspective->addSeparator();
  menu_Perspective->addAction(saveCurrentPluginsLayoutAct);
}

void ComposerMainWindow::updateMenuLanguages()
{
  QStringList languages;
  languages << tr("English") << tr("Portugues (Brasil)");

  for(int i = 0; i < languages.size(); i++)
  {
    QAction *act = menu_Language->addAction(languages.at(i),
                                            this,
                                            SLOT(changeLanguageFromMenu()));
    act->setData(languages[i]);
  }
}

void ComposerMainWindow::currentTabChanged(int n)
{
  if(n)
  {
    tbPerspectiveDropList->setEnabled(true);
    saveCurrentPluginsLayoutAct->setEnabled(true);
    restorePluginsLayoutAct->setEnabled(true);
    ui->action_CloseProject->setEnabled(true);
    ui->action_Save->setEnabled(true);
    ui->action_SaveAs->setEnabled(true);
    ui->action_RunNCL->setEnabled(true);
    ui->action_StopRemoteNCL->setEnabled(true);
  }
  else
  {
    tbPerspectiveDropList->setEnabled(false);
    saveCurrentPluginsLayoutAct->setEnabled(false);
    restorePluginsLayoutAct->setEnabled(false);
    ui->action_CloseProject->setEnabled(false);
    ui->action_Save->setEnabled(false);
    ui->action_SaveAs->setEnabled(false);
    ui->action_RunNCL->setEnabled(false);
    ui->action_StopRemoteNCL->setEnabled(false);
  }
}

void ComposerMainWindow::focusChanged(QWidget *old, QWidget *now)
{
  if(!isActiveWindow() || now == NULL)
      return; // Do nothing!!

  // qDebug() << "Locking allDocksMutex 1";
  allDocksMutex.lock();
  for(int i = 0; i < allDocks.size(); i++)
  {
    // if(old != NULL && allDocks.at(i)->isAncestorOf(old))
    updateDockStyle(allDocks.at(i), false);
  }
  allDocksMutex.unlock();
  // qDebug() << "Unlocked allDocksMutex 1";

  // qDebug() << "Locking allDocksMutex 2";
  allDocksMutex.lock();
  for(int i = 0 ; i < allDocks.size(); i++)
  {
    if(now != NULL)
    {
      bool isAncestor = false;
      QWidget *child = now;
      // qDebug() << "Start";
      while (child)
      {
        // qDebug() << "child pointer" << child;
        // qDebug() << child->metaObject()->className();
        if (child == allDocks.at(i))
        {
          isAncestor = true;
          break;
        }
        child = child->parentWidget();
      }
      // qDebug() << "End";

      if(isAncestor)
        updateDockStyle(allDocks.at(i), true);
    }
  }
  allDocksMutex.unlock();
  // qDebug() << "Unlocked allDocksMutex 2";
}

void ComposerMainWindow::setProjectDirty(QString location, bool isDirty)
{
  QMainWindow *window = projectsWidgets[location];
  QString projectId =
      ProjectControl::getInstance()->
      getOpenProject(location)->getAttribute("id");

  int index = tabProjects->indexOf(window);

  ui->action_Save->setEnabled(true);

  if(index >= 0) {
    if(isDirty)
      tabProjects->setTabText(index, QString("*")+projectId);
    else
      tabProjects->setTabText(index, projectId);
  }
}

void ComposerMainWindow::undo()
{
  int index = tabProjects->currentIndex();

  if(index != -1)
  {
    QString location = tabProjects->tabToolTip(index);
    Project *project = ProjectControl::getInstance()->getOpenProject(location);
    MessageControl *msgControl =
        PluginControl::getInstance()->getMessageControl(project);
    msgControl->undo();
  }
}

void ComposerMainWindow::redo()
{
  int index = tabProjects->currentIndex();

  if(index != -1)
  {
    QString location = tabProjects->tabToolTip(index);
    Project *project = ProjectControl::getInstance()->getOpenProject(location);
    MessageControl *msgControl =
        PluginControl::getInstance()->getMessageControl(project);
    msgControl->redo();
  }

}

void ComposerMainWindow::gotoNCLClubWebsite()
{
  QDesktopServices::openUrl(QUrl("http://club.ncl.org.br"));
}

bool ComposerMainWindow::showHelp()
{
  composerHelpWidget.show();
  return true;

  // Old implementation based on Assistant
  /*if (!proc)
    proc = new QProcess();

  if (proc->state() != QProcess::Running)
  {
    QString app = QLibraryInfo::location(QLibraryInfo::BinariesPath) +
        QDir::separator();
#if !defined(Q_OS_MAC)
    app += QLatin1String("assistant");
#else
    app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif

    QStringList args;
//  args << QLatin1String("-collectionFile")
//       << QLatin1String("help/composerhelp.qhc")
//       << QLatin1String("-enableRemoteControl");

    proc->start(app, args);

    if (!proc->waitForStarted()) {
      QMessageBox::critical(0, QObject::tr("Simple Text Viewer"),
                    QObject::tr("Unable to launch Qt Assistant (%1)").arg(app));
      return false;
    }
  }
  return true; */
}

void ComposerMainWindow::autoSaveCurrentProjects()
{
  for(int i = 1; i < tabProjects->count(); i++)
  {
    QString location = tabProjects->tabToolTip(i);
    Project *project = ProjectControl::getInstance()->getOpenProject(location);
    if(project->isDirty())
    {
      PluginControl::getInstance()->savePluginsData(project);
      ProjectControl::getInstance()->saveTemporaryProject(location);
    }
  }
}

// we create the menu entries dynamically, dependant on the existing translations
void ComposerMainWindow::createLanguageMenu(void)
{
  QActionGroup* langGroup = new QActionGroup(menu_Language);
  langGroup->setExclusive(true);

  connect(langGroup, SIGNAL(triggered(QAction *)), this, SLOT(slotLanguageChanged(QAction *)));

  // format systems language
  QString defaultLocale = QLocale::system().name();       // e.g. "de_DE"
  defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"

  m_langPath = QApplication::applicationDirPath();
  m_langPath.append("/languages");
  QDir dir(m_langPath);
  QStringList fileNames = dir.entryList(QStringList("composer_*.qm"));

  qDebug() << fileNames;
  for (int i = 0; i < fileNames.size(); ++i)
  {
    // get locale extracted by filename
    QString locale;
    locale = fileNames[i];                  // "TranslationExample_de.qm"
    locale.truncate(locale.lastIndexOf('.'));   // "TranslationExample_de"
    locale.remove(0, locale.indexOf('_') + 1);   // "de"

    QString lang = QLocale::languageToString(QLocale(locale).language());
    QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

    QAction *action = new QAction(ico, lang, this);
    action->setCheckable(true);
    action->setData(locale);

//    ui->menu_Language->addAction(action);
    menu_Language->addAction(action);
    langGroup->addAction(action);

    // set default translators and language checked
    if (defaultLocale == locale)
    {
      action->setChecked(true);
    }
  }
}

// Called every time, when a menu entry of the language menu is called
void ComposerMainWindow::slotLanguageChanged(QAction* action)
{
  if(0 != action)
  {
    // load the language dependant on the action content
    loadLanguage(action->data().toString());
    setWindowIcon(action->icon());
  }
}

void ComposerMainWindow::switchTranslator(QTranslator& translator,
                                          const QString& filename)
{
  // remove the old translator
  qApp->removeTranslator(&translator);

  // load the new translator
  if(translator.load(filename))
    qApp->installTranslator(&translator);
}

void ComposerMainWindow::loadLanguage(const QString& rLanguage)
{
  qDebug() << rLanguage;
  if(m_currLang != rLanguage)
  {
    m_currLang = rLanguage;
    QLocale locale = QLocale(m_currLang);
    QLocale::setDefault(locale);
    QString languageName = QLocale::languageToString(locale.language());
    switchTranslator(m_translator,
                     m_langPath + "/" + QString("composer_%1.qm").arg(rLanguage));
    switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));

//  ui->statusBar->showMessage(tr("Current Language changed to %1").arg(languageName));
  }
}

void ComposerMainWindow::changeEvent(QEvent* event)
{
  if(0 != event)
  {
    switch(event->type())
    {
    // this event is send if a translator is loaded
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
      // this event is send, if the system, language changes
    case QEvent::LocaleChange:
    {
      QString locale = QLocale::system().name();
      locale.truncate(locale.lastIndexOf('_'));
      loadLanguage(locale);
    }
    break;
    default:
      break;
    }
  }

  QMainWindow::changeEvent(event);
}

void ComposerMainWindow::updateTabWithProject(int index, QString newLocation)
{
  QString oldLocation = tabProjects->tabToolTip(index);

  if(oldLocation == newLocation) return; /* do nothing */

  /* Already had a project in this tab */
  if(!oldLocation.isNull() && !oldLocation.isEmpty())
  {
    // Update projectsWidgets
    if(projectsWidgets.contains(oldLocation))
    {
      projectsWidgets.insert(newLocation, projectsWidgets.value(oldLocation));
      projectsWidgets.remove(oldLocation);
    }

    //update firstDock
    if(firstDock.contains(oldLocation))
    {
      firstDock.insert(newLocation, firstDock.value(oldLocation));
      firstDock.remove(oldLocation);

    };
  }

  tabProjects->setTabToolTip(index, newLocation);
  Project *project = ProjectControl::getInstance()->getOpenProject(newLocation);
  if(project != NULL)
  {
    QString projectId = project->getAttribute("id");
    tabProjects->setTabText(index, projectId);
  }
}

void ComposerMainWindow::saveLoadPluginData(int)
{
  ComposerSettings settings;
  settings.beginGroup("loadPlugins");
  QTreeWidgetItem *item;
  qDebug() << treeWidgetItem2plFactory.keys();
  foreach(item, treeWidgetItem2plFactory.keys())
  {
    if(item->checkState(1))
    {
      settings.setValue(treeWidgetItem2plFactory.value(item)->id(), true);
    }
    else
    {
      qDebug() << treeWidgetItem2plFactory.value(item) << "2";
      settings.setValue(treeWidgetItem2plFactory.value(item)->id(), false);
    }
  }
  settings.endGroup();
}

void ComposerMainWindow::on_actionReport_Bug_triggered()
{
  QDesktopServices::openUrl(QUrl("http://composer.telemidia.puc-rio.br/en/contact"));
}

} } //end namespace

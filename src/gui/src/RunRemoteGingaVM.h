/* Copyright (c) 2011-2012 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef RUNREMOTEGINGAVM_H
#define RUNREMOTEGINGAVM_H

#include <QThread>
#include <QProgressBar>
#include <model/Project.h>

#include "SimpleSSHClient.h"

class RunRemoteGingaVMAction : public QObject
{
  Q_OBJECT

public:
  explicit RunRemoteGingaVMAction();
  void setCurrentProject(Project* project);

public slots:
  void copyCurrentProject();
  void stopExecution();
  void setAutoplay(bool autoplay);

signals:
  void finished();
  void copyFinished();

  void startTask();
  void taskDescription(QString);
  void taskMaximumValue(int);
  void taskValue(int);

private:
  Project *project;
  bool mustStop;
  bool _autoplay;

  /*!
   * \brief Return the list of files to be sent to the Ginga Set-top box
   *  Virtual.
   *
   * \param project The project that we want to send.
   */
  QStringList filesToSendToGingaVM(Project *project, QString nclLocalPath);

  bool sendFilesToGingaVM(SimpleSSHClient &sshclient, QString baseLocalPath,
                          QString baseRemotePath, QStringList filesToSend);

  bool fixSrcsFromNCLFile(const QString &nclLocalPath);
};

class StopRemoteGingaVMAction : public QObject
{
  Q_OBJECT

public slots:
  void stopRunningApplication();

};
#endif // RUNREMOTEGINGAVM_H
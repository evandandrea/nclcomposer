/* Copyright (c) 2011 Telemidia/PUC-Rio.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Telemidia/PUC-Rio - initial API and implementation
 */
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "ComposerCoreControl_global.h"

#include <QString>
#include <QMap>
#include <QDir>
#include <QFileInfo>

#include "util/ComposerSettings.h"

/*!
 \brief
*/
typedef enum {
      CPR,
      NCL,
      SMIL,
      HTML,
      NONE
} LanguageType;

namespace composer {
    namespace core {
        namespace util {

/*!
 \brief Groups useful functions.
*/
class COMPOSERCORESHARED_EXPORT Utilities
{

private:
    static QMap<QString,LanguageType> types; /*!< TODO */

public:
    /*!
     * \brief
     * \param ext
     */
    static LanguageType getLanguageTypeByExtension(QString ext);
    /*!
     * \brief
     * \param type
     */
    static QString getExtensionForLanguageType(LanguageType type);
    /*!
     * \brief
     */
    static QString relativePath( QString absolutePath, QString relativeTo,
                                         bool bIsFile = false );
    /*!
     * \brief
     */
    static QString getLastFileDialogPath();

    /*!
     * \brief
     */
    static void updateLastFileDialogPath(QString filepath);
};

} } } //end namespace

#endif // DEFINITIONS_H

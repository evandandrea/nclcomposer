#ifndef CORECONTROL_GLOBAL_H
#define CORECONTROL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COMPOSERCORECONTROL_LIBRARY)
#  define COMPOSERCORECONTROLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define COMPOSERCORECONTROLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CORECONTROL_GLOBAL_H
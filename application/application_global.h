#ifndef APPLICATION_GLOBAL_H
#define APPLICATION_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(APPLICATION_LIBRARY)
#  define APPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define APPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // APPLICATION_GLOBAL_H

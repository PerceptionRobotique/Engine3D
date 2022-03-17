#ifndef ENGINE3D_GLOBAL_H
#define ENGINE3D_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ENGINE3D_LIBRARY)
#  define ENGINE3D_EXPORT Q_DECL_EXPORT
#else
#  define ENGINE3D_EXPORT Q_DECL_IMPORT
#endif

#endif // ENGINE3D_GLOBAL_H

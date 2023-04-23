#ifndef UTILS_H
#define UTILS_H

#include <utils_export.h>

class QScreen;

namespace Utils {

UTILS_EXPORT QPoint centralized(const QSize& size, const QScreen* screen);
UTILS_EXPORT QIcon si(const QString& fileName, qreal dpr);
UTILS_EXPORT QPixmap sp(const QString& fileName, const QSize& size, qreal dpr);

} // Utils

#endif // UTILS_H

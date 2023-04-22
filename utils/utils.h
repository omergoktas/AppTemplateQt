#ifndef UTILS_H
#define UTILS_H

#include <utils_export.h>

#include <QGuiApplication>
#include <QDir>

class QByteArray;
class QNetworkReply;
class QNetworkRequest;
class QJsonObject;

namespace Utils {

UTILS_EXPORT QPoint centerPos(const QSize& size, const QScreen* screen = QGuiApplication::primaryScreen());
UTILS_EXPORT QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
UTILS_EXPORT QString translate(const QByteArray& data);
UTILS_EXPORT QByteArray stringify(const QJsonObject& obj);
UTILS_EXPORT QStringList files(const QString& dir, QDir::Filters f = QDir::Files, QDir::SortFlags s = QDir::NoSort);
UTILS_EXPORT QIcon si(const QString& fileName, qreal dpr);
UTILS_EXPORT QPixmap sp(const QString& fileName, const QSize& size, qreal dpr);

} // Utils

#endif // UTILS_H

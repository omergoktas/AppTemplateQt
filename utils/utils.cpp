#include "utils.h"

#include <QScreen>
#include <QStyle>
#include <QFile>

QPoint Utils::centralized(const QSize& size, const QScreen* screen)
{
    return QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size,
                               screen->availableGeometry()).topLeft();
}

QIcon Utils::si(const QString& fileName, qreal dpr)
{
    static const QString SUFFIX_175 = u"-175.svg"_s;
    static const QString SUFFIX_150 = u"-150.svg"_s;
    static const QString SUFFIX_125 = u"-125.svg"_s;
    static const QString SUFFIX_100 = u"-100.svg"_s;

    const qreal MOD_100 = std::fmod(dpr, 1.00);
    const qreal MOD_125 = std::fmod(dpr, 1.25);
    const qreal MOD_150 = std::fmod(dpr, 1.50);
    const qreal MOD_175 = std::fmod(dpr, 1.75);

    QString suffix = SUFFIX_175;
    qreal remainder = MOD_175;

    if (MOD_150 < remainder) {
        suffix = SUFFIX_150;
        remainder = MOD_150;
    }
    if (MOD_125 < remainder) {
        suffix = SUFFIX_125;
        remainder = MOD_125;
    }
    if (MOD_100 < remainder)
        suffix = SUFFIX_100;

    if (QFile::exists(fileName + suffix))
        return QIcon(fileName + suffix);

    return QIcon(fileName + u".svg"_s);
}

QPixmap Utils::sp(const QString& fileName, const QSize& size, qreal dpr)
{
    return si(fileName, dpr).pixmap(size, dpr);
}

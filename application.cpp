#include "application.h"
#include <cmath>
#include <utils.h>
#include <signalhandler.h>

#include <private/qhighdpiscaling_p.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface_p.h>

#include <QTranslator>
#include <QLibraryInfo>
#include <QFontDatabase>
#include <QSettings>
#include <QDir>
#include <QToolTip>
#include <QScreen>
#include <QStyle>
#include <QStyleHints>
#include <QStyleFactory>
#include <QPixmapCache>
#include <QStandardPaths>

// TODOs:
// Settings-driven (default system dependent - as 3rd option) Dark-light mode (appearance mode) palette + Runtime change support
// Settings-driven (default system language) language/translation + Runtime change support
// Platform/Settings-driven small font tweaks + what should be adjustable whatnot + Runtime change support
// Platform/Settings-driven small geometry (widget size & icon/pixmap/image size) tweaks + what should be adjustable whatnot + Runtime change support
// Cospetic-pen painting artifacts + what should be cosmetic whatnot + scaling's affects on it
// Regular-pen painting artifacts + what should be regular whatnot + scaling's affects on it
// deployların otomatik olarak imzalanması eklenecek
// pluginlerin instancelarının opsiyonel olmasını sağla, yani önce nullptr kontrolu yap sonra instancı kullan, eğer plugin yoksa warning de göster
// her yeri, özellikle cmake dosyalarını, yorum satırları ve copyrightlar la süsle
// Bütün build scriptleri/static/shared plugin vs hepsi github actions ile çalışıp çalışmadığı denenecek
// Tüm scriptlerin adam akıllı çalışıp çalışmadığını denetlemek için testler yazılacak (github actions üzerinde misal)
// Genel manada test konusu düşünülecek ui test, cpp test, build test, install test, qt sürüm değişimi test, dll/plugin varmı yokmu yüklendimi test,
// uygulama isminde boşlık olması hiç bir platformda build install vs leri engellememeli, misal dmg oluşumunu bi test et
// tüm sistem property change observers/connections class destructorlarında disconnect edilmeli --gerekliyse
// uygulama çapında loglama işini düşünelim
// alt plugin ve kütüphanelerde translate olsun, her targetin translate dosyaları targetismi_tr.qs vs şeklinde olsun
// insanları svg ye zorlama, dosyaların svg olmaması durumunda da tüm scriptler vs düzgün çalışsın
// we should not hard link to openssl or any other plugin -except when build is static, instead we should copy them into the deploy dir
// bug: windowsda apptemplateqt.exe kapatılsa da arkada çalışmayı sürdürüyor
// cmake kodlarında ${TARGET} kullanılan yerlerde uygunsa onun yerine ${CMAKE_PROJECT_NAME} kullan
// QApplication::paletteChanged, QEvent::LocaleChange, QEvent::PaletteChange, QEvent::StyleChange, QEvent::ApplicationFontChange, QEvent::ApplicationPaletteChange, QEvent::FontChange, QEvent::OrientationChange

Application::Application(int& argc, char** argv, int) : QApplication(argc, argv)
  , m_signalHandler(new SignalHandler(this))
  , m_settings(new QSettings(settingsPath(), QSettings::IniFormat, this))
{
    // Handle system signals
    connect(m_signalHandler, &SignalHandler::interrupted,
            m_signalHandler, &SignalHandler::exitGracefully);

    /* Prepare setting instances */
    //    s_settings = new QSettings(settingsPath(), QSettings::IniFormat);
    //  FIXME  QSettings settings(settingsPath(), QSettings::IniFormat);
    //    if (settings.value(u"General/Interface.Theme"_s, InterfaceSettings().theme).toString() == u"Light"_s)

    // App settings
    Application::setDprAdjustmentHook();
    Application::setApplicationDisplayName(APP_NAME);
    Application::setWindowIcon(QIcon(u":/images/icon.svg"_s));
    Application::setStyle(QStyleFactory::create(u"ApplicationStyle"_s));
    Application::setPalette(style()->standardPalette());
    Application::setFonts(); // Set after style, see: github.com/qt/qtbase/commit/14071b5
    Application::setTranslators();

    //    QObject::connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged, [=]{
    //        QApplication::setPalette(palette());
    //        QPixmapCache::clear(); // Cached QIcon pixmaps makes use of old palette, so we have to update
    //    });

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &Application::updatePalette);
}

int Application::prepare()
{
    // qputenv("QT_ENABLE_HIGHDPI_SCALING", "0"_ba);
    // qputenv("QT_SCALE_FACTOR", "2.0"_ba);
    // qputenv("QSG_RHI_BACKEND", "opengl"_ba);

#if !defined(Q_OS_ANDROID)
    qputenv("QT_FORCE_STDERR_LOGGING", "1"_ba);
#endif

    // Set these here, QStandardPaths needs them
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION);
    QApplication::setOrganizationName(APP_URL);
    QApplication::setOrganizationDomain(APP_URL);

    return {};
}

QString Application::appDataPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString Application::settingsPath()
{
    return appDataPath() + "/Settings.ini"_L1;
}

void Application::updatePalette()
{
    QApplication::setPalette(style()->standardPalette());
    QPixmapCache::clear(); // Cached QIcon pixmaps make use of the old palette
}

void Application::setFonts()
{
    // Install the fonts shipped with the app
    const QStringList& fonts =  QDir(u":/fonts"_s).entryList(QDir::Files);
    for (const QString& fontName : fonts)
        QFontDatabase::addApplicationFont(u":/fonts/"_s + fontName);

    // Set default font
    QFont font(u"Archivo"_s);
    font.setPixelSize(14);
    QApplication::setFont(font);

    // Set tooltip font (one bit smaller)
    font.setPixelSize(font.pixelSize() - 1);
    QToolTip::setFont(font);
}

void Application::setTranslators()
{
    // Translate the Qt
    auto qtTranslator = new QTranslator(instance());
    if (qtTranslator->load(QLocale::system(), u"qt"_s, u"_"_s,
                           QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(qtTranslator);
    } else if (qtTranslator->load(QLocale::system(), u"qtbase"_s, u"_"_s,
                                  QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(qtTranslator);
    }

    // Translate the app
    auto appTranslator = new QTranslator(instance());
    if (appTranslator->load(QLocale::system(), u"app"_s, u"_"_s,
                            QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(appTranslator);
    } else if (appTranslator->load(QLocale::system(), u"app"_s, u"_"_s, u":/translations"_s)) {
        installTranslator(appTranslator);
    } else if (appTranslator->load(QLocale::system(), u"app"_s, u"_"_s,
                                   QCoreApplication::applicationDirPath())) {
        installTranslator(appTranslator);
    }
}

void Application::setDprAdjustmentHook()
{
    static constexpr auto roundedDpr = [] (qreal dpr) {
        if (dpr < 1.125)
            return 1.00;
        else if (dpr < 1.375)
            return 1.25;
        else if (dpr < 1.625)
            return 1.50;
        else if (dpr < 1.875)
            return 1.75;
        else
            return std::round(dpr * 2) / 2;
    };

    static auto adjustScreenDpr = [] (QScreen* screen) {
        const QPlatformScreen* platformScreen = screen->handle();
        qreal platformFactor = platformScreen->devicePixelRatio();
        qreal logicalFactor = qreal(platformScreen->logicalDpi().first)
                / qreal(platformScreen->logicalBaseDpi().first);
        qreal roundedFactor = roundedDpr(platformFactor * logicalFactor);
        if (!qFuzzyCompare(screen->devicePixelRatio(), roundedFactor)) {
            QHighDpiScaling::setScreenFactor(screen, roundedFactor / platformFactor);
            QHighDpiScaling::updateHighDpiScaling();
            emit screen->physicalDotsPerInchChanged(screen->physicalDotsPerInch());
        }
    };

    for (QScreen* screen : QGuiApplication::screens())
        adjustScreenDpr(screen);

    QObject::connect(qApp, &QGuiApplication::screenAdded, adjustScreenDpr);

    class WindowSystemEventHandler final : public QWindowSystemEventHandler
    {
    public:
        bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent* e) override
        {
            if (e->type == QWindowSystemInterfacePrivate::ScreenGeometry) {
                auto event = static_cast<QWindowSystemInterfacePrivate::ScreenGeometryEvent*>(e);
                adjustScreenDpr(event->screen);
            }
            return QWindowSystemEventHandler::sendEvent(e);
        }
    };

    QWindowSystemInterfacePrivate::installWindowSystemEventHandler(new WindowSystemEventHandler());
}

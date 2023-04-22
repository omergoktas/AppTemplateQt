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

// TODOs:
// Settings-driven (default system dependent - as 3rd option) Dark-light mode (appearance mode) palette + Runtime change support
// Settings-driven (default system language) language/translation + Runtime change support
// Platform/Settings-driven small font tweaks + what should be adjustable whatnot + Runtime change support
// Platform/Settings-driven small geometry (widget size & icon/pixmap/image size) tweaks + what should be adjustable whatnot + Runtime change support
// Cospetic-pen painting artifacts + what should be cosmetic whatnot + scaling's affects on it
// Regular-pen painting artifacts + what should be regular whatnot + scaling's affects on it
// Should higher dpi devices have lighter fonts?
// Tüm kodlar bi genel olarak baştan yeniden gözden geçirilecek
// macosda uygulamaismi.app yanı sıra deploy dmg oluştursun
// linuxde eksik olan macos da fazla olan varsayılan plugin ve libler karşılaştırılsın
// interin geri kalan tüm fontları s1 s4 c1 c6 c11 ile ZORT lansın ve resource olarak eklensin
// lisans MIT, APACHI ya da BSD yapılsın
// readme oluşturulsun
// (özellikle windows, linux, macos olmak üzere) tüm yüklenen gereksiz pluginler ve libler silinsin ya da build scripte bunu otomatikleştirme eklensin
// deployların otomatik olarak imzalanması eklenecek
// appimage için gereken appstream standardı vb. gereklilikler yerine getirilip appler appimagehub'a yüklemeye hazır hale getirilecek
// readme de qt creator içinden rahatlıkla "play" tuşuna basar basmaz ekstra birşey yapmadan debugging yapma olayı öğretilecek
// pluginlerin instancelarının opsiyonel olmasını sağla, yani önce nullptr kontrolu yap sonra instancı kullan, eğer plugin yoksa warning de göster
// her şeyin dosyaların vs kolayca replace edilebilirliğinden + relocate edilebilirliğinden emin ol (x86_64 arch olayıda relocatable olacak şekilde düzeltilsin)
// cmake static buildlerde *.a dosyaları deploy edilmediği için boş klasörleri deploy ediyor
// tüm copyrighted metaryali ücretsiz/lisansımıza uygun olanla değiştir ve kaynakları da ekle
// linuxde gtk karanlık mode detection eklensin
// her yeri, özellikle cmake dosyalarını, yorum satırları ve copyrightlar la süsle
// deploy klasörü ile resources klasörlerindeki tekrar eden dosyaları resource aleyhine olacak şekilde azaltalım, gerekirse ilgili dosyalar (.icns vs) install aşamasında kopyalansın
// Linux dmg si için de ayrı volume icon yapalım, app in ikonunu kullanmasın --mümkünse/anlamlıysa
// Bütün build scriptleri/static/shared plugin vs hepsi github actions ile çalışıp çalışmadığı denenecek
// Tüm scriptlerin adam akıllı çalışıp çalışmadığını denetlemek için testler yazılacak (github actions üzerinde misal)
// Genel manada test konusu düşünülecek ui test, cpp test, build test, install test, qt sürüm değişimi test, dll/plugin varmı yokmu yüklendimi test,
// mac deploy scripte yorum eklenebilir, ayrıca pencere boyutları imageconvert den getirilmeli, değişken olabilmeli
// uygulama isminde boşlık olması hiç bir platformda build install vs leri engellememeli, misal dmg oluşumunu bi test et
// ui her bir tuşun fonksiyonelliği test, örneğin submit tls plugini yüklümü ve nete bağlanabiliyor mu
// lgtm olayına bak https://code.qt.io/cgit/qt/qtbase.git/tree/.lgtm.yml?h=6.4.1
// github actions işini hallet
// tüm sistem property change observers/connections class destructorlarında disconnect edilmeli --gerekliyse
// uygulama çapında loglama işini düşünelim
// alt plugin ve kütüphanelerde translate olsun, her targetin translate dosyaları targetismi_tr.qs vs şeklinde olsun
// insanları svg ye zorlama, dosyaların svg olmaması durumunda da tüm scriptler vs düzgün çalışsın
// we should not hard link to openssl or any other plugin -except when build is static, instead we should copy them into the deploy dir
// include font license
// bug: windowsda apptemplateqt.exe kapatılsa da arkada çalışmayı sürdürüyor
// deploy paket isimlerine sürüm ve cpu arch ekle
// cmake kodlarında ${TARGET} kullanılan yerlerde uygunsa onun yerine ${CMAKE_PROJECT_NAME} kullan
// QApplication::paletteChanged, QEvent::LocaleChange, QEvent::PaletteChange, QEvent::StyleChange, QEvent::ApplicationFontChange, QEvent::ApplicationPaletteChange, QEvent::FontChange, QEvent::OrientationChange

static constexpr qreal roundedDpr(qreal dpr)
{
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
}

Application::Application(int& argc, char** argv, int) : QApplication(argc, argv)
  , m_signalHandler(new SignalHandler(this))
{
    // Handle system signals
    connect(m_signalHandler, &SignalHandler::interrupted,
            m_signalHandler, &SignalHandler::exitGracefully);
    // SignalHandler bi son kontrol, inherited:QObject olmasa mesela??

    /* Prepare setting instances */
    //    s_settings = new QSettings(settingsPath(), QSettings::IniFormat);
    //  FIXME  QSettings settings(settingsPath(), QSettings::IniFormat);
    //    if (settings.value(u"General/Interface.Theme"_s, InterfaceSettings().theme).toString() == u"Light"_s)

    /** Core initialization **/
    Application::setDprAdjustmentHook();
    QApplication::setApplicationDisplayName(APP_NAME);
    QApplication::setWindowIcon(QIcon(u":/images/icon.svg"_s));
    QApplication::setStyle(QStyleFactory::create(u"ApplicationStyle"_s));
    QApplication::setPalette(style()->standardPalette());
    Application::setFont(); // Set after style, see: github.com/qt/qtbase/commit/14071b5

    // qDebug() << "@@@@@@@@@@@@@" << QLibraryInfo::path(QLibraryInfo::TranslationsPath);

    //    QObject::connect(GeneralSettings::instance(), &GeneralSettings::interfaceSettingsChanged, [=]{
    //        QApplication::setPalette(palette());
    //        QPixmapCache::clear(); // Cached QIcon pixmaps makes use of old palette, so we have to update
    //    });

    /* Set application ui settings */
    auto qtTranslator = new QTranslator(this);
    auto appTranslator = new QTranslator(this);

    if (qtTranslator->load(QLocale::system(), u"qt"_s, u"_"_s,
                           QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(qtTranslator);
    } else if (qtTranslator->load(QLocale::system(), u"qtbase"_s, u"_"_s,
                                  QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(qtTranslator);
    }
    if (appTranslator->load(QLocale::system(), u"app"_s, u"_"_s,
                            QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        installTranslator(appTranslator);
    } else if (appTranslator->load(QLocale::system(), u"app"_s, u"_"_s, u":/translations"_s)) {
        installTranslator(appTranslator);
    }

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &Application::updatePalette);
}

int Application::prepare()
{
    // qputenv("QT_ENABLE_HIGHDPI_SCALING", "0"_ba);
    // qputenv("QT_SCALE_FACTOR", "2.0"_ba);
    // qputenv("QSG_RHI_BACKEND", "opengl"_ba);
#if QT_CONFIG(xcb)
    qputenv("QT_QPA_PLATFORM", "xcb"_ba);
#endif

#if !defined(Q_OS_ANDROID)
    qputenv("QT_FORCE_STDERR_LOGGING", "1"_ba);
#endif

    // Set these here, needed by QStandardPaths
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION);
    QApplication::setOrganizationName(APP_URL);
    QApplication::setOrganizationDomain(APP_URL);

    return {};
}

void Application::updatePalette()
{
    QApplication::setPalette(style()->standardPalette());
    QPixmapCache::clear(); // Cached QIcon pixmaps makes use of old palette
}

void Application::setFont()
{
    // Load fonts
    const QStringList& fontFiles =  Utils::files(u":/fonts"_s);
    for (const QString& fontName : fontFiles)
        QFontDatabase::addApplicationFont(u":/fonts/"_s + fontName);

    // Set default app font
    QFont font(u"Archivo"_s);
    font.setPixelSize(14);
    QApplication::setFont(font);

    // Set tooltip font (just a bit smaller than the default font)
    font.setPixelSize(font.pixelSize() - 1);
    QToolTip::setFont(font);
}

void Application::setDprAdjustmentHook()
{
    static auto adjustScreenDpr = [] (QScreen* screen) {
        const QPlatformScreen* platformScreen = screen->handle();
        qreal platformFactor = platformScreen->devicePixelRatio();
        qreal logicalFactor = qreal(platformScreen->logicalDpi().first) / qreal(platformScreen->logicalBaseDpi().first);
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
    class WindowSystemEventHandler : public QWindowSystemEventHandler
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

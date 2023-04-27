#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class SignalHandler;
class QSettings;

class Application final : public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY(Application)

public:
    Application(int& argc, char** argv, int = prepare());

    static int prepare();

    static QString appDataPath();
    static QString settingsPath();

private slots:
    void updatePalette();

private:
    static void setFonts();
    static void setTranslators();
    static void setDprAdjustmentHook();

private:
    SignalHandler* m_signalHandler;
    QSettings* m_settings;
};

#endif // APPLICATION_H

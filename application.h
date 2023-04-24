#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class SignalHandler;

class Application final : public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY(Application)

public:
    Application(int& argc, char** argv, int = prepare());

    static int prepare();

private slots:
    void updatePalette();

private:
    static void setFont();
    static void setDprAdjustmentHook();

private:
    SignalHandler* m_signalHandler;
};

#endif // APPLICATION_H

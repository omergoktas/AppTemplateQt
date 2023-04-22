#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui { class MainWindow; }

class MainWindow final : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    ~MainWindow() override;

    QSize sizeHint() const override;

private:
    void changeEvent(QEvent* event) override;

    void on_screen_changed(QScreen* screen);
    void on_submitButton_clicked();

private:
    Ui::MainWindow* ui;
    QMetaObject::Connection m_dprChangeConnection;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_UI_H
#define MAINWINDOW_UI_H

class QMainWindow;
class QTextEdit;
class QPushButton;

namespace Ui {

class MainWindow
{
public:
    MainWindow(QMainWindow*);

    void setup();
    void retranslate();
    void revisualize();

    QMainWindow* widget;
    QTextEdit* dreamEdit;
    QPushButton* submitButton;
};

} // namespace Ui

#endif // MAINWINDOW_UI_H

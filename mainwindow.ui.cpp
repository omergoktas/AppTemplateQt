#include "mainwindow.ui.h"
#include <utils.h>

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

Ui::MainWindow::MainWindow(QMainWindow* w) : widget(w)
{
}

void Ui::MainWindow::setup()
{
    auto centralWidget = new QWidget(widget);
    widget->resize(widget->sizeHint()); // Don't use adjustSize() on Windows
    widget->move(Utils::centralized(widget->size(), widget->screen()));
    widget->setCentralWidget(centralWidget);

    dreamEdit = new QTextEdit(centralWidget);
    submitButton = new QPushButton(centralWidget);
    submitButton->setCursor(Qt::PointingHandCursor);

    auto layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);
    layout->addStretch(2);
    layout->addWidget(dreamEdit);
    layout->addWidget(submitButton);
}

void Ui::MainWindow::retranslate()
{
    dreamEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter Turkish text you want translated to English"));
    submitButton->setText(QCoreApplication::translate("MainWindow", "Translate"));
}

void Ui::MainWindow::revisualize()
{
    submitButton->setIcon(Utils::si(":/images/bookmark", submitButton->devicePixelRatio()));
}

#include "mainwindow.h"
#include "mainwindow.ui.h"

#include <utils.h>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPlainTextEdit>
#include <QJsonObject>
#include <QPushButton>
#include <QWindow>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
  , ui(new Ui::MainWindow(this))
{
    ui->setup();
    ui->retranslate();
    connect(ui->submitButton, &QPushButton::clicked,
            this, &MainWindow::on_submitButton_clicked);
    winId(); // Make sure window resources are established before calling windowHandle()
    connect(windowHandle(), &QWindow::screenChanged,
            this, &MainWindow::on_screen_changed);
    on_screen_changed(screen());
}

MainWindow::~MainWindow()
{
    delete ui;
}

QSize MainWindow::sizeHint() const
{
    return QSize(324, 576);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslate();
    QMainWindow::changeEvent(event);
}

void MainWindow::on_screen_changed(QScreen* screen)
{
    auto revisualize = [this] {
        ui->revisualize();
        update();
    };
    disconnect(m_dprChangeConnection);
    m_dprChangeConnection = connect(screen, &QScreen::physicalDotsPerInchChanged,
        this, revisualize, Qt::QueuedConnection);
    revisualize();
}

void MainWindow::on_submitButton_clicked()
{
    setEnabled(false);
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_ba);
    req.setUrl(QUrl(u"https://api-inference.huggingface.co/models/Helsinki-NLP/opus-mt-tr-en"_s));
    QNetworkReply* reply = Utils::post(req, Utils::stringify({
        { u"inputs"_s, ui->dreamEdit->toPlainText() },
        { u"options"_s, QJsonObject {
              { u"wait_for_model"_s, true }
          }
        }
    }));

    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [=] {
        ui->dreamEdit->setPlainText(ui->dreamEdit->toPlainText() + u"\n\n"_s +
                                    Utils::translate(reply->readAll()));
        setEnabled(true);
    });
}

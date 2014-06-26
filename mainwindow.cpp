#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QCloseEvent>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cntlm = new QProcess(this);
    cntlm->setReadChannelMode(QProcess::MergedChannels);
    cntlm->closeWriteChannel();
    connect(cntlm, SIGNAL(readyReadStandardOutput()), this, SLOT(cntlmTalking()));

    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(closeApp()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/Free.png"));
    trayIcon->setToolTip("Stopped");
    trayIcon->show();

}

void MainWindow::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void MainWindow::on_pbStart_clicked()
{
    QProcess::ProcessState state = cntlm->state();
    if (state == QProcess::NotRunning) {
        QString cmd = QString("cntlm -u %1@uci.cu -p %2 -f -l -v localhost:%3 10.0.0.1:8080").arg(ui->leUser->text(),
                                                                                               ui->lePass->text(),
                                                                                               QString::number(ui->spinPort->value()));
//        qDebug() << cmd;
        cntlm->start(cmd);
        cntlm->waitForStarted(300);
        if (cntlm->state() == QProcess::Running) {
            ui->pbStart->setText("Stop");
            trayIcon->setToolTip("Running");
        }
    }
    else if (state == QProcess::Running) {
        cntlm->close();
        while (cntlm->state() != QProcess::NotRunning) {
            qDebug() << "trying to stop...";
            cntlm->waitForFinished(1000);
        }
        ui->pbStart->setText("Start");
        trayIcon->setToolTip("Stopped");
    }
}

void MainWindow::cntlmTalking()
{
    QString output = cntlm->readAllStandardOutput();
    qDebug() << Q_FUNC_INFO << output;
}

void MainWindow::closeApp()
{
    if (cntlm->state() == QProcess::Running) {
        cntlm->close();
        cntlm->waitForFinished(1000);
    }
    qApp->quit();
}

//void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
//{
////    switch (reason) {
////    case QSystemTrayIcon::Trigger:
////    case QSystemTrayIcon::DoubleClick:
////        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
////        break;
////    case QSystemTrayIcon::MiddleClick:
////        showMessage();
////        break;
////    default:
////        ;
////    }
//}

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // creat timer and update timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout,this, &MainWindow::updateTime);
    timer->start(1000);

    updateTime();

    ui->graphicsView_map->loadSvg(":/map/resources/hospital_map.svg");

    patientDb = new PatientDatabase(this);
    if(patientDb->connectToDatabase(""))
    {

    }



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    ui->label_currentTime->setText(timeStr);
}

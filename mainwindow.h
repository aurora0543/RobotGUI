#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

#include "patientdatabase.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void updateTime();

private:
    PatientDatabase *patientDb;

private slots:
    void translateTextBrowserContent();

private:
    QNetworkAccessManager *networkManager;
};
#endif // MAINWINDOW_H

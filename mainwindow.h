#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


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
};
#endif // MAINWINDOW_H

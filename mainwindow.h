#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QLabel>  
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <string>
#include "patientdatabase.h"
#include "face_recognizer.h"
#include "svgviewer.h"
#include <QVideoWidget> 
#include "maincontroller.h"
#include "manual.h"
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

private:
    void onEnterClicked();
    void onClearClicked();
    void on_pushButton_nav_clicked();
    void onDepartmentSelected(const QString &deptName);


    MainController controller;

private:
    bool isFullScreenNow = false;
    bool takePhoto(const std::string& savePath, int delay_ms = 500);
    QLabel* photoLabel = nullptr;
    QVideoWidget* videoWidget = nullptr;
    void showPhotoOnCameraWidget(const QString& photoPath);
    int face_recognition(const std::string& imagePath);
    void loadPatientInfoByID(int id);
    void onCaptureButtonClicked();
};
#endif // MAINWINDOW_H

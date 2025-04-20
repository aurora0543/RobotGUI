#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "svgviewer.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <QMessageBox>
#include "maincontroller.h"
#include "face_recognizer.h"
#include <QMediaDevices>

#include <QPixmap>
#include <QImage>

std::string run_spark_asr();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);

    connect(ui->pushButton_help, &QPushButton::clicked, this, &MainWindow::onCaptureButtonClicked);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::translateTextBrowserContent);
    connect(ui->pushButton_enter, &QPushButton::clicked, this, &MainWindow::onEnterClicked);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->pushButton_nav, &QPushButton::clicked, this, &MainWindow::on_pushButton_nav_clicked);
    connect(ui->actionExit, &QAction::triggered, this, [](){
        qApp->quit(); 
    });
    connect(ui->actionFull_Screen, &QAction::triggered, this, [this] {
        if (isFullScreenNow) {
            showNormal();
            isFullScreenNow = false;
        } else {
            showFullScreen();
            isFullScreenNow = true;
        }
    });

    connect(ui->actionAbout, &QAction::triggered, this, []() {
        QMessageBox::about(nullptr, "About", "Hospital Robot Guide\nVersion 2.1\nAuthor: Team 24");
    });

    connect(ui->graphicsView_map, &SvgViewer::departmentSelected, this, [=](const QString& deptName){
        ui->label_apart->setText(deptName); 
    });



    // create timer and update timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000);

    updateTime();

    ui->graphicsView_map->loadSvg("/home/team24/RoboHospitalGuide/RobotGUI/resources/hospital_map.svg");
    PatientDatabase Db;
    if(!Db.connectToDatabase(""))
    {
        qDebug() << "Database error: Unable to connect to the database!";
    }
    // 连接信号槽
    controller.init();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_nav_clicked()
{
    // 1. 切换页面
    ui->stackedWidget_mainDisplay->setCurrentWidget(ui->page_navigation);
    QString departmentName = ui->label_apart->text();
    controller.startNavigationTo(departmentName);
    qDebug() << "Navigating to department:" << departmentName;
}

void MainWindow::updateTime()
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    ui->label_currentTime->setText(timeStr);
}

void MainWindow::translateTextBrowserContent()
{
    QString originalText = ui->plainTextEdit_translate->toPlainText();
    if (originalText.trimmed().isEmpty()) {
        qDebug() << "No text to translate!";
        return;
    }

    // 获取语言设置
    QString sourceLang, targetLang;

    QString from = ui->comboBox_from->currentText().toLower();
    QString to = ui->comboBox_to->currentText().toLower();

    if (from == "english") sourceLang = "en";
    else if (from == "chinese") sourceLang = "zh";
    else sourceLang = "auto";

    if (to == "english") targetLang = "en";
    else if (to == "chinese") targetLang = "zh";
    else targetLang = "zh";

    QUrl url("https://translate.googleapis.com/translate_a/single");
    QUrlQuery query;
    query.addQueryItem("client", "gtx");
    query.addQueryItem("sl", sourceLang);
    query.addQueryItem("tl", targetLang);
    query.addQueryItem("dt", "t");
    query.addQueryItem("q", originalText);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        QJsonDocument json = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "JSON parsing failed:" << parseError.errorString();
            return;
        }

        if (!json.isArray()) {
            qDebug() << "Response is not in array format!";
            return;
        }

        QJsonArray rootArray = json.array();
        if (rootArray.isEmpty() || !rootArray[0].isArray()) {
            qDebug() << "Translation content is empty!";
            return;
        }

        QJsonArray textChunks = rootArray[0].toArray();
        QString translatedText;
        for (const QJsonValue &chunk : textChunks) {
            if (chunk.isArray() && chunk.toArray().size() > 0)
                translatedText += chunk.toArray().at(0).toString();
        }

        ui->plainTextEdit_translate->setPlainText(translatedText);
    });
}

void MainWindow::onEnterClicked()
{
    QString name = ui->lineEdit_name->text();
    QDate birthDate = ui->dateEdit->date();
    QString gender = ui->comboBox_gender->currentText();

    int id = patientDb->getPatientID(name, birthDate, gender);
    ui->label_id->setText(QString::number(id));

    if (id <= 0) {
        qDebug() << "Patient not found:" << name << birthDate << gender;
        ui->label_apart->setText("None");
        ui->label_date->setText("None");
        ui->label_reg->setText("None");
        return;
    }

    QList<QVariantMap> registrations = patientDb->getRegistrationsForPatient(id);
    if (registrations.isEmpty()) {
        ui->label_apart->setText("No appointments");
        ui->label_date->setText("-");
        ui->label_reg->setText("No notes");
        return;
    }

    // 这里仅取第一条挂号信息
    QVariantMap record = registrations.first();
    int deptId = record["DepartmentID"].toInt();
    QString appointmentTime = record["AppointmentTime"].toDateTime().toString("yyyy-MM-dd hh:mm");
    QString notes = record["AdditionalNotes"].toString();

    // 查找科室名称
    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    QString departmentName = "Unknown Department";

    if (db.isOpen()) {
        QSqlQuery deptQuery(db);
        deptQuery.prepare("SELECT Name FROM Departments WHERE DepartmentID = :id");
        deptQuery.bindValue(":id", deptId);

        if (deptQuery.exec() && deptQuery.next()) {
            departmentName = deptQuery.value("Name").toString();
        } else {
            qDebug() << "Query department failed:" << deptQuery.lastError().text();
        }
    } else {
        qDebug() << "Database not open, unable to find department name";
    }

    ui->label_apart->setText(departmentName);
    ui->label_date->setText(appointmentTime);
    ui->label_reg->setText(notes);
}

void MainWindow::onClearClicked()
{
    ui->lineEdit_name->clear();
    ui->dateEdit->clear();
    ui->label_id->clear();
    ui->label_reg->clear();
}

bool MainWindow::takePhoto(const std::string& savePath, int delay_ms)
{
    std::string command = "libcamera-still -t " + std::to_string(delay_ms) +
                          " --width 640 --height 480 -o " + savePath + " --nopreview";
    
    int ret = std::system(command.c_str());
    if (ret != 0) {
        std::cerr << "❌ Photo capture failed, command execution error, exit code: " << ret << std::endl;
        return false;
    }

    std::cout << "📸 Photo saved to: " << savePath << std::endl;
    return true;
}

void MainWindow::showPhotoOnCameraWidget(const QString& photoPath) {
    if (!QFile::exists(photoPath)) {
        qWarning() << "Photo does not exist:" << photoPath;
        return;
    }

    if (!photoLabel) {
        photoLabel = new QLabel(ui->cameraWidget);
        photoLabel->setGeometry(ui->cameraWidget->rect());
        photoLabel->setScaledContents(true); // 自动缩放
        photoLabel->setStyleSheet("background-color: black;");
    }

    QPixmap pix(photoPath);
    if (pix.isNull()) {
        qWarning() << "Photo does not exist:" << photoPath;
        return;
    }

    photoLabel->setPixmap(pix);
    photoLabel->show();
}

int MainWindow::face_recognition(const std::string& imagePath)
{
    FaceRecognizerLib recognizer;
    recognizer.init("/home/team24/RoboHospitalGuide/source/face");

    auto [idStr, confidence] = recognizer.recognize(imagePath);

    qDebug() << "Recognized ID:" << QString::fromStdString(idStr) << "Confidence:" << confidence;

    // 提取文件名前缀当作整数ID
    QString qid = QString::fromStdString(idStr).split(".").first();
    bool ok = false;
    int id = qid.toInt(&ok);

    return ok ? id : -1;
}

void MainWindow::loadPatientInfoByID(int id)
{
    if (id <= 0) {
        qDebug() << "Invalid ID:" << id;
        ui->label_apart->setText("None");
        ui->label_date->setText("None");
        ui->label_reg->setText("None");
        ui->lineEdit_name->setText("None");
        ui->dateEdit->setDate(QDate::currentDate());
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    if (!db.isOpen()) {
        qDebug() << "Database not open, unable to load patient information";
        return;
    }

    QSqlQuery infoQuery(db);
    infoQuery.prepare("SELECT Name, BirthDate FROM Patients WHERE PatientID = :id");
    infoQuery.bindValue(":id", id);

    if (infoQuery.exec() && infoQuery.next()) {
        QString name = infoQuery.value("Name").toString();
        QDate birthDate = infoQuery.value("BirthDate").toDate();
        ui->lineEdit_name->setText(name);
        ui->dateEdit->setDate(birthDate);
    } else {
        qDebug() << "Failed to query patient basic information:" << infoQuery.lastError().text();
        ui->lineEdit_name->setText("Unknown");
        ui->dateEdit->setDate(QDate::currentDate());
    }

    QList<QVariantMap> registrations = patientDb->getRegistrationsForPatient(id);
    if (registrations.isEmpty()) {
        ui->label_apart->setText("No appointments");
        ui->label_date->setText("-");
        ui->label_reg->setText("No notes");
        return;
    }

    QVariantMap record = registrations.first();
    int deptId = record["DepartmentID"].toInt();
    QString appointmentTime = record["AppointmentTime"].toDateTime().toString("yyyy-MM-dd hh:mm");
    QString notes = record["AdditionalNotes"].toString();

    QString departmentName = "Unknown Department";

    QSqlQuery deptQuery(db);
    deptQuery.prepare("SELECT Name FROM Departments WHERE DepartmentID = :id");
    deptQuery.bindValue(":id", deptId);

    if (deptQuery.exec() && deptQuery.next()) {
        departmentName = deptQuery.value("Name").toString();
    } else {
        qDebug() << "Failed to query department:" << deptQuery.lastError().text();
    }

    ui->label_apart->setText(departmentName);
    ui->label_date->setText(appointmentTime);
    ui->label_reg->setText(notes);
}

void MainWindow::onCaptureButtonClicked()
{
    takePhoto("/home/team24/RoboHospitalGuide/source/tmp/tmp.jpg");
    showPhotoOnCameraWidget("/home/team24/RoboHospitalGuide/source/tmp/tmp.jpg");
    int id = face_recognition("/home/team24/RoboHospitalGuide/source/tmp/tmp.jpg");
    loadPatientInfoByID(id);
}
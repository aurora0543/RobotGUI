#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "svgviewer.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QDate>

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

std::string run_spark_asr();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::translateTextBrowserContent);
    connect(ui->pushButton_enter, &QPushButton::clicked, this, &MainWindow::onEnterClicked);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->pushButton_nav, &QPushButton::clicked, this, &MainWindow::on_pushButton_nav_clicked);
    connect(ui->actionExit, &QAction::triggered, this, [](){
        qApp->quit(); // 替代 close()
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
        QMessageBox::about(nullptr, "关于", "医院导诊系统\n版本 1.0\n作者: Your Name");
    });

    connect(ui->graphicsView_map, &SvgViewer::departmentSelected, this, [=](const QString& deptName){
        ui->label_apart->setText(deptName);  // 假设你界面上有 QLabel 叫 label_department
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
        qDebug()<<"数据库错误", "无法连接到数据库！";
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

    // 2. 获取要导航的科室名称（假设你有一个 label_aprt 里保存了目标科室名）
    QString departmentName = ui->label_apart->text();
    controller.startNavigationTo(departmentName);
    // 3. 在终端输出或后续使用
    qDebug() << "🧭 正在导航到科室：" << departmentName;

    // 如果你希望返回这个值，也可以通过信号发出去或保存到成员变量
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
        qDebug() << "⚠️ 无原始文本，不执行翻译。";
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
            qDebug() << "❌ 网络错误:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        QJsonDocument json = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "⚠️ JSON 解析失败:" << parseError.errorString();
            return;
        }

        if (!json.isArray()) {
            qDebug() << "⚠️ 响应不是数组格式！";
            return;
        }

        QJsonArray rootArray = json.array();
        if (rootArray.isEmpty() || !rootArray[0].isArray()) {
            qDebug() << "⚠️ 无翻译内容！";
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
        qDebug() << "病人未找到：" << name << birthDate << gender;
        ui->label_apart->setText("无");
        ui->label_date->setText("无");
        ui->label_reg->setText("无");
        return;
    }

    QList<QVariantMap> registrations = patientDb->getRegistrationsForPatient(id);
    if (registrations.isEmpty()) {
        ui->label_apart->setText("无预约");
        ui->label_date->setText("-");
        ui->label_reg->setText("无备注");
        return;
    }

    // 这里仅取第一条挂号信息
    QVariantMap record = registrations.first();
    int deptId = record["DepartmentID"].toInt();
    QString appointmentTime = record["AppointmentTime"].toDateTime().toString("yyyy-MM-dd hh:mm");
    QString notes = record["AdditionalNotes"].toString();

    // 查找科室名称
    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    QString departmentName = "未知科室";

    if (db.isOpen()) {
        QSqlQuery deptQuery(db);
        deptQuery.prepare("SELECT Name FROM Departments WHERE DepartmentID = :id");
        deptQuery.bindValue(":id", deptId);

        if (deptQuery.exec() && deptQuery.next()) {
            departmentName = deptQuery.value("Name").toString();
        } else {
            qDebug() << "查询科室失败：" << deptQuery.lastError().text();
        }
    } else {
        qDebug() << "数据库未打开，无法查找科室名";
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

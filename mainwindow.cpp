#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

std::string run_spark_asr();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QTimer::singleShot(500, this, &MainWindow::translateTextBrowserContent);
    networkManager = new QNetworkAccessManager(this);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::translateTextBrowserContent);
    connect(ui->pushButton_enter, &QPushButton::clicked, this, &MainWindow::onEnterClicked);

    // creat timer and update timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout,this, &MainWindow::updateTime);
    timer->start(1000);

    updateTime();

    ui->graphicsView_map->loadSvg(":/map/resources/hospital_map.svg");
    PatientDatabase Db;
    if(!Db.connectToDatabase(""))
    {
        qDebug()<<"数据库错误", "无法连接到数据库！";
    }
    //Db.printAllPatients();




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

void MainWindow::translateTextBrowserContent()
{
    QString originalText = ui->textBrowser->toPlainText();
    if (originalText.trimmed().isEmpty()) {
        ui->textBrowser->append("\n⚠️ 无原始文本，不执行翻译。");
        return;
    }

    QString sourceLang = "auto";  // 自动检测源语言
    QString targetLang = "zh";

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
            ui->textBrowser->append("❌ 网络错误: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        QJsonDocument json = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            ui->textBrowser->append("⚠️ JSON 解析失败: " + parseError.errorString());
            return;
        }

        if (!json.isArray()) {
            ui->textBrowser->append("⚠️ 响应不是数组格式！");
            return;
        }

        QJsonArray rootArray = json.array();
        if (rootArray.isEmpty() || !rootArray[0].isArray()) {
            ui->textBrowser->append("⚠️ 无翻译内容！");
            return;
        }

        QJsonArray textChunks = rootArray[0].toArray();
        QString translatedText;
        for (const QJsonValue &chunk : textChunks) {
            if (chunk.isArray() && chunk.toArray().size() > 0)
                translatedText += chunk.toArray().at(0).toString();
        }

        if (!translatedText.isEmpty()) {
            ui->textBrowser->append("\n🌍 翻译结果：\n" + translatedText);
        } else {
            ui->textBrowser->append("⚠️ 翻译内容为空。");
        }
    });
}

void MainWindow::onEnterClicked()
{
    QString name = ui->lineEdit_name->text();
    QDate birthDate = ui->dateEdit->date();
    QString gender = ui->comboBox_gender->currentText();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入姓名！");
        return;
    }

    int id = patientDb->getPatientID(name, birthDate, gender);

    if (id > 0) {
        QMessageBox::information(this, "查询成功", QString("找到病人！Patient ID: %1").arg(id));
    } else {
        QMessageBox::warning(this, "未找到", "未在数据库中找到该病人。");
    }
}

#include "patientdatabase.h"
#include <QDate>

PatientDatabase::PatientDatabase(QObject *parent)
    : QObject(parent)
{
}

PatientDatabase::~PatientDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool PatientDatabase::connectToDatabase(const QString &dbPath)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("HospitalGuide");
    db.setUserName("root");
    db.setPassword("team24");


    if (!db.open()) {
        qDebug() << "数据库连接失败:" << db.lastError().text();
        return false;
    }

    qDebug() << "成功连接到数据库";
    return true;
}

bool PatientDatabase::addPatient(int id, const QString &name, const QDate &birthDate,
                                 const QString &gender, const QString &visitType,
                                 const QByteArray &photo)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Patients (PatientID, Name, BirthDate, Gender, VisitType, Photo) "
                  "VALUES (:id, :name, :birthDate, :gender, :visitType, :photo)");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":birthDate", birthDate);
    query.bindValue(":gender", gender);
    query.bindValue(":visitType", visitType);
    query.bindValue(":photo", photo);

    if (!query.exec()) {
        qDebug() << "添加病人失败:" << query.lastError().text();
        return false;
    }

    return true;
}

QSqlQuery PatientDatabase::getAllPatients()
{
    QSqlQuery query("SELECT * FROM Patients");
    return query;
}

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

bool PatientDatabase::connectToDatabase(const QString &)
{
    if (QSqlDatabase::contains("hospital_connection")) {
        db = QSqlDatabase::database("hospital_connection");
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL", "hospital_connection");
        db.setHostName("0.0.0.0");
        db.setPort(3306);
        db.setDatabaseName("HospitalGuide");
        db.setUserName("remote_user");
        db.setPassword("");
    }

    if (!db.open()) {
        qDebug() << "Failed to connect to database:" << db.lastError().text();
        return false;
    }

    qDebug() << "Successfully connected to database";
    return true;
}

bool PatientDatabase::addPatient(int id, const QString &name, const QDate &birthDate,
                                 const QString &gender, const QString &visitType,
                                 const QString &photoPath)
{
    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    if (!db.isOpen()) {
        qDebug() << "Failed to add patient: Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Patients (PatientID, Name, BirthDate, Gender, VisitType, PhotoPath) "
                  "VALUES (:id, :name, :birthDate, :gender, :visitType, :photoPath)");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":birthDate", birthDate);
    query.bindValue(":gender", gender);
    query.bindValue(":visitType", visitType);
    query.bindValue(":photoPath", photoPath);

    if (!query.exec()) {
        qDebug() << "Failed to add patient:" << query.lastError().text();
        return false;
    }

    qDebug() << "Successfully added patient:" << name;
    return true;
}

int PatientDatabase::getPatientID(const QString &name, const QDate &birthDate, const QString &gender)
{
    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return -1;
    }


    QSqlQuery query(db);  

    query.prepare("SELECT PatientID FROM Patients WHERE Name = :name");
    query.bindValue(":name", name);
    query.bindValue(":birthDate", birthDate);
    query.bindValue(":gender", gender);

    if (!query.exec()) {
        qDebug() << "Failed to query patient information:" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    } else {
        return 0;
    }
}

QSqlQuery PatientDatabase::getAllPatients()
{
    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Patients");

    if (!query.exec()) {
        qDebug() << "Failed to query patient information:" << query.lastError().text();
    }

    return query;
}


QList<QVariantMap> PatientDatabase::getRegistrationsForPatient(int patientId)
{
    QList<QVariantMap> results;

    QSqlDatabase db = QSqlDatabase::database("hospital_connection");
    if (!db.isOpen()) {
        qDebug() << "Database is not open, unable to query registration information";
        return results;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM Registrations WHERE PatientID = :patientId");
    query.bindValue(":patientId", patientId);

    if (!query.exec()) {
        qDebug() << "Failed to query appointment information:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QVariantMap record;
        record["RegistrationID"] = query.value("RegistrationID");
        record["PatientID"] = query.value("PatientID");
        record["DepartmentID"] = query.value("DepartmentID");
        record["AppointmentTime"] = query.value("AppointmentTime");
        record["AdditionalNotes"] = query.value("AdditionalNotes");
        results.append(record);
    }

    return results;
}

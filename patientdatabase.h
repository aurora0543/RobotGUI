#ifndef PATIENTDATABASE_H
#define PATIENTDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDebug>

class PatientDatabase : public QObject
{
    Q_OBJECT

public:
    explicit PatientDatabase(QObject *parent = nullptr);
    ~PatientDatabase();

    bool connectToDatabase(const QString &dbPath);
    bool addPatient(int id, const QString &name, const QDate &birthDate,
                    const QString &gender, const QString &visitType,
                    const QByteArray &photo);

    QSqlQuery getAllPatients();

private:
    QSqlDatabase db;
};
#endif // PATIENTDATABASE_H

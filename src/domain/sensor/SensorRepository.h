#ifndef SENSORREPOSITORY_H
#define SENSORREPOSITORY_H

#include <QSqlDatabase>
#include <QVector>
#include "LecturaSensor.h"
#include "../../config/FirebaseConfig.h"

class SensorRepository {
public:
    explicit SensorRepository(QSqlDatabase& db);
    
    void registrarLectura(const LecturaSensor& lectura);
    
    int obtenerIdSensor(int idQuirofano, const QString& tipo);

private:
    QSqlDatabase& database;
    FirebaseConfig& firebase;
};

#endif // SENSORREPOSITORY_H

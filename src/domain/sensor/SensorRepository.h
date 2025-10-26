#ifndef SENSORREPOSITORY_H
#define SENSORREPOSITORY_H

#include "Sensor.h"
#include <QSqlDatabase>
#include <QVector>

class SensorRepository {
public:
    explicit SensorRepository(QSqlDatabase& db);

    QVector<Sensor*> listarPorQuirofano(int idQuirofano);

private:
    QSqlDatabase& database;
};

#endif

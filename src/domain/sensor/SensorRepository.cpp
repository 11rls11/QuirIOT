#include "SensorRepository.h"
#include <QSqlQuery>
#include <QDebug>

SensorRepository::SensorRepository(QSqlDatabase& db) : database(db) {}

QVector<Sensor*> SensorRepository::listarPorQuirofano(int idQuirofano) {
    qInfo() << "[INFO] Listando sensores del quirofano" << idQuirofano;
    return QVector<Sensor*>();
}

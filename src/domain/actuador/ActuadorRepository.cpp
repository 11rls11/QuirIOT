#include "ActuadorRepository.h"
#include <QSqlQuery>
#include <QDebug>

ActuadorRepository::ActuadorRepository(QSqlDatabase& db) : database(db) {}

QVector<Actuador*> ActuadorRepository::listarPorQuirofano(int idQuirofano) {
    qInfo() << "[INFO] Listando actuadores del quirofano" << idQuirofano;
    return QVector<Actuador*>();
}
